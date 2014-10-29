// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BuildingFragShader.fsh ********

// File data
static const char _BuildingFragShader_fsh[] = 
	"#version 300 es\r\n"
	"uniform lowp sampler2D sTexture;\r\n"
	"uniform lowp vec4   FlatColour;\r\n"
	"\r\n"
	"in highp vec2  vTexCoord;\r\n"
	"in highp float vDiffuse;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\t\r\n"
	"\tlowp vec4 colour = texture(sTexture, vTexCoord);\r\n"
	"\toColour.rgb = colour.rgb * vDiffuse;\t\r\n"
	"\toColour.a = colour.a;\t\r\n"
	"}\r\n";

// Register BuildingFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BuildingFragShader_fsh("BuildingFragShader.fsh", _BuildingFragShader_fsh, 325);

// ******** End: BuildingFragShader.fsh ********

