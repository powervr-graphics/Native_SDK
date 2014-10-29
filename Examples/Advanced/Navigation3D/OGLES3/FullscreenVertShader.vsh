#version 300 es

#define VERTEX_ARRAY	0
layout (location = VERTEX_ARRAY) in highp vec2	inVertex;

uniform   lowp  vec4  FlatColour;
out   lowp  vec4  vColour;

void main()
{
	gl_Position = vec4(inVertex, 0.0, 1.0);
	vColour = FlatColour;
}
