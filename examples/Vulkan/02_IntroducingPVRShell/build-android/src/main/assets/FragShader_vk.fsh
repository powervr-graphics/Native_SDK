#version 450

//// Shader Resources ////
layout(set = 0, binding = 0) uniform sampler2D triangleTexture;

//// Vertex Inputs ////
layout(location = 0) in vec2 UV;

//// Fragment Outputs ////
layout(location = 0) out vec4 fragColor;

void main()
{
	// Sample the checkerboard texture and write to the framebuffer attachment
	fragColor = texture(triangleTexture, UV);
}