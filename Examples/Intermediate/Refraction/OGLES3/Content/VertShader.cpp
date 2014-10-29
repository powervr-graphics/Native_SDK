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
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3\tinVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in mediump vec3\tinNormal;\r\n"
	"\r\n"
	"uniform highp   mat4  MVPMatrix;\r\n"
	"uniform mediump vec3  LightDirModel;\r\n"
	"uniform mediump vec3  EyePosModel;\r\n"
	"uniform         bool  bSpecular;\r\n"
	"uniform\t\t\tbool  bRotate;\r\n"
	"\r\n"
	"out lowp    float  SpecularIntensity;\r\n"
	"out mediump vec2   RefractCoord;\r\n"
	"\r\n"
	"const mediump float  cShininess = 3.0;\r\n"
	"const mediump float  cRIR = 1.015;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Transform position\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"\t\r\n"
	"\t// Eye direction in model space\r\n"
	"\tmediump vec3 eyeDirModel = normalize(inVertex - EyePosModel);\t\r\n"
	"\t\r\n"
	"\t// GLSL offers a nice built-in refaction function\r\n"
	"\t// Calculate refraction direction in model space\r\n"
	"\tmediump vec3 refractDir = refract(eyeDirModel, inNormal, cRIR);\r\n"
	"\t\r\n"
	"\t// Project refraction\r\n"
	"\trefractDir = (MVPMatrix * vec4(refractDir, 0.0)).xyw;\r\n"
	"\r\n"
	"\t// Map refraction direction to 2d coordinates\r\n"
	"\tRefractCoord = 0.5 * (refractDir.xy / refractDir.z) + 0.5;\r\n"
	"\r\n"
	"\tif(bRotate) // If the screen is rotated then rotate the uvs\r\n"
	"\t{\r\n"
	"\t\tRefractCoord.xy = RefractCoord.yx;\r\n"
	"\t\tRefractCoord.y = -RefractCoord.y;\r\n"
	"\t}\r\n"
	"\t\t\r\n"
	"\t// Specular lighting\r\n"
	"\t// We ignore that N dot L could be negative (light coming \r\n"
	"\t// from behind the surface)\r\n"
	"\tSpecularIntensity = 0.0;\r\n"
	"\r\n"
	"\tif (bSpecular)\r\n"
	"\t{\r\n"
	"\t\tmediump vec3 halfVector = normalize(LightDirModel + eyeDirModel);\r\n"
	"\t\tlowp float NdotH = max(dot(inNormal, halfVector), 0.0);\t\t\r\n"
	"\t\tSpecularIntensity = pow(NdotH, cShininess);\r\n"
	"\t}\r\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1566);

// ******** End: VertShader.vsh ********

