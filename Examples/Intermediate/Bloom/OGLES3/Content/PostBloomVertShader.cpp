// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PostBloomVertShader.vsh ********

// File data
static const char _PostBloomVertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define AXIS_ALIGNED_QUAD_VERTEX_ARRAY\t0\r\n"
	"#define AXIS_ALIGNED_QUAD_TEXCOORD_ARRAY\t1\r\n"
	"\r\n"
	"layout (location = AXIS_ALIGNED_QUAD_VERTEX_ARRAY) in highp vec2\tinVertex;\r\n"
	"layout (location = AXIS_ALIGNED_QUAD_TEXCOORD_ARRAY) in mediump vec2\tinTexCoord;\r\n"
	"\r\n"
	"out mediump vec2   TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"    // Pass through vertex\r\n"
	"\tgl_Position = vec4(inVertex, 0.0, 1.0);\r\n"
	"\t\r\n"
	"\t// Pass through texcoords\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}\r\n";

// Register PostBloomVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PostBloomVertShader_vsh("PostBloomVertShader.vsh", _PostBloomVertShader_vsh, 444);

// ******** End: PostBloomVertShader.vsh ********

