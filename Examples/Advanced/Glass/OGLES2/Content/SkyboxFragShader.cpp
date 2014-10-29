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
	"#version 100\n"
	"\n"
	"uniform samplerCube sSkybox;\n"
	"\n"
	"varying mediump vec3 RayDir;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Sample skybox cube map\n"
	"\tgl_FragColor = textureCube(sSkybox, RayDir);\n"
	"}\n";

// Register SkyboxFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SkyboxFragShader_fsh("SkyboxFragShader.fsh", _SkyboxFragShader_fsh, 163);

// ******** End: SkyboxFragShader.fsh ********

