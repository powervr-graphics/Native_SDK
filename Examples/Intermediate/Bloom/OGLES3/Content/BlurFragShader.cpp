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
	"#version 300 es\r\n"
	"uniform lowp sampler2D  sTexture;\r\n"
	"\r\n"
	"/* \r\n"
	"  Separated Gaussian 5x5 filter, first row:\r\n"
	"\r\n"
	"              1  5  6  5  1\r\n"
	"*/\r\n"
	"\r\n"
	"in mediump vec2  TexCoord0;\r\n"
	"in mediump vec2  TexCoord1;\r\n"
	"in mediump vec2  TexCoord2;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"    lowp vec3 colour = texture(sTexture, TexCoord0).rgb * 0.333333;\r\n"
	"    colour = colour + texture(sTexture, TexCoord1).rgb * 0.333333;\r\n"
	"    colour = colour + texture(sTexture, TexCoord2).rgb * 0.333333;    \r\n"
	"\r\n"
	"    oColour.rgb = colour;\r\n"
	"}\r\n";

// Register BlurFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BlurFragShader_fsh("BlurFragShader.fsh", _BlurFragShader_fsh, 535);

// ******** End: BlurFragShader.fsh ********

