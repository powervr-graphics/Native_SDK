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
	"attribute mediump vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform mediump vec3  LightDirection;\n"
	"\n"
	"varying lowp    float  LightIntensity;\n"
	"varying mediump vec2   TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\t\n"
	"\n"
	"\t// Pass through texcoords\n"
	"\tTexCoord = inTexCoord;\n"
	"\t\n"
	"\t// Simple diffuse lighting in model space\n"
	"\tLightIntensity = dot(inNormal, -LightDirection);\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 484);

// ******** End: VertShader.vsh ********

