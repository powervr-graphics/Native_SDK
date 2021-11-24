#version 320 es

layout(set = 0, binding = 0) uniform mediump samplerCube skybox;

layout(push_constant) uniform pushConstantsBlock{
	mediump float exposure;
};

layout(location = 0) in mediump vec3 rayDirection;

layout(location = 0) out mediump vec4 oColor;
layout(location = 1) out mediump float oFilter;

// Max value that can be stored in an FP16 render target
const mediump float FP16Max = 65000.0;

// Scale value to prevent the luminance from overflowing
const mediump float FP16Scale = 10.0;

mediump float luma(mediump vec3 color)
{
	return clamp(dot(color, vec3(0.2126, 0.7152, 0.0722)), 0.0001, FP16Max);
}

void main()
{
	// Sample skybox cube map
	oColor = texture(skybox, rayDirection);

	// Apply the exposure value
	oFilter = luma(exposure * oColor.rgb) / FP16Scale;
}