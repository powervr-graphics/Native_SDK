// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BuildingVertShader.vsh ********

// File data
static const char _BuildingVertShader_vsh[] = 
	"attribute highp   vec3  inVertex;\n"
	"attribute highp   vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp mat4    ModelViewProjMatrix;\n"
	"uniform highp vec3    LightDirection;\n"
	"\n"
	"varying highp   float   vDiffuse;\n"
	"varying mediump vec2    vTexCoord;\n"
	"\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\tvDiffuse = 0.4 + max(dot(inNormal, LightDirection), 0.0) * 0.6;\n"
	"\tvTexCoord = inTexCoord;\n"
	"\tgl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);\t\n"
	"}\n";

// Register BuildingVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BuildingVertShader_vsh("BuildingVertShader.vsh", _BuildingVertShader_vsh, 424);

// ******** End: BuildingVertShader.vsh ********

