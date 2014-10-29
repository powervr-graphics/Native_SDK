// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FastVertShader.vsh ********

// File data
static const char _FastVertShader_vsh[] = 
	"/******************************************************************************\n"
	"* Vertex Shader (Fast method)\n"
	"*******************************************************************************\n"
	" This technique uses the dot product between the light direction and the normal\n"
	" to generate an x coordinate. The dot product between the half angle vector \n"
	" (vector half way between the viewer's eye and the light direction) and the \n"
	" normal to generate a y coordinate. These coordinates are used to lookup the \n"
	" intensity of light from the special image, which is accessible to the shader \n"
	" as a 2d texture. The intensity is then used to shade a fragment and hence \n"
	" create an anisotropic lighting effect.\n"
	"******************************************************************************/\n"
	"\n"
	"attribute highp vec3  inVertex;\n"
	"attribute highp vec3  inNormal;\n"
	"\n"
	"uniform highp mat4  MVPMatrix;\n"
	"uniform highp vec3  msLightDir;\n"
	"uniform highp vec3  msEyePos;\n"
	"\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main() \n"
	"{ \n"
	"\t// transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1);\n"
	"\t\n"
	"\t// Calculate eye direction in model space\n"
	"\thighp vec3 msEyeDir = normalize(msEyePos - inVertex);\n"
	"\t\n"
	"\t// Calculate vector half way between the vertexToEye and light directions.\n"
	"\t// (division by 2 ignored as it is irrelevant after normalisation)\n"
	"\thighp vec3 halfAngle = normalize(msEyeDir + msLightDir); \n"
	"\t\n"
	"\t// Use dot product of light direction and normal to generate s coordinate.\n"
	"\t// We use GL_CLAMP_TO_EDGE as texture wrap mode to clamp to 0 \n"
	"\tTexCoord.s = dot(msLightDir, inNormal); \n"
	"\t// Use dot product of half angle and normal to generate t coordinate.\n"
	"\tTexCoord.t = dot(halfAngle, inNormal); \n"
	"} \n";

// Register FastVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FastVertShader_vsh("FastVertShader.vsh", _FastVertShader_vsh, 1659);

// ******** End: FastVertShader.vsh ********

