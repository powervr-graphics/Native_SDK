#version 300 es

layout (location = 0) in highp vec3	inVertex;
layout (location = 1) in mediump vec3 inNormal;
layout (location = 2) in mediump vec2 inTexCoord;
layout (location = 3) in mediump vec3 inTangent;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  EyePosModel;

out mediump vec3  EyeDirection;
out mediump vec2  TexCoord;

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
