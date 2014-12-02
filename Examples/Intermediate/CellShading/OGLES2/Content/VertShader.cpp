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
	"\n"
	"uniform highp mat4  MVPMatrix;\t\t// model view projection transformation\n"
	"uniform highp vec3  LightDirection;\t// light direction in model space\n"
	"uniform highp vec3  EyePosition;\t// eye position in model space\n"
	"\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex,1.0);\n"
	"\t\n"
	"\tmediump vec3 eyeDirection = normalize(EyePosition - inVertex);\n"
	"\t\n"
	"\tTexCoord.x = dot(LightDirection, inNormal);\n"
	"\tTexCoord.y = dot(eyeDirection, inNormal);\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 528);

// ******** End: VertShader.vsh ********

