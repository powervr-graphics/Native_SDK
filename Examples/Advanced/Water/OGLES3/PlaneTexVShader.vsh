#version 300 es

#define VERTEX_ARRAY	0
layout (location = VERTEX_ARRAY) in highp vec3	inVertex;

uniform highp mat4  MVPMatrix;

void main()
{
	// Convert each vertex into projection-space and output the value
	highp vec4 vInVertex = vec4(inVertex, 1.0);
	gl_Position = MVPMatrix * vInVertex;
}
