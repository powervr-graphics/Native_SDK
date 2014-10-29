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
	"layout (location = VERTEX_ARRAY) in highp vec4\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in highp vec3\tinNormal;\r\n"
	"layout (location = TEXCOORD_ARRAY) in highp vec2\tinTexCoord;\r\n"
	"\r\n"
	"uniform highp   mat4  MVPMatrix;\r\n"
	"uniform highp   mat4  ModelViewMatrix;\r\n"
	"uniform highp vec3  LightDirection;\r\n"
	"// fog uniforms\r\n"
	"uniform lowp    int    iFogMode;\r\n"
	"uniform highp float  FogDensity;\r\n"
	"uniform highp float  FogEnd;\r\n"
	"uniform highp float  FogRcpEndStartDiff;\r\n"
	"\r\n"
	"out mediump vec2  TexCoord;\r\n"
	"out lowp    vec3  DiffuseLight;\r\n"
	"out lowp    vec3  FogIntensity;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// transform position to view space as we need the distance to the eye for fog\r\n"
	"\thighp vec3 viewPos = vec3(ModelViewMatrix * inVertex);\r\n"
	"\thighp float eyeDist = length(viewPos);\r\n"
	"\t\r\n"
	"\t// transform vertex position\r\n"
	"\tgl_Position = MVPMatrix * inVertex;\r\n"
	"\t\r\n"
	"\t// texcoords pass through\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"\r\n"
	"\t// calculate lighting\r\n"
	"\t// We use a directional light with direction given in model space\r\n"
	"\tlowp float DiffuseIntensity = dot(inNormal, normalize(LightDirection));\r\n"
	"\t\r\n"
	"\t// clamp negative values and add some ambient light\r\n"
	"\tDiffuseLight = vec3(max(DiffuseIntensity, 0.0) * 0.5 + 0.5);\r\n"
	"\r\n"
	"\t\r\n"
	"\t// select fog function. 1 is linear, 2 is exponential, 3 is exponential squared, 0 is no fog.\r\n"
	"\thighp float fogIntensity = 1.0;\r\n"
	"\tif(iFogMode == 1)\r\n"
	"\t{\r\n"
	"\t\tfogIntensity = (FogEnd - eyeDist) * FogRcpEndStartDiff;\r\n"
	"\t}\r\n"
	"\telse if(iFogMode >= 2)\r\n"
	"\t{\r\n"
	"\t\thighp float scaledDist = eyeDist * FogDensity;\r\n"
	"\t\tif (iFogMode == 3)\r\n"
	"\t\t{\r\n"
	"\t\t\tscaledDist *= scaledDist;\r\n"
	"\t\t}\r\n"
	"\t\tfogIntensity = exp2(-scaledDist);\r\n"
	"\r\n"
	"\t}\r\n"
	"\r\n"
	"\t// clamp the intensity within a valid range\r\n"
	"\tFogIntensity = vec3(clamp(fogIntensity, 0.0, 1.0));\r\n"
	"}\r\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1764);

// ******** End: VertShader.vsh ********

