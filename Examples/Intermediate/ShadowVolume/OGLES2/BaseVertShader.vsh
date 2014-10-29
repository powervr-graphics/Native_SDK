/*
  Simple vertex shader:
  - standard vertex transformation
  - diffuse lighting for one directional light
  - texcoord passthrough
*/

attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;
attribute mediump vec2  inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightPosModel;

varying lowp    float  LightIntensity;
varying mediump vec2   TexCoord;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	mediump vec3 lightDir = normalize(LightPosModel - inVertex);
	LightIntensity = max(0.0, dot(inNormal, lightDir));
	
	TexCoord = inTexCoord;
}
