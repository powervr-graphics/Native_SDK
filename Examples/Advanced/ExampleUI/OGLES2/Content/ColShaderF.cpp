// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ColShaderF.fsh ********

// File data
static const char _ColShaderF_fsh[] = 
	"uniform lowp vec4 vRGBA;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_FragColor = vRGBA;\n"
	"}";

// Register ColShaderF.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ColShaderF_fsh("ColShaderF.fsh", _ColShaderF_fsh, 64);

// ******** End: ColShaderF.fsh ********

