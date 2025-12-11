#version 320 es

layout(std430, set = 0, binding = 0) buffer screenSpaceBoxBuffer
{
	float screenSpaceBox[];
};

// Each element in this buffer takes 5 indices from the buffer:
//      index 0: Patch index (which depends on the number of rows and columns the texture approximated is partitioned in)
//      index 1: Patch bounding box m.x Smallest x coordinates of the screen space box to process where pixels of the patch are present
//      index 2: Patch bounding box m.y Smallest y coordinates of the screen space box to process where pixels of the patch are present
//      index 3: Patch bounding box M.x Largest x coordinates of the screen space box to process where pixels of the patch are present
//      index 4: Patch bounding box M.y Largest y coordinates of the screen space box to process where pixels of the patch are present
layout(std430, set = 0, binding = 1) buffer organizePatchWorkloadBuffer
{
    float organizePatchWorkload[];
};

layout(std430, set = 0, binding = 2) buffer atomicCounterBuffer
{
    int atomicCounter[];
};

layout(set = 0, binding = 3) uniform nnUniformBuffer
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

layout(std430, set = 0, binding = 4) buffer organizePatchWorkloadDebugBuffer
{
    float organizePatchWorkloadDebug[];
};


// Number of elements per each slot in organizePatchWorkload (patch index and the screen space region to cover, m.x, m.y, M.x, M.y)
const int numberElementPerSlot = 5;

// The shader code is loaded and in "local_size_x = %d", "%d" is changed to match the size of the subgroup in the current GPU
// layout(local_size_x = [Subgroup_size], local_size_y = 1, local_size_z = 1) in;
%s1

void main()
{
    int globalInvocationIDX = int(gl_GlobalInvocationID.x);

    if(globalInvocationIDX > (textureNumberPatchXDimension * textureNumberPatchYDimension))
    {
        return;
    }

    // Each thread in the whole dispatch will verify the information at index 4 * globalInvocationIDX in screenSpaceBox
    int index = 4 * globalInvocationIDX;

    if((screenSpaceBox[index] != 0.0) || (screenSpaceBox[index + 1] != 0.0) || (screenSpaceBox[index + 2] != 0.0) || (screenSpaceBox[index + 3] != 0.0))
    {
        // Compute the extent of the screen space box with the patch to be infered by the neural network and divide it in a set of
        // patches with a balanced amount of pixels so several workgroups separately take care of infering the pixels in each set
        vec2 m = vec2(screenSpaceBox[index],     screenSpaceBox[index + 1]);
        vec2 M = vec2(screenSpaceBox[index + 2], screenSpaceBox[index + 3]);

        // Each division of the screen space box is done horizontally
        //        |-------------------------------------------------------| M(x, y)
        //        |                                                       |
        //        |                                                       |
        //        |-------------------------------------------------------|
        //        |                                                       |
        //        |                                                       |
        //        |-------------------------------------------------------|
        //        |                                                       |
        //        |                                                       |
        //m(x, y) |-------------------------------------------------------|

        vec2 dimensions = vec2(M.x - m.x, M.y - m.y);
        float screenArea = dimensions.x * dimensions.y;
        float numDivisions = ceil(float(screenArea) / float(organizePatchWorkloadNumberPixelScreenSpaceRegion));
        float verticalOffset = ceil(dimensions.y / numDivisions);

        // As some patches can have regions completely outside the screen, thses regions should not be added for later processing.
        // A first pass is done to know how many of these regions exist in the current patch before adding the final ones which are not completely outside screen
        int numberInsideRegions = 0;

        float yOffset = m.y;
        for(int i = 0; i < int(numDivisions); ++i)
        {
            vec2 mTemp = vec2(m.x, yOffset);
            vec2 MTemp = vec2(M.x, yOffset + verticalOffset);

            // Avoid adding regions which are completely out of screen
            if(all(lessThan(mTemp, vec2(0.0))) && all(lessThan(MTemp, vec2(0.0))))
            {
                continue;
            }

            mTemp = clamp(mTemp, m, M);
            MTemp = clamp(MTemp, m, M);

            if((mTemp.x == MTemp.x) || (mTemp.y == MTemp.y))
            {
                continue;
            }

            yOffset += verticalOffset + 1.0;
            numberInsideRegions++;
        }

        // Use atomic counter to contiguously build the different sets of regions each screen space box needs to cover, each with organizePatchWorkloadNumberPixelScreenSpaceRegion
        // in organizePatchWorkload, where later another dispatch will load the corresponding neural network and approximate the information
        int bufferIndex = atomicAdd(atomicCounter[0], numberInsideRegions);

        yOffset = m.y;
        numberInsideRegions = 0;
        for(int i = 0; i < int(numDivisions); ++i)
        {
            vec2 mTemp = vec2(m.x, yOffset);
            vec2 MTemp = vec2(M.x, yOffset + verticalOffset);

            // Avoid adding regions which are completely out of screen
            if(all(lessThan(mTemp, vec2(0.0))) && all(lessThan(MTemp, vec2(0.0))))
            {
                continue;
            }

            mTemp = clamp(mTemp, m, M);
            MTemp = clamp(MTemp, m, M);

            if((mTemp.x == MTemp.x) || (mTemp.y == MTemp.y))
            {
                continue;
            }

            int indexOffset = bufferIndex * numberElementPerSlot + numberInsideRegions * numberElementPerSlot;

            organizePatchWorkload[indexOffset + 0] = float(index / 4); // patch index
            organizePatchWorkload[indexOffset + 1] = mTemp.x;
            organizePatchWorkload[indexOffset + 2] = mTemp.y;
            organizePatchWorkload[indexOffset + 3] = MTemp.x;
            organizePatchWorkload[indexOffset + 4] = MTemp.y;

            yOffset += verticalOffset + 1.0;
            numberInsideRegions++;
        }
    }
}
