// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ConstFragShader.fsh ********

// File data
static const char _ConstFragShader_fsh[] = 
	"uniform lowp vec4  Color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_FragColor = Color;\n"
	"}\n";

// Register ConstFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ConstFragShader_fsh("ConstFragShader.fsh", _ConstFragShader_fsh, 66);

// ******** End: ConstFragShader.fsh ********

