#version 300 es

#define VERTEX_ARRAY_UI	0
#define TEXCOORD_ARRAY_UI 1

layout (location = VERTEX_ARRAY_UI) in highp vec2	inVertex;
layout (location = TEXCOORD_ARRAY_UI) in mediump vec2	inTexCoord;

uniform   highp   mat2  RotationMatrix;

out   mediump vec2  vTexCoord;

void main()
{	
	highp vec2 vertex = RotationMatrix * inVertex;
	gl_Position = vec4(vertex, 0.0, 1.0);	
	vTexCoord = inTexCoord;
}
