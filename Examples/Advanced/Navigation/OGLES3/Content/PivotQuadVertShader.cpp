// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PivotQuadVertShader.vsh ********

// File data
static const char _PivotQuadVertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define WORDINDEX_ARRAY\t1\r\n"
	"#define ATTRIB_ARRAY\t2\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec2\tinVertex;\r\n"
	"layout (location = WORDINDEX_ARRAY) in mediump vec2\tinWordIndex;\r\n"
	"layout (location = ATTRIB_ARRAY) in mediump vec2\tinTexCoords;\r\n"
	"\r\n"
	"// inWordIndex: { horizontal multiplier | vertical muliplier }\r\n"
	"\r\n"
	"out mediump vec2    TexCoord;\r\n"
	"\r\n"
	"uniform highp   mat4    ModelViewProjMatrix;\r\n"
	"uniform mediump vec3    PivotDirection;\r\n"
	"uniform mediump vec3    Up;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Span a quad depending on the texture coordinates and the camera's up and right vector\t\t\r\n"
	"\t\r\n"
	"\t// Convert each vertex into projection-space and output the value\r\n"
	"\tmediump vec3 offset = PivotDirection * inWordIndex.x + Up * inWordIndex.y;\t\t\r\n"
	"\t\r\n"
	"\t// Pass the texcoords\r\n"
	"\tTexCoord = inTexCoords;\r\n"
	"\t\r\n"
	"\t// Calculate the world position of the vertex\r\n"
	"\thighp vec4 vInVertex = vec4(vec3(inVertex, 0.0) + offset, 1.0);\t\r\n"
	"\t\t\r\n"
	"\t// Transform the vertex\r\n"
	"\tgl_Position = ModelViewProjMatrix * vInVertex;\t\r\n"
	"}\r\n";

// Register PivotQuadVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PivotQuadVertShader_vsh("PivotQuadVertShader.vsh", _PivotQuadVertShader_vsh, 1015);

// ******** End: PivotQuadVertShader.vsh ********

