// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FragShader.fsh ********

// File data
static const char _FragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"in highp float varTimeToLive;\r\n"
	"\r\n"
	"layout(location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\toColour = vec4(vec3(0.9, 0.61, 0.42) * varTimeToLive, 1.0);\r\n"
	"}\r\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 180);

// ******** End: FragShader.fsh ********

