#version 100

uniform highp mat4 MVPMatrix;
uniform mediump vec3 LightDir;
uniform highp vec3 EyePos;

attribute highp vec3 inVertex;
attribute mediump vec3 inNormal;
attribute highp vec2 inTexCoords;

varying highp vec2 TexCoords;
varying highp float LightIntensity;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	// Pass through texture coordinates
	TexCoords = inTexCoords;

	// Calculate light intensity
	// Ambient
	LightIntensity = 0.4;
	
	// Diffuse
	LightIntensity += max(dot(inNormal, LightDir), 0.0) * 0.3;

	// Specular
	mediump vec3 EyeDir = normalize(EyePos - inVertex);
	LightIntensity += pow(max(dot(reflect(-LightDir, inNormal), EyeDir), 0.0), 5.0) * 0.8;
}
