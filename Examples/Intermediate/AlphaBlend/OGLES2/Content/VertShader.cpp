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
	"attribute highp  vec2  inVertex;\n"
	"\n"
	"uniform highp vec2  LowerLeft;\n"
	"uniform highp mat2  ScaleMatrix; // width/height and screen rotation\n"
	"\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = vec4(ScaleMatrix * inVertex + LowerLeft, 0, 1);\n"
	"\t\n"
	"\tTexCoord = inVertex;\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 271);

// ******** End: VertShader.vsh ********

