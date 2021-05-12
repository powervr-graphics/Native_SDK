#version 320 es

#define VERTEX_ARRAY 0
#define NORMAL_ARRAY 1
#define TEXCOORD_ARRAY 2

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoords;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mInvVPMatrix;
	highp vec4 vCameraPosition;
};

layout(push_constant) uniform PushConsts { highp mat4 mWorldMatrix; };

layout(location = 0) out mediump vec2 vTexCoord;
layout(location = 1) out mediump vec3 vNormal;
layout(location = 2) out highp vec3 vWorldPosition;

void main() 
{
	vec4 worldPosition = mWorldMatrix * vec4(inVertex, 1.0);
	vWorldPosition = worldPosition.xyz;

	gl_Position = mProjectionMatrix * mViewMatrix * worldPosition;

	// Transform normal from model space to world space
	vNormal = mat3(mWorldMatrix) * inNormal;

	// Pass the texture coordinates to the fragment shader
	vTexCoord = inTexCoords;				
}