#version 320 es

layout(location = 0) in highp vec3 inVertex;

layout(location = 0) out mediump vec3 RayDir;

layout(std140, set = 0, binding = 1) uniform Dynamic
{
	highp mat4 InvVPMatrix;
	highp vec4 EyePos;
};

void main()
{
	// Set position
	gl_Position = vec4(inVertex, 1.0);

	// Calculate world space vertex position
	highp vec4 pos = gl_Position;

	vec4 WorldPos = InvVPMatrix * pos;
	// fip the y here to convert from vulkan +Y down corrdinate to Opengl +Y up.
	WorldPos /= WorldPos.w;

	// Calculate ray direction
	RayDir = normalize(WorldPos.xyz - vec3(EyePos));
}
