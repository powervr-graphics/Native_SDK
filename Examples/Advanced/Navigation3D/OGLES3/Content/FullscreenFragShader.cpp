// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FullscreenFragShader.fsh ********

// File data
static const char _FullscreenFragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"in lowp vec4 vColour;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\t\r\n"
	"\toColour = vColour;\r\n"
	"}\r\n";

// Register FullscreenFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FullscreenFragShader_fsh("FullscreenFragShader.fsh", _FullscreenFragShader_fsh, 133);

// ******** End: FullscreenFragShader.fsh ********

