// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: VertShader.vsh ********

// File data
static const char _VertShader_vsh[] = 
	"attribute highp vec4  inVertex;\n"
	"attribute highp vec3  inNormal;\n"
	"attribute highp vec2  inTexCoord;\n"
	"\n"
	"uniform highp mat4  MVPMatrix;\n"
	"uniform highp vec3  LightDirection;\n"
	"uniform highp vec3  EyePosition;\n"
	"\n"
	"varying mediump float  CosViewAngle;\n"
	"varying mediump float  LightIntensity;\n"
	"varying mediump vec2   TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = MVPMatrix * inVertex;\n"
	"\t\n"
	"\thighp vec3 eyeDirection = normalize(EyePosition - inVertex.xyz);\n"
	"\t\n"
	"\t// Simple diffuse lighting \n"
	"\tLightIntensity = max(dot(LightDirection, inNormal), 0.0);\n"
	"\n"
	"\t// Cosine of the angle between surface normal and eye direction\n"
	"\t// We clamp at 0.1 to avoid ugly aliasing at near 90\xc2\xb0 angles\n"
	"\tCosViewAngle = max(dot(eyeDirection, inNormal), 0.1);\n"
	"\t\n"
	"\tTexCoord = inTexCoord;\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 730);

// ******** End: VertShader.vsh ********

