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
	"layout (location = 0) in highp vec3\tinVertex;\r\n"
	"layout (location = 1) in mediump vec3 inNormal;\r\n"
	"layout (location = 2) in mediump vec2 inTexCoord;\r\n"
	"layout (location = 3) in mediump vec3 inTangent;\r\n"
	"\r\n"
	"uniform highp   mat4  MVPMatrix;\r\n"
	"uniform mediump vec3  EyePosModel;\r\n"
	"\r\n"
	"out mediump vec3  EyeDirection;\r\n"
	"out mediump vec2  TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Transform position\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex,1.0);\r\n"
	"\t\r\n"
	"\t// Calculate direction from eye position in model space\r\n"
	"\tmediump vec3 eyeDirModel = normalize(EyePosModel - inVertex);\r\n"
	"\t\t\t\r\n"
	"\t// transform light direction from model space to tangent space\r\n"
	"\tmediump vec3 binormal = cross(inNormal, inTangent);\r\n"
	"\tmediump mat3 tangentSpaceXform = mat3(inTangent, binormal, inNormal);\r\n"
	"\tEyeDirection = eyeDirModel * tangentSpaceXform;\t\r\n"
	"\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}\r\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 844);

// ******** End: VertShader.vsh ********

