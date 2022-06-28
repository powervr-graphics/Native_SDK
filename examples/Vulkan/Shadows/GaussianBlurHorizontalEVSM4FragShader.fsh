#version 320 es

precision highp float;

layout(set = 0, binding = 0) uniform sampler2D sDepth;

layout (location = 0) in mediump vec2 vTexCoord;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform GAUSSIAN1D 
{ 
	vec4 factors[4];
	layout(offset = 64) uvec2 blurSizeShadowMapSize;
} _gaussian;

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

	vec2 exponents = GetEVSMExponents(PositiveExponent, NegativeExponent);
	vec2 vsmDepth = WarpDepth(depth, exponents);

	vec4 average = vec4(vsmDepth.xy, vsmDepth.xy * vsmDepth.xy);

	return average; 
}

void main()
{
	vec4 value = vec4(0.0); 
	vec2 texelIncrement = (vec2(1.0) / vec2(float(_gaussian.blurSizeShadowMapSize.y))) * vec2(1.0f, 0.0f); 

	for (int x = -1 * int(_gaussian.blurSizeShadowMapSize.x); x <= int(_gaussian.blurSizeShadowMapSize.x); x++) 
	{
		vec4 moments = getMoment(vTexCoord + texelIncrement * float(x));
		int gaussianIndex = x + int(_gaussian.blurSizeShadowMapSize.x);
		value += moments * _gaussian.factors[gaussianIndex / 4][gaussianIndex % 4];
	} 
	oColor = value;
}