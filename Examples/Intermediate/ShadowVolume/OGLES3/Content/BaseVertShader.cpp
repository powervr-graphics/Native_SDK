// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BaseVertShader.vsh ********

// File data
static const char _BaseVertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"/*\r\n"
	"  Simple vertex shader:\r\n"
	"  - standard vertex transformation\r\n"
	"  - diffuse lighting for one directional light\r\n"
	"  - texcoord passthrough\r\n"
	"*/\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define NORMAL_ARRAY\t1\r\n"
	"#define TEXCOORD_ARRAY\t2\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in mediump vec3\tinNormal;\r\n"
	"layout (location = TEXCOORD_ARRAY) in mediump vec2\tinTexCoord;\r\n"
	"\r\n"
	"uniform highp   mat4  MVPMatrix;\r\n"
	"uniform mediump vec3  LightPosModel;\r\n"
	"\r\n"
	"out lowp    float  LightIntensity;\r\n"
	"out mediump vec2   TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"\t\r\n"
	"\tmediump vec3 lightDir = normalize(LightPosModel - inVertex);\r\n"
	"\tLightIntensity = max(0.0, dot(inNormal, lightDir));\r\n"
	"\t\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}\r\n";

// Register BaseVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BaseVertShader_vsh("BaseVertShader.vsh", _BaseVertShader_vsh, 782);

// ******** End: BaseVertShader.vsh ********

