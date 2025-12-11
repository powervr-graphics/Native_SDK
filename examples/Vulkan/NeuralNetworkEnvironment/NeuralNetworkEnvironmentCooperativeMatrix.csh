#version 450 core

#pragma use_vulkan_memory_model
#extension GL_KHR_memory_scope_semantics : enable
#extension GL_KHR_cooperative_matrix : require
#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_KHR_shader_subgroup_basic : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : enable
#extension GL_EXT_control_flow_attributes : enable

precision highp float;

// The shader code is loaded and in "local_size_x = %d", "%d" is changed to match the size of the subgroup in the current GPU
// Workgroup size is declared here and not before main() as it is used in functions defined before main() and in that case
// the value before it is declared is (1, 1, 1) ( see https://github.com/KhronosGroup/glslang/issues/2479 )
// layout(local_size_x = [Subgroup_size], local_size_y = 1, local_size_z = 1) in;
layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

// Each element in this buffer takes 5 indices from the buffer:
//      index 0: Patch index (which depends on the number of rows and columns the texture approximated is partitioned in)
//      index 1: Patch bounding box m.x Smallest x coordinates of the screen space box to process where pixels of the patch are present
//      index 2: Patch bounding box m.y Smallest y coordinates of the screen space box to process where pixels of the patch are present
//      index 3: Patch bounding box M.x Largest x coordinates of the screen space box to process where pixels of the patch are present
//      index 4: Patch bounding box M.y Largest y coordinates of the screen space box to process where pixels of the patch are present
layout(std430, set = 0, binding = 0) buffer organizePatchWorkloadBuffer
{
    float organizePatchWorkload[];
};

// For each patch, this buffer has initially nnBiasesNumberElement elements with the nnBiases buffer information,
// and then nnWeightsNumberElement elements with the nnWeight buffer information
layout(std430, set = 0, binding = 1) buffer neuralNetworkEnvironmentBuffer
{
    float16_t neuralNetworkEnvironment[];
};

layout(set = 0, binding = 2) uniform nnUniformBuffer
{
    highp mat4 inverseViewMatrix;
    highp mat4 inverseProjectionMatrix;
    highp mat4 viewMatrix;
    highp mat4 projectionMatrix;
    highp vec4 eyePos;
    highp float exposure;
    highp int patchSide;
    highp int textureWidth;
    highp int textureHeight;
    highp int screenSpaceBoxNumberPixelPerThread;
    highp int textureNumberPatchXDimension;
    highp int textureNumberPatchYDimension;
    highp int screenWidthScreenSpaceBox;
    highp int screenHeightScreenSpaceBox;
    highp int screenWidth;
    highp int screenHeight;
    highp int organizePatchWorkloadNumberPixelScreenSpaceRegion;
    highp int neuronsPerLayer0;
    highp int neuronsPerLayer1;
    highp int neuronsPerLayer2;
    highp int neuronsPerLayer3;
    highp int neuronsPerLayer4;
    highp int connectionOffset0;
    highp int connectionOffset1;
    highp int connectionOffset2;
    highp int connectionOffset3;
    highp int connectionOffset4;
    highp int neuronOffset0;
    highp int neuronOffset1;
    highp int neuronOffset2;
    highp int neuronOffset3;
    highp int neuronOffset4;
    highp int layerCount;
    highp int nnWeightsNumberElement;
    highp int nnBiasesNumberElement;
};

uniform layout(rgba8, set = 0, binding = 3) writeonly highp image2D imageOut;

#define NEW_DEBUG_TYPE float

layout(std430, set = 0, binding = 4) buffer nnEnvironmentDebugBuffer
{
    NEW_DEBUG_TYPE nnEnvironmentDebug[];
};

layout(set = 0, binding = 5) uniform highp sampler2D imageIn;

#define MAX_NEURONS_PER_LAYER 64
#define MAX_LAYERS 5
#define NUM_FREQUENCIES 8
#define PI 3.141593

// Build a shared variable where to sotre for this workgroup the information from the neural network patch that will be used
// by all threads in the subgroup to approximate pixels from the environment
shared float sharedArrayNNBiases[45];

// Build a shared variavble where to sotre for this workgroup the information from the neural network patch that will be used
// by all threads in the subgroup to approximate pixels from the environment
shared float16_t sharedArrayNNWeights[128];

shared float16_t sharedArrayNNWeightsFinalTemp[128];

shared float sharedMatrixResults32BitFloat[128];

// Where to store the values of the neurons in the second layer before writing them in the sharedMatrixActivations for the last layer evaluation
shared float secondLayerNeuronValues[20];

// Store in a shared array the coordinates of those screen pixels which require inference
const int numberElementSubgroup = 32;
const int numberPixelPerThread = 10;
shared vec4 sharedArrayUVInference[numberElementSubgroup * numberPixelPerThread];
shared float16_t sharedMatrixActivations[8 * 8];
shared float sharedDebug[16 * 8];

#define LEAKY_RELU_SLOPE 0.01f

float leakyRelu(float x)
{
    return (x >= 0.0f) ? x : (x * LEAKY_RELU_SLOPE);
}

highp vec2 convertDirectionToUVEquirectangular(highp vec3 direction)
{
    highp float theta = (atan(direction.z, direction.x) / (2.0 * PI)) + 0.5;
    highp float thetaOffsetted = theta + 0.75; // Offset to match IBL SDK sample environment
    if(thetaOffsetted > 1.0)
    {
        thetaOffsetted -= 1.0;
    }

    highp float phi = acos(-direction.y) / PI;
    highp float phiOffsetted = phi + 0.0005; // Offset to match IBL SDK sample environment
    if (phiOffsetted > 1.0)
    {
        phiOffsetted -= 1.0;
    }

    return vec2(1.0 - thetaOffsetted, 1.0 - phiOffsetted);
}

// Load the different weights from the neuralNetworkEnvironment buffer for the different neurons to evaluate
// As the target neural network is a MLP with 32x10x3 layers
//  + Two matrices will be used to load the weights of four neurons from each neuron in layer #1 (12 neurons) for inference (weights for neurons 0-3, weights for neurons 4-7):
//  + A third matrix will be used to load the weights from each neuron in layer #1 for inference (weights for neurons 8-9):
coopmat<float16_t, gl_ScopeSubgroup, 16, 8, gl_MatrixUseA> matrixWeightN0N3;
coopmat<float16_t, gl_ScopeSubgroup, 16, 8, gl_MatrixUseA> matrixWeightN4N7;
coopmat<float16_t, gl_ScopeSubgroup, 16, 8, gl_MatrixUseA> matrixWeightN8N9;
coopmat<float16_t, gl_ScopeSubgroup, 16, 8, gl_MatrixUseA> matrixWeightN0N2LastLayer;

// The matrix with the activations is a 8x8 matrix.
// It can be used to process two inference values at the same time (one value in the first four columns, and another in the second set of four columns)
// The initial values from frequency encoding will be computed before each inference step
coopmat<float16_t, gl_ScopeSubgroup, 8, 8, gl_MatrixUseB> matrixActivations;

// The results of the neural network weight and the multiplications by the activations will be stored in this matrix.
// These results need to be collected, added up (together with the bias) and set for evaluating the layer #2 (last three neurons)
coopmat<float, gl_ScopeSubgroup, 16, 8, gl_MatrixUseAccumulator> matrixResults;

// Load the neural network information corresponding to the patch this workgroup will use to approximate screen pixels
void loadNNInformation(int patchIndex)
{
    float workGroupSizeXFloat = float(gl_WorkGroupSize.x);
    int workGroupSizeXInt = int(gl_WorkGroupSize.x);
    int localInvocationIDxInt = int(gl_LocalInvocationID.x);
    int workgroupIDxInt = int(gl_WorkGroupID.x);

    // Load the nnBiases buffer information from neuralNetworkEnvironment
    int numElementPerThread = int(ceil(float(nnBiasesNumberElement) / workGroupSizeXFloat));
    int bufferIndexStart = (nnBiasesNumberElement + nnWeightsNumberElement) * patchIndex;
    int sharedIndexStart = localInvocationIDxInt * numElementPerThread;
    int sharedIndexEnd = sharedIndexStart + numElementPerThread;
    sharedIndexEnd = min(sharedIndexEnd, nnBiasesNumberElement);

    for(int i = sharedIndexStart; i < sharedIndexEnd; ++i)
    {
        sharedArrayNNBiases[i] = float(neuralNetworkEnvironment[bufferIndexStart + i]);
    }

    // Load the nnWeights buffer information from neuralNetworkEnvironment
    // The neural network has structure 32x10x3 layers. Each neuron in the layer #1 has 32 weights to the neurons in layer #0 which need to be evaluated
    // As the matrices used are 16x8 = 32x4, the weights of all neurons from layer #1 are stored in three cooperative matrices 
    // of size 32x4 (the last matrix is half empty). A shared variable is used to load the corresponding weights into each matrix

    // Load weights for neurons 0-3 from layer #1
    int numElementToCopy = 32 * 4;
    numElementPerThread = int(floor(float(numElementToCopy) / workGroupSizeXFloat));
    bufferIndexStart += nnBiasesNumberElement; // The nnBiases information is stored in neuralNetworkEnvironmentBuffer after the nnWeights information
    sharedIndexStart = localInvocationIDxInt * numElementPerThread;
    sharedIndexEnd = sharedIndexStart + numElementPerThread;
    sharedIndexEnd = min(sharedIndexEnd, numElementToCopy);

    for(int i = sharedIndexStart; i < sharedIndexEnd; ++i)
    {
        sharedArrayNNWeights[i] = float16_t(neuralNetworkEnvironment[bufferIndexStart + i]);
    }

    barrier();

    coopMatLoad(matrixWeightN0N3, sharedArrayNNWeights, 0, 8, gl_CooperativeMatrixLayoutRowMajor);
    barrier();

    // Load weights for neurons 4-7 from layer #1
    bufferIndexStart += 32 * 4;
    for(int i = sharedIndexStart; i < sharedIndexEnd; ++i)
    {
        sharedArrayNNWeights[i] = float16_t(neuralNetworkEnvironment[bufferIndexStart + i]);
    }

    barrier();
    coopMatLoad(matrixWeightN4N7, sharedArrayNNWeights, 0, 8, gl_CooperativeMatrixLayoutRowMajor);

    // Load weights for neurons 8-9 from layer #1. Only 64 indices are required here
    bufferIndexStart += 32 * 4;
    numElementToCopy = 32 * 2;
    numElementPerThread = int(floor(float(numElementToCopy) / workGroupSizeXFloat));
    sharedIndexStart = localInvocationIDxInt * numElementPerThread;
    sharedIndexEnd = sharedIndexStart + numElementPerThread;
    sharedIndexEnd = min(sharedIndexEnd, numElementToCopy);
    for(int i = sharedIndexStart; i < sharedIndexEnd; ++i)
    {
        sharedArrayNNWeights[i] = float16_t(neuralNetworkEnvironment[bufferIndexStart + i]);
    }
    for(int i = sharedIndexStart + numElementToCopy; i < sharedIndexEnd + numElementToCopy; ++i)
    {
        sharedArrayNNWeights[i] = float16_t(0.0);
    }

    barrier();
    coopMatLoad(matrixWeightN8N9, sharedArrayNNWeights, 0, 8, gl_CooperativeMatrixLayoutRowMajor);
    barrier();

    // Reset sharedArrayNNWeightsFinalTemp load the last set of weights, which will be used to evaluate the third layer with three neurons (the last layer, 32x10x3)
    numElementToCopy = 128;
    numElementPerThread = int(floor(float(numElementToCopy) / workGroupSizeXFloat));
    sharedIndexStart = localInvocationIDxInt * numElementPerThread;
    sharedIndexEnd = sharedIndexStart + numElementPerThread;
    sharedIndexEnd = min(sharedIndexEnd, numElementToCopy);
    for(int i = sharedIndexStart; i < sharedIndexEnd; ++i)
    {
        sharedArrayNNWeightsFinalTemp[i] = float16_t(0.0);
    }

    barrier();

    bufferIndexStart += 32 * 2;
    numElementToCopy = 30;
    numElementPerThread = int(floor(float(numElementToCopy) / workGroupSizeXFloat));
    numElementPerThread = max(numElementPerThread, 1);
    sharedIndexStart = localInvocationIDxInt * numElementPerThread;
    sharedIndexEnd = sharedIndexStart + numElementPerThread;
    sharedIndexEnd = min(sharedIndexEnd, numElementToCopy);
    // Offset the copy as the weights need to be rearranged for the MMA operations in the first half of the array
    for(int i = sharedIndexStart; i < sharedIndexEnd; ++i)
    {
        sharedArrayNNWeightsFinalTemp[i + 64] = float16_t(neuralNetworkEnvironment[bufferIndexStart + i]);
    }

    barrier();

    // To facilitate the evaluation of the neural networks later, the last ten weights just loaded onto sharedArrayNNWeightsFinalTemp need to be replicated
    if(localInvocationIDxInt == 0)
    {
        for(int i = 0; i < 10; ++i)
        {
            sharedArrayNNWeightsFinalTemp[i] = sharedArrayNNWeightsFinalTemp[i + 64];
            sharedArrayNNWeightsFinalTemp[i + 16] = sharedArrayNNWeightsFinalTemp[i + 64 + 10];
            sharedArrayNNWeightsFinalTemp[i + 32] = sharedArrayNNWeightsFinalTemp[i + 64 + 20];
        }

        for(int i = 64; i < 128; ++i)
        {
            sharedArrayNNWeightsFinalTemp[i] = float16_t(0.0);
        }
    }

    barrier();
}

void frequencyEncodingCooperativeMatrix(highp vec2 inputData, int writeOffset)
{
    int inputCount = 2;

    int index = 0;
    
    for (int inputIndex = 0; inputIndex < inputCount; inputIndex++)
    {
        float p = PI * inputData[inputIndex];
        int modifier = 1;
        
        for (int f = 0; f < NUM_FREQUENCIES; f++)
        {
            float x = float(modifier) * p;
            sharedMatrixActivations[writeOffset + index++] = float16_t(sin(x));
            sharedMatrixActivations[writeOffset + index++] = float16_t(cos(x));
            modifier *= 2;
        }
    }
}

const int arrayPixelValueRetrieveOffsets[2] = {0, 2};

// Test the different pixels between (pixelXStart, pixelYStart) and (pixelXEnd, pixelYStart)
// If they belong to the same texturee patch being worked by this neural network, then add them to sharedArrayUVInference for later processing
void testSamePatch(int pixelXStart, int pixelXEnd, int pixelYStart, int patchIndex, ivec2 maxCoordinates, inout int writeOffset)
{
    for(int i = pixelXStart; i < pixelXEnd; ++i)
    {
        // Depending on the screen area to process, some subgroups might have threads operating outside the processing area.
        // Verify it so work is not duplicated (for the next subgroup taking care of that area)
        if(any(greaterThan(ivec2(i, pixelYStart), ivec2(maxCoordinates))))
        {
            writeOffset++;
            continue;
        }

        // Use the mask to avoid overwriting scene geometry / doing ALU operations
        if(texelFetch(imageIn, ivec2(i, pixelYStart), 0).r == 0.0)
        {
            sharedArrayUVInference[int(gl_LocalInvocationID.x) * numberPixelPerThread + writeOffset] = vec4(-1.0);
            writeOffset++;
            continue;
        }

        highp vec2 pixelCenter = vec2(float(i), pixelYStart) + vec2(0.5);
        highp vec2 inUV        = pixelCenter / vec2(screenWidth, screenHeight);
        highp vec2 d           = inUV * 2.0 - 1.0;
        highp vec4 origin      = inverseViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);
        highp vec4 target      = inverseProjectionMatrix * vec4(d.x, d.y, 1, 1);
        highp vec4 direction   = inverseViewMatrix * vec4(normalize(target.xyz), 0);
        highp vec2 UV          = convertDirectionToUVEquirectangular(direction.xyz);
        vec2 texturePixelCoordinates = UV * vec2(textureWidth, textureHeight);
        
        // Then, obtain the pixel coordinates in the environment texture and compute the texture patch it belongs to
        vec2 patchCoordinatesFloat = floor(vec2(texturePixelCoordinates) / vec2(patchSide));
        ivec2 patchCoordinates = ivec2(patchCoordinatesFloat);
        int currentPixelPatchIndex = patchCoordinates.y * textureNumberPatchXDimension + patchCoordinates.x;

        if(currentPixelPatchIndex != patchIndex)
        {
            sharedArrayUVInference[int(gl_LocalInvocationID.x) * numberPixelPerThread + writeOffset] = vec4(-1.0);
            writeOffset++;
            continue;
        }

        vec2 patchTextureCoordinates = texturePixelCoordinates;
        patchTextureCoordinates /= vec2(textureWidth - 1, textureHeight - 1);
        sharedArrayUVInference[int(gl_LocalInvocationID.x) * numberPixelPerThread + writeOffset] = vec4(patchTextureCoordinates, vec2(i, pixelYStart));
        writeOffset++;
    }
}

void main()
{
    // Initially, each thread will evaluate one pixel. This will need to be greatly optimised in the future.
    for(int i = 0; i < numberElementSubgroup * numberPixelPerThread; ++i)
    {
        sharedArrayUVInference[i] = vec4(-1.0);
    }

    barrier();

    // Each workgroup will read the information of one screen box from the organizePatchWorkload buffer
    int workgroupIDxInt = int(gl_WorkGroupID.x);
    int patchIndex = int(organizePatchWorkload[workgroupIDxInt * 5]);
    vec2 m = vec2(organizePatchWorkload[workgroupIDxInt * 5 + 1], organizePatchWorkload[workgroupIDxInt * 5 + 2]);
    vec2 M = vec2(organizePatchWorkload[workgroupIDxInt * 5 + 3], organizePatchWorkload[workgroupIDxInt * 5 + 4]);

    if((patchIndex == 0) && all(equal(m, vec2(0.0))) && all(equal(M, vec2(0.0))))
    {
        return;
    }

    float workGroupSizeXFloat = float(gl_WorkGroupSize.x);
    int workGroupSizeXInt = int(gl_WorkGroupSize.x);

    float localInvocationIDxFloat = float(gl_LocalInvocationID.x);
    int localInvocationIDxInt = int(gl_LocalInvocationID.x);

    // After the information in the sharedArrayNNBiases and sharedArrayNNWeights is stored, each thread will start testing pixels in the screen space box assigned
    // for the workgroup and infering pixels when required.
    // Together with the cooperative matrix, more performant approaches where the subgroup collaborates to evaluate the nn need to be investigated.

    // Compute the pixels each sugroup thread will take care of within the screen-space box to process
    // For the time being, each thread will process a line of pixels
    vec2 screenSpaceBox = vec2(M.x - m.x, M.y - m.y);

    // The + 1.0 below come from the fact that all the pixels in [m.x, m.y]x[M.x, M.y] region need to be processed. 
    // If m=(5, 20) and M=(10, 30) then the number of pixels to process includes x in {5, 6, 7, 8, 9, 10}
    // (6 elements) and y in {20, 21, 22, ..., 30} (11 elements)
    float totalPixel = (screenSpaceBox.x + 1.0) * (screenSpaceBox.y + 1.0);
    float totalPixelPerThread = ceil(totalPixel / workGroupSizeXFloat);
    float threadPixelYStart = floor((localInvocationIDxFloat * totalPixelPerThread) / screenSpaceBox.x);

    float threadPixelXStart = mod(float(gl_LocalInvocationID.x) * totalPixelPerThread, screenSpaceBox.x);

    float threadPixelXEnd = threadPixelXStart + totalPixelPerThread;
    float numberPixelNewLine = 0.0;

    // In case the number of pixels processed by this thread go out of the screen space box, add the equivalent
    // of "carriage return" continuing in the row below
    if((threadPixelXStart + totalPixelPerThread) > screenSpaceBox.x)
    {
        threadPixelXEnd = screenSpaceBox.x;
        numberPixelNewLine = (threadPixelXStart + totalPixelPerThread) - screenSpaceBox.x;
    }

    // The last thread might need to process a few extra pixels to cover the whole region (due to the mount of pixels
    // processed by each thread not being an exact amount)
    // threadPixelXEnd = (localInvocationIDxInt == (workGroupSizeXInt - 1)) ? screenSpaceBox.x : threadPixelXEnd;

    int pixelXStart = int(threadPixelXStart + m.x);
    int pixelXEnd = int(threadPixelXEnd + m.x);
    int pixelYStart = int(threadPixelYStart + m.y);
    int indexCounter = 0;

    testSamePatch(pixelXStart, pixelXEnd, pixelYStart, patchIndex, ivec2(M), indexCounter);

    int pixelStartXNew = -1;
    int numberPixelNewLineInt = -1;
    int pixelEndXNew = -1;

    // Process also this pixels in the line pixelYStart + 1
    if(numberPixelNewLine > 0.0)
    {
        pixelStartXNew = int(m.x);
        numberPixelNewLineInt = int(numberPixelNewLine);   
        pixelEndXNew = pixelStartXNew + numberPixelNewLineInt + 1;

        testSamePatch(pixelStartXNew, pixelEndXNew, pixelYStart + 1, patchIndex, ivec2(M), indexCounter);
    }

    // At this point, the sharedArrayUVInference has the UVs that need inference for the patch this subgroup is taking care of
    barrier();

    loadNNInformation(patchIndex);

    barrier();

    // Now, start the inference of pixels. First do the layer #1 with 10 neurons, where each neuron will evaluate its 32 connections with the previous layer
    int numberIndex = (numberElementSubgroup * numberPixelPerThread) / 2;

    barrier();

    for(int i = 0; i < numberIndex; ++i)
    {
        // 1. Reset activations and results matrix

        // For now, consider all pixels in sharedArrayUVInference have correct values (UVs from a pixel from the patch this subgroup takes care of)
        
        // Fill the sharedMatrixActivations with the initial information to infer two different pixels
        frequencyEncodingCooperativeMatrix(sharedArrayUVInference[2 * i + 0].xy, 0);
        frequencyEncodingCooperativeMatrix(sharedArrayUVInference[2 * i + 1].xy, 32);
        bool pixelCorrectArray[2];
        pixelCorrectArray[0] = sharedArrayUVInference[2 * i + 0] != vec4(-1.0);
        pixelCorrectArray[1] = sharedArrayUVInference[2 * i + 1] != vec4(-1.0);

        barrier();

        // Init with 0's the sharedMatrixActivations matrix before writing the activations
        coopMatLoad(matrixActivations, sharedMatrixActivations, 0, 8, gl_CooperativeMatrixLayoutColumnMajor);
        matrixResults = coopmat<float, gl_ScopeSubgroup, 16, 8, gl_MatrixUseAccumulator>(0.0);

        barrier();

        // 2. Perform the evaluation of the first layer
        matrixResults = coopMatMulAdd(matrixWeightN0N3, matrixActivations, matrixResults);

        barrier();

        // Collect the activation values from matrixResults (load the cooperative matrix onto sharedArrayNNWeights 
        // and read the corresponding items, which are displayed diagonally in matrixResults)
        coopMatStore(matrixResults, sharedMatrixResults32BitFloat, 0, 8, gl_CooperativeMatrixLayoutRowMajor);

        barrier();

        // Load information from matrixResults now in sharedMatrixResults32BitFloat, eight threads will each take care
        secondLayerNeuronValues[0] = leakyRelu(sharedMatrixResults32BitFloat[0]  + sharedMatrixResults32BitFloat[9]   + sharedMatrixResults32BitFloat[18]  + sharedMatrixResults32BitFloat[27]  + sharedArrayNNBiases[32]);
        secondLayerNeuronValues[1] = leakyRelu(sharedMatrixResults32BitFloat[32] + sharedMatrixResults32BitFloat[41]  + sharedMatrixResults32BitFloat[50]  + sharedMatrixResults32BitFloat[59]  + sharedArrayNNBiases[33]);
        secondLayerNeuronValues[2] = leakyRelu(sharedMatrixResults32BitFloat[64] + sharedMatrixResults32BitFloat[73]  + sharedMatrixResults32BitFloat[82]  + sharedMatrixResults32BitFloat[91]  + sharedArrayNNBiases[34]);
        secondLayerNeuronValues[3] = leakyRelu(sharedMatrixResults32BitFloat[96] + sharedMatrixResults32BitFloat[105] + sharedMatrixResults32BitFloat[114] + sharedMatrixResults32BitFloat[123] + sharedArrayNNBiases[35]);

        secondLayerNeuronValues[10] = leakyRelu(sharedMatrixResults32BitFloat[4]   + sharedMatrixResults32BitFloat[13]  + sharedMatrixResults32BitFloat[22]  + sharedMatrixResults32BitFloat[31]  + sharedArrayNNBiases[32]);
        secondLayerNeuronValues[11] = leakyRelu(sharedMatrixResults32BitFloat[36]  + sharedMatrixResults32BitFloat[45]  + sharedMatrixResults32BitFloat[54]  + sharedMatrixResults32BitFloat[63]  + sharedArrayNNBiases[33]);
        secondLayerNeuronValues[12] = leakyRelu(sharedMatrixResults32BitFloat[68]  + sharedMatrixResults32BitFloat[77]  + sharedMatrixResults32BitFloat[86]  + sharedMatrixResults32BitFloat[95]  + sharedArrayNNBiases[34]);
        secondLayerNeuronValues[13] = leakyRelu(sharedMatrixResults32BitFloat[100] + sharedMatrixResults32BitFloat[109] + sharedMatrixResults32BitFloat[118] + sharedMatrixResults32BitFloat[127] + sharedArrayNNBiases[35]);

        // Next round of neuron inference. Initialize matrixResults
        matrixResults = coopmat<float, gl_ScopeSubgroup, 16, 8, gl_MatrixUseAccumulator>(0.0);

        barrier();

        // 3. Perform the evaluation of the first layer for the next set of neurons
        matrixResults = coopMatMulAdd(matrixWeightN4N7, matrixActivations, matrixResults);

        barrier();

        // Collect the activation values from matrixResults (load the cooperative matrix onto sharedArrayNNWeights 
        // and read the corresponding items, which are displayed diagonally in matrixResults)
        coopMatStore(matrixResults, sharedMatrixResults32BitFloat, 0, 8, gl_CooperativeMatrixLayoutRowMajor);

        barrier();

        // Load information from matrixResults now in sharedMatrixResults32BitFloat, eight threads will each take care
        secondLayerNeuronValues[4] = leakyRelu(sharedMatrixResults32BitFloat[0]  + sharedMatrixResults32BitFloat[9]   + sharedMatrixResults32BitFloat[18]  + sharedMatrixResults32BitFloat[27]  + sharedArrayNNBiases[36]);
        secondLayerNeuronValues[5] = leakyRelu(sharedMatrixResults32BitFloat[32] + sharedMatrixResults32BitFloat[41]  + sharedMatrixResults32BitFloat[50]  + sharedMatrixResults32BitFloat[59]  + sharedArrayNNBiases[37]);
        secondLayerNeuronValues[6] = leakyRelu(sharedMatrixResults32BitFloat[64] + sharedMatrixResults32BitFloat[73]  + sharedMatrixResults32BitFloat[82]  + sharedMatrixResults32BitFloat[91]  + sharedArrayNNBiases[38]);
        secondLayerNeuronValues[7] = leakyRelu(sharedMatrixResults32BitFloat[96] + sharedMatrixResults32BitFloat[105] + sharedMatrixResults32BitFloat[114] + sharedMatrixResults32BitFloat[123] + sharedArrayNNBiases[39]);

        secondLayerNeuronValues[14] = leakyRelu(sharedMatrixResults32BitFloat[4]   + sharedMatrixResults32BitFloat[13]  + sharedMatrixResults32BitFloat[22]  + sharedMatrixResults32BitFloat[31]  + sharedArrayNNBiases[36]);
        secondLayerNeuronValues[15] = leakyRelu(sharedMatrixResults32BitFloat[36]  + sharedMatrixResults32BitFloat[45]  + sharedMatrixResults32BitFloat[54]  + sharedMatrixResults32BitFloat[63]  + sharedArrayNNBiases[37]);
        secondLayerNeuronValues[16] = leakyRelu(sharedMatrixResults32BitFloat[68]  + sharedMatrixResults32BitFloat[77]  + sharedMatrixResults32BitFloat[86]  + sharedMatrixResults32BitFloat[95]  + sharedArrayNNBiases[38]);
        secondLayerNeuronValues[17] = leakyRelu(sharedMatrixResults32BitFloat[100] + sharedMatrixResults32BitFloat[109] + sharedMatrixResults32BitFloat[118] + sharedMatrixResults32BitFloat[127] + sharedArrayNNBiases[39]);

        // Next round of neuron inference. Initialize matrixResults
        matrixResults = coopmat<float, gl_ScopeSubgroup, 16, 8, gl_MatrixUseAccumulator>(0.0);

        barrier();

        // 4. Perform the evaluation of the first layer for the last set of neurons (only two in this case)
        matrixResults = coopMatMulAdd(matrixWeightN8N9, matrixActivations, matrixResults);

        barrier();

        // Collect the activation values from matrixResults (load the cooperative matrix onto sharedArrayNNWeights 
        // and read the corresponding items, which are displayed diagonally in matrixResults)
        coopMatStore(matrixResults, sharedMatrixResults32BitFloat, 0, 8, gl_CooperativeMatrixLayoutRowMajor);

        barrier();

        secondLayerNeuronValues[8] = leakyRelu(sharedMatrixResults32BitFloat[0]  + sharedMatrixResults32BitFloat[9]  + sharedMatrixResults32BitFloat[18] + sharedMatrixResults32BitFloat[27] + sharedArrayNNBiases[40]);
        secondLayerNeuronValues[9] = leakyRelu(sharedMatrixResults32BitFloat[32] + sharedMatrixResults32BitFloat[41] + sharedMatrixResults32BitFloat[50] + sharedMatrixResults32BitFloat[59] + sharedArrayNNBiases[41]);

        secondLayerNeuronValues[18] = leakyRelu(sharedMatrixResults32BitFloat[4]  + sharedMatrixResults32BitFloat[13] + sharedMatrixResults32BitFloat[22] + sharedMatrixResults32BitFloat[31] + sharedArrayNNBiases[40]);
        secondLayerNeuronValues[19] = leakyRelu(sharedMatrixResults32BitFloat[36] + sharedMatrixResults32BitFloat[45] + sharedMatrixResults32BitFloat[54] + sharedMatrixResults32BitFloat[63] + sharedArrayNNBiases[41]);

        // 5. Now that the first layer of neurons (the 10 in 32x10x3) has been evaluated, reuse matrixWeightN0N3 and matrixActivations with the information for the last 
        // layer evaluation, which will provide the information of two pixels.
        // The last set of weights are the ones which connect each one of the 3 neurons in the last layer with the 10 neurons in the prevous layer
        // There is a total of 30 weights. They are set per-neuron, in groups of 10, in two rows of 8 elements (leaving the last 6 as 0)
        // The activations matrix will now have the information generated in secondLayerNeuronValues

        // Init to 0 sharedMatrixActivations

        sharedMatrixActivations[2 * localInvocationIDxInt + 0] = float16_t(0.0);
        sharedMatrixActivations[2 * localInvocationIDxInt + 1] = float16_t(0.0);

        for(int j = 0; j < 10; ++j)
        {
            sharedMatrixActivations[j]      = float16_t(secondLayerNeuronValues[j]);
            sharedMatrixActivations[j + 16] = float16_t(secondLayerNeuronValues[j + 10]); 
        }

        barrier();

        // Load the values from sharedMatrixActivations onto matrixActivations
        coopMatLoad(matrixActivations, sharedMatrixActivations, 0, 8, gl_CooperativeMatrixLayoutColumnMajor); // could be the one?

        // sharedArrayNNWeightsFinalTemp has the correct information from the call to loadNNInformation
        // Reuse matrixWeightN0N3 to load the information from sharedArrayNNWeightsFinalTemp
        coopMatLoad(matrixWeightN0N2LastLayer, sharedArrayNNWeightsFinalTemp, 0, 8, gl_CooperativeMatrixLayoutRowMajor); // could be the one?

        // Evaluate last layer
        matrixResults = coopmat<float, gl_ScopeSubgroup, 16, 8, gl_MatrixUseAccumulator>(0.0);
        barrier();

        matrixResults = coopMatMulAdd(matrixWeightN0N2LastLayer, matrixActivations, matrixResults);

        barrier();

        // Collect the activation values from matrixResults (load the cooperative matrix onto sharedArrayNNWeights 
        // and read the corresponding items, which are displayed diagonally in matrixResults)
        coopMatStore(matrixResults, sharedMatrixResults32BitFloat, 0, 8, gl_CooperativeMatrixLayoutRowMajor);

        barrier();

        // Get neuron values from first pixel being evaluated
        // Neuron 0 has its values spread in matrix coordinates (0, 0), (1,  1)
        // Neuron 1 has its values spread in matrix coordinates (2, 0), (3,  1)
        // Neuron 2 has its values spread in matrix coordinates (4, 0), (5,  1)

        // Get neuron values from second pixel being evaluated
        // Neuron 0 has its values spread in matrix coordinates (0, 2), (1,  3)
        // Neuron 1 has its values spread in matrix coordinates (2, 2), (3,  3)
        // Neuron 2 has its values spread in matrix coordinates (4, 2), (5,  3)
        if(localInvocationIDxInt < 2)
        {
            vec3 pixel = vec3(
                leakyRelu(sharedMatrixResults32BitFloat[0 + arrayPixelValueRetrieveOffsets[localInvocationIDxInt]] + 
                          sharedMatrixResults32BitFloat[9 + arrayPixelValueRetrieveOffsets[localInvocationIDxInt]] + 
                          sharedArrayNNBiases[42]),

                leakyRelu(sharedMatrixResults32BitFloat[16 + arrayPixelValueRetrieveOffsets[localInvocationIDxInt]] + 
                          sharedMatrixResults32BitFloat[25 + arrayPixelValueRetrieveOffsets[localInvocationIDxInt]] + 
                          sharedArrayNNBiases[43]),

                leakyRelu(sharedMatrixResults32BitFloat[32 + arrayPixelValueRetrieveOffsets[localInvocationIDxInt]] + 
                          sharedMatrixResults32BitFloat[41 + arrayPixelValueRetrieveOffsets[localInvocationIDxInt]] + 
                          sharedArrayNNBiases[44])
                );

            if(pixelCorrectArray[localInvocationIDxInt])
            {
                vec3 toneMappedColor = min(pixel, 50.0 / exposure);
                toneMappedColor = min(toneMappedColor, 50.0 / exposure);
                toneMappedColor *= exposure;

                // http://filmicworlds.com/blog/filmic-tonemapping-operators/
                // Our favorite is the optimized formula by Jim Hejl and Richard Burgess-Dawson
                // We particularly like its high contrast and the fact that it is very cheap, with
                // only 4 mads and a reciprocal.
                mediump vec3 x = max(vec3(0.0), toneMappedColor - vec3(0.004));
                toneMappedColor = (x * (6.2 * x + .49)) / (x * (6.175 * x + 1.7) + 0.06);
                imageStore(imageOut, ivec2(sharedArrayUVInference[2 * i + localInvocationIDxInt].zw), vec4(toneMappedColor, 1.0));
            }
        }
    }
}
