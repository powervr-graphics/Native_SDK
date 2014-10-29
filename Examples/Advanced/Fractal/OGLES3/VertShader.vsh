#version 300 es

#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1

layout (location = VERTEX_ARRAY) in highp vec4	inVertex;
layout (location = TEXCOORD_ARRAY) in mediump vec2	inTexCoord;

uniform mediump mat4  MVPMatrix;

out mediump vec2  TexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;
	TexCoord = inTexCoord;
}
