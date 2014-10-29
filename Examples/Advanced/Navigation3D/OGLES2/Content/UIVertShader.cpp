// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: UIVertShader.vsh ********

// File data
static const char _UIVertShader_vsh[] = 
	"attribute highp   vec2  inVertex;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform   highp   mat2  RotationMatrix;\n"
	"\n"
	"varying   mediump vec2  vTexCoord;\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\thighp vec2 vertex = RotationMatrix * inVertex;\n"
	"\tgl_Position = vec4(vertex, 0.0, 1.0);\t\n"
	"\tvTexCoord = inTexCoord;\n"
	"}\n";

// Register UIVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_UIVertShader_vsh("UIVertShader.vsh", _UIVertShader_vsh, 278);

// ******** End: UIVertShader.vsh ********

