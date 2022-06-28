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
