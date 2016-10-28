#version 450

#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = TEXCOORD_ARRAY) in highp vec2 inUVs;

layout(set=0, binding=0) uniform Dynamic
{
	mediump mat4 MVPMatrix;
};

layout(location = 0)out highp vec2 TexCoord;

void main()
{
	TexCoord = inUVs;
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}