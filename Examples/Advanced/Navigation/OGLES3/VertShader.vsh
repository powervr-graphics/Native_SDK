#version 300 es

#define VERTEX_ARRAY	0
layout (location = VERTEX_ARRAY) in highp vec3	inVertex;

uniform highp mat4    ModelViewProjMatrix;

void main()
{
	// Convert each vertex into projection-space and output the value
	gl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);	
}
