#version 300 es

layout (location = 0) in highp vec4	inVertex;
layout (location = 1) in highp vec2 inTexCoord;

uniform highp mat4  MVPMatrix;

out mediump vec2  TexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;
	TexCoord = inTexCoord;
}
