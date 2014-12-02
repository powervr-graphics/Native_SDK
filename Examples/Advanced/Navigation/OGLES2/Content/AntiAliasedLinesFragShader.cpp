// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: AntiAliasedLinesFragShader.fsh ********

// File data
static const char _AntiAliasedLinesFragShader_fsh[] = 
	"uniform mediump sampler2D  sTexture;\n"
	"uniform lowp    vec4       FlatColour;\n"
	"\n"
	"varying mediump vec2    TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tlowp vec4 texcol = texture2D(sTexture, TexCoord);\n"
	"\tgl_FragColor = vec4(FlatColour.rgb * texcol.r, texcol.a);\n"
	"}\n";

// Register AntiAliasedLinesFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_AntiAliasedLinesFragShader_fsh("AntiAliasedLinesFragShader.fsh", _AntiAliasedLinesFragShader_fsh, 238);

// ******** End: AntiAliasedLinesFragShader.fsh ********

