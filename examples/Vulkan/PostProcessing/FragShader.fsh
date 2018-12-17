#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D sBaseTex;
layout(set = 0, binding = 1) uniform mediump sampler2D sNormalMap;
layout(set = 0, binding = 2) uniform mediump samplerCube irradianceMap;

layout(std140, set = 0, binding = 3) uniform SceneBuffer
{
	highp mat4 inverseViewProjectionMatrix;
	mediump vec3 eyePosition;
	mediump vec3 lightPosition;
};

layout(set = 0, binding = 5) uniform BloomConfig
{
	mediump float luminosityThreshold;
};

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 worldPosition;
layout(location = 2) in highp mat3 TBN_worldSpace;

layout(location = 0) out mediump vec4 oColor;
layout(location = 1) out mediump float oFilter;

mediump vec3 RGBMDecode(mediump vec4 rgbm) 
{
  return 6.0 * rgbm.rgb * rgbm.a;
}

mediump float luma(mediump vec3 color)
{
	return dot(vec3(0.2126, 0.7152, 0.0722), color);
}

void main()
{
	// read the per-pixel normal from the normal map and expand to [-1, 1]
	mediump vec3 normal = texture(sNormalMap, vTexCoord).rgb * 2.0 - 1.0;
	mediump vec3 worldSpaceNormal = normalize(TBN_worldSpace * normal);

	mediump vec3 texColor = texture(sBaseTex, vTexCoord).rgb;
    
	mediump vec3 diffIR = RGBMDecode(texture(irradianceMap, worldSpaceNormal)).rgb;

	mediump vec3 directionToLight = normalize(lightPosition - worldPosition);
	mediump float lightIntensity = max(0.0, dot(directionToLight, worldSpaceNormal));

	mediump vec3 directionalLight = texColor * lightIntensity * 0.7;
	mediump vec3 ambientLight = texColor * diffIR * 0.3;
	
	oColor = vec4(directionalLight + ambientLight, 1.0);

	mediump float luminance = luma(oColor.rgb);
	oFilter = mix(0.0, luminance, (luminance - luminosityThreshold) > 0.0);
}