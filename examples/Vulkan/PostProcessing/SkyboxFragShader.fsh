#version 320 es

layout(set = 0, binding = 0) uniform mediump samplerCube skybox;

layout(push_constant) uniform pushConstantsBlock{
	mediump float exposure;
};

layout(location = 0) in mediump vec3 rayDirection;

layout(location = 0) out mediump vec4 oColor;
layout(location = 1) out mediump float oFilter;

mediump float luma(mediump vec3 color)
{
	return max(dot(color, vec3(0.2126, 0.7152, 0.0722)), 0.0001);
}

void main()
{
	// Sample skybox cube map
	oColor = texture(skybox, rayDirection);

	// Apply the exposure value
	oFilter = luma(exposure * oColor.rgb);
}