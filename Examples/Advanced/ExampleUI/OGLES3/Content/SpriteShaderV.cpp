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
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define TEXCOORD_ARRAY\t1\r\n"
	"#define TRANSFORM_ARRAY\t2\r\n"
	"#define RGBA_ARRAY\t\t3\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3 inVertex;\r\n"
	"layout (location = TEXCOORD_ARRAY) in highp vec2 inUVs;\r\n"
	"layout (location = TRANSFORM_ARRAY) in mediump float inTransIdx;\r\n"
	"layout (location = RGBA_ARRAY) in lowp vec4\tinRGBA;\r\n"
	"\r\n"
	"uniform mediump mat4 MTransforms[30];\r\n"
	"uniform mediump mat4 MVPMatrix;\r\n"
	"\r\n"
	"out highp vec2 TexCoord;\r\n"
	"out lowp vec4 RGBA;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tTexCoord = inUVs;\r\n"
	"\tRGBA     = inRGBA;\r\n"
	"\t\r\n"
	"\tlowp int iTransIdx = int(inTransIdx);\r\n"
	"\thighp vec4 position = MTransforms[iTransIdx] * vec4(inVertex, 1.0);\r\n"
	"\t\t\r\n"
	"\tgl_Position = MVPMatrix * position;\r\n"
	"}";

// Register SpriteShaderV.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SpriteShaderV_vsh("SpriteShaderV.vsh", _SpriteShaderV_vsh, 693);

// ******** End: SpriteShaderV.vsh ********

