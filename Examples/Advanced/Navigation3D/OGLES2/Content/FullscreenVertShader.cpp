// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FullscreenVertShader.vsh ********

// File data
static const char _FullscreenVertShader_vsh[] = 
	"attribute highp vec2  inVertex;\n"
	"uniform   lowp  vec4  FlatColour;\n"
	"varying   lowp  vec4  vColour;\n"
	"\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = vec4(inVertex, 0.0, 1.0);\n"
	"\tvColour = FlatColour;\n"
	"}\n";

// Register FullscreenVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FullscreenVertShader_vsh("FullscreenVertShader.vsh", _FullscreenVertShader_vsh, 179);

// ******** End: FullscreenVertShader.vsh ********

