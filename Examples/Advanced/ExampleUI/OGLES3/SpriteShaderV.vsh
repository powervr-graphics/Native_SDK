#version 300 es

#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1
#define TRANSFORM_ARRAY	2
#define RGBA_ARRAY		3

layout (location = VERTEX_ARRAY) in highp vec3 inVertex;
layout (location = TEXCOORD_ARRAY) in highp vec2 inUVs;
layout (location = TRANSFORM_ARRAY) in mediump float inTransIdx;
layout (location = RGBA_ARRAY) in lowp vec4	inRGBA;

uniform mediump mat4 MTransforms[30];
uniform mediump mat4 MVPMatrix;

out highp vec2 TexCoord;
out lowp vec4 RGBA;

void main()
{
	TexCoord = inUVs;
	RGBA     = inRGBA;
	
	lowp int iTransIdx = int(inTransIdx);
	highp vec4 position = MTransforms[iTransIdx] * vec4(inVertex, 1.0);
		
	gl_Position = MVPMatrix * position;
}