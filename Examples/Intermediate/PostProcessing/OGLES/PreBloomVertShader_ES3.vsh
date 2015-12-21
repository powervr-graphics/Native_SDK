#version 300 es

#define VERTEX_ARRAY		0
#define TEXCOORD_ARRAY	1

layout (location = VERTEX_ARRAY) in highp 	vec3	 inVertex;
layout (location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;
out mediump vec2 TexCoord;
uniform highp   mat4  MVPMatrix;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	TexCoord = inTexCoord;		
}