#version 450

#define VERTEX_ARRAY	0
layout (location = VERTEX_ARRAY) in highp vec3 inVertex;

layout(set=0, binding=0) uniform MVP
{
	mediump mat4 MVPMatrix;
};


void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}