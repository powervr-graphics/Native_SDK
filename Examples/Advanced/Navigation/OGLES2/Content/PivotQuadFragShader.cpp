// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PivotQuadFragShader.fsh ********

// File data
static const char _PivotQuadFragShader_fsh[] = 
	"uniform lowp    sampler2D  sTexture;\n"
	"uniform lowp    vec4       Colour;\n"
	"\n"
	"varying mediump vec2       TexCoord;\n"
	"\n"
	"void main()\n"
	"{    \n"
	"\t// Multiply the texture colour with the constant colour\n"
	"\tgl_FragColor = texture2D(sTexture, TexCoord) * Colour;\t\n"
	"}\n";

// Register PivotQuadFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PivotQuadFragShader_fsh("PivotQuadFragShader.fsh", _PivotQuadFragShader_fsh, 245);

// ******** End: PivotQuadFragShader.fsh ********

