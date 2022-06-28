#version 310 es

layout(binding = 0) uniform mediump samplerCube skybox;

layout(location = 0) in mediump vec3 rayDirection;

layout(location = 0) out mediump vec4 oColor;
layout(location = 1) out mediump float oFilter;

layout(location = 0) uniform mediump float exposure;

mediump float luma(mediump vec3 color)
{
	return clamp(dot(vec3(0.2126, 0.7152, 0.0722), color), 0.0, 65504.0);
}

void main()
{
	// Sample skybox cube map
	oColor = texture(skybox, rayDirection);

	// Apply the exposure value
	oFilter = luma(clamp(exposure * oColor.rgb, 0.0, 65504.0));
}