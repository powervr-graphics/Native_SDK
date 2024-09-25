#version 320 es

layout(set = 0, binding = 0) uniform highp sampler2D sBaseTex;
layout(set = 0, binding = 1) uniform highp sampler2D sNormalMap;
layout(set = 0, binding = 2) uniform highp samplerCube irradianceMap;

layout(push_constant) uniform pushConstantsBlock{
	mediump float exposure;
};

layout(location = 0) in highp vec2 vTexCoord;
layout(location = 1) in highp vec3 worldPosition;
layout(location = 2) in highp mat3 TBN_worldSpace;

layout(location = 0) out highp vec4 oColor;
layout(location = 1) out highp float oFilter;

// Max value that can be stored in an FP16 render target
const highp float FP16Max = 65000.0;

// Scale value to prevent the luminance from overflowing
const highp float FP16Scale = 10.0;

highp float luma(highp vec3 color)
{
	return clamp(dot(color, vec3(0.2126, 0.7152, 0.0722)), 0.0001, FP16Max);
}

void main()
{
	// read the per-pixel normal from the normal map and expand to [-1, 1]
	highp vec3 normal = texture(sNormalMap, vTexCoord).rgb * 2.0 - 1.0;
	highp vec3 worldSpaceNormal = normalize(TBN_worldSpace * normal);

	highp vec3 texColor = texture(sBaseTex, vTexCoord).rgb;
	highp vec3 diffuseIrradiance = texColor * texture(irradianceMap, worldSpaceNormal).rgb;
	
	oColor = vec4(diffuseIrradiance, 1.0);

	// Apply the exposure value
	oFilter = luma(exposure * oColor.rgb) / FP16Scale;
}