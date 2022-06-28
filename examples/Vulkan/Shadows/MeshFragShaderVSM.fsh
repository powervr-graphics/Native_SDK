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

	vec2 moments = texture(sShadowMap, projCoords.xy).rg;

	return ChebyshevUpperBound(moments, projCoords.z, VSM_BIAS, LIGHT_BLEEDING_REDUCTION);
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
