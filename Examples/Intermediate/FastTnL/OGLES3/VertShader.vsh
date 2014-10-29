#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec4	inVertex;
layout (location = NORMAL_ARRAY) in highp vec3	inNormal;
layout (location = TEXCOORD_ARRAY) in highp vec2	inTexCoord;

uniform highp mat4   MVPMatrix;
uniform highp vec3   LightDirection;
uniform highp float  MaterialBias;
uniform highp float  MaterialScale;

out lowp vec3  DiffuseLight;
out lowp vec3  SpecularLight;
out mediump vec2  TexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;
	
	DiffuseLight = vec3(max(dot(inNormal, LightDirection), 0.0));
	SpecularLight = vec3(max((DiffuseLight.x - MaterialBias) * MaterialScale, 0.0));
	
	TexCoord = inTexCoord;
}
