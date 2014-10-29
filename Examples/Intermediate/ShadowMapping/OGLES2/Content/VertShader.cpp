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
	"attribute highp vec3  inNormal;\n"
	"attribute highp vec2  inTexCoord;\n"
	"\n"
	"uniform highp mat4  ModelViewMatrix;\n"
	"uniform highp mat4  ProjectionMatrix;\n"
	"varying highp vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = ProjectionMatrix * ModelViewMatrix * vec4(inVertex, 1.0);\n"
	"\tTexCoord = inTexCoord;\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 317);

// ******** End: VertShader.vsh ********

