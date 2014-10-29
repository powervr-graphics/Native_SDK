attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;
attribute mediump vec2  inTexCoord;
attribute mediump vec3  inTangent;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  EyePosModel;

varying mediump vec3  EyeDirection;
varying mediump vec2  TexCoord;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex,1.0);
	
	// Calculate direction from eye position in model space
	mediump vec3 eyeDirModel = normalize(EyePosModel - inVertex);
			
	// transform light direction from model space to tangent space
	mediump vec3 binormal = cross(inNormal, inTangent);
	mediump mat3 tangentSpaceXform = mat3(inTangent, binormal, inNormal);
	EyeDirection = eyeDirModel * tangentSpaceXform;	

	TexCoord = inTexCoord;
}
