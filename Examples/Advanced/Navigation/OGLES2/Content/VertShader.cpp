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
	"attribute highp vec3  inVertex;\n"
	"\n"
	"uniform highp mat4    ModelViewProjMatrix;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Convert each vertex into projection-space and output the value\n"
	"\tgl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);\t\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 219);

// ******** End: VertShader.vsh ********

