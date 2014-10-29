attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightDirModel;
uniform mediump vec3  EyePosModel;
uniform         bool  bSpecular;
uniform			bool  bRotate;

varying lowp    float  SpecularIntensity;
varying mediump vec2   RefractCoord;

const mediump float  cShininess = 3.0;
const mediump float  cRIR = 1.015;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	// Eye direction in model space
	mediump vec3 eyeDirModel = normalize(inVertex - EyePosModel);	
	
	// GLSL offers a nice built-in refaction function
	// Calculate refraction direction in model space
	mediump vec3 refractDir = refract(eyeDirModel, inNormal, cRIR);
	
	// Project refraction
	refractDir = (MVPMatrix * vec4(refractDir, 0.0)).xyw;

	// Map refraction direction to 2d coordinates
	RefractCoord = 0.5 * (refractDir.xy / refractDir.z) + 0.5;

	if(bRotate) // If the screen is rotated then rotate the uvs
	{
		RefractCoord.xy = RefractCoord.yx;
		RefractCoord.y = -RefractCoord.y;
	}
		
	// Specular lighting
	// We ignore that N dot L could be negative (light coming 
	// from behind the surface)
	SpecularIntensity = 0.0;

	if (bSpecular)
	{
		mediump vec3 halfVector = normalize(LightDirModel + eyeDirModel);
		lowp float NdotH = max(dot(inNormal, halfVector), 0.0);		
		SpecularIntensity = pow(NdotH, cShininess);
	}
}