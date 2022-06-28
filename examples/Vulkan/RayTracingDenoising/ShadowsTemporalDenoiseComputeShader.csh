#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mPrevViewProjMatrix;
	highp mat4 mViewProjInverseMatrix;
	highp mat4 mPrevViewProjInverseMatrix;
	highp vec4 vAmbientLightColor;
	highp vec4 vCameraPosition;
};

layout(set = 1, binding = 0) uniform sampler2D gbufferAlbedo_Metallic;
layout(set = 1, binding = 1) uniform sampler2D gbufferNormal_Reflectivity;
layout(set = 1, binding = 2) uniform sampler2D gbufferMotionVector_F90;
layout(set = 1, binding = 3) uniform sampler2D gbufferF0_Roughness;
layout(set = 1, binding = 4) uniform sampler2D gbufferDepth;

layout(set = 2, binding = 0) uniform sampler2D historyGbufferAlbedo_Metallic;
layout(set = 2, binding = 1) uniform sampler2D historyGbufferNormal_Reflectivity;
layout(set = 2, binding = 2) uniform sampler2D historyGbufferMotionVector_F90;
layout(set = 2, binding = 3) uniform sampler2D historyGbufferF0_Roughness;
layout(set = 2, binding = 4) uniform sampler2D historyGbufferDepth;

layout(set = 3, binding = 0, r16f) uniform image2D outputImage;
layout(set = 3, binding = 1) uniform sampler2D currentShadowMask;
layout(set = 3, binding = 2) uniform sampler2D historyShadowMask;
layout(set = 3, binding = 3) uniform sampler2D currentShadowMaskDownsampled;

#define NORMAL_DISTANCE 0.1f
#define PLANE_DISTANCE 1.0f
#define NEIGHBORHOOD_RADIUS 1

bool planeDistanceDisocclusionCheck(vec3 currentPos, vec3 historyPos, vec3 currentNormal)
{
	vec3 toCurrent = currentPos - historyPos;
	float distToPlane = abs(dot(toCurrent, currentNormal));

	return distToPlane > PLANE_DISTANCE;
}

bool outOfFrameDisocclusionCheck(ivec2 coord, ivec2 imageDim)
{
	// check whether reprojected pixel is inside of the screen
	if (any(lessThan(coord, ivec2(0, 0))) || any(greaterThan(coord, imageDim - ivec2(1, 1))))
		return true;
	else
		return false;
}

bool normalsDisocclusionCheck(vec3 currentNormal, vec3 historyNormal)
{
	if (pow(abs(dot(currentNormal, historyNormal)), 2) > NORMAL_DISTANCE)
		return false;
	else
		return true;
}

bool isReprojectionValid(ivec2 coord, vec3 currentPos, vec3 historyPos, vec3 currentNormal, vec3 historyNormal, ivec2 imageDim)
{
	// check if history sample is on the same plane
	if (planeDistanceDisocclusionCheck(currentPos, historyPos, currentNormal)) return false;

	// check if the history sample is within the frame
	if (outOfFrameDisocclusionCheck(coord, imageDim)) return false;

	// check normals for compatibility
	if (normalsDisocclusionCheck(currentNormal, historyNormal)) return false;

	return true;
}

// Reconstruct world position from the depth buffer value
vec3 worldPositionFromDepth(vec2 texCoords, float ndcDepth, mat4 viewProjInverse)
{
	// Take texture coordinate and remap to [-1.0, 1.0] range.
	vec2 screenPos = texCoords * 2.0 - 1.0;

	// Create NDC position.
	vec4 ndcPos = vec4(screenPos, ndcDepth, 1.0);

	// Transform back into world position.
	vec4 worldPos = viewProjInverse * ndcPos;

	// Undo projection.
	return worldPos.xyz / worldPos.w;
}

bool reproject(ivec2 coord, out float historyVisibility)
{
	const vec2 imageDim = vec2(textureSize(historyShadowMask, 0));
	const vec2 pixelCenter = vec2(coord) + vec2(0.5f);
	const vec2 texCoord = pixelCenter / vec2(imageDim);

	const vec3 currentPos = worldPositionFromDepth(texCoord, texelFetch(gbufferDepth, coord, 0).r, mViewProjInverseMatrix);
	const vec2 currentVelocity = texelFetch(gbufferMotionVector_F90, coord, 0).rg;
	const vec3 currentNormal = texelFetch(gbufferNormal_Reflectivity, coord, 0).rgb;

	// +0.5 to account for texel center offset
	const ivec2 historyCoord = ivec2(vec2(coord) + currentVelocity.xy * imageDim + vec2(0.5f));
	const vec2 historyCoordFloor = floor(coord.xy) + currentVelocity.xy * imageDim;
	const vec2 historyTexCoord = texCoord + currentVelocity.xy;

	const vec3 historyPos = worldPositionFromDepth(historyTexCoord, texelFetch(historyGbufferDepth, historyCoord, 0).r, mPrevViewProjInverseMatrix);
	const vec3 historyNormal = texelFetch(historyGbufferNormal_Reflectivity, historyCoord, 0).rgb;

	if (isReprojectionValid(coord, currentPos, historyPos, currentNormal, historyNormal, ivec2(imageDim)))
	{
		historyVisibility = texelFetch(historyShadowMask, historyCoord, 0).r;
		return true;
	}
	else
		return false;
}

float neighborhoodMean(vec2 texCoord, int radius)
{
	const int mipLevel = 1;
	const ivec2 size = textureSize(currentShadowMaskDownsampled, mipLevel);
	const vec2 invSize = vec2(1.0f / size.x, 1.0f / size.y);

	float sum = 0.0f;
	float kernelSize = float((radius * 2) + 1);
	kernelSize = kernelSize * kernelSize;

	for (int x = -radius; x <= radius; x++)
	{
		for (int y = -radius; y <= radius; y++)
		{
			vec2 sampleCoord = texCoord + (vec2(float(x), float(y)) * invSize);
			sum += textureLod(currentShadowMaskDownsampled, sampleCoord, float(mipLevel)).r;
		}
	}

	return sum / float(kernelSize);
}

void main()
{
	const ivec2 currentCoord = ivec2(gl_GlobalInvocationID.xy);

	float depth = texelFetch(gbufferDepth, currentCoord, 0).r;

	// Skip background pixels
	if (depth != 0.0f)
	{
		float outputVisibility = texelFetch(currentShadowMask, currentCoord, 0).r;
		float historyVisibility = 0.0f;

		if (reproject(currentCoord, historyVisibility))
		{
			const vec2 imageDim = vec2(textureSize(historyShadowMask, 0));
			const vec2 pixelCenter = vec2(currentCoord);
			const vec2 texCoord = pixelCenter / vec2(imageDim);

			// Compute the neighborhood mean using a higher mip level of the downsample shadow mask
			// in order to reduce the neighborhood radius (11x11 -> 3x3).
			float mean = neighborhoodMean(texCoord, NEIGHBORHOOD_RADIUS);
			float spatial_variance = mean;
			spatial_variance = max(spatial_variance - mean * mean, 0.0f);

			// Compute the clamping bounding box
			const float std_deviation = sqrt(spatial_variance);
			const float nmin = mean - 0.5f * std_deviation;
			const float nmax = mean + 0.5f * std_deviation;

			historyVisibility = clamp(historyVisibility, nmin, nmax);

			outputVisibility = mix(historyVisibility, outputVisibility, 0.05f);
		}

		imageStore(outputImage, currentCoord, vec4(outputVisibility));
	}
	else
		imageStore(outputImage, currentCoord, vec4(0.0f));
}
