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
	"#define TEXCOORD_ARRAY\t2\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in mediump vec3\tinNormal;\r\n"
	"layout (location = TEXCOORD_ARRAY) in mediump vec2\tinTexCoord;\r\n"
	"\r\n"
	"uniform highp   mat4  MVPMatrix;\r\n"
	"uniform mediump vec3  LightDirection;\r\n"
	"uniform mediump\tfloat  DisplacementFactor;\r\n"
	"\r\n"
	"out lowp    float  LightIntensity;\r\n"
	"out mediump vec2   TexCoord;\r\n"
	"\r\n"
	"uniform sampler2D  sDisMap;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t/* \r\n"
	"\t\tCalculate the displacemnt value by taking the colour value from our texture\r\n"
	"\t\tand scale it by out displacement factor.\r\n"
	"\t*/\r\n"
	"\tmediump float disp = texture(sDisMap, inTexCoord).r * DisplacementFactor;\r\n"
	"\r\n"
	"\t/* \r\n"
	"\t\tTransform position by the model-view-projection matrix but first\r\n"
	"\t\tmove the untransformed position along the normal by our displacement\r\n"
	"\t\tvalue.\r\n"
	"\t*/\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex + (inNormal * disp), 1.0);\r\n"
	"\r\n"
	"\t// Pass through texcoords\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"\t\r\n"
	"\t// Simple diffuse lighting in model space\r\n"
	"\tLightIntensity = dot(inNormal, -LightDirection);\r\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1109);

// ******** End: VertShader.vsh ********

