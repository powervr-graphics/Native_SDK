#version 300 es

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

in highp vec3	inVertex;
in mediump vec3 inNormal;
in mediump vec3 inTangent;
in mediump vec3 inBiNormal;
in mediump vec2 inTexCoord;
in highp vec4 inBoneWeights;
in highp vec4 inBoneIndex;

uniform highp   mat4 ViewProjMatrix;
uniform mediump vec3 LightPos;
uniform mediump	int	 BoneCount;
uniform highp   mat4 BoneMatrixArray[MAX_BONE_COUNT];
uniform highp   mat3 BoneMatrixArrayIT[MAX_BONE_COUNT];

out mediump vec3 vLight;
out mediump vec2 vTexCoord;

void main()
{
	// On PowerVR GPUs it is possible to index the components of a vector
	// with the [] operator. However this can cause trouble with PC
	// emulation on some hardware so we "rotate" the vectors instead.
	mediump ivec4 boneIndex = ivec4(inBoneIndex);
	mediump vec4 boneWeights = inBoneWeights;

	highp mat4 boneMatrix;
	mediump mat3 normalMatrix;

	mediump vec3 worldTangent = vec3(0, 0, 0);
	mediump vec3 worldBiNormal = vec3(0, 0, 0);

	highp vec4 position = vec4(0, 0, 0, 0);
	mediump vec3 worldNormal = vec3(0, 0, 0);

	for (lowp int i = 0; i < BoneCount; ++i)
	{
		boneMatrix = BoneMatrixArray[boneIndex.x]; normalMatrix = BoneMatrixArrayIT[boneIndex.x];

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
