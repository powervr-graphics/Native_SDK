/******************************************************************************
* Vertex Shader (Fast method)
*******************************************************************************
 This technique uses the dot product between the light direction and the normal
 to generate an x coordinate. The dot product between the half angle vector 
 (vector half way between the viewer's eye and the light direction) and the 
 normal to generate a y coordinate. These coordinates are used to lookup the 
 intensity of light from the special image, which is accessible to the shader 
 as a 2d texture. The intensity is then used to shade a fragment and hence 
 create an anisotropic lighting effect.
******************************************************************************/

attribute highp vec3  inVertex;
attribute highp vec3  inNormal;

uniform highp mat4  MVPMatrix;
uniform highp vec3  msLightDir;
uniform highp vec3  msEyePos;

varying mediump vec2  TexCoord;

void main() 
{ 
	// transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1);
	
	// Calculate eye direction in model space
	highp vec3 msEyeDir = normalize(msEyePos - inVertex);
	
	// Calculate vector half way between the vertexToEye and light directions.
	// (division by 2 ignored as it is irrelevant after normalisation)
	highp vec3 halfAngle = normalize(msEyeDir + msLightDir); 
	
	// Use dot product of light direction and normal to generate s coordinate.
	// We use GL_CLAMP_TO_EDGE as texture wrap mode to clamp to 0 
	TexCoord.s = dot(msLightDir, inNormal); 
	// Use dot product of half angle and normal to generate t coordinate.
	TexCoord.t = dot(halfAngle, inNormal); 
} 
