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
	"attribute highp vec4  inVertex;\n"
	"attribute highp vec3  inNormal;\n"
	"attribute highp vec2  inTexCoord;\n"
	"\n"
	"uniform highp mat4   MVPMatrix;\n"
	"uniform highp vec3   LightDirection;\n"
	"uniform highp float  MaterialBias;\n"
	"uniform highp float  MaterialScale;\n"
	"\n"
	"varying lowp vec3  DiffuseLight;\n"
	"varying lowp vec3  SpecularLight;\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = MVPMatrix * inVertex;\n"
	"\t\n"
	"\tDiffuseLight = vec3(max(dot(inNormal, LightDirection), 0.0));\n"
	"\tSpecularLight = vec3(max((DiffuseLight.x - MaterialBias) * MaterialScale, 0.0));\n"
	"\t\n"
	"\tTexCoord = inTexCoord;\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 566);

// ******** End: VertShader.vsh ********

