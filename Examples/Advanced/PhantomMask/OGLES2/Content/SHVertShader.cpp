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
	"attribute highp   vec3  inVertex;\n"
	"attribute mediump vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform highp   mat3  Model;\n"
	"\n"
	"// Precalculated constants used for lighting\n"
	"uniform highp   vec4  cAr;\n"
	"uniform highp   vec4  cAg;\n"
	"uniform highp   vec4  cAb;\n"
	"\n"
	"uniform highp\tvec4  cBr;\n"
	"uniform highp\tvec4  cBg;\n"
	"uniform highp\tvec4  cBb;\n"
	"\n"
	"uniform highp\tvec3  cC;\n"
	"\n"
	"// varyings\n"
	"varying lowp    vec4  LightColour;\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\thighp vec4 r0, r1;\n"
	"\thighp vec3 r2, r3;\n"
	"\t\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\n"
	"\t// Transform the Normal and add a homogenous 1\n"
	"\tr0 = vec4(Model * inNormal, 1.0);\n"
	"\n"
	"\t// Compute 1st 4 basis functions - linear + constant\n"
	"\t// r0 is the normal with a homegenous 1\n"
	"\t// c* are precomputed constants\n"
	"\tr2.x = dot(cAr, r0);\n"
	"\tr2.y = dot(cAg, r0);\n"
	"\tr2.z = dot(cAb, r0);\n"
	"\t\n"
	"\t// Compute polynomials for the next 4 basis functions\n"
	"\tr1 = r0.yzzx * r0.xyzz; // r1 is { yx, zy, z^2, xz}\n"
	"\n"
	"\t// Add contributions and store them in r3\n"
	"\tr3.x = dot(cBr, r1);\n"
	"\tr3.y = dot(cBg, r1);\n"
	"\tr3.z = dot(cBb, r1);\n"
	"\n"
	"\t// Compute the final basis function x^2 - y^2\n"
	"\tr0.z = r0.y * r0.y;\n"
	"\tr0.w = (r0.x * r0.x) - r0.z;\n"
	"\t\n"
	"\t// Combine the first 2 sets : 8 basis functions\n"
	"\tr1.xyz = r2 + r3;\n"
	"\n"
	"\t// Add in the final 9th basis function to create the final RGB Lighting\n"
	"\tLightColour.xyz = (cC * r0.w) + r1.xyz;\n"
	"\t\n"
	"\t// Set light alpha to 1.0\n"
	"\tLightColour.a = 1.0;\n"
	"\n"
	"\t// Pass through texcoords\n"
	"\tTexCoord = inTexCoord;\n"
	"}";

// Register SHVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SHVertShader_vsh("SHVertShader.vsh", _SHVertShader_vsh, 1490);

// ******** End: SHVertShader.vsh ********

