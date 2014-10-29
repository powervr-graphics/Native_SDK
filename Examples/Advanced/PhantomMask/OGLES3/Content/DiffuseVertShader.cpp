// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: DiffuseVertShader.vsh ********

// File data
static const char _DiffuseVertShader_vsh[] = 
	"#version 300 es\r\n"
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
	"uniform highp   mat3  Model;\r\n"
	"\r\n"
	"// Precalculated constants used for lighting\r\n"
	"uniform mediump   vec3  LightDir1;\r\n"
	"uniform mediump   vec3  LightDir2;\r\n"
	"uniform mediump   vec3  LightDir3;\r\n"
	"uniform mediump   vec3  LightDir4;\r\n"
	"uniform mediump   vec4  Ambient;\r\n"
	"\r\n"
	"// varyings\r\n"
	"out lowp    vec4  LightColour;\r\n"
	"out mediump vec2  TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\thighp vec4 r1;\r\n"
	"\thighp vec3 norm, r2, r3;\r\n"
	"\t\r\n"
	"\t// Transform position\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"\r\n"
	"\t// Transform the Normal\r\n"
	"\tnorm = normalize(Model * inNormal);\r\n"
	"\r\n"
	"\t// compute lighting\r\n"
	"\tr1.x =\tmax(0.0, dot(norm, LightDir1));\t// White Light\r\n"
	"\tr1.y =\tmax(0.0, dot(norm, LightDir2));\t// Blue Light\r\n"
	"\tr1.z =\tmax(0.0, dot(norm, LightDir3));\t// Green Light\r\n"
	"\tr1.w =\tmax(0.0, dot(norm, LightDir4));\t// Red Light\r\n"
	"\r\n"
	"\tLightColour.r = (r1.x + r1.w) + Ambient.r; // White Light (BGRA)\r\n"
	"\tLightColour.g = (r1.x + r1.z) + Ambient.g; // Red Light (BGRA)\r\n"
	"\tLightColour.b = (r1.x + r1.y) + Ambient.b; // Green Light (BGRA)\r\n"
	"\tLightColour.a = r1.x + Ambient.a; // Blue Light (BGRA)\r\n"
	"\r\n"
	"\t// Pass through texcoords\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}";

// Register DiffuseVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_DiffuseVertShader_vsh("DiffuseVertShader.vsh", _DiffuseVertShader_vsh, 1408);

// ******** End: DiffuseVertShader.vsh ********

