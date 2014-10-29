// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: UIFragShader.fsh ********

// File data
static const char _UIFragShader_fsh[] = 
	"uniform lowp sampler2D sTexture;\n"
	"uniform lowp vec4      ColourScale;\n"
	" \n"
	"varying mediump vec2 vTexCoord;\n"
	"\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\tgl_FragColor = texture2D(sTexture, vTexCoord) * ColourScale;\n"
	"}\n";

// Register UIFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_UIFragShader_fsh("UIFragShader.fsh", _UIFragShader_fsh, 184);

// ******** End: UIFragShader.fsh ********

