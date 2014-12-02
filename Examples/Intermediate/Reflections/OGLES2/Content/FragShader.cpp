// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FragShader.fsh ********

// File data
static const char _FragShader_fsh[] = 
	"uniform sampler2D s2DMap;\n"
	"uniform samplerCube sCubeMap;\n"
	"\n"
	"uniform bool bCubeReflection;\n"
	"\n"
	"varying mediump vec3  ReflectDir;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// select whether to use cube map reflection or 2d reflection\t\n"
	"\tif(bCubeReflection)\n"
	"\t{\n"
	"\t\tgl_FragColor = textureCube(sCubeMap, ReflectDir);\n"
	"\t}\n"
	"\telse \n"
	"\t{\n"
	"\t\tgl_FragColor = texture2D(s2DMap, ReflectDir.xy * 0.5 + 0.5);\n"
	"\t}\n"
	"}";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 357);

// ******** End: FragShader.fsh ********

