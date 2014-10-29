// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PreFragShader.fsh ********

// File data
static const char _PreFragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"uniform lowp vec4 inColor;\t// Colour and ID passed in from vertex.\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Simply assigns the colour and ID number of the object it renders.\r\n"
	"\toColour = inColor;\r\n"
	"}\r\n"
	"\r\n";

// Register PreFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PreFragShader_fsh("PreFragShader.fsh", _PreFragShader_fsh, 248);

// ******** End: PreFragShader.fsh ********

