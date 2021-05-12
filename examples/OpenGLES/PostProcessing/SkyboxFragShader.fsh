#version 310 es

layout(binding = 0) uniform mediump samplerCube skybox;

layout(location = 0) in mediump vec3 rayDirection;

layout(location = 0) out mediump vec4 oColor;
layout(location = 1) out mediump float oFilter;

layout(location = 0) uniform mediump float exposure;

mediump float luma(mediump vec3 color)
{
	return dot(vec3(0.2126, 0.7152, 0.0722), color);
}

void main()
{
	// Sample skybox cube map
	oColor = texture(skybox, rayDirection);

	// Apply the exposure value
	oFilter = luma(exposure * oColor.rgb);
}