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
	"attribute highp   vec3  inVertex;\n"
	"attribute highp   vec3\tinNormal;\n"
	"attribute highp   vec4  inColor;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"\n"
	"varying mediump vec2   TexCoord;\n"
	"varying mediump vec4   Colors;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\n"
	"\t// Pass through texcoords\n"
	"\tTexCoord = inTexCoord;\n"
	"\tColors = inColor;\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 394);

// ******** End: VertShader.vsh ********

