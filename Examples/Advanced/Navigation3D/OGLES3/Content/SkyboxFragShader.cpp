// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SkyboxFragShader.fsh ********

// File data
static const char _SkyboxFragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"uniform samplerCube sCubeMap;\r\n"
	"\r\n"
	"in mediump vec3 vEyeDir;\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\toColour = texture(sCubeMap, vEyeDir);\r\n"
	"}";

// Register SkyboxFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SkyboxFragShader_fsh("SkyboxFragShader.fsh", _SkyboxFragShader_fsh, 183);

// ******** End: SkyboxFragShader.fsh ********

