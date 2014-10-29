#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = NORMAL_ARRAY) in mediump vec3	inNormal;
layout (location = TEXCOORD_ARRAY) in mediump vec2	inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightDirection;

out lowp    float  LightIntensity;
out mediump vec2   TexCoord;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);	

	// Pass through texcoords
	TexCoord = inTexCoord;
	
	// Simple diffuse lighting in model space
	LightIntensity = dot(inNormal, -LightDirection);
}