// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FastFragShader.fsh ********

// File data
static const char _FastFragShader_fsh[] = 
	"uniform sampler2D  sTexture;\n"
	"\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_FragColor = texture2D(sTexture, TexCoord);\n"
	"}\n";

// Register FastFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FastFragShader_fsh("FastFragShader.fsh", _FastFragShader_fsh, 126);

// ******** End: FastFragShader.fsh ********

