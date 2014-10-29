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
	"attribute highp   vec2  inVertex;\n"
	"attribute mediump vec2  inWordIndex;\n"
	"attribute mediump vec2  inTexCoords;\n"
	"\n"
	"// inWordIndex: { horizontal multiplier | vertical muliplier }\n"
	"\n"
	"varying mediump vec2    TexCoord;\n"
	"\n"
	"uniform highp   mat4    ModelViewProjMatrix;\n"
	"uniform mediump vec3    PivotDirection;\n"
	"uniform mediump vec3    Up;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Span a quad depending on the texture coordinates and the camera's up and right vector\t\t\n"
	"\t\n"
	"\t// Convert each vertex into projection-space and output the value\n"
	"\tmediump vec3 offset = PivotDirection * inWordIndex.x + Up * inWordIndex.y;\t\t\n"
	"\t\n"
	"\t// Pass the texcoords\n"
	"\tTexCoord = inTexCoords;\n"
	"\t\n"
	"\t// Calculate the world position of the vertex\n"
	"\thighp vec4 vInVertex = vec4(vec3(inVertex, 0.0) + offset, 1.0);\t\n"
	"\t\t\n"
	"\t// Transform the vertex\n"
	"\tgl_Position = ModelViewProjMatrix * vInVertex;\t\n"
	"}\n";

// Register PivotQuadVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PivotQuadVertShader_vsh("PivotQuadVertShader.vsh", _PivotQuadVertShader_vsh, 819);

// ******** End: PivotQuadVertShader.vsh ********

