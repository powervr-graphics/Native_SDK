// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PreBloomVertShader.vsh ********

// File data
static const char _PreBloomVertShader_vsh[] = 
	"attribute highp   vec3  inVertex;\n"
	"attribute mediump vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform mediump vec3  LightDirection;\n"
	"\n"
	"varying mediump vec2   TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\t\n"
	"\n"
	"\t// Use the light intensity as texture coords for the bloom mapping\n"
	"\tTexCoord = vec2(dot(inNormal, -LightDirection), 0);\t\n"
	"}";

// Register PreBloomVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PreBloomVertShader_vsh("PreBloomVertShader.vsh", _PreBloomVertShader_vsh, 421);

// ******** End: PreBloomVertShader.vsh ********

