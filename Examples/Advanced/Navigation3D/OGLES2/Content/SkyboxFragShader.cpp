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
	"uniform samplerCube sCubeMap;\n"
	"\n"
	"varying mediump vec3 vEyeDir;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_FragColor = textureCube(sCubeMap, vEyeDir);\n"
	"}";

// Register SkyboxFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SkyboxFragShader_fsh("SkyboxFragShader.fsh", _SkyboxFragShader_fsh, 125);

// ******** End: SkyboxFragShader.fsh ********

