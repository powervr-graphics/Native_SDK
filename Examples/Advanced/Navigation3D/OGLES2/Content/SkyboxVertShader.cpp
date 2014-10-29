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
	"attribute mediump vec3 inVertex;\n"
	"\n"
	"uniform mediump mat4 ModelViewProjMatrix;\n"
	"\n"
	"varying mediump vec3 vEyeDir;\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\tvEyeDir = -inVertex.xzy;\n"
	"\tgl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);\t\n"
	"}";

// Register SkyboxVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SkyboxVertShader_vsh("SkyboxVertShader.vsh", _SkyboxVertShader_vsh, 209);

// ******** End: SkyboxVertShader.vsh ********

