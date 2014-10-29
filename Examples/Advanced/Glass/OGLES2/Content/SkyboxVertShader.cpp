// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SkyboxVertShader.vsh ********

// File data
static const char _SkyboxVertShader_vsh[] = 
	"#version 100\n"
	"\n"
	"uniform highp mat4 InvVPMatrix;\n"
	"uniform mediump vec3 EyePos;\n"
	"\n"
	"attribute highp vec3 inVertex;\n"
	"\n"
	"varying mediump vec3 RayDir;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Set position\n"
	"\tgl_Position = vec4(inVertex, 1.0);\n"
	"\n"
	"\t// Calculate world space vertex position\n"
	"\tvec4 WorldPos = InvVPMatrix * gl_Position;\n"
	"\tWorldPos /= WorldPos.w;\n"
	"\n"
	"\t// Calculate ray direction\n"
	"\tRayDir = normalize(WorldPos.xyz - EyePos);\n"
	"}";

// Register SkyboxVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SkyboxVertShader_vsh("SkyboxVertShader.vsh", _SkyboxVertShader_vsh, 391);

// ******** End: SkyboxVertShader.vsh ********

