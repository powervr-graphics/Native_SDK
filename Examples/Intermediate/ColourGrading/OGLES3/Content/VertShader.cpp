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
	"#define VERTEX_ARRAY  0\r\n"
	"#define TEXCOORD_ARRAY  1\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp   vec4  inVertex;\r\n"
	"layout (location = TEXCOORD_ARRAY) in mediump vec2  inTexCoord;\r\n"
	"\r\n"
	"out mediump    vec2  texCoords;\r\n"
	"\t\t\r\n"
	"void main() \r\n"
	"{ \r\n"
	"\tgl_Position  = inVertex;\r\n"
	"\ttexCoords    = inTexCoord;\r\n"
	"} \r\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 316);

// ******** End: VertShader.vsh ********

