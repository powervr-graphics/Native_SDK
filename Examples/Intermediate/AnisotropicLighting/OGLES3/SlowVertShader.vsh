#version 300 es
/******************************************************************************
* Vertex Shader (Slow method)
*******************************************************************************
 This technique uses the most significant normal to the grained surface (i.e.
 normal in same plane as light vector and eye direction vector) to calculate
 intensities for the diffuse and specular lighting, which create an anisotropic
 effect. The diffuse lighting factor is defined as the dot product of the light
 direction and the normal (L.N). The specular lighting factor is defined as the
 square of the dot product of the view vector (eye direction) and the
 reflection vector ((V.R) * (V.R)). 
 For convenience these can be expressed in  terms of the the light direction 
 (L), view direction (V) and the tangent to the surface (T). Where the 
 direction of the tangent points along the grain.
******************************************************************************/
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = NORMAL_ARRAY) in highp vec3	inNormal;

uniform highp mat4  MVPMatrix;
uniform highp vec3  msLightDir;
uniform highp vec3  msEyeDir;
uniform highp vec4  Material; 
uniform highp vec3  GrainDir;

out lowp vec3  DiffuseIntensity; 
out lowp vec3  SpecularIntensity; 

void main() 
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	// Calculate the cross product of normal and grain direction.
	// Cross product this with the normal. The result is a vector which is 
	// perpendicular to the surface and follows the direction of the grain.
	highp vec3 normalXgrain = cross(inNormal, GrainDir);
	highp vec3 tangent = normalize(cross(normalXgrain, inNormal));
	
	highp float LdotT = dot(tangent, msLightDir);
	highp float VdotT = dot(tangent, msEyeDir);
	
	highp float NdotL = sqrt(1.0 - LdotT * LdotT);
	highp float VdotR = NdotL * sqrt(1.0 - VdotT * VdotT) - VdotT * LdotT;	

	// Calculate the diffuse intensity, applying scale and bias.
	DiffuseIntensity = vec3(NdotL * Material.x + Material.y);
	
	// Calculate the specular intensity, applying scale and bias.
	SpecularIntensity = vec3(VdotR * VdotR * Material.z + Material.w); 
}
