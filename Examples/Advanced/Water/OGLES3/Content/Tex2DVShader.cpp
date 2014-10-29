// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: Tex2DVShader.vsh ********

// File data
static const char _Tex2DVShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define TEXCOORD_ARRAY\t2\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = TEXCOORD_ARRAY) in highp vec2 inTexCoord;\r\n"
	"\r\n"
	"uniform mediump mat4 MVPMatrix;\r\n"
	"\r\n"
	"out highp vec2 TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"}";

// Register Tex2DVShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_Tex2DVShader_vsh("Tex2DVShader.vsh", _Tex2DVShader_vsh, 347);

// ******** End: Tex2DVShader.vsh ********

