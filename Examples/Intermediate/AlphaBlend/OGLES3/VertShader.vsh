#version 300 es
#define VERTEX_ARRAY	0

layout (location = VERTEX_ARRAY) in highp vec2	inVertex;

uniform highp vec2  LowerLeft;
uniform highp mat2  ScaleMatrix; // width/height and screen rotation

out mediump vec2  TexCoord;

void main()
{
	gl_Position = vec4(ScaleMatrix * inVertex + LowerLeft, 0, 1);
	
	TexCoord = inVertex;
}
