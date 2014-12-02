// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PreBloomFragShader.fsh ********

// File data
static const char _PreBloomFragShader_fsh[] = 
	"#version 300 es\r\n"
	"uniform sampler2D  sBloomMapping;\r\n"
	"\r\n"
	"uniform mediump float fBloomIntensity;\r\n"
	"\r\n"
	"in mediump vec2 TexCoord;\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"    oColour = texture(sBloomMapping, TexCoord) * fBloomIntensity;\r\n"
	"}\r\n";

// Register PreBloomFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PreBloomFragShader_fsh("PreBloomFragShader.fsh", _PreBloomFragShader_fsh, 257);

// ******** End: PreBloomFragShader.fsh ********

