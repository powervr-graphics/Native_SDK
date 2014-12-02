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
	"attribute vec2 inVertex;\r\n"
	"attribute vec2 inTexCoord;\r\n"
	"\r\n"
	"varying mediump vec2 vTexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Pass through position\r\n"
	"\tgl_Position = vec4(inVertex, 0.0, 1.0);\t\r\n"
	"\r\n"
	"\t// Pass through texcoords\r\n"
	"\tvTexCoord = inTexCoord;\t\r\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 235);

// ******** End: VertShader.vsh ********

