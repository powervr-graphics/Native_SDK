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

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TANGENT_ARRAY	2
#define BINORMAL_ARRAY	3
#define TEXCOORD_ARRAY	4
#define BONEWEIGHT_ARRAY	5
#define BONEINDEX_ARRAY	6

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout (location = TANGENT_ARRAY) in mediump vec3 inTangent;
layout (location = BINORMAL_ARRAY) in mediump vec3 inBiNormal;
layout (location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;
layout (location = BONEWEIGHT_ARRAY) in mediump vec4 inBoneWeights;
layout (location = BONEINDEX_ARRAY) in mediump vec4 inBoneIndex;

uniform highp   mat4 ViewProjMatrix;
uniform mediump vec3 LightPos;
uniform mediump	int	 BoneCount;
uniform highp   mat4 BoneMatrixArray[8];
uniform highp   mat3 BoneMatrixArrayIT[8];
uniform bool	bUseDot3;

out mediump vec3 Light;
out mediump vec2 TexCoord;

void main()
{
	if(BoneCount > 0)
	{
		// On PowerVR SGX it is possible to index the components of a vector
		// with the [] operator. However this can cause trouble with PC
		// emulation on some hardware so we "rotate" the vectors instead.
		mediump ivec4 boneIndex = ivec4(inBoneIndex);
		mediump vec4 boneWeights = inBoneWeights;
	
		highp mat4 boneMatrix = BoneMatrixArray[boneIndex.x];
		mediump mat3 normalMatrix = BoneMatrixArrayIT[boneIndex.x];
	
		highp vec4 position = boneMatrix * vec4(inVertex, 1.0) * boneWeights.x;
		mediump vec3 worldNormal = normalMatrix * inNormal * boneWeights.x;
		
		mediump vec3 worldTangent;
		mediump vec3 worldBiNormal;
		
		if(bUseDot3)
		{
			worldTangent = normalMatrix * inTangent * boneWeights.x;
			worldBiNormal = normalMatrix * inBiNormal * boneWeights.x;
		}
	
		for (lowp int i = 1; i < 3; ++i)
		{
			if(i < BoneCount)
			{
				// "rotate" the vector components
				boneIndex = boneIndex.yzwx;
				boneWeights = boneWeights.yzwx;
			
				boneMatrix = BoneMatrixArray[boneIndex.x];
				normalMatrix = BoneMatrixArrayIT[boneIndex.x];

				position += boneMatrix * vec4(inVertex, 1.0) * boneWeights.x;
				worldNormal += normalMatrix * inNormal * boneWeights.x;
				
				if(bUseDot3)
				{
					worldTangent += normalMatrix * inTangent * boneWeights.x;
					worldBiNormal += normalMatrix * inBiNormal * boneWeights.x;
				}
			}
		}		
		gl_Position = ViewProjMatrix * position;
		
		// lighting
		mediump vec3 TmpLightDir = normalize(LightPos - position.xyz);
		
		if(bUseDot3)
		{
			Light.x = dot(normalize(worldTangent), TmpLightDir);
			Light.y = dot(normalize(worldBiNormal), TmpLightDir);
			Light.z = dot(normalize(worldNormal), TmpLightDir);
		}
		else
		{
			Light.x = dot(normalize(worldNormal), TmpLightDir);
		}
	}

	
	// Pass through texcoords
	TexCoord = inTexCoord;
}
 