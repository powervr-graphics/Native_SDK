// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FullscreenFragShader.fsh ********

// File data
static const char _FullscreenFragShader_fsh[] = 
	"varying lowp vec4 vColour;\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\tgl_FragColor = vColour;\n"
	"}\n";

// Register FullscreenFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FullscreenFragShader_fsh("FullscreenFragShader.fsh", _FullscreenFragShader_fsh, 70);

// ******** End: FullscreenFragShader.fsh ********

