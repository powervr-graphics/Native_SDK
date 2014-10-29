// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SpriteShaderV.vsh ********

// File data
static const char _SpriteShaderV_vsh[] = 
	"attribute highp vec3 inVertex;\n"
	"attribute highp vec2 inUVs;\n"
	"attribute mediump float inTransIdx;\n"
	"attribute lowp vec4\t inRGBA;\n"
	"\n"
	"uniform mediump mat4 MTransforms[30];\n"
	"uniform mediump mat4 MVPMatrix;\n"
	"\n"
	"varying highp vec2 TexCoord;\n"
	"varying lowp vec4 RGBA;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tTexCoord = inUVs;\n"
	"\tRGBA     = inRGBA;\n"
	"\t\n"
	"\tlowp int iTransIdx = int(inTransIdx);\n"
	"\thighp vec4 position = MTransforms[iTransIdx] * vec4(inVertex, 1.0);\n"
	"\t\t\n"
	"\tgl_Position = MVPMatrix * position;\n"
	"}";

// Register SpriteShaderV.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SpriteShaderV_vsh("SpriteShaderV.vsh", _SpriteShaderV_vsh, 454);

// ******** End: SpriteShaderV.vsh ********

