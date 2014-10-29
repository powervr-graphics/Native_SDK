// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: Tex2DVShader.vsh ********

// File data
static const char _Tex2DVShader_vsh[] = 
	"attribute highp vec3 inVertex;\n"
	"attribute highp vec2 inTexCoord;\n"
	"\n"
	"\n"
	"uniform mediump mat4 MVPMatrix;\n"
	"\n"
	"varying highp vec2 TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tTexCoord = inTexCoord;\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"}";

// Register Tex2DVShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_Tex2DVShader_vsh("Tex2DVShader.vsh", _Tex2DVShader_vsh, 216);

// ******** End: Tex2DVShader.vsh ********

