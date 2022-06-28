#version 320 es

precision highp float;

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 vWorldNormal;
layout(location = 2) in highp vec3 vWorldPos;
layout(location = 3) in highp vec4 vPosition;

layout(location = 0) out vec4 oColor;

layout(std140, set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 ViewProjMat;
	highp mat4 ProjMat;
	highp mat4 ViewMat;
	highp mat4 ShadowMat;
	highp vec4 LightDir;
	highp vec4 LightPosVS;
	highp vec4 LightDirVS;
};

layout(push_constant) uniform PushConsts { layout(offset = 64) vec4 ShadowParams; };

layout(set = 1, binding = 0) uniform  sampler2D sDiffuse;

layout(set = 2, binding = 0) uniform  sampler2D sShadowMap;

#define DEPTH_OFFSET_BIAS ShadowParams.x
#define VSM_BIAS ShadowParams.x
#define SHADOW_MAP_SIZE vec2(ShadowParams.w)
#define LIGHT_BLEEDING_REDUCTION ShadowParams.y

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

float testShadow(vec4 ls_pos)
{
	vec3 projCoords = ls_pos.xyz / ls_pos.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;

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
