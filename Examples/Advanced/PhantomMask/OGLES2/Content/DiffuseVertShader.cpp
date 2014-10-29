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
	"attribute highp   vec3  inVertex;\n"
	"attribute mediump vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform highp   mat3  Model;\n"
	"\n"
	"// Precalculated constants used for lighting\n"
	"uniform mediump   vec3  LightDir1;\n"
	"uniform mediump   vec3  LightDir2;\n"
	"uniform mediump   vec3  LightDir3;\n"
	"uniform mediump   vec3  LightDir4;\n"
	"uniform mediump   vec4  Ambient;\n"
	"\n"
	"// varyings\n"
	"varying lowp    vec4  LightColour;\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\thighp vec4 r1;\n"
	"\thighp vec3 norm, r2, r3;\n"
	"\t\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\n"
	"\t// Transform the Normal\n"
	"\tnorm = normalize(Model * inNormal);\n"
	"\n"
	"\t// compute lighting\n"
	"\tr1.x =\tmax(0.0, dot(norm, LightDir1));\t// White Light\n"
	"\tr1.y =\tmax(0.0, dot(norm, LightDir2));\t// Blue Light\n"
	"\tr1.z =\tmax(0.0, dot(norm, LightDir3));\t// Green Light\n"
	"\tr1.w =\tmax(0.0, dot(norm, LightDir4));\t// Red Light\n"
	"\n"
	"\tLightColour.r = (r1.x + r1.w) + Ambient.r; // White Light (BGRA)\n"
	"\tLightColour.g = (r1.x + r1.z) + Ambient.g; // Red Light (BGRA)\n"
	"\tLightColour.b = (r1.x + r1.y) + Ambient.b; // Green Light (BGRA)\n"
	"\tLightColour.a = r1.x + Ambient.a; // Blue Light (BGRA)\n"
	"\n"
	"\t// Pass through texcoords\n"
	"\tTexCoord = inTexCoord;\n"
	"}";

// Register DiffuseVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_DiffuseVertShader_vsh("DiffuseVertShader.vsh", _DiffuseVertShader_vsh, 1204);

// ******** End: DiffuseVertShader.vsh ********

