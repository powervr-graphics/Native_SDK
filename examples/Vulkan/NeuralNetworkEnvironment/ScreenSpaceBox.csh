#version 320 es

layout(std430, set = 0, binding = 0) buffer screenSpaceBoxBuffer
{
	float screenSpaceBox[];
};

layout(set = 0, binding = 1) uniform highp sampler2D imageIn;

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

layout(std430, set = 0, binding = 3) buffer debugBuffer
{
    float debug[];
};

const highp float PI = 3.1415920257568359375;
const mediump int maxNumberTilePerThread = 4;

// The shader code is loaded and is changed to make a shared variable array with number elements
// shared int sharedArrayVisiblePatches[maxNumberTilePerThread * NUMBER_WORKGROUP_THREADS];
// %s0

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

// Convert some texture coordinates in the environment analytical (unit) sphere to screen pixels
highp vec2 convertTexturePixelToScreenPixel(highp vec2 texturePixelCoordinates)
{
    highp vec2 UVs = texturePixelCoordinates / vec2(textureWidth, textureHeight);
    
    UVs = vec2(1.0 - UVs.x, 1.0 - UVs.y);
    highp float theta = UVs.x;
    highp float phi = UVs.y;

    phi -= 0.0005; // Undo offset to match IBL SDK sample environment
    phi *= PI; // Map

    theta -= 0.75; // Undo offset to match IBL SDK sample environment
    theta -= 0.5; // Map
    theta *= 2.0 * PI; // Map

    // Compute cartesian coordinates in a sphere of radius 1.0 (all directions are normalised)
    highp float x = sin(phi) * cos(theta);
    highp float y = sin(phi) * sin(theta);
    highp float z = cos(phi);

    // Map to axis used by API
    highp vec4 direction = vec4(x, -1.0 * z, y, 0.0);

    // Transform
    highp vec4 temp0 = viewMatrix * direction;
    temp0 = vec4(normalize(vec3(temp0.x, temp0.y, temp0.z)), 0);
    highp vec4 temp1 = projectionMatrix * temp0;

    // Perspective division after projection
    highp vec2 screenPixelCoordinates = vec2(temp1.x, temp1.y) / temp1.w;

    // Move from [-1, 1] to [0, 1]
    screenPixelCoordinates = (screenPixelCoordinates + vec2(1.0)) * vec2(0.5);

    // Scale to screen texture
    screenPixelCoordinates *= vec2(screenWidth, screenHeight);

    return screenPixelCoordinates;
}

// The shader code is loaded and in "local_size_x = %d", "%d" is changed to match the size of the subgroup in the current GPU
// layout(local_size_x = [Subgroup_size], local_size_y = 1, local_size_z = 1) in;
%s1
void main()
{
    // For the screen pixel being processes, compute the UV coordinates of the environment texture
    vec2 coordinatesFloat = vec2(mod(float(gl_GlobalInvocationID.x), float(screenWidthScreenSpaceBox)), floor(float(gl_GlobalInvocationID.x) / float(screenWidthScreenSpaceBox)));

    // Scale the pixel coordinates to test. Not all the screen pixels are tested (a factor is used to test the equivalent of a downscaled version of the scene)
    coordinatesFloat     *= vec2(4.0);
    ivec2 coordinates     = ivec2(coordinatesFloat);

    if(texelFetch(imageIn, coordinates, 0).r == 0.0)
    {
        return;
    }

    if(coordinates.y > screenHeight)
    {
        return;
    }

    highp vec2 pixelCenter = vec2(coordinates) + vec2(0.5);
    highp vec2 inUV        = pixelCenter / vec2(screenWidth, screenHeight); // inUV MATCHES RASTER
    highp vec2 d           = inUV * 2.0 - 1.0;
    highp vec4 origin      = inverseViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);
    highp vec4 target      = inverseProjectionMatrix * vec4(d.x, d.y, 1, 1); // target MATCHES RASTER
    highp vec4 direction   = inverseViewMatrix * vec4(normalize(target.xyz), 0);
    highp vec2 UV          = convertDirectionToUVEquirectangular(direction.xyz);
    vec2 texturePixelCoordinates  = UV * vec2(textureWidth, textureHeight);
    
    // Then, obtain the pixel coordinates in the environment texture and compute the texture patch it belongs to
    vec2 patchCoordinatesFloat = floor(vec2(texturePixelCoordinates) / vec2(patchSide));
    ivec2 patchCoordinates = ivec2(patchCoordinatesFloat);
    int patchIndex = patchCoordinates.y * textureNumberPatchXDimension + patchCoordinates.x;

    vec2 arrayPatchCorners[4];
    arrayPatchCorners[0] = patchCoordinatesFloat * vec2(patchSide); // Lower left corner (min)
    arrayPatchCorners[1] = arrayPatchCorners[0] + vec2(patchSide, 0); // Upper left corner
    arrayPatchCorners[2] = arrayPatchCorners[0] + vec2(0, patchSide); // Lower right corner
    arrayPatchCorners[3] = arrayPatchCorners[0] + vec2(patchSide); // Upper right corner (max)

    // Compute screen box containing patch
    vec2 m = vec2( 100000.0);
    vec2 M = vec2(-100000.0);

    for(int i = 0; i < 4; ++i)
    {
        highp vec2 temp = convertTexturePixelToScreenPixel(arrayPatchCorners[i]);

        m = min(m, temp);
        M = max(M, temp);
    }

    m = vec2(floor(m));
    M = vec2(ceil(M));

    screenSpaceBox[patchIndex * 4    ] = m.x;
    screenSpaceBox[patchIndex * 4 + 1] = m.y;
    screenSpaceBox[patchIndex * 4 + 2] = M.x;
    screenSpaceBox[patchIndex * 4 + 3] = M.y;
}
