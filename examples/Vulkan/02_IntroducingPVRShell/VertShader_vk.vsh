#version 450

//// Vertex Shader inputs
layout(location = 0) in highp vec4 vertex;
layout(location = 1) in highp vec2 uv;

//// Shader Resources ////
layout(std140, set = 1, binding = 0) uniform modelViewProjectionBuffer
{
	mat4 modelViewProjectionMatrix;
};

//// Per Vertex Outputs ////
layout(location = 0) out vec2 UV_OUT;

void main()
{
	// Simply calculate the ndc position for the current vertex using the model view projection matrix
	gl_Position = modelViewProjectionMatrix * vertex;
	UV_OUT = uv;
}