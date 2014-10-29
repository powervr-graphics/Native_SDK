#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec4	inVertex;
layout (location = NORMAL_ARRAY) in highp vec3	inNormal;
layout (location = TEXCOORD_ARRAY) in highp vec2 inTexCoord;

uniform highp mat4  MVPMatrix;
uniform highp vec3  LightDirection;
uniform highp vec3  EyePosition;

out mediump float  CosViewAngle;
out mediump float  LightIntensity;
out mediump vec2   TexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;
	
	highp vec3 eyeDirection = normalize(EyePosition - inVertex.xyz);
	
	// Simple diffuse lighting 
	LightIntensity = max(dot(LightDirection, inNormal), 0.0);

	// Cosine of the angle between surface normal and eye direction
	// We clamp at 0.1 to avoid ugly aliasing at near 90° angles
	CosViewAngle = max(dot(eyeDirection, inNormal), 0.1);
	
	TexCoord = inTexCoord;
}