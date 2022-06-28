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
#define SHADOW_MAP_SIZE vec2(ShadowParams.w)
#define PCF_FILTER_SIZE ShadowParams.y
#define POISSON_SAMPLE_COUNT ShadowParams.z

// For Poisson Disk PCF sampling
const vec2 kPoissonSamples[8] = vec2[](vec2(0.698538f, 0.847957f), vec2(0.122776f, 0.0558489f), vec2(0.0139775f, 0.727103f), vec2(0.727073f, 0.0938444f),
	vec2(0.337809f, 0.997192f), vec2(0.417554f, 0.4214f), vec2(0.964904f, 0.580401f), vec2(0.416364f, 0.0875271f));

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
	projCoords.xy = projCoords.xy * 0.5 + 0.5;

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
