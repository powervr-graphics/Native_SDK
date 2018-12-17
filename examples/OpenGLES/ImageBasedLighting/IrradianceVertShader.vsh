#version 310 es

layout(location = 0) in highp vec3 inVertex;

layout(location = 0) uniform highp mat4 rotate;

layout(location = 0) out mediump vec3 outUVW;

void main()
{
	// Set position
	outUVW = (mat3(rotate) * inVertex);

	// Calculate ray direction
	gl_Position = vec4(inVertex, 1.0);
}
