#version 320 es

#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#extension GL_EXT_shader_16bit_storage : enable

precision highp float;

// The shader code is loaded and in "local_size_x = %d", "%d" is changed to match the size of the subgroup in the current GPU
// Workgroup size is declared here and not before main() as it is used in functions defined before main() and in that case
// the value before it is declared is (1, 1, 1) ( see https://github.com/KhronosGroup/glslang/issues/2479 )
// layout(local_size_x = [Subgroup_size], local_size_y = 1, local_size_z = 1) in;
%s3

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

layout(std430, set = 0, binding = 4) buffer nnEnvironmentDebugBuffer
{
    float nnEnvironmentDebug[];
};

layout(set = 0, binding = 5) uniform highp sampler2D imageIn;

const int layerMaxNumber = 5;
const int neuronLayerMaxNumber = 32;
const int frequencyNumber = 8;
const float PI = 3.141593;

// Build a shared variavble where to sotre for this workgroup the information from the neural network patch that will be used
// by all threads in the subgroup to approximate pixels from the environment
// shared float sharedArrayNNBiases[];
%s0

// Build a shared variavble where to sotre for this workgroup the information from the neural network patch that will be used
// by all threads in the subgroup to approximate pixels from the environment
// shared float sharedArrayNNWeights[];
%s1

// Define a custom-size array for sotring the activation values
float arrayActivation[layerMaxNumber * neuronLayerMaxNumber];

int globalNeuronLayerStartIndex[layerMaxNumber];

int numberNeuronPerLayer[layerMaxNumber];

int totalLayerConnectionOffset[layerMaxNumber];

// Use positional encoding for the uv coordinates:
// sin(2^0 * PI * p), cos(2^0 * PI * p)
// sin(2^1 * PI * p), cos(2^1 * PI * p)
// ...
// sin(2^(L-1) * PI * p), cos(2^(L-1) * PI * p)
void positionalEncoding(vec2 uv)
{
    int indexX = 0;
    int indexY = frequencyNumber * 2;
    vec2 p = uv * PI;
    float powerOfTwo = 1.0f;

    for (int i = 0; i < frequencyNumber; i++)
    {
        float x = float(powerOfTwo) * p.x;
        float y = float(powerOfTwo) * p.y;
        arrayActivation[indexX++] = sin(x);
        arrayActivation[indexX++] = cos(x);
        arrayActivation[indexY++] = sin(y);
        arrayActivation[indexY++] = cos(y);
        powerOfTwo *= 2;
    }
}

const float leakyReLUGradient=0.01;

float activationLeakyReLU(float x)
{
    if(x >= 0.0f)
    {
        return x;
    }

    return x * leakyReLUGradient;
}

void forwardPass(vec2 inputData)
{
    for(int i = 0; i < int(layerMaxNumber * neuronLayerMaxNumber); ++i)
    {
        arrayActivation[i] = 0.0;
    }

    positionalEncoding(inputData);
    
    // Start evaluating from layer #1 based on the connections of each neuron
    // of the layer with the neurons of the previous layer. As this is a
    // Multi Layer Perceptron (MLP), each neuron is connected with all neurons of the
    // previous layer.
    for (int i = 1; i < layerCount; i++)
    {
        int neuronNumberCurrentLayer = numberNeuronPerLayer[i];
    
        // Evaluate each neuron of the current layer: Take each connection (weight)
        // with each neuron from the previous layer and multiply by the activation of
        // the neuron of the previous layer. Accumulate the value for all connections.
        for (int j = 0; j < neuronNumberCurrentLayer; j++)
        {
            int neuronIndex = globalNeuronLayerStartIndex[i] + j;
            float accumulatedNeuronValue = sharedArrayNNBiases[neuronIndex];
            
            for (int k = 0; k < numberNeuronPerLayer[i - 1]; k++)
            {
                int neuronPreviousLayerConnectionWeight = totalLayerConnectionOffset[i] + (j * numberNeuronPerLayer[i - 1]) + k;
                int neuronPreviousLayerActivationIndex = globalNeuronLayerStartIndex[i - 1] + k;
                accumulatedNeuronValue += sharedArrayNNWeights[neuronPreviousLayerConnectionWeight] * arrayActivation[neuronPreviousLayerActivationIndex];
            }
            
            arrayActivation[neuronIndex] = activationLeakyReLU(accumulatedNeuronValue);
        }
    }
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

// Load neural general network information needed to infer pixel information
void loadUniformBufferInfo()
{
    globalNeuronLayerStartIndex[0] = neuronOffset0;
    globalNeuronLayerStartIndex[1] = neuronOffset1;
    globalNeuronLayerStartIndex[2] = neuronOffset2;
    globalNeuronLayerStartIndex[3] = neuronOffset3;
    globalNeuronLayerStartIndex[4] = neuronOffset4;

    numberNeuronPerLayer[0] = neuronsPerLayer0;
    numberNeuronPerLayer[1] = neuronsPerLayer1;
    numberNeuronPerLayer[2] = neuronsPerLayer2;
    numberNeuronPerLayer[3] = neuronsPerLayer3;
    numberNeuronPerLayer[4] = neuronsPerLayer4;

    totalLayerConnectionOffset[0] = connectionOffset0;
    totalLayerConnectionOffset[1] = connectionOffset1;
    totalLayerConnectionOffset[2] = connectionOffset2;
    totalLayerConnectionOffset[3] = connectionOffset3;
    totalLayerConnectionOffset[4] = connectionOffset4;
}

// Load the neural network information corresponding to the patch this workgroup will use to approximate screen pixels
void loadNNInformation(int patchIndex)
{
    float workGroupSizeXFloat = float(gl_WorkGroupSize.x);
    int workGroupSizeXInt = int(gl_WorkGroupSize.x);
    int localInvocationIDxInt = int(gl_LocalInvocationID.x);
    int workgroupIDxInt = int(gl_WorkGroupID.x);

    // Load the nnBiases buffer information from neuralNetworkEnvironment
    int numElementPerThread = int(floor(float(nnBiasesNumberElement) / workGroupSizeXFloat));
    int bufferIndexStart = (nnBiasesNumberElement + nnWeightsNumberElement) * patchIndex;
    int sharedIndexStart = localInvocationIDxInt * numElementPerThread;
    int sharedIndexEnd = (localInvocationIDxInt == (workGroupSizeXInt - 1)) ? nnBiasesNumberElement : sharedIndexStart + numElementPerThread; // The last thread might need to load some extra indices due to the "floor" operation in the computation of numElementPerThread

    for(int i = sharedIndexStart; i < sharedIndexEnd; ++i)
    {
        sharedArrayNNBiases[i] = float(neuralNetworkEnvironment[bufferIndexStart + i]);
    }

    // Load the nnWeights buffer information from neuralNetworkEnvironment
    numElementPerThread = int(floor(float(nnWeightsNumberElement) / workGroupSizeXFloat));
    bufferIndexStart += nnBiasesNumberElement; // The nnBiases information is stored in neuralNetworkEnvironmentBuffer after the nnWeights information
    sharedIndexStart = localInvocationIDxInt * numElementPerThread;
    sharedIndexEnd = (localInvocationIDxInt == (workGroupSizeXInt - 1)) ? nnWeightsNumberElement : sharedIndexStart + numElementPerThread; // The last thread might need to load some extra indices due to the "floor" operation in the computation of 

    for(int i = sharedIndexStart; i < sharedIndexEnd; ++i)
    {
        sharedArrayNNWeights[i] = float(neuralNetworkEnvironment[bufferIndexStart + i]);
    }
}

void processPixels(int pixelXStart, int pixelYStart, int pixelXEnd, ivec2 M, int patchIndex)
{
    for(int i = pixelXStart; i < pixelXEnd; ++i)
    {
        // Depending on the screen area to process, some subgroups might have threads operating outside the processing area.
        // Verify it so work is not duplicated (for the next subgroup taking care of that area)
        if(any(greaterThan(ivec2(i, pixelYStart), ivec2(M))))
        {
            continue;
        }

        // Use the mask to avoid overwriting scene geometry / doing ALU operations
        if(texelFetch(imageIn, ivec2(i, pixelYStart), 0).r == 0.0)
        {
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
            continue;
        }

        vec2 patchTextureCoordinates = texturePixelCoordinates;
        patchTextureCoordinates /= vec2(textureWidth - 1, textureHeight - 1);

        forwardPass(patchTextureCoordinates);

        int resultBaseIndex = globalNeuronLayerStartIndex[layerCount - 1];
        vec3 result = vec3(arrayActivation[resultBaseIndex + 0], arrayActivation[resultBaseIndex + 1], arrayActivation[resultBaseIndex + 2]);

        // http://filmicworlds.com/blog/filmic-tonemapping-operators/
        // Our favorite is the optimized formula by Jim Hejl and Richard Burgess-Dawson
        // We particularly like its high contrast and the fact that it is very cheap, with
        // only 4 mads and a reciprocal.
        vec3 toneMappedColor = min(result, 50.0 / exposure);
        toneMappedColor = min(toneMappedColor, 50.0 / exposure);
        toneMappedColor *= exposure;
        mediump vec3 x = max(vec3(0.0), toneMappedColor - vec3(0.004));
        toneMappedColor = (x * (6.2 * x + .49)) / (x * (6.175 * x + 1.7) + 0.06);
        imageStore(imageOut, ivec2(i, pixelYStart), vec4(toneMappedColor, 1.0));
    }
}

// Unoptimized implementation to infer neural network values for a specific screen-space region.
// The subgroup will load the neural network information (bias and weight) and eahc thread will take care of
// infering at least one pixel.
void main()
{
    // Initially, each thread will evaluate one pixel. This will need to be greatly optimised in the future.
    loadUniformBufferInfo();

    // Each workgroup will read the information of one screen box from the organizePatchWorkload buffer
    int workgroupIDxInt = int(gl_WorkGroupID.x);
    int patchIndex = int(organizePatchWorkload[workgroupIDxInt * 5]);
    vec2 m = vec2(organizePatchWorkload[workgroupIDxInt * 5 + 1], organizePatchWorkload[workgroupIDxInt * 5 + 2]);
    vec2 M = vec2(organizePatchWorkload[workgroupIDxInt * 5 + 3], organizePatchWorkload[workgroupIDxInt * 5 + 4]);

    if((patchIndex == 0) && all(equal(m, vec2(0.0))) && all(equal(M, vec2(0.0))))
    {
        return;
    }

    loadNNInformation(patchIndex);

    barrier();

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
    float totalPixelPerThread = ceil(totalPixel / float(gl_WorkGroupSize.x));
    float threadPixelYStart = floor((float(gl_LocalInvocationID.x) * totalPixelPerThread) / screenSpaceBox.x);

    float threadPixelXStart = mod(float(gl_LocalInvocationID.x) * totalPixelPerThread, screenSpaceBox.x);

    float threadPixelXEnd = threadPixelXStart + totalPixelPerThread;
    float numberPixelNewLine = 0.0;

    // In case the number of pixels processed by this thread go out of the screen space box, add the equivalent
    // of "carriage return"l continuing in the row below
    if((threadPixelXStart + totalPixelPerThread) > screenSpaceBox.x)
    {
        threadPixelXEnd = screenSpaceBox.x;
        numberPixelNewLine = (threadPixelXStart + totalPixelPerThread) - screenSpaceBox.x;
    }
    
    int pixelXStart = int(threadPixelXStart + m.x);
    int pixelXEnd = int(threadPixelXEnd + m.x);
    int pixelYStart = int(threadPixelYStart + m.y);

    processPixels(pixelXStart, pixelYStart, pixelXEnd, ivec2(M), patchIndex);

    // Process also this pixels in the line pixelYStart + 1
    if(numberPixelNewLine > 0.0)
    {
        int pixelStartXNew = int(m.x);
        int numberPixelNewLineInt = int(numberPixelNewLine);   
        int pixelEndXNew = pixelStartXNew + numberPixelNewLineInt + 1;

        processPixels(pixelStartXNew, pixelYStart + 1, pixelEndXNew, ivec2(M), patchIndex);
    }
}
