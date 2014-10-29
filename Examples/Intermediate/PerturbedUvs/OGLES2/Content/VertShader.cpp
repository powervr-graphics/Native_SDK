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
	"attribute highp   vec3  inVertex;\n"
	"attribute mediump vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"attribute mediump vec3  inTangent;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform mediump vec3  EyePosModel;\n"
	"\n"
	"varying mediump vec3  EyeDirection;\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex,1.0);\n"
	"\t\n"
	"\t// Calculate direction from eye position in model space\n"
	"\tmediump vec3 eyeDirModel = normalize(EyePosModel - inVertex);\n"
	"\t\t\t\n"
	"\t// transform light direction from model space to tangent space\n"
	"\tmediump vec3 binormal = cross(inNormal, inTangent);\n"
	"\tmediump mat3 tangentSpaceXform = mat3(inTangent, binormal, inNormal);\n"
	"\tEyeDirection = eyeDirModel * tangentSpaceXform;\t\n"
	"\n"
	"\tTexCoord = inTexCoord;\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 753);

// ******** End: VertShader.vsh ********

