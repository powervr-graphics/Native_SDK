// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ShadowVolFragShader.fsh ********

// File data
static const char _ShadowVolFragShader_fsh[] = 
	"varying lowp vec4 vColour;\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\tgl_FragColor = vColour;\n"
	"}\n";

// Register ShadowVolFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ShadowVolFragShader_fsh("ShadowVolFragShader.fsh", _ShadowVolFragShader_fsh, 70);

// ******** End: ShadowVolFragShader.fsh ********

