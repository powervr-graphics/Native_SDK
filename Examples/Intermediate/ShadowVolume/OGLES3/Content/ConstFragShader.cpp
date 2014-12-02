// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ConstFragShader.fsh ********

// File data
static const char _ConstFragShader_fsh[] = 
	"#version 300 es\r\n"
	"uniform lowp vec4  Color;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\toColour = Color;\r\n"
	"}\r\n";

// Register ConstFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ConstFragShader_fsh("ConstFragShader.fsh", _ConstFragShader_fsh, 132);

// ******** End: ConstFragShader.fsh ********

