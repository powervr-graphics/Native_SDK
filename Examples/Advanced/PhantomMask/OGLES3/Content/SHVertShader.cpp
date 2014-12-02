// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SHVertShader.vsh ********

// File data
static const char _SHVertShader_vsh[] = 
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
	"uniform highp   vec4  cAr;\r\n"
	"uniform highp   vec4  cAg;\r\n"
	"uniform highp   vec4  cAb;\r\n"
	"\r\n"
	"uniform highp\tvec4  cBr;\r\n"
	"uniform highp\tvec4  cBg;\r\n"
	"uniform highp\tvec4  cBb;\r\n"
	"\r\n"
	"uniform highp\tvec3  cC;\r\n"
	"\r\n"
	"// varyings\r\n"
	"out lowp    vec4  LightColour;\r\n"
	"out mediump vec2  TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\thighp vec4 r0, r1;\r\n"
	"\thighp vec3 r2, r3;\r\n"
	"\t\r\n"
	"\t// Transform position\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"\r\n"
	"\t// Transform the Normal and add a homogenous 1\r\n"
	"\tr0 = vec4(Model * inNormal, 1.0);\r\n"
	"\r\n"
	"\t// Compute 1st 4 basis functions - linear + constant\r\n"
	"\t// r0 is the normal with a homegenous 1\r\n"
	"\t// c* are precomputed constants\r\n"
	"\tr2.x = dot(cAr, r0);\r\n"
	"\tr2.y = dot(cAg, r0);\r\n"
	"\tr2.z = dot(cAb, r0);\r\n"
	"\t\r\n"
	"\t// Compute polynomials for the next 4 basis functions\r\n"
	"\tr1 = r0.yzzx * r0.xyzz; // r1 is { yx, zy, z^2, xz}\r\n"
	"\r\n"
	"\t// Add contributions and store them in r3\r\n"
	"\tr3.x = dot(cBr, r1);\r\n"
	"\tr3.y = dot(cBg, r1);\r\n"
	"\tr3.z = dot(cBb, r1);\r\n"
	"\r\n"
	"\t// Compute the final basis function x^2 - y^2\r\n"
	"\tr0.z = r0.y * r0.y;\r\n"
	"\tr0.w = (r0.x * r0.x) - r0.z;\r\n"
	"\t\r\n"
	"\t// Combine the first 2 sets : 8 basis functions\r\n"
	"\tr1.xyz = r2 + r3;\r\n"
	"\r\n"
	"\t// Add in the final 9th basis function to create the final RGB Lighting\r\n"
	"\tLightColour.xyz = (cC * r0.w) + r1.xyz;\r\n"
	"\t\r\n"
	"\t// Set light alpha to 1.0\r\n"
	"\tLightColour.a = 1.0;\r\n"
	"\r\n"
	"\t// Pass through texcoords\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}";

// Register SHVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SHVertShader_vsh("SHVertShader.vsh", _SHVertShader_vsh, 1715);

// ******** End: SHVertShader.vsh ********

