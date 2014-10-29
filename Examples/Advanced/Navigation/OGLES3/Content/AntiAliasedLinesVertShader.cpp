// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: AntiAliasedLinesVertShader.vsh ********

// File data
static const char _AntiAliasedLinesVertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define TEXCOORD_ARRAY\t1\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = TEXCOORD_ARRAY) in mediump vec2\tinTexCoord;\r\n"
	"\r\n"
	"out mediump  vec2 TexCoord;\r\n"
	"\r\n"
	"uniform highp mat4 ModelViewProjMatrix;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Convert each vertex into projection-space and output the value\r\n"
	"\tgl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);\t\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}\r\n";

// Register AntiAliasedLinesVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_AntiAliasedLinesVertShader_vsh("AntiAliasedLinesVertShader.vsh", _AntiAliasedLinesVertShader_vsh, 441);

// ******** End: AntiAliasedLinesVertShader.vsh ********

