#version 300 es

#define VERTEX_ARRAY	0
layout (location = VERTEX_ARRAY) in mediump vec3	inVertex;

uniform mediump mat4 ModelViewProjMatrix;

out mediump vec3 vEyeDir;

void main()
{	
	vEyeDir = -inVertex.xzy;
	gl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);	
}