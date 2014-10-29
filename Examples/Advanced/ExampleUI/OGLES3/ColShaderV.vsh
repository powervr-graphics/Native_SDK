#version 300 es

#define VERTEX_ARRAY	0
layout (location = VERTEX_ARRAY) in highp vec3 inVertex;

uniform mediump mat4 MVPMatrix;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}