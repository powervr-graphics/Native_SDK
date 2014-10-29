#version 300 es

#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = TEXCOORD_ARRAY) in highp vec2 inTexCoord;

uniform mediump mat4 MVPMatrix;

out highp vec2 TexCoord;

void main()
{
	TexCoord = inTexCoord;
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}