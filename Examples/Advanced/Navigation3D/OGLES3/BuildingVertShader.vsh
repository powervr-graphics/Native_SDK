#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = NORMAL_ARRAY) in highp vec3	inNormal;
layout (location = TEXCOORD_ARRAY) in mediump vec2	inTexCoord;

uniform highp mat4    ModelViewProjMatrix;
uniform highp vec3    LightDirection;

out highp   float   vDiffuse;
out mediump vec2    vTexCoord;

void main()
{	
	vDiffuse = 0.4 + max(dot(inNormal, LightDirection), 0.0) * 0.6;
	vTexCoord = inTexCoord;
	gl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);	
}
