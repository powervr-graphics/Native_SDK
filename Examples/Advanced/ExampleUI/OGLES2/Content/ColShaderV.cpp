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
	"attribute highp vec3 inVertex;\n"
	"\n"
	"uniform mediump mat4 MVPMatrix;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"}";

// Register ColShaderV.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ColShaderV_vsh("ColShaderV.vsh", _ColShaderV_vsh, 128);

// ******** End: ColShaderV.vsh ********

