// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: TexColShaderV.vsh ********

// File data
static const char _TexColShaderV_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define TEXCOORD_ARRAY\t1\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = TEXCOORD_ARRAY) in highp vec2\tinUVs;\r\n"
	"\r\n"
	"uniform mediump mat4 MVPMatrix;\r\n"
	"\r\n"
	"out highp vec2 TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tTexCoord = inUVs;\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"}";

// Register TexColShaderV.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_TexColShaderV_vsh("TexColShaderV.vsh", _TexColShaderV_vsh, 337);

// ******** End: TexColShaderV.vsh ********

