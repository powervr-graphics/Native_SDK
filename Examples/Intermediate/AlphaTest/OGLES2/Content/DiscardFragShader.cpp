// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: DiscardFragShader.fsh ********

// File data
static const char _DiscardFragShader_fsh[] = 
	"uniform sampler2D  sTexture;\n"
	"\n"
	"uniform lowp float  AlphaReference;\n"
	"\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tlowp vec4 color = texture2D(sTexture, TexCoord);\n"
	"\tif (color.a < AlphaReference) \n"
	"\t{\n"
	"\t\tdiscard;\n"
	"\t}\n"
	"\tgl_FragColor = color;\n"
	"}\n";

// Register DiscardFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_DiscardFragShader_fsh("DiscardFragShader.fsh", _DiscardFragShader_fsh, 238);

// ******** End: DiscardFragShader.fsh ********

