#version 320 es

#define VERTEX_ARRAY   0
#define NORMAL_ARRAY   1
#define TEXCOORD_ARRAY 2
#define TANGENT_ARRAY  3

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in highp vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoords;
layout(location = TANGENT_ARRAY) in mediump vec3 inTangent;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 viewMatrix;
	highp mat4 projectionMatrix;
	highp mat4 inverseViewProjectionMatrix;
	highp vec4 cameraPosition;
};

layout(set = 0, binding = 9) uniform SceneTransform
{
	highp mat4 ModelMatrix; // model view projection transformation
};

layout(location = 0) out mediump vec2 vTexCoord;
layout(location = 1) out highp vec3 vNormal;
layout(location = 2) out highp vec3 vWorldPosition;

void main()
{
	vec4 worldPosition = ModelMatrix * vec4(inVertex, 1.0);
	vWorldPosition     = worldPosition.xyz;
	gl_Position        = projectionMatrix * viewMatrix * worldPosition;
	vNormal            = normalize(transpose(inverse(mat3(ModelMatrix))) * inNormal); // Transform normal from model space to world space	
	vTexCoord          = inTexCoords; // Pass the texture coordinates to the fragment shader
}
