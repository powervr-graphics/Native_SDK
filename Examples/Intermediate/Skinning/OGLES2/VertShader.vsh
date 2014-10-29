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

attribute highp   vec3 inVertex;
attribute mediump vec3 inNormal;
attribute mediump vec2 inTexCoord;
attribute mediump vec4 inBoneIndex;
attribute mediump vec4 inBoneWeights;

uniform highp   mat4 MVPMatrix;
uniform highp   mat4 ViewProjMatrix;
uniform mediump vec3 LightDirModel;
uniform mediump vec3 LightDirWorld;
uniform mediump	int	 BoneCount;
uniform highp   mat4 BoneMatrixArray[8];
uniform highp   mat3 BoneMatrixArrayIT[8];

varying lowp    float LightIntensity;
varying mediump vec2  TexCoord;

void main()
{
	
	// On PowerVR SGX it is possible to index the components of a vector
	// with the [] operator. However this can cause trouble with PC
	// emulation on some hardware so we "rotate" the vectors instead.
	mediump ivec4 boneIndex = ivec4(inBoneIndex);
	mediump vec4 boneWeights = inBoneWeights;
	
	if (BoneCount > 0)
	{
		highp mat4 boneMatrix = BoneMatrixArray[boneIndex.x];
		mediump mat3 normalMatrix = BoneMatrixArrayIT[boneIndex.x];
	
		highp vec4 position = boneMatrix * vec4(inVertex, 1.0) * boneWeights.x;
		mediump vec3 worldNormal = normalMatrix * inNormal * boneWeights.x;
		
		// PowerVR SGX supports uniforms in the for loop and nested conditionals.
		// For performance reasons, the code below should be like this:
		//	for (lowp int i = 1; i < BoneCount; ++i)
		//	{
		//		boneIndex = boneIndex.yzwx;
		//		boneWeights = boneWeights.yzwx;
		//	
		//		boneMatrix = BoneMatrixArray[boneIndex.x];
		//		normalMatrix = BoneMatrixArrayIT[boneIndex.x];
		//	
		//		if (boneWeights.x > 0.0)
		//		{
		//			position += boneMatrix * vec4(inVertex, 1.0) * boneWeights.x;
		//			worldNormal += normalMatrix * inNormal * boneWeights.x;
		//		}
		//	}
		// However this code causes a severe crash on PCEmulation
		// in some ATI hardware due to a very limited loop support.
		// If you are targeting SGX, please, modify the code below.
		for (lowp int i = 1; i < 3; ++i)
		{
			if(i<BoneCount)
			{
				// "rotate" the vector components
				boneIndex = boneIndex.yzwx;
				boneWeights = boneWeights.yzwx;
			
				boneMatrix = BoneMatrixArray[boneIndex.x];
				normalMatrix = BoneMatrixArrayIT[boneIndex.x];

				position += boneMatrix * vec4(inVertex, 1.0) * boneWeights.x;
				worldNormal += normalMatrix * inNormal * boneWeights.x;
			}
		}		
		gl_Position = ViewProjMatrix * position;
		// Simple diffuse lighting
		LightIntensity = max(0.0, dot(normalize(worldNormal), -LightDirWorld));
	}
	else
	{
		gl_Position = MVPMatrix * vec4(inVertex, 1.0);
		LightIntensity = max(0.0, dot(inNormal, -LightDirModel));
	}
	
	// Pass through texcoords
	TexCoord = inTexCoord;
}
 