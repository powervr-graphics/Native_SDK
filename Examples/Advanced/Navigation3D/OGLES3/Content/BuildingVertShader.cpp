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
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define NORMAL_ARRAY\t1\r\n"
	"#define TEXCOORD_ARRAY\t2\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in highp vec3\tinNormal;\r\n"
	"layout (location = TEXCOORD_ARRAY) in mediump vec2\tinTexCoord;\r\n"
	"\r\n"
	"uniform highp mat4    ModelViewProjMatrix;\r\n"
	"uniform highp vec3    LightDirection;\r\n"
	"\r\n"
	"out highp   float   vDiffuse;\r\n"
	"out mediump vec2    vTexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\t\r\n"
	"\tvDiffuse = 0.4 + max(dot(inNormal, LightDirection), 0.0) * 0.6;\r\n"
	"\tvTexCoord = inTexCoord;\r\n"
	"\tgl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);\t\r\n"
	"}\r\n";

// Register BuildingVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BuildingVertShader_vsh("BuildingVertShader.vsh", _BuildingVertShader_vsh, 599);

// ******** End: BuildingVertShader.vsh ********

