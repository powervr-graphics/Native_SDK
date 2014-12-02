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
	"#version 300 es\r\n"
	"/******************************************************************************\r\n"
	"* Vertex Shader (Fast method)\r\n"
	"*******************************************************************************\r\n"
	" This technique uses the dot product between the light direction and the normal\r\n"
	" to generate an x coordinate. The dot product between the half angle vector \r\n"
	" (vector half way between the viewer's eye and the light direction) and the \r\n"
	" normal to generate a y coordinate. These coordinates are used to lookup the \r\n"
	" intensity of light from the special image, which is accessible to the shader \r\n"
	" as a 2d texture. The intensity is then used to shade a fragment and hence \r\n"
	" create an anisotropic lighting effect.\r\n"
	"******************************************************************************/\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define NORMAL_ARRAY\t1\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in highp vec3\tinNormal;\r\n"
	"\r\n"
	"uniform highp mat4  MVPMatrix;\r\n"
	"uniform highp vec3  msLightDir;\r\n"
	"uniform highp vec3  msEyePos;\r\n"
	"\r\n"
	"out mediump vec2  TexCoord;\r\n"
	"\r\n"
	"void main() \r\n"
	"{ \r\n"
	"\t// transform position\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1);\r\n"
	"\t\r\n"
	"\t// Calculate eye direction in model space\r\n"
	"\thighp vec3 msEyeDir = normalize(msEyePos - inVertex);\r\n"
	"\t\r\n"
	"\t// Calculate vector half way between the vertexToEye and light directions.\r\n"
	"\t// (division by 2 ignored as it is irrelevant after normalisation)\r\n"
	"\thighp vec3 halfAngle = normalize(msEyeDir + msLightDir); \r\n"
	"\t\r\n"
	"\t// Use dot product of light direction and normal to generate s coordinate.\r\n"
	"\t// We use GL_CLAMP_TO_EDGE as texture wrap mode to clamp to 0 \r\n"
	"\tTexCoord.s = dot(msLightDir, inNormal); \r\n"
	"\t// Use dot product of half angle and normal to generate t coordinate.\r\n"
	"\tTexCoord.t = dot(halfAngle, inNormal); \r\n"
	"} \r\n";

// Register FastVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FastVertShader_vsh("FastVertShader.vsh", _FastVertShader_vsh, 1811);

// ******** End: FastVertShader.vsh ********

