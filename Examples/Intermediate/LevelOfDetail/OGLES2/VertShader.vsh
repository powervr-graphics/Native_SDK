/******************************************************************************
* Vertex Shader
******************************************************************************/
/* 
	The vertex and fragment shaders implement two techniques for reflections.
	Which version is used for drawing the object is dependent on the value of 
	bHighDetail. If bHighDetail is true it uses the method described in 
	OGLES2PerturbedUVs and for false it uses OGLES2Reflections. 
	
	Reason for using 2 methods is that when the object is far away you aren't
	going to notice all the detail that the PerturbedUVs method adds to
	the mesh so you may as well use a simpler method for creating reflections.
	This way you aren't using valuable resources on something you aren't 
	going to notice.

	Also, when the training course is in 'low detail' mode it uses a different mesh.
	The mesh that is being drawn contains only 7% of the original meshes vertices.
*/

attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;
attribute mediump vec2  inTexCoord;
attribute mediump vec3  inTangent;

uniform highp   mat4  MVPMatrix;
uniform mediump mat3  ModelWorld;
uniform mediump vec3  EyePosModel;
uniform bool          bHighDetail;

varying mediump vec3  EyeDirection;
varying mediump vec2  TexCoord;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex,1.0);

	// Calculate direction from eye position in model space
	mediump vec3 eyeDirModel = normalize(EyePosModel - inVertex);

	if (bHighDetail)
	{	
		// transform light direction from model space to tangent space
		mediump vec3 binormal = cross(inNormal,inTangent);
		mediump mat3 tangentSpaceXform = mat3(inTangent, binormal, inNormal);
		EyeDirection = eyeDirModel * tangentSpaceXform;	

		TexCoord = inTexCoord;
	}
	else
	{
		// reflect eye direction over normal and transform to world space
		mediump vec3 reflectDir = ModelWorld * reflect(-eyeDirModel, inNormal);
		
		if(inNormal.z >= 0.0)// facing the viewer
		{
			reflectDir = normalize(reflectDir) * 0.5 + 0.5;
		}
		else// facing away from the viewer
		{
			reflectDir = -normalize(reflectDir) * 0.5 + 0.5;
		}
		TexCoord = reflectDir.xy;
	}
}
