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
	"attribute highp vec3  inVertex;\n"
	"\n"
	"uniform highp mat4  ModelViewMatrix;\n"
	"uniform highp mat4  MVPMatrix;\n"
	"uniform highp vec3  EyePosition;\t\t// Eye (aka Camera) positon in model-space\n"
	"uniform mediump vec2 BumpTranslation0;\n"
	"uniform mediump vec2 BumpScale0;\n"
	"uniform mediump vec2 BumpTranslation1;\n"
	"uniform mediump vec2 BumpScale1;\n"
	" \n"
	"varying mediump vec2 BumpCoord0;\n"
	"varying mediump vec2 BumpCoord1;\n"
	"varying highp   vec3\tWaterToEye;\n"
	"varying mediump float\tWaterToEyeLength;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Convert each vertex into projection-space and output the value\n"
	"\thighp vec4 vInVertex = vec4(inVertex, 1.0);\n"
	"\tgl_Position = MVPMatrix * vInVertex;\n"
	"\n"
	"\t// The texture coordinate is calculated this way to reduce the number of attributes needed\n"
	"\tmediump vec2 vTexCoord = inVertex.xz;\n"
	"\n"
	"\t// Scale and translate texture coordinates used to sample the normal map - section 2.2 of white paper\n"
	"\tBumpCoord0 = vTexCoord.xy * BumpScale0;\n"
	"\tBumpCoord0 += BumpTranslation0;\n"
	"\t\n"
	"\tBumpCoord1 = vTexCoord.xy * BumpScale1;\n"
	"\tBumpCoord1 += BumpTranslation1;\n"
	"\t\n"
	"\t/* \t\n"
	"\t\tThe water to eye vector is used to calculate the Fresnel term\n"
	"\t\tand to fade out perturbations based on distance from the viewer\n"
	"\t*/\n"
	"\tWaterToEye = EyePosition - inVertex;\n"
	"\tWaterToEyeLength = length(WaterToEye);\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1236);

// ******** End: VertShader.vsh ********

