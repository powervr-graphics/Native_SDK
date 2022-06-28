#version 320 es

#define VERTEX_ARRAY 0
#define NORMAL_ARRAY 1
#define TEXCOORD_ARRAY 2
#define NUMBER_OF_MESHES 5

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoords;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mPrevViewProjMatrix;
	highp mat4 mViewProjInverseMatrix;
	highp vec4 vAmbientLightColor;
	highp vec4 vCameraPosition;
};

layout(set = 0, binding = 7, std140) uniform UboMeshTransforms { highp mat4 worldMatrix[NUMBER_OF_MESHES]; };
layout(set = 0, binding = 8, std140) uniform UboPrevMeshTransforms { highp mat4 prevWorldMatrix[NUMBER_OF_MESHES]; };

layout(push_constant) uniform PushConsts { uint meshID; };

layout(location = 0) out mediump vec2 vTexCoord;
layout(location = 1) out mediump vec3 vNormal;
layout(location = 2) out highp vec3 vWorldPosition;
layout(location = 3) out highp vec4 vClipPosition;
layout(location = 4) out highp vec4 vPrevClipPosition;

void main()
{
	vec4 worldPosition = worldMatrix[meshID] * vec4(inVertex, 1.0);
	vec4 prevWorldPosition = prevWorldMatrix[meshID] * vec4(inVertex, 1.0);
	vWorldPosition = worldPosition.xyz;

	gl_Position = mProjectionMatrix * mViewMatrix * worldPosition;

	// Pass clip space positions for motion vectors
	vClipPosition = gl_Position;
	vPrevClipPosition = mPrevViewProjMatrix * prevWorldPosition;

	// Transform normal from model space to world space
	vNormal = mat3(worldMatrix[meshID]) * inNormal;

	// Pass the texture coordinates to the fragment shader
	vTexCoord = inTexCoords;
}