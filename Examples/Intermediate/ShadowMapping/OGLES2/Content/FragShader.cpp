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
	"varying highp vec2 TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
	"}\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 88);

// ******** End: FragShader.fsh ********

