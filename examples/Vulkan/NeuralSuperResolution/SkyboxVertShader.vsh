#version 320 es

layout(std140, set = 0, binding = 1) uniform Dynamic
{
    highp mat4 inverseViewProjectionMatrix;
    highp mat4 projectionMatrix;
    highp mat4 viewMatrixCurrentFrame;
	highp mat4 viewMatrixPreviousFrame;
	highp vec3 cameraPosition;
    highp float exposure;
    highp int screenWidth;
    highp int screenHeight;
	mediump float emissiveIntensity;
	mediump float textureLODBias;
	highp vec2 jitter;
} ubo;

layout(location = 0) out mediump vec3 RayDir;
layout(location = 1) out highp vec4 clipPosition;
layout(location = 2) out highp vec4 clipPositionPreviousFrame;

void main()
{
		const highp vec3 quadVertices[6] = vec3[6](
		vec3(-1., -1., 1.f), // upper left
		vec3(-1.,  1., 1.f), // lower left
		vec3( 1., -1., 1.f), // upper right
		vec3( 1., -1., 1.f), // upper right
		vec3(-1.,  1., 1.f), // lower left
		vec3( 1.,  1., 1.f) // lower right
	);
	
	highp vec3 inVertex = quadVertices[gl_VertexIndex];

	// Set position
	gl_Position = vec4(inVertex, 1.0);

	// Calculate world space vertex position
	highp vec4 pos = gl_Position;

	vec4 WorldPos = ubo.inverseViewProjectionMatrix * pos;
	// flip the y here to convert from vulkan +Y down coordinate to OpenGL +Y up.
	WorldPos /= WorldPos.w;

	// Calculate ray direction
	RayDir = normalize(WorldPos.xyz - vec3(ubo.cameraPosition));

	// Add jitter to the vertex shader output in clip space
	gl_Position += vec4(ubo.jitter * gl_Position.w, 0.0, 0.0);

	// Motion vector information
	clipPosition = ubo.projectionMatrix * ubo.viewMatrixCurrentFrame * vec4(inVertex, 1.0);
	clipPositionPreviousFrame = ubo.projectionMatrix * ubo.viewMatrixPreviousFrame * vec4(inVertex, 1.0);
}
