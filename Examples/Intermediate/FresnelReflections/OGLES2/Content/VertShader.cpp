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
	"attribute highp vec4  inVertex;\n"
	"attribute highp vec3  inNormal;\n"
	"attribute highp vec2  inTexCoord;\n"
	"\n"
	"uniform highp mat4   MVPMatrix;\n"
	"uniform highp vec3   EyePosition;\n"
	"uniform highp float  RIRSquare;\n"
	"\n"
	"varying mediump vec2   ReflectCoord;\n"
	"varying mediump vec2   TexCoord;\n"
	"varying lowp    float  ReflectRatio;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * inVertex;\n"
	"\t\n"
	"\t// Calculate direction from vertex to eye (model space)\n"
	"\thighp vec3 eyeDir = normalize(EyePosition - inVertex.xyz);\n"
	"\t\n"
	"\t// The reflection intensity depends on the angle between eye direction and\n"
	"\t// surface normal.\n"
	"\t// The relative index of refraction (RIR) is a material parameter\n"
	"\thighp float c = abs(dot(eyeDir, inNormal));\n"
	"\thighp float g = sqrt(RIRSquare + c * c - 1.0);\n"
	"\thighp float f1 = (g - c) / (g + c);\n"
	"\thighp float f2 = (c * (g + c) - 1.0) / (c * (g - c) + 1.0);\n"
	"\tReflectRatio = 0.5 * f1 * f1 * (1.0 + f2 * f2);\n"
	"\t\n"
	"\t// map reflection vector to 2D\n"
	"\tReflectCoord = normalize(reflect(eyeDir, inNormal)).xy * 0.5;\n"
	"\t\n"
	"\tTexCoord = inTexCoord;\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1029);

// ******** End: VertShader.vsh ********

