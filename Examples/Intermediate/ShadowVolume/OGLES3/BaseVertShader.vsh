#version 300 es

/*
  Simple vertex shader:
  - standard vertex transformation
  - diffuse lighting for one directional light
  - texcoord passthrough
*/

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = NORMAL_ARRAY) in mediump vec3	inNormal;
layout (location = TEXCOORD_ARRAY) in mediump vec2	inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightPosModel;

out lowp    float  LightIntensity;
out mediump vec2   TexCoord;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	mediump vec3 lightDir = normalize(LightPosModel - inVertex);
	LightIntensity = max(0.0, dot(inNormal, lightDir));
	
	TexCoord = inTexCoord;
}
