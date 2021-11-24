#version 320 es

layout(location = 0) out mediump vec2 outUV;
layout(location = 1) out mediump vec3 outRayDir;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mInvVPMatrix;
	highp vec4 vCameraPosition;
};

void main()
{
	const highp vec3 quadVertices[6] = vec3[6](
		vec3(-1.0, -1.0, 1.0), // upper left
		vec3(-1.0,  1.0, 1.0), // lower left
		vec3( 1.0, -1.0, 1.0), // upper right
		vec3( 1.0, -1.0, 1.0), // upper right
		vec3(-1.0,  1.0, 1.0), // lower left
		vec3( 1.0,  1.0, 1.0)  // lower right
	);

	highp vec3 inVertex = quadVertices[gl_VertexIndex];

	// Set position
	gl_Position = vec4(inVertex, 1.0);

	// Calculate world space vertex position
	highp vec4 pos = gl_Position;

	highp vec4 WorldPos = mInvVPMatrix * pos;
	// flip the y here to convert from vulkan +Y down coordinate to OpenGL +Y up.
	WorldPos /= WorldPos.w;

	// Calculate ray direction
	outRayDir = normalize(WorldPos.xyz - vCameraPosition.xyz);

	// Compute UV
	outUV = inVertex.xy * 0.5 + vec2(0.5);
}
