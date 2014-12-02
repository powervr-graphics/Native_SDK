// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ColShaderV.vsh ********

// File data
static const char _ColShaderV_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3 inVertex;\r\n"
	"\r\n"
	"uniform mediump mat4 MVPMatrix;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"}";

// Register ColShaderV.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ColShaderV_vsh("ColShaderV.vsh", _ColShaderV_vsh, 204);

// ******** End: ColShaderV.vsh ********

