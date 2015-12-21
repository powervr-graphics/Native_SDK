#version 100

uniform highp mat4 InvVPMatrix;
uniform mediump vec3 EyePos;

attribute highp vec3 inVertex;

varying mediump vec3 RayDir;

void main()
{
	// Set position
	gl_Position = vec4(inVertex, 1.0);

	// Calculate world space vertex position
	vec4 WorldPos = InvVPMatrix * gl_Position;
	WorldPos /= WorldPos.w;

	// Calculate ray direction
	RayDir = normalize(WorldPos.xyz - EyePos);
}