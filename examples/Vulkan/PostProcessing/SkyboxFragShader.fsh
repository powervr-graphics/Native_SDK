#version 320 es

layout(set = 0, binding = 0) uniform mediump samplerCube skybox;

layout(set = 0, binding = 2) uniform BloomConfig
{
	mediump float luminosityThreshold;
};

layout(location = 0) in mediump vec3 rayDirection;

layout(location = 0) out mediump vec4 oColor;
layout(location = 1) out mediump float oFilter;

const mediump float HDR_SCALER = 100.0f;

mediump float luma(mediump vec3 color)
{
	return dot(vec3(0.2126, 0.7152, 0.0722), color);
}

void main()
{
	// Sample skybox cube map
	mediump vec4 skyboxTexture = texture(skybox, rayDirection);
	mediump float lightMapScaler = HDR_SCALER * skyboxTexture.a * skyboxTexture.a + 1.0;
	oColor.rgb = skyboxTexture.rgb * lightMapScaler;

	mediump float luminance = luma(oColor.rgb);
	oFilter = mix(0.0, luminance, (luminance - luminosityThreshold) > 0.0);
}