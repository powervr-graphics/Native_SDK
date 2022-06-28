#version 310 es

precision highp float;

in  vec2 vTexCoord;
in  vec3 vWorldNormal;
in highp vec3 vWorldPos;
in highp vec4 vPosition;

out  vec4 oColor;

layout(std140, binding = 0) uniform GlobalUBO
{
	highp mat4 ViewProjMat;
	highp mat4 ProjMat;
	highp mat4 ViewMat;
	highp mat4 ShadowMat;
	highp vec4 LightDir;
	highp vec4 LightPosVS;
	highp vec4 LightDirVS;
};

uniform highp vec4 ShadowParams;

#if defined(SHADOW_TYPE_VSM) || defined(SHADOW_TYPE_EVSM2) || defined(SHADOW_TYPE_EVSM4)
uniform  sampler2D sShadowMap;
#else
uniform highp sampler2DShadow sShadowMap;
#endif

uniform  sampler2D sDiffuse;

#define DEPTH_OFFSET_BIAS ShadowParams.x
#define VSM_BIAS ShadowParams.x
#define SHADOW_MAP_SIZE vec2(ShadowParams.w)
#define LIGHT_BLEEDING_REDUCTION ShadowParams.y

#if defined(SHADOW_TYPE_PCF_GRID)

#define PCF_FILTER_SIZE ShadowParams.y

#elif defined(SHADOW_TYPE_PCF_POISSON_DISK)

#define PCF_FILTER_SIZE ShadowParams.y
#define POISSON_SAMPLE_COUNT ShadowParams.z

// For Poisson Disk PCF sampling
const vec2 kPoissonSamples[8] = vec2[](vec2(0.698538f, 0.847957f), vec2(0.122776f, 0.0558489f), vec2(0.0139775f, 0.727103f), vec2(0.727073f, 0.0938444f),
	vec2(0.337809f, 0.997192f), vec2(0.417554f, 0.4214f), vec2(0.964904f, 0.580401f), vec2(0.416364f, 0.0875271f));

#endif

#if defined(SHADOW_TYPE_HARD)

float testShadow(vec4 lsPos)
{
	 vec3 projCoords = lsPos.xyz / lsPos.w;
	projCoords = projCoords * 0.5 + 0.5;

	// Make sure everything outside of the shadow frustum is not shadowed.
	if (any(greaterThan(projCoords, vec3(1.0))) || any(lessThan(projCoords, vec3(0.0))))
		return 1.0;

	return texture(sShadowMap, vec3(projCoords.xy, projCoords.z - DEPTH_OFFSET_BIAS));
}

#elif defined(SHADOW_TYPE_PCF_POISSON_DISK)

#define PI 3.1415926535897932384626433832795
#define PI_2 (PI * 2.0)
#define RANDOMIZE_OFFSETS

float interleavedGradientNoise()
{
  vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
  return fract(magic.z * fract(dot(gl_FragCoord.xy, magic.xy)));
}

float testShadow(vec4 lsPos)
{
	vec3 projCoords = lsPos.xyz / lsPos.w;
	projCoords = projCoords * 0.5 + 0.5;

	// Make sure everything outside of the shadow frustum is not shadowed.
	if (any(greaterThan(projCoords, vec3(1.0))) || any(lessThan(projCoords, vec3(0.0))))
		return 1.0;

	vec2 texelSize = 1.0 / SHADOW_MAP_SIZE;

#if defined(RANDOMIZE_OFFSETS)
    // Get a value to randomly rotate the kernel by
    float theta = interleavedGradientNoise() * PI_2;
    float c = cos(theta);
    float s = sin(theta);
    mat2 randomRotationMatrix = mat2(vec2(c, -s), vec2(s, c));
#endif
    vec2 sampleScale = (0.5 * PCF_FILTER_SIZE) / SHADOW_MAP_SIZE;
    
    float sum = 0.0;

    for (int i = 0; i < int(POISSON_SAMPLE_COUNT); ++i)
    {
        #if defined(RANDOMIZE_OFFSETS)
            vec2 sampleOffset = randomRotationMatrix * (kPoissonSamples[i] * sampleScale);
        #else
            vec2 sampleOffset = kPoissonSamples[i] * sampleScale;
        #endif
    
        vec2 samplePos = projCoords.xy + sampleOffset;

        sum += texture(sShadowMap, vec3(samplePos, projCoords.z - DEPTH_OFFSET_BIAS));
    }
    
    return sum / POISSON_SAMPLE_COUNT;
}

#elif defined(SHADOW_TYPE_PCF_OPTIMISED_2x2) || defined(SHADOW_TYPE_PCF_OPTIMISED_3x3) || defined(SHADOW_TYPE_PCF_OPTIMISED_5x5) || defined(SHADOW_TYPE_PCF_OPTIMISED_7x7) || defined(SHADOW_TYPE_PCF_OPTIMISED_9x9)

float sampleShadowMap(in vec2 base_uv, in float u, in float v, in vec2 shadowMapSizeInv, in float depth) 
{
    vec2 uv = base_uv + vec2(u, v) * shadowMapSizeInv;
    return texture(sShadowMap, vec3(uv, depth - DEPTH_OFFSET_BIAS));
}

float testShadow(vec4 lsPos)
{
	vec3 projCoords = lsPos.xyz / lsPos.w;
	projCoords = projCoords * 0.5 + 0.5;

	// Make sure everything outside of the shadow frustum is not shadowed.
	if (any(greaterThan(projCoords, vec3(1.0))) || any(lessThan(projCoords, vec3(0.0))))
		return 1.0;

    vec2 uv = projCoords.xy * SHADOW_MAP_SIZE; // 1 unit - 1 texel

    vec2 shadowMapSizeInv = 1.0 / SHADOW_MAP_SIZE;

    vec2 base_uv;
    base_uv.x = floor(uv.x + 0.5);
    base_uv.y = floor(uv.y + 0.5);

    float s = (uv.x + 0.5 - base_uv.x);
    float t = (uv.y + 0.5 - base_uv.y);

    base_uv -= vec2(0.5, 0.5);
    base_uv *= shadowMapSizeInv;

    float sum = 0.0;

#if defined(SHADOW_TYPE_PCF_OPTIMISED_2x2)
    return texture(sShadowMap, vec3(projCoords.xy, projCoords.z - DEPTH_OFFSET_BIAS)); 
#elif defined(SHADOW_TYPE_PCF_OPTIMISED_3x3)

    float uw0 = (3.0 - 2.0 * s);
    float uw1 = (1.0 + 2.0 * s);

    float u0 = (2.0 - s) / uw0 - 1.0;
    float u1 = s / uw1 + 1.0;

    float vw0 = (3.0 - 2.0 * t);
    float vw1 = (1.0 + 2.0 * t);

    float v0 = (2.0 - t) / vw0 - 1.0;
    float v1 = t / vw1 + 1.0;

    sum += uw0 * vw0 * sampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, projCoords.z);
    sum += uw1 * vw0 * sampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, projCoords.z);
    sum += uw0 * vw1 * sampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, projCoords.z);
    sum += uw1 * vw1 * sampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, projCoords.z);

    return sum * 1.0 / 16.0;

#elif defined(SHADOW_TYPE_PCF_OPTIMISED_5x5)

    float uw0 = (4.0 - 3.0 * s);
    float uw1 = 7.0;
    float uw2 = (1.0 + 3.0 * s);

    float u0 = (3.0 - 2.0 * s) / uw0 - 2.0;
    float u1 = (3.0 + s) / uw1;
    float u2 = s / uw2 + 2.0;

    float vw0 = (4.0 - 3.0 * t);
    float vw1 = 7.0;
    float vw2 = (1.0 + 3.0 * t);

    float v0 = (3.0 - 2.0 * t) / vw0 - 2.0;
    float v1 = (3.0 + t) / vw1;
    float v2 = t / vw2 + 2.0;

    sum += uw0 * vw0 * sampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, projCoords.z);
    sum += uw1 * vw0 * sampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, projCoords.z);
    sum += uw2 * vw0 * sampleShadowMap(base_uv, u2, v0, shadowMapSizeInv, projCoords.z);
                                                                          
    sum += uw0 * vw1 * sampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, projCoords.z);
    sum += uw1 * vw1 * sampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, projCoords.z);
    sum += uw2 * vw1 * sampleShadowMap(base_uv, u2, v1, shadowMapSizeInv, projCoords.z);
                                                                          
    sum += uw0 * vw2 * sampleShadowMap(base_uv, u0, v2, shadowMapSizeInv, projCoords.z);
    sum += uw1 * vw2 * sampleShadowMap(base_uv, u1, v2, shadowMapSizeInv, projCoords.z);
    sum += uw2 * vw2 * sampleShadowMap(base_uv, u2, v2, shadowMapSizeInv, projCoords.z);

    return sum * 1.0 / 144.0;

#elif defined(SHADOW_TYPE_PCF_OPTIMISED_7x7)

    float uw0 = (5.0 * s - 6.0);
    float uw1 = (11.0 * s - 28.0);
    float uw2 = -(11.0 * s + 17.0);
    float uw3 = -(5.0 * s + 1.0);

    float u0 = (4.0 * s - 5.0) / uw0 - 3.0;
    float u1 = (4.0 * s - 16.0) / uw1 - 1.0;
    float u2 = -(7.0 * s + 5.0) / uw2 + 1.0;
    float u3 = -s / uw3 + 3.0;

    float vw0 = (5.0 * t - 6.0);
    float vw1 = (11.0 * t - 28.0);
    float vw2 = -(11.0 * t + 17.0);
    float vw3 = -(5.0 * t + 1.0);

    float v0 = (4.0 * t - 5.0) / vw0 - 3.0;
    float v1 = (4.0 * t - 16.0) / vw1 - 1.0;
    float v2 = -(7.0 * t + 5.0) / vw2 + 1.0;
    float v3 = -t / vw3 + 3.0;

    sum += uw0 * vw0 * sampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, projCoords.z);
    sum += uw1 * vw0 * sampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, projCoords.z);
    sum += uw2 * vw0 * sampleShadowMap(base_uv, u2, v0, shadowMapSizeInv, projCoords.z);
    sum += uw3 * vw0 * sampleShadowMap(base_uv, u3, v0, shadowMapSizeInv, projCoords.z);
                                                                          
    sum += uw0 * vw1 * sampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, projCoords.z);
    sum += uw1 * vw1 * sampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, projCoords.z);
    sum += uw2 * vw1 * sampleShadowMap(base_uv, u2, v1, shadowMapSizeInv, projCoords.z);
    sum += uw3 * vw1 * sampleShadowMap(base_uv, u3, v1, shadowMapSizeInv, projCoords.z);
                                                                         
    sum += uw0 * vw2 * sampleShadowMap(base_uv, u0, v2, shadowMapSizeInv, projCoords.z);
    sum += uw1 * vw2 * sampleShadowMap(base_uv, u1, v2, shadowMapSizeInv, projCoords.z);
    sum += uw2 * vw2 * sampleShadowMap(base_uv, u2, v2, shadowMapSizeInv, projCoords.z);
    sum += uw3 * vw2 * sampleShadowMap(base_uv, u3, v2, shadowMapSizeInv, projCoords.z);
                                                                          
    sum += uw0 * vw3 * sampleShadowMap(base_uv, u0, v3, shadowMapSizeInv, projCoords.z);
    sum += uw1 * vw3 * sampleShadowMap(base_uv, u1, v3, shadowMapSizeInv, projCoords.z);
    sum += uw2 * vw3 * sampleShadowMap(base_uv, u2, v3, shadowMapSizeInv, projCoords.z);
    sum += uw3 * vw3 * sampleShadowMap(base_uv, u3, v3, shadowMapSizeInv, projCoords.z);

    return sum * 1.0 / 2704.0;

#endif
}

#elif defined(SHADOW_TYPE_EVSM2) || defined(SHADOW_TYPE_EVSM4) || defined(SHADOW_TYPE_VSM)

	const highp float PositiveExponent = 40.0f;
	const highp float NegativeExponent = 5.0f;

	vec2 GetEVSMExponents(highp float positiveExponent, highp float negativeExponent)
	{
		const highp float maxExponent = 5.54f;

		highp vec2 lightSpaceExponents = vec2(positiveExponent, negativeExponent);

		// Clamp to maximum range of fp32/fp16 to prevent overflow/underflow
		return min(lightSpaceExponents, maxExponent);
	}

	// Applies exponential warp to shadow map depth, input depth should be in [0, 1]
	vec2 WarpDepth(highp float depth, highp vec2 exponents)
	{
		// Rescale depth into [-1, 1]
		depth = 2.0f * depth - 1.0f;
		highp float pos =  exp( exponents.x * depth);
		highp float neg = -exp(-exponents.y * depth);
		return vec2(pos, neg);
	}

	float Linstep(highp float a, highp float b, highp float v)
	{
		return clamp((v - a) / (b - a), 0.0, 1.0);
	}

	// Reduces VSM light bleedning
	highp float ReduceLightBleeding(highp float pMax, highp float amount)
	{
	  // Remove the [0, amount] tail and linearly rescale (amount, 1].
	   return Linstep(amount, 1.0, pMax);
	}

	float ChebyshevUpperBound(highp vec2 moments, highp float mean, highp float minVariance, highp float lightBleedingReduction)
	{
		// Compute variance
		highp float variance = moments.y - (moments.x * moments.x);
		variance = max(variance, minVariance);

		// Compute probabilistic upper bound
		highp float d = mean - moments.x;
		highp float pMax = variance / (variance + (d * d));

		pMax = ReduceLightBleeding(pMax, lightBleedingReduction);

		// One-tailed Chebyshev
		return (mean <= moments.x ? 1.0f : pMax);
	}

	#if defined(SHADOW_TYPE_VSM)

		float testShadow(vec4 ls_pos)
		{
			vec3 projCoords = ls_pos.xyz / ls_pos.w;
			projCoords = projCoords * 0.5 + 0.5;

			// Make sure everything outside of the shadow frustum is not shadowed.
			if (any(greaterThan(projCoords, vec3(1.0))) || any(lessThan(projCoords, vec3(0.0))))
				return 1.0;

			vec2 moments = texture(sShadowMap, projCoords.xy).rg;
	
			return ChebyshevUpperBound(moments, projCoords.z, VSM_BIAS, LIGHT_BLEEDING_REDUCTION);
		}

	#elif defined(SHADOW_TYPE_EVSM2)

		float testShadow(vec4 ls_pos)
		{
			vec3 projCoords = ls_pos.xyz / ls_pos.w;
			projCoords = projCoords * 0.5 + 0.5;

			// Make sure everything outside of the shadow frustum is not shadowed.
			if (any(greaterThan(projCoords, vec3(1.0))) || any(lessThan(projCoords, vec3(0.0))))
				return 1.0;

			vec2 exponents = GetEVSMExponents(PositiveExponent, NegativeExponent);
			vec2 warpedDepth = WarpDepth(projCoords.z, exponents);

			vec4 occluder = texture(sShadowMap, projCoords.xy);

			// Derivative of warping at depth
			vec2 depthScale = VSM_BIAS * exponents * warpedDepth;
			vec2 minVariance = depthScale * depthScale;

			// Positive only
			return ChebyshevUpperBound(occluder.xy, warpedDepth.x, minVariance.x, LIGHT_BLEEDING_REDUCTION);
		}

	#elif defined(SHADOW_TYPE_EVSM4)

		float testShadow(vec4 ls_pos)
		{
			vec3 projCoords = ls_pos.xyz / ls_pos.w;
			projCoords = projCoords * 0.5 + 0.5;

			// Make sure everything outside of the shadow frustum is not shadowed.
			if (any(greaterThan(projCoords, vec3(1.0))) || any(lessThan(projCoords, vec3(0.0))))
				return 1.0;

			vec2 exponents = GetEVSMExponents(PositiveExponent, NegativeExponent);
			vec2 warpedDepth = WarpDepth(projCoords.z, exponents);

			vec4 occluder = texture(sShadowMap, projCoords.xy);

			// Derivative of warping at depth
			vec2 depthScale = VSM_BIAS * exponents * warpedDepth;
			vec2 minVariance = depthScale * depthScale;

			float posContrib = ChebyshevUpperBound(occluder.xz, warpedDepth.x, minVariance.x, LIGHT_BLEEDING_REDUCTION);
			float negContrib = ChebyshevUpperBound(occluder.yw, warpedDepth.y, minVariance.y, LIGHT_BLEEDING_REDUCTION);
			return min(posContrib, negContrib);
		}

	#endif

#else

 float testShadow( vec4 lsPos)
{
	return 1.0;
}

#endif

void main()
{
	 vec4 ws = vec4(vWorldPos, 1.0);
	 vec4 lightSpaceFragPos = ShadowMat * ws;
	 float shadow = testShadow(lightSpaceFragPos);

	 float ambient = 0.05;
	 vec3 diffuse = texture(sDiffuse, vTexCoord).rgb;
	 vec3 color = shadow * diffuse * clamp(dot(normalize(vWorldNormal), normalize(-LightDir.xyz)), 0.0, 1.0) + diffuse * ambient;
	oColor = vec4(color, 1.0);
}
