#version 320 es

#define VERTEX_ARRAY   0
#define NORMAL_ARRAY   1
#define INSTANCE_ARRAY 2

// Vertex attributes
layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;

// Per-instance attributes
layout(location = INSTANCE_ARRAY) in mediump vec3 instancePosition;

layout(location = 0) out highp vec3 vNormal;

layout (std140, set = 0, binding = 0) uniform DynamicModel
{
	highp mat4 viewProjectionMatrix;
	highp mat4 modelMatrix;
};

void main()
{
	vec3 worldVertexPosition = (modelMatrix * vec4(inVertex.xyz, 1.0)).xyz;
	gl_Position = viewProjectionMatrix * vec4(worldVertexPosition + instancePosition, 1.0);
	vNormal = inNormal;
}
