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
	"attribute highp vec3 inVertex;\n"
	"attribute highp vec3 inNormal;\n"
	"\n"
	"uniform highp mat4  uModelViewMatrix;\n"
	"uniform highp mat3  uModelViewITMatrix;\n"
	"uniform highp mat4  uModelViewProjectionMatrix;\n"
	"\n"
	"uniform highp vec3  uLightPosition;\n"
	"\n"
	"varying highp vec3  vNormal;\n"
	"varying highp vec3  vLightDirection;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = uModelViewProjectionMatrix * vec4(inVertex, 1.0);\n"
	"\tvNormal = uModelViewITMatrix * inNormal;\n"
	"\n"
	"\thighp vec3 position = (uModelViewMatrix * vec4(inVertex, 1.0)).xyz;\n"
	"\tvLightDirection = uLightPosition - position;\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 533);

// ******** End: VertShader.vsh ********

