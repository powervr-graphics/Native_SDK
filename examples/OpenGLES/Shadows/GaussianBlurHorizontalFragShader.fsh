#version 310 es

precision highp float;

uniform sampler2D sDepth;

in mediump vec2 vTexCoord;

out vec4 oColor;

uniform vec4 gaussianFactors[4];
uniform uvec2 blurSizeShadowMapSize;

vec2 GetEVSMExponents(float positiveExponent, float negativeExponent)
{
    const  float maxExponent = 5.54f;

    vec2 lightSpaceExponents = vec2(positiveExponent, negativeExponent);

    // Clamp to maximum range of fp32/fp16 to prevent overflow/underflow
    return min(lightSpaceExponents, maxExponent);
}

// Applies exponential warp to shadow map depth, input depth should be in [0, 1]
vec2 WarpDepth(float depth, vec2 exponents)
{
    // Rescale depth into [-1, 1]
    depth = 2.0f * depth - 1.0f;
    float pos =  exp( exponents.x * depth);
    float neg = -exp(-exponents.y * depth);
    return vec2(pos, neg);
}

const float PositiveExponent = 40.0f;
const float NegativeExponent = 5.0f;

vec4 getMoment(vec2 texcoord)
{
	float depth = texture(sDepth, texcoord).x;
	vec4 value;

#if defined(SHADOW_TYPE_VSM)
	float moment1 = depth;
	float moment2 = depth * depth;

	value = vec4(moment1, moment2, 0.0, 0.0);

#elif defined(SHADOW_TYPE_EVSM2) || defined(SHADOW_TYPE_EVSM4)

	vec2 exponents = GetEVSMExponents(PositiveExponent, NegativeExponent);
	vec2 vsmDepth = WarpDepth(depth, exponents);

	vec4 average = vec4(vsmDepth.xy, vsmDepth.xy * vsmDepth.xy);

    #if defined(SHADOW_TYPE_EVSM2)
        value = average.xzxz;
    #else
        value = average;
    #endif
#endif

	return value; 
}

void main()
{
	vec4 value = vec4(0.0); 
	vec2 texelIncrement = (vec2(1.0) / vec2(float(blurSizeShadowMapSize.y))) * vec2(1.0f, 0.0f); 

	for (int x = -1 * int(blurSizeShadowMapSize.x); x <= int(blurSizeShadowMapSize.x); x++) 
	{
		vec4 moments = getMoment(vTexCoord + texelIncrement * float(x));
		int gaussianIndex = x + int(blurSizeShadowMapSize.x);
		value += moments * gaussianFactors[gaussianIndex / 4][gaussianIndex % 4];
	} 
	oColor = value;
	//oColor = getMoment(vTexCoord);
}