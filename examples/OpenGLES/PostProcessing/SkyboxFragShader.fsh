#version 310 es
#extension GL_IMG_framebuffer_downsample : enable

layout(binding = 0) uniform mediump samplerCube skybox;

layout(location = 0) in mediump vec3 rayDirection;

layout(location = 0) out mediump vec4 oOffScreen;
#ifdef GL_IMG_framebuffer_downsample
layout(location = 1) out mediump float oDownsampledFilter;
#else
layout(location = 1) out mediump float oFilter;
#endif

const mediump float HDR_SCALER = 100.0f;

layout(binding = 0) uniform BloomConfig
{
	mediump float luminosityThreshold;
};

mediump float luma(mediump vec3 color)
{
	return dot(vec3(0.2126, 0.7152, 0.0722), color);
}

void main()
{
	// Sample skybox cube map
	mediump vec4 skyboxTexture = texture(skybox, rayDirection);
	mediump float lightMapScaler = HDR_SCALER * skyboxTexture.a * skyboxTexture.a + 1.0;
	oOffScreen = vec4(skyboxTexture.rgb * lightMapScaler, 1.0);

	mediump float luminance = luma(oOffScreen.rgb);
	mediump float thresholdedLuminance = mix(0.0, luminance, (luminance - luminosityThreshold) > 0.0);
#ifdef GL_IMG_framebuffer_downsample
	oDownsampledFilter = thresholdedLuminance;
#else
	oFilter = thresholdedLuminance;
#endif
}