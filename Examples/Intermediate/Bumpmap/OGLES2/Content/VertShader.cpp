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
	"attribute highp vec3  inTangent;\n"
	"\n"
	"uniform highp mat4  MVPMatrix;\t\t// model view projection transformation\n"
	"uniform highp vec3  LightPosModel;\t// Light position (point light) in model space\n"
	"\n"
	"varying lowp vec3  LightVec;\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * inVertex;\n"
	"\t\n"
	"\t// Calculate light direction from light position in model space\n"
	"\t// You can skip this step for directional lights\n"
	"\thighp vec3 lightDirection = normalize(LightPosModel - vec3(inVertex));\n"
	"\t\n"
	"\t// transform light direction from model space to tangent space\n"
	"\thighp vec3 bitangent = cross(inNormal, inTangent);\n"
	"\thighp mat3 tangentSpaceXform = mat3(inTangent, bitangent, inNormal);\n"
	"\tLightVec = lightDirection * tangentSpaceXform;\n"
	"\t\n"
	"\tTexCoord = inTexCoord;\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 876);

// ******** End: VertShader.vsh ********

