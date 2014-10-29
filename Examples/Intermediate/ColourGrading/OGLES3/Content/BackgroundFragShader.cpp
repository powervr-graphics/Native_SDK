// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BackgroundFragShader.fsh ********

// File data
static const char _BackgroundFragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"uniform  sampler2D      sTexture;\r\n"
	"\r\n"
	"in mediump vec2 texCoords;\r\n"
	"layout(location = 0) out lowp vec4 oFragColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"    highp vec3 vCol = texture(sTexture, texCoords).rgb;\r\n"
	"    oFragColour = vec4(vCol, 1.0);\r\n"
	"}\r\n";

// Register BackgroundFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BackgroundFragShader_fsh("BackgroundFragShader.fsh", _BackgroundFragShader_fsh, 247);

// ******** End: BackgroundFragShader.fsh ********

