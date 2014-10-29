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
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"\r\n"
	"uniform mediump mat4 ModelMatrix;\r\n"
	"uniform mediump mat4 ModelViewMatrix;\r\n"
	"uniform highp mat4 MVPMatrix;\r\n"
	"uniform mediump float WaterHeight;\t\t//Assume water always lies on the y-axis\r\n"
	"#ifdef ENABLE_DISCARD_CLIP\r\n"
	"uniform bool ClipPlaneBool;\r\n"
	"uniform mediump vec4 ClipPlane;\r\n"
	"#endif\r\n"
	"\r\n"
	"out mediump vec3 EyeDir;\r\n"
	"out mediump float VertexHeight;\r\n"
	"#ifdef ENABLE_DISCARD_CLIP\r\n"
	"out highp float ClipDist;\r\n"
	"#endif\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tEyeDir = -inVertex;\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"\t\r\n"
	"\t#ifdef ENABLE_DISCARD_CLIP\r\n"
	"\t\t// Compute the distance between the vertex and clipping plane (in world space coord system)\r\n"
	"\t\tmediump vec4 vVertexView = ModelMatrix * vec4(inVertex.xyz,1.0);\r\n"
	"\t\tClipDist = dot(vVertexView, ClipPlane);\r\n"
	"\t#endif\r\n"
	"\t\r\n"
	"\t// Calculate the vertex's distance ABOVE water surface.\r\n"
	"\tmediump float vVertexHeight = (ModelMatrix * vec4(inVertex,1.0)).y;\r\n"
	"\tVertexHeight = vVertexHeight - WaterHeight;\r\n"
	"}\r\n";

// Register SkyboxVShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SkyboxVShader_vsh("SkyboxVShader.vsh", _SkyboxVShader_vsh, 1024);

// ******** End: SkyboxVShader.vsh ********

