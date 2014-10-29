// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BaseFragShader.fsh ********

// File data
static const char _BaseFragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"/*\r\n"
	"  Simple fragment shader:\r\n"
	"  - Single texturing modulated with vertex lighting\r\n"
	"*/\r\n"
	"\r\n"
	"uniform sampler2D sTexture;\r\n"
	"\r\n"
	"in lowp    float LightIntensity;\r\n"
	"in mediump vec2  TexCoord;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"    oColour = vec4(texture(sTexture, TexCoord).rgb * LightIntensity, 1.0);\r\n"
	"}\r\n";

// Register BaseFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BaseFragShader_fsh("BaseFragShader.fsh", _BaseFragShader_fsh, 347);

// ******** End: BaseFragShader.fsh ********

