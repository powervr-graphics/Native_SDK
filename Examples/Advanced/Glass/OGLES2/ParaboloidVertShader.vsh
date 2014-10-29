#version 100

uniform highp mat4 MVMatrix;
uniform mediump vec3 LightDir;
uniform mediump vec3 EyePos;
uniform highp float Near;
uniform highp float Far;

attribute highp vec3 inVertex;
attribute mediump vec3 inNormal;
attribute highp vec2 inTexCoords;

varying highp vec2 TexCoords;
varying highp float LightIntensity;

void main()
{
	// Transform position to the paraboloid's view space
	gl_Position = MVMatrix * vec4(inVertex, 1.0);

	// Store the distance
	highp float Distance = -gl_Position.z;

	// Calculate and set the X and Y coordinates
	gl_Position.xyz = normalize(gl_Position.xyz);
	gl_Position.xy /= 1.0 - gl_Position.z;

	// Calculate and set the Z and W coordinates
	gl_Position.z = ((Distance / Far) - 0.5) * 2.0;
	gl_Position.w = 1.0;
	
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
