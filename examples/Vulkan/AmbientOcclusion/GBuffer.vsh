#version 320 es

// Per model uniform buffer
layout(set = 0, binding = 0) uniform PerModel
{
	highp mat4 MVPMatrix;
	highp mat3 NormalMatrix;
};

// Vertex shader inputs
#define VERTEX_ARRAY 0
#define NORMAL_ARRAY 1
#define TEX_COORD 2
layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEX_COORD) in mediump vec2 inTexCoord;

// Vertex shader outputs
layout(location = 0) out mediump vec2 vTexCoord;
layout(location = 1) out highp vec3 vNormal;

void main()
{
	// Transform the vertices to clip space
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Transform the normals to view space and then pass on the fragment shader
	vNormal = NormalMatrix * inNormal;

	// Pass on texture coordinates to the fragment shader
	vTexCoord = inTexCoord;
}
