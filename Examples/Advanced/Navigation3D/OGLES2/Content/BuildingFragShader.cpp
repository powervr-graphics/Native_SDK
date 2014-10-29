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
	"uniform lowp sampler2D sTexture;\n"
	"uniform lowp vec4   FlatColour;\n"
	"\n"
	"varying highp vec2  vTexCoord;\n"
	"varying highp float vDiffuse;\n"
	"\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\tlowp vec4 colour = texture2D(sTexture, vTexCoord);\n"
	"\tgl_FragColor.rgb = colour.rgb * vDiffuse;\t\n"
	"\tgl_FragColor.a = colour.a;\t\n"
	"}\n";

// Register BuildingFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BuildingFragShader_fsh("BuildingFragShader.fsh", _BuildingFragShader_fsh, 271);

// ******** End: BuildingFragShader.fsh ********

