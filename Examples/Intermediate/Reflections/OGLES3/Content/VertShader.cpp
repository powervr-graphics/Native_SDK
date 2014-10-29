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
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in mediump vec3 inNormal;\r\n"
	"\r\n"
	"uniform highp   mat4  MVPMatrix;\r\n"
	"uniform mediump mat3  ModelWorld;\r\n"
	"uniform mediump vec3  EyePosModel;\r\n"
	"\r\n"
	"out mediump vec3  ReflectDir;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Transform position\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"\t\r\n"
	"\t// Calculate eye direction in model space\r\n"
	"\tmediump vec3 eyeDir = normalize(inVertex - EyePosModel);\r\n"
	"\t\r\n"
	"\t// reflect eye direction over normal and transform to world space\r\n"
	"\tReflectDir = ModelWorld * reflect(eyeDir, inNormal);\r\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 651);

// ******** End: VertShader.vsh ********

