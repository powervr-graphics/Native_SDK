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

layout(set = 2, binding = 0) uniform highp sampler2DShadow sShadowMap;

#define DEPTH_OFFSET_BIAS ShadowParams.x
#define VSM_BIAS ShadowParams.x
#define SHADOW_MAP_SIZE vec2(ShadowParams.w)
#define LIGHT_BLEEDING_REDUCTION ShadowParams.y

float sampleShadowMap(in vec2 base_uv, in float u, in float v, in vec2 shadowMapSizeInv, in float depth) 
{
    vec2 uv = base_uv + vec2(u, v) * shadowMapSizeInv;
    return texture(sShadowMap, vec3(uv, depth - DEPTH_OFFSET_BIAS));
}

float testShadow(vec4 lsPos)
{
	vec3 projCoords = lsPos.xyz / lsPos.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;

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
