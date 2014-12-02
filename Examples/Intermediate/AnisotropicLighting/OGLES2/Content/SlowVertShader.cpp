// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SlowVertShader.vsh ********

// File data
static const char _SlowVertShader_vsh[] = 
	"/******************************************************************************\n"
	"* Vertex Shader (Slow method)\n"
	"*******************************************************************************\n"
	" This technique uses the most significant normal to the grained surface (i.e.\n"
	" normal in same plane as light vector and eye direction vector) to calculate\n"
	" intensities for the diffuse and specular lighting, which create an anisotropic\n"
	" effect. The diffuse lighting factor is defined as the dot product of the light\n"
	" direction and the normal (L.N). The specular lighting factor is defined as the\n"
	" square of the dot product of the view vector (eye direction) and the\n"
	" reflection vector ((V.R) * (V.R)). \n"
	" For convenience these can be expressed in  terms of the the light direction \n"
	" (L), view direction (V) and the tangent to the surface (T). Where the \n"
	" direction of the tangent points along the grain.\n"
	"******************************************************************************/\n"
	"attribute highp vec3  inVertex; \n"
	"attribute highp vec3  inNormal;\n"
	"\n"
	"uniform highp mat4  MVPMatrix;\n"
	"uniform highp vec3  msLightDir;\n"
	"uniform highp vec3  msEyeDir;\n"
	"uniform highp vec4  Material; \n"
	"uniform highp vec3  GrainDir;\n"
	"\n"
	"varying lowp vec3  DiffuseIntensity; \n"
	"varying lowp vec3  SpecularIntensity; \n"
	"\n"
	"void main() \n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\t\n"
	"\t// Calculate the cross product of normal and grain direction.\n"
	"\t// Cross product this with the normal. The result is a vector which is \n"
	"\t// perpendicular to the surface and follows the direction of the grain.\n"
	"\thighp vec3 normalXgrain = cross(inNormal, GrainDir);\n"
	"\thighp vec3 tangent = normalize(cross(normalXgrain, inNormal));\n"
	"\t\n"
	"\thighp float LdotT = dot(tangent, msLightDir);\n"
	"\thighp float VdotT = dot(tangent, msEyeDir);\n"
	"\t\n"
	"\thighp float NdotL = sqrt(1.0 - LdotT * LdotT);\n"
	"\thighp float VdotR = NdotL * sqrt(1.0 - VdotT * VdotT) - VdotT * LdotT;\t\n"
	"\n"
	"\t// Calculate the diffuse intensity, applying scale and bias.\n"
	"\tDiffuseIntensity = vec3(NdotL * Material.x + Material.y);\n"
	"\t\n"
	"\t// Calculate the specular intensity, applying scale and bias.\n"
	"\tSpecularIntensity = vec3(VdotR * VdotR * Material.z + Material.w); \n"
	"}\n";

// Register SlowVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SlowVertShader_vsh("SlowVertShader.vsh", _SlowVertShader_vsh, 2161);

// ******** End: SlowVertShader.vsh ********

