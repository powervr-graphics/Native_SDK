#version 430

// because es doesn't support rg32f
precision highp float;

// powervr rogue has a wavefront of 32
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in; // processing 8x8 area (2 texels per thread)

layout(set = 0, binding = 0) uniform sampler2D sDepth;
layout(set = 1, binding = 0, rg16f) uniform highp writeonly image2D oShadowMap;

layout(constant_id = 0) const uint kBlurRadius = 6;

layout(push_constant) uniform GAUSSIAN1D
{
	vec4 factors[4];
	layout(offset = 64) uvec2 blurSizeShadowMapSize;
}
_gaussian;

#define CACHE_TYPE vec2

#define KERNEL_WIDTH (8u + 2u * kBlurRadius)
#define SAMPLES_PER_WORKGROUP KERNEL_WIDTH* KERNEL_WIDTH

shared highp CACHE_TYPE _sharedStorage[SAMPLES_PER_WORKGROUP]; // first used as a cache for depth values, then as intermediate storage between the horizontal and vertical blurs
// indexed linearly: [(x - x0) + (y - y0) * sampleWidth] where (x0, y0) is the top right corner of the sample area (including edge values)

vec2 GetEVSMExponents(float positiveExponent, float negativeExponent)
{
	const float maxExponent = 5.54f;

	vec2 lightSpaceExponents = vec2(positiveExponent, negativeExponent);

	// Clamp to maximum range of fp32/fp16 to prevent overflow/underflow
	return min(lightSpaceExponents, maxExponent);
}

// Applies exponential warp to shadow map depth, input depth should be in [0, 1]
vec2 WarpDepth(float depth, vec2 exponents)
{
	// Rescale depth into [-1, 1]
	depth = 2.0f * depth - 1.0f;
	float pos = exp(exponents.x * depth);
	float neg = -exp(-exponents.y * depth);
	return vec2(pos, neg);
}

const float PositiveExponent = 40.0f;
const float NegativeExponent = 5.0f;

vec2 getMoment(vec2 texcoord)
{
	float depth = texture(sDepth, texcoord).x;
	vec2 value;

	float moment1 = depth;
	float moment2 = depth * depth;

	return vec2(moment1, moment2);
}

void main()
{
	uint sampleWidth = 8u + 2u * kBlurRadius;
	uint samplesPerWorkGroup = sampleWidth * sampleWidth; // = (8 + 2n)^2

	// load in relevant depth values to shared memory
	ivec2 sampleOffset = ivec2(gl_WorkGroupID.xy) * ivec2(8) - ivec2(kBlurRadius); // top right corner of sample area
	uint samplesPerThread = (samplesPerWorkGroup + 31u) / 32u; // = ceil(samplesPerWorkGroup / 32)

	for (uint r = 0u; r < samplesPerThread; r++)
	{
		uint sharedIndex = gl_LocalInvocationIndex + r * 32u;
		if (sharedIndex < samplesPerWorkGroup)
		{
			ivec2 globalCoord = ivec2(sharedIndex % sampleWidth, sharedIndex / sampleWidth) + sampleOffset;
			vec2 texcoord = vec2(float(globalCoord.x), float(globalCoord.y)) / float(_gaussian.blurSizeShadowMapSize.y - 1);
			_sharedStorage[sharedIndex] = getMoment(texcoord); // apply exponential warping function before storing (doing it here means the shadow map pass can be a depth only pass?)
		}
	}

	// horizontal pass
	uint blurWidth = kBlurRadius * 2u + 1u;
	uint depthIndex = gl_LocalInvocationID.x + gl_LocalInvocationID.y * sampleWidth;
	uint horizontalLoops = (5u + kBlurRadius) / 2u; // = ceil((64 + n * 16) / 32)

	memoryBarrierShared(); // try to put memory barriers as late as possible!

	for (uint h = 0u; h < horizontalLoops; h++)
	{
		if (kBlurRadius + depthIndex < samplesPerWorkGroup)
		{
			CACHE_TYPE M1M2 = CACHE_TYPE(0.0);

			for (uint x = 0u; x < blurWidth; x++)
			{
				CACHE_TYPE warpedDepth = _sharedStorage[x + depthIndex];
				M1M2 += warpedDepth * _gaussian.factors[x / 4u][x % 4u]; // accumulate x*p(x) and x*x*p(x)
			}
			_sharedStorage[kBlurRadius + depthIndex] = M1M2;
		}
		depthIndex += 4u * sampleWidth; // move down 4 rows to process next texel
	}

	// vertical pass texel 1
	depthIndex = kBlurRadius + gl_LocalInvocationID.x + gl_LocalInvocationID.y * sampleWidth;
	CACHE_TYPE M1M2 = CACHE_TYPE(0);

	memoryBarrierShared();

	for (uint y = 0u; y < blurWidth; y++)
	{
		CACHE_TYPE M1M2Value = _sharedStorage[depthIndex];
		M1M2 += M1M2Value * _gaussian.factors[y / 4u][y % 4u];
		depthIndex += sampleWidth;
	}

	ivec2 globalCoord = ivec2(gl_WorkGroupID.xy * uvec2(8) + gl_LocalInvocationID.xy);

	imageStore(oShadowMap, globalCoord, vec4(M1M2, 0.0f, 0.0f));

	// vertical pass texel 2
	depthIndex = kBlurRadius + gl_LocalInvocationID.x + (gl_LocalInvocationID.y + 4u) * sampleWidth;
	M1M2 = CACHE_TYPE(0);

	for (uint y = 0u; y < blurWidth; y++)
	{
		CACHE_TYPE M1M2Value = _sharedStorage[depthIndex];
		M1M2 += M1M2Value * _gaussian.factors[y / 4u][y % 4u];
		depthIndex += sampleWidth;
	}

	imageStore(oShadowMap, globalCoord + ivec2(0, 4), vec4(M1M2, 0.0f, 0.0f));
}