// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: VertShader.vsh ********

// File data
static const char _VertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define NORMAL_ARRAY\t1\r\n"
	"#define TEXCOORD_ARRAY\t2\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec4\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in highp vec3\tinNormal;\r\n"
	"layout (location = TEXCOORD_ARRAY) in highp vec2\tinTexCoord;\r\n"
	"\r\n"
	"uniform highp mat4  MVPMatrix;\r\n"
	"uniform highp vec3  LightDirection;\r\n"
	"uniform highp vec3  EyePosition;\r\n"
	"\r\n"
	"out mediump float CosViewAngle;\r\n"
	"out mediump float LightIntensity;\r\n"
	"out mediump vec2  TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tgl_Position = MVPMatrix * inVertex;\r\n"
	"\t\r\n"
	"\thighp vec3 eyeDirection = normalize(EyePosition - inVertex.xyz);\r\n"
	"\t\r\n"
	"\t// Simple diffuse lighting \r\n"
	"\tLightIntensity = max(dot(LightDirection, inNormal), 0.0);\r\n"
	"\r\n"
	"\t// Cosine of the angle between surface normal and eye direction\r\n"
	"\t// We clamp at 0.1 to avoid ugly aliasing at near 90\xc2\xb0 angles\r\n"
	"\tCosViewAngle = max(dot(eyeDirection, inNormal), 0.1);\r\n"
	"\t\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 913);

// ******** End: VertShader.vsh ********

