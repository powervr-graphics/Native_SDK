// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: Tex2DFShader.fsh ********

// File data
static const char _Tex2DFShader_fsh[] = 
	"uniform sampler2D Texture;\n"
	"\n"
	"varying highp vec2 TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_FragColor = texture2D(Texture, TexCoord);\n"
	"}";

// Register Tex2DFShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_Tex2DFShader_fsh("Tex2DFShader.fsh", _Tex2DFShader_fsh, 119);

// ******** End: Tex2DFShader.fsh ********

