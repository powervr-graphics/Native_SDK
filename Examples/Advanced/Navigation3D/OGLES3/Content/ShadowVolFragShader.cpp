// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ShadowVolFragShader.fsh ********

// File data
static const char _ShadowVolFragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"in lowp vec4 vColour;\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\t\r\n"
	"\toColour = vColour;\r\n"
	"}\r\n";

// Register ShadowVolFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ShadowVolFragShader_fsh("ShadowVolFragShader.fsh", _ShadowVolFragShader_fsh, 131);

// ******** End: ShadowVolFragShader.fsh ********

