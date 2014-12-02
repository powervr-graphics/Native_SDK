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
	"#define VERTEX_ARRAY 0\r\n"
	"#define NORMAL_ARRAY 1\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in mediump vec3\tinNormal;\r\n"
	"\r\n"
	"uniform highp mat4  MVPMatrix;\t\t// model view projection transformation\r\n"
	"uniform highp vec3  LightDirection;\t// light direction in model space\r\n"
	"uniform highp vec3  EyePosition;\t// eye position in model space\r\n"
	"\r\n"
	"out mediump vec2  TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex,1.0);\r\n"
	"\t\r\n"
	"\tmediump vec3 eyeDirection = normalize(EyePosition - inVertex);\r\n"
	"\t\r\n"
	"\tTexCoord.x = dot(LightDirection, inNormal);\r\n"
	"\tTexCoord.y = dot(eyeDirection, inNormal);\r\n"
	"}\r\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 657);

// ******** End: VertShader.vsh ********

