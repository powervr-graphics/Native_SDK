// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BlurFragShader.fsh ********

// File data
static const char _BlurFragShader_fsh[] = 
	"uniform lowp sampler2D  sTexture;\n"
	"\n"
	"/* \n"
	"  Separated Gaussian 5x5 filter, first row:\n"
	"\n"
	"              1  5  6  5  1\n"
	"*/\n"
	"\n"
	"varying mediump vec2  TexCoord0;\n"
	"varying mediump vec2  TexCoord1;\n"
	"varying mediump vec2  TexCoord2;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    lowp vec3 color = texture2D(sTexture, TexCoord0).rgb * 0.333333;\n"
	"    color = color + texture2D(sTexture, TexCoord1).rgb * 0.333333;\n"
	"    color = color + texture2D(sTexture, TexCoord2).rgb * 0.333333;    \n"
	"\n"
	"    gl_FragColor.rgb = color;\n"
	"}\n";

// Register BlurFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BlurFragShader_fsh("BlurFragShader.fsh", _BlurFragShader_fsh, 470);

// ******** End: BlurFragShader.fsh ********

