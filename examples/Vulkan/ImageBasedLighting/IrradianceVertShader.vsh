#version 320 es

layout(location = 0) in highp vec3 inVertex;

layout(location = 0) out mediump vec3 outUVW;

layout(std140, set = 0, binding = 1) uniform Dynamic
{
	highp mat4 rotate;
};

void main()
{
	// Set position
	outUVW = (rotate * vec4(inVertex, 1.0)).xyz;

	// Calculate ray direction
	gl_Position = vec4(inVertex, 1.0);
}