#version 310 es
#extension GL_IMG_framebuffer_downsample : enable

layout(binding = 0) uniform mediump sampler2D sBaseTex;
layout(binding = 1) uniform mediump sampler2D sNormalMap;
layout(binding = 2) uniform mediump samplerCube irradianceMap;

layout(std140, binding = 1) uniform SceneBuffer
{
	highp mat4 inverseViewProjectionMatrix;
	mediump vec3 eyePosition;
	mediump vec3 lightPosition;
};
		
layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 worldPosition;
layout(location = 2) in highp mat3 TBN_worldSpace;

layout(location = 0) out mediump vec4 oOffScreen;
#ifdef GL_IMG_framebuffer_downsample
layout(location = 1) out mediump float oDownsampledFilter;
#else
layout(location = 1) out mediump float oFilter;
#endif

mediump vec3 RGBMDecode(mediump vec4 rgbm) 
{
  return 6.0 * rgbm.rgb * rgbm.a;
}

layout(std140, binding = 2) uniform BloomConfig
{
	mediump float luminosityThreshold;
};

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
	
	oOffScreen = vec4(directionalLight + ambientLight, 1.0);

	mediump float luminance = luma(oOffScreen.rgb);
	mediump float thresholdedLuminance = mix(0.0, luminance, (luminance - luminosityThreshold) > 0.0);
#ifdef GL_IMG_framebuffer_downsample
	oDownsampledFilter = thresholdedLuminance;
#else
	oFilter = thresholdedLuminance;
#endif
}