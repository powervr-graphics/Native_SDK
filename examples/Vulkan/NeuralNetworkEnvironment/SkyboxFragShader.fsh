#version 320 es

layout(set = 0, binding = 0) uniform mediump samplerCube skybox;
layout(set = 0, binding = 1) uniform mediump sampler2D equirectangularEnvironment;
layout(location = 0) in mediump vec3 RayDir;

layout(location = 0) out mediump vec4 outColor;
layout(location = 1) out mediump float outColorMask;

const highp float ExposureBias = 1.0;
const highp float PI = 3.1415920257568359375;

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

highp vec2 convertDirectionToUVEquirectangular(highp vec3 direction)
{
	// Theta is normalized to [0, 1]
    highp float theta = (atan(direction.z, direction.x) / (2.0 * PI)) + 0.5;
    highp float thetaOffsetted = theta + 0.75; // Offset to match IBL SDK sample environment
	if(thetaOffsetted > 1.0)
	{
		thetaOffsetted -= 1.0;
	}

	// Phi is normalized to [0, 1]
	highp float phi = acos(-direction.y) / PI;
	highp float phiOffsetted = phi + 0.0005; // Offset to match IBL SDK sample environment
    if (phiOffsetted > 1.0)
    {
        phiOffsetted -= 1.0;
    }

    return vec2(1.0 - thetaOffsetted, 1.0 - phiOffsetted);
}

highp vec2 convertTexturePixelToScreenPixel(highp vec2 texturePixelCoordinates)
{
    highp vec2 UVs = texturePixelCoordinates / vec2(textureWidth, textureHeight);
    
    UVs = vec2(1.0 - UVs.x, 1.0 - UVs.y);
    highp float theta = UVs.x;
    highp float phi = UVs.y;

    phi -= 0.0005; // Undo offset to match IBL SDK sample environment
    phi *= 3.1415920257568359375; // Map

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

void main()
{
	highp vec2 pixelCenter = vec2(gl_FragCoord.xy);
    highp vec2 inUV = pixelCenter/vec2(screenWidth, screenHeight); // Screen size
    highp vec2 d = inUV * 2.0 - 1.0;
    highp vec4 target    = inverseProjectionMatrix * vec4(d.x, d.y, 1, 1);
    highp vec4 direction = inverseViewMatrix * vec4(normalize(target.xyz), 0);

    highp vec2 UV = convertDirectionToUVEquirectangular(direction.xyz);
	highp vec3 environment = texture(equirectangularEnvironment, UV).rgb;
	mediump vec3 toneMappedColor = min(environment, 50. / exposure);
	toneMappedColor *= exposure;

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Our favorite is the optimized formula by Jim Hejl and Richard Burgess-Dawson
	// We particularly like its high contrast and the fact that it is very cheap, with
	// only 4 mads and a reciprocal.
	mediump vec3 x = max(vec3(0.), toneMappedColor - vec3(0.004));
	toneMappedColor = (x * (6.2 * x + .49)) / (x * (6.175 * x + 1.7) + 0.06);
	outColor = vec4(toneMappedColor, 1.0);
}
