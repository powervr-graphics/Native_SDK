#version 310 es

layout(binding = 0) uniform mediump sampler2D sBaseTex;
layout(binding = 1) uniform mediump sampler2D sNormalMap;
layout(binding = 2) uniform mediump samplerCube irradianceMap;

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 T;
layout(location = 2) in highp vec3 B;
layout(location = 3) in highp vec3 N;

layout(location = 0) out mediump vec4 oColor;
layout(location = 1) out mediump float oFilter;

layout(location = 0) uniform mediump float exposure;

mediump float luma(mediump vec3 color)
{
	return max(dot(color, vec3(0.2126, 0.7152, 0.0722)), 0.0001);
}

void main()
{
	// read the per-pixel normal from the normal map and expand to [-1, 1]
	mediump vec3 normal = texture(sNormalMap, vTexCoord).rgb * 2.0 - 1.0;

	// Note that we pass through T, B and N separately rather than constructing the matrix in the vertex shader to work around
	// an issue we encountered with a desktop compiler.
	highp mat3 TBN_worldSpace = mat3(T, B, N);
	mediump vec3 worldSpaceNormal = normalize(TBN_worldSpace * normal);

	mediump vec3 texColor = texture(sBaseTex, vTexCoord).rgb;
	mediump vec3 directionalLight = texColor * texture(irradianceMap, worldSpaceNormal).rgb;

	oColor = vec4(directionalLight, 1.0);

	// Apply the exposure value
	oFilter = luma(exposure * oColor.rgb);
}
