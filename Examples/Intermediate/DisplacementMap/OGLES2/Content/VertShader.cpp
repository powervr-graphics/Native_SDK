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
	"attribute highp   vec3  inVertex;\n"
	"attribute mediump vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform mediump vec3  LightDirection;\n"
	"uniform mediump\tfloat  DisplacementFactor;\n"
	"\n"
	"varying lowp    float  LightIntensity;\n"
	"varying mediump vec2   TexCoord;\n"
	"\n"
	"uniform sampler2D  sDisMap;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t/* \n"
	"\t\tCalculate the displacemnt value by taking the colour value from our texture\n"
	"\t\tand scale it by out displacement factor.\n"
	"\t*/\n"
	"\tmediump float disp = texture2D(sDisMap, inTexCoord).r * DisplacementFactor;\n"
	"\n"
	"\t/* \n"
	"\t\tTransform position by the model-view-projection matrix but first\n"
	"\t\tmove the untransformed position along the normal by our displacement\n"
	"\t\tvalue.\n"
	"\t*/\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex + (inNormal * disp), 1.0);\n"
	"\n"
	"\t// Pass through texcoords\n"
	"\tTexCoord = inTexCoord;\n"
	"\t\n"
	"\t// Simple diffuse lighting in model space\n"
	"\tLightIntensity = dot(inNormal, -LightDirection);\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 916);

// ******** End: VertShader.vsh ********

