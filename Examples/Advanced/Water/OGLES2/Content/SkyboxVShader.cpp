// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SkyboxVShader.vsh ********

// File data
static const char _SkyboxVShader_vsh[] = 
	"attribute mediump vec3 inVertex;\n"
	"\n"
	"uniform mediump mat4 ModelMatrix;\n"
	"uniform mediump mat4 ModelViewMatrix;\n"
	"uniform highp mat4 MVPMatrix;\n"
	"uniform mediump float WaterHeight;\t\t//Assume water always lies on the y-axis\n"
	"#ifdef ENABLE_DISCARD_CLIP\n"
	"uniform bool ClipPlaneBool;\n"
	"uniform mediump vec4 ClipPlane;\n"
	"#endif\n"
	"\n"
	"varying mediump vec3 EyeDir;\n"
	"varying mediump float VertexHeight;\n"
	"#ifdef ENABLE_DISCARD_CLIP\n"
	"varying highp float ClipDist;\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tEyeDir = -inVertex;\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\t\n"
	"\t#ifdef ENABLE_DISCARD_CLIP\n"
	"\t\t// Compute the distance between the vertex and clipping plane (in world space coord system)\n"
	"\t\tmediump vec4 vVertexView = ModelMatrix * vec4(inVertex.xyz,1.0);\n"
	"\t\tClipDist = dot(vVertexView, ClipPlane);\n"
	"\t#endif\n"
	"\t\n"
	"\t// Calculate the vertex's distance ABOVE water surface.\n"
	"\tmediump float vVertexHeight = (ModelMatrix * vec4(inVertex,1.0)).y;\n"
	"\tVertexHeight = vVertexHeight - WaterHeight;\n"
	"}";

// Register SkyboxVShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SkyboxVShader_vsh("SkyboxVShader.vsh", _SkyboxVShader_vsh, 936);

// ******** End: SkyboxVShader.vsh ********

