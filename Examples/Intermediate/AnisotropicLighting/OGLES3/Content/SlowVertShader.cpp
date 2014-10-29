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
	"#version 300 es\r\n"
	"/******************************************************************************\r\n"
	"* Vertex Shader (Slow method)\r\n"
	"*******************************************************************************\r\n"
	" This technique uses the most significant normal to the grained surface (i.e.\r\n"
	" normal in same plane as light vector and eye direction vector) to calculate\r\n"
	" intensities for the diffuse and specular lighting, which create an anisotropic\r\n"
	" effect. The diffuse lighting factor is defined as the dot product of the light\r\n"
	" direction and the normal (L.N). The specular lighting factor is defined as the\r\n"
	" square of the dot product of the view vector (eye direction) and the\r\n"
	" reflection vector ((V.R) * (V.R)). \r\n"
	" For convenience these can be expressed in  terms of the the light direction \r\n"
	" (L), view direction (V) and the tangent to the surface (T). Where the \r\n"
	" direction of the tangent points along the grain.\r\n"
	"******************************************************************************/\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define NORMAL_ARRAY\t1\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in highp vec3\tinNormal;\r\n"
	"\r\n"
	"uniform highp mat4  MVPMatrix;\r\n"
	"uniform highp vec3  msLightDir;\r\n"
	"uniform highp vec3  msEyeDir;\r\n"
	"uniform highp vec4  Material; \r\n"
	"uniform highp vec3  GrainDir;\r\n"
	"\r\n"
	"out lowp vec3  DiffuseIntensity; \r\n"
	"out lowp vec3  SpecularIntensity; \r\n"
	"\r\n"
	"void main() \r\n"
	"{\r\n"
	"\t// Transform position\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"\t\r\n"
	"\t// Calculate the cross product of normal and grain direction.\r\n"
	"\t// Cross product this with the normal. The result is a vector which is \r\n"
	"\t// perpendicular to the surface and follows the direction of the grain.\r\n"
	"\thighp vec3 normalXgrain = cross(inNormal, GrainDir);\r\n"
	"\thighp vec3 tangent = normalize(cross(normalXgrain, inNormal));\r\n"
	"\t\r\n"
	"\thighp float LdotT = dot(tangent, msLightDir);\r\n"
	"\thighp float VdotT = dot(tangent, msEyeDir);\r\n"
	"\t\r\n"
	"\thighp float NdotL = sqrt(1.0 - LdotT * LdotT);\r\n"
	"\thighp float VdotR = NdotL * sqrt(1.0 - VdotT * VdotT) - VdotT * LdotT;\t\r\n"
	"\r\n"
	"\t// Calculate the diffuse intensity, applying scale and bias.\r\n"
	"\tDiffuseIntensity = vec3(NdotL * Material.x + Material.y);\r\n"
	"\t\r\n"
	"\t// Calculate the specular intensity, applying scale and bias.\r\n"
	"\tSpecularIntensity = vec3(VdotR * VdotR * Material.z + Material.w); \r\n"
	"}\r\n";

// Register SlowVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SlowVertShader_vsh("SlowVertShader.vsh", _SlowVertShader_vsh, 2318);

// ******** End: SlowVertShader.vsh ********

