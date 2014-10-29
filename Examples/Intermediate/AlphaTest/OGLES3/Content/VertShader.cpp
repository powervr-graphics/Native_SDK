// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: VertShader.vsh ********

// File data
static const char _VertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"layout (location = 0) in highp vec4\tinVertex;\r\n"
	"layout (location = 1) in highp vec2 inTexCoord;\r\n"
	"\r\n"
	"uniform highp mat4  MVPMatrix;\r\n"
	"\r\n"
	"out mediump vec2  TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tgl_Position = MVPMatrix * inVertex;\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}\r\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 264);

// ******** End: VertShader.vsh ********

