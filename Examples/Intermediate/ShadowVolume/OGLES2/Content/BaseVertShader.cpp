// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BaseVertShader.vsh ********

// File data
static const char _BaseVertShader_vsh[] = 
	"/*\n"
	"  Simple vertex shader:\n"
	"  - standard vertex transformation\n"
	"  - diffuse lighting for one directional light\n"
	"  - texcoord passthrough\n"
	"*/\n"
	"\n"
	"attribute highp   vec3  inVertex;\n"
	"attribute mediump vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform mediump vec3  LightPosModel;\n"
	"\n"
	"varying lowp    float  LightIntensity;\n"
	"varying mediump vec2   TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\t\n"
	"\tmediump vec3 lightDir = normalize(LightPosModel - inVertex);\n"
	"\tLightIntensity = max(0.0, dot(inNormal, lightDir));\n"
	"\t\n"
	"\tTexCoord = inTexCoord;\n"
	"}\n";

// Register BaseVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BaseVertShader_vsh("BaseVertShader.vsh", _BaseVertShader_vsh, 594);

// ******** End: BaseVertShader.vsh ********

