#version 320 es

layout(location = 0) out highp vec3 outUVW;

layout(std140, set = 0, binding = 1) uniform Dynamic
{
	highp mat3 rotateMtx;
};

void main()
{
	const highp vec3 quadVertices[6] = vec3[6](
		vec3(-1., -1., 1.f), // upper left
		vec3(-1.,  1., 1.f), // lower left
		vec3( 1., -1., 1.f), // upper right
		vec3( 1., -1., 1.f), // upper right
		vec3(-1.,  1., 1.f), // lower left
		vec3( 1.,  1., 1.f)  // lower right
	);
	
	highp vec3 inVertex = quadVertices[gl_VertexIndex];

	// Set position
	outUVW = rotateMtx * inVertex;

	// Calculate ray direction
	gl_Position = vec4(inVertex, 1.0);
}