#version 300 es

layout (location = 0) in highp vec2	inVertex;

void main()
{
	gl_Position = vec4(inVertex, 0.0, 1.0);
}
