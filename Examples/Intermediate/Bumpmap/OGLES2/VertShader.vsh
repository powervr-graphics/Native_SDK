attribute highp vec4  inVertex;
attribute highp vec3  inNormal;
attribute highp vec2  inTexCoord;
attribute highp vec3  inTangent;

uniform highp mat4  MVPMatrix;		// model view projection transformation
uniform highp vec3  LightPosModel;	// Light position (point light) in model space

varying lowp vec3  LightVec;
varying mediump vec2  TexCoord;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * inVertex;
	
	// Calculate light direction from light position in model space
	// You can skip this step for directional lights
	highp vec3 lightDirection = normalize(LightPosModel - vec3(inVertex));
	
	// transform light direction from model space to tangent space
	highp vec3 bitangent = cross(inNormal, inTangent);
	highp mat3 tangentSpaceXform = mat3(inTangent, bitangent, inNormal);
	LightVec = lightDirection * tangentSpaceXform;
	
	TexCoord = inTexCoord;
}
