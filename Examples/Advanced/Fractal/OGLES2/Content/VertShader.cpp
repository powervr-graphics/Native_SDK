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
	"attribute highp   vec4  inVertex;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform mediump mat4  MVPMatrix;\n"
	"\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = MVPMatrix * inVertex;\n"
	"\tTexCoord = inTexCoord;\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 215);

// ******** End: VertShader.vsh ********

