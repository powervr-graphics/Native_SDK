// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: AntiAliasedLinesVertShader.vsh ********

// File data
static const char _AntiAliasedLinesVertShader_vsh[] = 
	"attribute highp   vec3 inVertex;\n"
	"attribute mediump vec2 inTexCoord;\n"
	"\n"
	"varying mediump  vec2 TexCoord;\n"
	"\n"
	"uniform highp mat4 ModelViewProjMatrix;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Convert each vertex into projection-space and output the value\n"
	"\tgl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);\t\n"
	"\tTexCoord = inTexCoord;\n"
	"}\n";

// Register AntiAliasedLinesVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_AntiAliasedLinesVertShader_vsh("AntiAliasedLinesVertShader.vsh", _AntiAliasedLinesVertShader_vsh, 309);

// ******** End: AntiAliasedLinesVertShader.vsh ********

