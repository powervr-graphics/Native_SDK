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
	"attribute highp vec3 inVertex;\n"
	"attribute highp vec2 inUVs;\n"
	"\n"
	"uniform mediump mat4 MVPMatrix;\n"
	"\n"
	"varying highp vec2 TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tTexCoord = inUVs;\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"}";

// Register TexColShaderV.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_TexColShaderV_vsh("TexColShaderV.vsh", _TexColShaderV_vsh, 205);

// ******** End: TexColShaderV.vsh ********

