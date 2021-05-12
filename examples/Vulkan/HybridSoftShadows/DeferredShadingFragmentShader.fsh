#version 450

#define NUMBER_OF_RANDOM_ROTATIONS (32 * 32)
#define MAX_KERNEL_WIDTH 8 // (MAX_KERNEL_WIDTH - 1) * 2 must be <= 32
#define MAXIMUM_DISTANCE_TO_OCCLUDER 150.0f

layout(set = 0, binding = 0) uniform CameraUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mInvViewProjectionMatrix;
	highp vec4 vEyePosition;
	highp vec4 vClipPlanes;
	uint uFrameIdx;
};

layout(set = 0, binding = 1) uniform LightUBO
{
	highp vec4 vLightColor;
	highp vec4 vLightPosition;
	highp vec4 vAmbientColor;
	highp vec4 vLightDirection;
	highp float penumbraAngle;
	highp float lightRadius;
	highp float innerConeAngle;
	highp float outerConeAngle;
	int numShadowRays;
};

layout(set = 0, binding = 5, std140) uniform UboPoissonRotation { highp vec4 poissonRotation[NUMBER_OF_RANDOM_ROTATIONS]; };

layout(set = 1, binding = 0) uniform sampler2D gbufferAlbedo_Shininess;
layout(set = 1, binding = 1) uniform sampler2D gbufferNormal_Visibility_HitDistance;
layout(set = 1, binding = 2) uniform sampler2D gbufferDepth;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outFragColor;

const highp float fDepthMax = 0.9999f;
const highp float fDepthDiscontinuityThreshold = 50.0f;
const int numSamplingPositions = 9;
// const int numSamplingPositions = 12;
const highp float inverseNumberSamplingPositions = 1.0 / float(numSamplingPositions);

const highp vec2 poissonDiscKernel[numSamplingPositions] = vec2[numSamplingPositions](
	// 9 tap filter
	vec2(0.15930, 0.089750), vec2(0.50147, -0.35807), vec2(0.69607, 0.35559), vec2(-0.0036825, -0.59150), vec2(0.95581, -0.18159), vec2(-0.65031, 0.058189), vec2(0.11915, 0.78449),
	vec2(-0.34296, 0.51575), vec2(-0.60380, -0.41527)
	/*
		vec2(-0.326212, -0.405810),	vec2(-0.840144, -0.073580), vec2(-0.695914,  0.457137),
		vec2(-0.203345,  0.620716),	vec2( 0.962340, -0.194983), vec2( 0.473434, -0.480026),
		vec2( 0.519456,  0.767022),	vec2( 0.185461, -0.893124), vec2( 0.507431,  0.064425),
		vec2( 0.896420,  0.412458),	vec2(-0.321940, -0.932615), vec2(-0.791559, -0.597710)
	*/
);

// https://www.opengl.org/wiki/Compute_eye_space_from_window_space
// Get the eye space depth from the window space depth values
float getEyeSpaceDepth(const float d)
{
	highp float n = vClipPlanes.x;
	highp float f = vClipPlanes.y;
	return n * f / (f - d * (f - n));
}

float getDepthDelta(const vec2 sampleCoord, const float referenceDepth) { return abs(referenceDepth - getEyeSpaceDepth(textureLod(gbufferDepth, sampleCoord, 0).x)); }

// Compute the partial derivatives of eye space Z w/ respect to screen space xy
vec2 computeDepthPartials(vec2 centerCoord, const float referenceDepth)
{
	const vec2 pixelSize = vec2(1.0f) / vec2(textureSize(gbufferNormal_Visibility_HitDistance, 0));
	vec2 result;
	// dZdx
	result.x = min(getDepthDelta(centerCoord + vec2(pixelSize.x, 0.0), referenceDepth), getDepthDelta(centerCoord + vec2(-pixelSize.x, 0.0), referenceDepth));
	// dZdy
	result.y = min(getDepthDelta(centerCoord + vec2(0.0, pixelSize.y), referenceDepth), getDepthDelta(centerCoord + vec2(0.0, -pixelSize.y), referenceDepth));

	return result;
}

float calculateWindowLength(const float depth, float lineLength)
{
	vec4 p0 = vec4(0, 0, depth, 1.0);
	vec4 p1 = vec4(lineLength, 0, depth, 1.0);

	// To clip space
	vec4 pp0 = mProjectionMatrix * p0;
	vec4 pp1 = mProjectionMatrix * p1;

	// To NDC
	pp0.x /= pp0.w;
	pp1.x /= pp1.w;
	
	float fHalfWindowHeight = float(textureSize(gbufferNormal_Visibility_HitDistance, 0).y) / 2.0f;

	// To window
	pp0.x = fHalfWindowHeight * pp0.x + fHalfWindowHeight;
	pp1.x = fHalfWindowHeight * pp1.x + fHalfWindowHeight;

	return pp0.x - pp1.x;
}

void computePoissonKernelSize(const highp float distanceToOccluder, const float referenceDepth, highp float inverseDistanceToLight, out float kernelSize)
{
	kernelSize = lightRadius * distanceToOccluder * inverseDistanceToLight;

	kernelSize = calculateWindowLength(referenceDepth, kernelSize);

	// the minimum returned kernel size will be 1.0
	kernelSize = max(min(kernelSize, float(MAX_KERNEL_WIDTH)), 1.0);
}

float lodSample(vec2 texCoords_, float LOD_)
{
	float mip0;
	float mip1;

	float fractVal;
	float mixVal;

	float mip2 = textureLod(gbufferNormal_Visibility_HitDistance, texCoords_, 2.0).b;

	if (LOD_ < 2.0)
	{
		mip0 = textureLod(gbufferNormal_Visibility_HitDistance, texCoords_, 1.0).b;
		mip1 = mip2;
	}
	else
	{
		mip0 = mip2;
		mip1 = textureLod(gbufferNormal_Visibility_HitDistance, texCoords_, 3.0).b;
	}

	fractVal = fract(LOD_);
	// never take too much from mip1 up to 0.2 of the contribution is mip1.
	mixVal = min(fractVal, 0.2);

	return mix(mip0, mip1, mixVal);
}

float getPoissonShadowSample(vec2 centerCoord, const vec2 offset, const float referenceDepth, vec2 dZdXY, float kernelSize)
{
	vec2 sampleCoord = centerCoord + offset;

	// determine whether the current sample should be included or not
	// use the partial derivatives
	float depthThreshold = float(abs(offset.x)) * dZdXY.x + (float(abs(offset.y)) * dZdXY.y + fDepthDiscontinuityThreshold);
	bool depthComparison = getDepthDelta(sampleCoord, referenceDepth) > depthThreshold;

	highp float shadowSample = lodSample(sampleCoord, kernelSize);

	return shadowSample; 
}

highp float poissonPCFFilter(highp float averageBlockerDistance, highp float kernelSize, vec2 dZdXY, highp float referenceDepth, highp vec4 randomRotation, highp vec2 inUV)
{
	highp vec2 textureStep = vec2(1.0f) / vec2(textureSize(gbufferNormal_Visibility_HitDistance, 0));

	highp float shadowDensityRunningTotal = 0.0;

	for (int i = 0; i < numSamplingPositions; i++)
	{
		// rotate offset
		highp vec2 rotatedOffset = vec2(randomRotation.x * poissonDiscKernel[i].x - randomRotation.y * poissonDiscKernel[i].y,
			randomRotation.y * poissonDiscKernel[i].x + randomRotation.y * poissonDiscKernel[i].y);
		highp vec2 scaledOffset = rotatedOffset * textureStep * kernelSize;

		shadowDensityRunningTotal += getPoissonShadowSample(
			inUV, scaledOffset, referenceDepth, dZdXY, kernelSize);
	}

	return shadowDensityRunningTotal * inverseNumberSamplingPositions;
}

vec3 reconstructWorldPosition(vec2 texCoords, float z)
{
	// Take texture coordinate and remap to [-1.0, 1.0] range.
    vec2 screenPos = texCoords * 2.0 - 1.0;

    // // Create NDC position.
    vec4 ndcPos = vec4(screenPos, z, 1.0);

    // Transform back into world position.
    vec4 worldPos = mInvViewProjectionMatrix * ndcPos;

    // Undo projection.
    worldPos = worldPos / worldPos.w;

    return worldPos.xyz;
}

// Unpack the  2-component octahedral packing into a 3-component direction vector
vec3 unpackNormal(vec2 e)
{
    vec3 v = vec3(e, 1.0 - abs(e.x) - abs(e.y));
    if (v.z < 0.0)
        v.xy = (1.0 - abs(v.yx)) * (step(0.0, v.xy) * 2.0 - vec2(1.0));
    return normalize(v);
}

float filteredVisibility(highp vec3 vWorldPosition, highp vec3 vNormal, highp float fDepth, highp float NdotL)
{
	// get the eye space depth for the current fragment
	mediump float referenceDepth = getEyeSpaceDepth(fDepth);

	// compute partial derivatives for the depth
	// the partial derivatives give us the rate of change of the eye space depths
	// 		The current fragment can be thought of as a plane in space.
	//		The center sample tells us where that plane lies in camera space Z
	// 		We would also like to know how that plane is oriented with respect to the screen.
	// 		The partial derivatives basically give the rate of change of camera space Z as we move horizontally and vertically in screen space.
	// 		I'm calling them partial derivatives because the plane is essentially a function of 3 variables, which I'm differentiating to determine the slope of that function.
	// 		These values tell me how much I expect the camera space Z value to change as I move horizontally and vertically in screen space.
	// In summary the partial derivatives allows us to reject pixel which change more in Z space than I expect (indicating they're likely part of another surface).
	// And conversely to not reject pixels which change as much in Z space as expected i.e. we provide a larger threshold the furhter away from the current fragment we are checking
	mediump vec2 dZdXY = computeDepthPartials(inUV, referenceDepth);

	// Determine the distance from the current fragment to the light source
	float distanceToLight = length(vLightPosition.xyz - vWorldPosition);
	float inverseDistanceToLight = 1.0 / distanceToLight;

	vec4 randomRotation = poissonRotation[int(abs(vWorldPosition.xyz) * 999999.0) % NUMBER_OF_RANDOM_ROTATIONS];

	// get second mip map of shadows texture to capture only the penumbra region
	// this provides the basis for rejecting filtering across the majority of the screen
	mediump float centerSampleShadowMip3 = textureLod(gbufferNormal_Visibility_HitDistance, inUV, 3.0).b;
	mediump float centerSampleShadowMip2 = textureLod(gbufferNormal_Visibility_HitDistance, inUV, 2.0).b;

	// we only want to filter when the current fragment is facing towards the light source
	bool facingLight = NdotL > 0.0;

	// when the mip mapped value is between 0 and 1 then we are between the expanded transition range
	// The shadow filter only needs to be applied when this condition holds
	// include the second mip map condition to avoid certain areas where differences in the 3rd mip are 0
	bool doShadowFilter = dFdx(centerSampleShadowMip3) != 0.0 || dFdy(centerSampleShadowMip3) != 0.0 || (centerSampleShadowMip2 < 1.0 && centerSampleShadowMip2 > 0.0);

	float visibility = 0.0f;
	float kernelSize = 0.0;

	if (facingLight && doShadowFilter && fDepth < fDepthMax)
	{
		highp float distanceToOccluder = textureLod(gbufferNormal_Visibility_HitDistance, inUV, 2.0).a;

		// we might have multiple distances to the occluder
		// we need to divide by the number of shadow samples which correspond to the current fragment
		distanceToOccluder = distanceToOccluder / centerSampleShadowMip3;

		computePoissonKernelSize(distanceToOccluder, referenceDepth, inverseDistanceToLight, kernelSize);
		visibility = poissonPCFFilter(distanceToOccluder, kernelSize, dZdXY, referenceDepth, randomRotation, inUV);
	}
	else
		visibility = textureLod(gbufferNormal_Visibility_HitDistance, inUV, 0).b;

	return visibility;
}

void main()
{
	// Unpack the values stored in the G-Buffer
	vec4 vGbuffer0 = textureLod(gbufferAlbedo_Shininess, inUV, 0);
	vec4 vGbuffer1 = textureLod(gbufferNormal_Visibility_HitDistance, inUV, 0);
	float fDepth = textureLod(gbufferDepth, inUV, 0).r;

	// Output black for any fragments belonging to the background 
	if (abs(vGbuffer1.x) + abs(vGbuffer1.y) == 0.0f)
	{
		outFragColor = vec4(0.0);
		return;
	}

	highp vec3 vAlbedo = vGbuffer0.xyz;
	highp float fShininess = vGbuffer0.w * 255.0f;
	highp vec3 vWorldPosition = reconstructWorldPosition(inUV, fDepth);

	highp vec3 vDirectionToLight = vLightPosition.xyz - vWorldPosition;
	highp vec3 vNormalisedDirectionToLight = normalize(vDirectionToLight);

	highp vec3 vNormalisedNormal = unpackNormal(vGbuffer1.xy);
	
	highp float NdotL = dot(vNormalisedNormal, vNormalisedDirectionToLight);

	highp vec3 vDirectionToEye = normalize(vEyePosition.xyz - vWorldPosition);

	mediump vec3 R = normalize(reflect(-vDirectionToLight, vNormalisedNormal));
	highp float fSpecularity = pow(max(dot(R, vDirectionToEye), 0.0), fShininess);

	// Filter the noisy ray traced visibility 
    mediump float fVisibility = filteredVisibility(vWorldPosition, vNormalisedNormal, fDepth, NdotL);
	
	// Shading 
	mediump vec3 ambientLight = vAmbientColor.rgb * vAlbedo;
	
	/*
		nDotL > 0 will produce hard shadow edges where we want slightly smoother results
	 		instead we use smoothstep to create a smooth transition between 0 and 1 when nDotL lies
	 		between [0,...,0.1] and store in l.
	 	The random rotation value is then used in conjunction with l in another smoothstep.
	 		if l lies between [randomRotation.x,...,1.0] it will be interpolated between 0.0 and 1.0
	 		depending on how similar it is to randomRotation.x and 1.0
	*/
	mediump vec3 vDirectLight = NdotL * fVisibility * vLightColor.rgb;
	
	mediump vec3 diffuseLight = vAlbedo * vDirectLight;
	mediump vec3 specularLight = fSpecularity * vDirectLight * vAlbedo;

    // Composite outputs 
	highp vec3 color = ambientLight + diffuseLight + specularLight;

	// Reinhard tone mapping
    color = color / (1.0 + color);

    outFragColor = vec4(color, 1.0f);
}