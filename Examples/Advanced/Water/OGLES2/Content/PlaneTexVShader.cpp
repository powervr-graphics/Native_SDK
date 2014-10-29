// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PlaneTexVShader.vsh ********

// File data
static const char _PlaneTexVShader_vsh[] = 
	"attribute highp vec3  inVertex;\n"
	"\n"
	"uniform highp mat4  MVPMatrix;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Convert each vertex into projection-space and output the value\n"
	"\thighp vec4 vInVertex = vec4(inVertex, 1.0);\n"
	"\tgl_Position = MVPMatrix * vInVertex;\n"
	"}\n";

// Register PlaneTexVShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PlaneTexVShader_vsh("PlaneTexVShader.vsh", _PlaneTexVShader_vsh, 231);

// ******** End: PlaneTexVShader.vsh ********

