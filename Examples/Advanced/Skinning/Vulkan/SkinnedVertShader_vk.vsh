#version 450

/*
	If the current vertex is affected by bones then the vertex position and
	normal will be transformed by the bone matrices. Each vertex wil have up
	to 4 bone indices (inBoneIndex) and bone weights (inBoneWeights).

	The indices are used to index into the array of bone matrices
	(BoneMatrixArray) to get the required bone matrix for transformation. The
	amount of influence a particular bone has on a vertex is determined by the
	weights which should always total 1. So if a vertex is affected by 2 bones
	the vertex position in world space is given by the following equation:

	position = (BoneMatrixArray[Index0] * inVertex) * Weight0 +
	           (BoneMatrixArray[Index1] * inVertex) * Weight1

	The same proceedure is applied to the normals but the translation part of
	the transformation is ignored.

	After this the position is multiplied by the view and projection matrices
	only as the bone matrices already contain the model transform for this
	particular mesh. The two-step transformation is required because lighting
	will not work properly in clip space.
*/

#define MAX_BONE_COUNT 8

layout(location = 0) in highp vec3	inVertex;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec3 inTangent;
layout(location = 3) in mediump vec3 inBiNormal;
layout(location = 4) in mediump vec2 inTexCoord;
layout(location = 5) in highp vec4 inBoneWeights;
layout(location = 6) in uvec4 inBoneIndex;

// perframe / per mesh
layout(std140,set = 1, binding = 0) uniform Dynamics
{
<<<<<<< HEAD
	mat4 BoneMatrixArray0;
    mat4 BoneMatrixArray1;
    mat4 BoneMatrixArray2;
    mat4 BoneMatrixArray3;
    mat4 BoneMatrixArray4;
    mat4 BoneMatrixArray5;
    mat4 BoneMatrixArray6;
    mat4 BoneMatrixArray7;
    mat3x3 BoneMatrixArrayIT0;
    mat3x3 BoneMatrixArrayIT1;
    mat3x3 BoneMatrixArrayIT2;
    mat3x3 BoneMatrixArrayIT3;
    mat3x3 BoneMatrixArrayIT4;
    mat3x3 BoneMatrixArrayIT5;
    mat3x3 BoneMatrixArrayIT6;
    mat3x3 BoneMatrixArrayIT7;
    int	 BoneCount;
=======
	mat4 BoneMatrixArray[8];
    mat3x3 BoneMatrixArrayIT[8];
    int BoneCount;
>>>>>>> 1776432f... 4.3
};

// static throughout the lifetime
layout(std140,set = 2, binding = 0) uniform Statics
{
    mat4 ViewProjMatrix;
    vec3 LightPos;
};

layout(location = 0) out mediump vec3 vLight;
layout(location = 1) out mediump vec2 vTexCoord;

void main()
{
	// On PowerVR GPUs it is possible to index the components of a vector
	// with the [] operator. However this can cause trouble with PC
	// emulation on some hardware so we "rotate" the vectors instead.
	mediump uvec4 boneIndex = uvec4(inBoneIndex);

	mediump vec4 boneWeights = inBoneWeights;

	highp mat4 boneMatrix;
	mediump mat3 normalMatrix;
	
	mediump vec3 worldTangent = vec3(0,0,0);
	mediump vec3 worldBiNormal = vec3(0,0,0);
	
	highp vec4 position = vec4(0,0,0,0);
	mediump vec3 worldNormal = vec3(0,0,0);
	
	for (lowp int i = 0; i < BoneCount; ++i)
	{
		boneMatrix = BoneMatrixArray[boneIndex.x];
		normalMatrix = BoneMatrixArrayIT[boneIndex.x];

		position += boneMatrix * vec4(inVertex, 1.0) * boneWeights.x;
		worldNormal += normalMatrix * inNormal * boneWeights.x;

		worldTangent += normalMatrix * inTangent * boneWeights.x;
		worldBiNormal += normalMatrix * inBiNormal * boneWeights.x;

		// "rotate" the vector components
		boneIndex = boneIndex.yzwx;
		boneWeights = boneWeights.yzwx;
	}
	
	gl_Position = ViewProjMatrix * position;

	// lighting
	mediump vec3 TmpLightDir = normalize(LightPos - position.xyz);

	vLight.x = dot(normalize(worldTangent), TmpLightDir);
	vLight.y = dot(normalize(worldBiNormal), TmpLightDir);
	vLight.z = dot(normalize(worldNormal), TmpLightDir);

	// Pass through texcoords
	vTexCoord = inTexCoord;
}
