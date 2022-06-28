#version 450

#define PI 3.1415926535897932384626433832795
#define PI_2 (PI * 2.0)
#define FILTER_RADIUS 16.0f
#define POISSON_SAMPLE_COUNT 8
#define PHI_NORMAL 32.0f
#define EPSILON 0.0000001f

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

layout(set = 2, binding = 0) uniform sampler2D currentShadowMask;

layout(set = 3, binding = 0, r16f) uniform image2D outputImage;

const vec2 kPoissonSamples[8] = vec2[](vec2(0.698538f, 0.847957f), vec2(0.122776f, 0.0558489f), vec2(0.0139775f, 0.727103f), vec2(0.727073f, 0.0938444f),
	vec2(0.337809f, 0.997192f), vec2(0.417554f, 0.4214f), vec2(0.964904f, 0.580401f), vec2(0.416364f, 0.0875271f));

float normalEdgeStoppingWeight(vec3 centerNormal, vec3 sampleNormal, float power) { return pow(clamp(dot(centerNormal, sampleNormal), 0.0f, 1.0f), power); }

float interleavedGradientNoise()
{
	vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
	return fract(magic.z * fract(dot(vec2(gl_GlobalInvocationID.xy), magic.xy)));
}

float poissonDiscBlur(vec2 texCoord)
{
	const vec2 texelSize = vec2(1.0f) / vec2(textureSize(currentShadowMask, 0));

	// Get a value to randomly rotate the kernel by
	float theta = interleavedGradientNoise() * PI_2;
	float c = cos(theta);
	float s = sin(theta);
	mat2 randomRotationMatrix = mat2(vec2(c, -s), vec2(s, c));

	vec2 sampleScale = (0.5f * FILTER_RADIUS) * texelSize;

	float centerVisibility = textureLod(currentShadowMask, texCoord, 0.0f).r;
	vec3 centerNormal = textureLod(gbufferNormal_Reflectivity, texCoord, 0.0f).rgb;

	float sum = centerVisibility;
	float sum_w = 1.0f;

	for (int i = 0; i < POISSON_SAMPLE_COUNT; ++i)
	{
		vec2 sampleOffset = randomRotationMatrix * (kPoissonSamples[i] * sampleScale);

		vec2 samplePos = texCoord + sampleOffset;

		float sampleDepth = textureLod(gbufferDepth, samplePos, 0).r;

		// If the sample belongs to the background, skip it!
		if (sampleDepth == 0.0f) continue;

		float sampleVisibility = textureLod(currentShadowMask, samplePos, 0.0f).r;
		vec3 sampleNormal = textureLod(gbufferNormal_Reflectivity, samplePos, 0.0f).rgb;

		float w = normalEdgeStoppingWeight(centerNormal, sampleNormal, PHI_NORMAL);
		sum += sampleVisibility * w;
		sum_w += w;
	}

	return sum / sum_w;
}

void main()
{
	const ivec2 size = textureSize(currentShadowMask, 0);
	const ivec2 currentCoord = ivec2(gl_GlobalInvocationID.xy);
	const vec2 texCoord = (vec2(currentCoord) + vec2(0.5f)) / vec2(size);

	float depth = texelFetch(gbufferDepth, currentCoord, 0).r;

	// Skip background pixels
	if (depth != 0.0f)
	{
		// We use a 8-tap poisson disc blur with an Interleaved Gradient Noise rotation applied to each sample.
		float currentVisibility = poissonDiscBlur(texCoord);

		imageStore(outputImage, currentCoord, vec4(currentVisibility));
	}
	else
		imageStore(outputImage, currentCoord, vec4(0.0f));
}
