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
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform mediump vec3  LightDirModel;\n"
	"uniform mediump vec3  EyePosModel;\n"
	"uniform         bool  bSpecular;\n"
	"uniform\t\t\tbool  bRotate;\n"
	"\n"
	"varying lowp    float  SpecularIntensity;\n"
	"varying mediump vec2   RefractCoord;\n"
	"\n"
	"const mediump float  cShininess = 3.0;\n"
	"const mediump float  cRIR = 1.015;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\t\n"
	"\t// Eye direction in model space\n"
	"\tmediump vec3 eyeDirModel = normalize(inVertex - EyePosModel);\t\n"
	"\t\n"
	"\t// GLSL offers a nice built-in refaction function\n"
	"\t// Calculate refraction direction in model space\n"
	"\tmediump vec3 refractDir = refract(eyeDirModel, inNormal, cRIR);\n"
	"\t\n"
	"\t// Project refraction\n"
	"\trefractDir = (MVPMatrix * vec4(refractDir, 0.0)).xyw;\n"
	"\n"
	"\t// Map refraction direction to 2d coordinates\n"
	"\tRefractCoord = 0.5 * (refractDir.xy / refractDir.z) + 0.5;\n"
	"\n"
	"\tif(bRotate) // If the screen is rotated then rotate the uvs\n"
	"\t{\n"
	"\t\tRefractCoord.xy = RefractCoord.yx;\n"
	"\t\tRefractCoord.y = -RefractCoord.y;\n"
	"\t}\n"
	"\t\t\n"
	"\t// Specular lighting\n"
	"\t// We ignore that N dot L could be negative (light coming \n"
	"\t// from behind the surface)\n"
	"\tSpecularIntensity = 0.0;\n"
	"\n"
	"\tif (bSpecular)\n"
	"\t{\n"
	"\t\tmediump vec3 halfVector = normalize(LightDirModel + eyeDirModel);\n"
	"\t\tlowp float NdotH = max(dot(inNormal, halfVector), 0.0);\t\t\n"
	"\t\tSpecularIntensity = pow(NdotH, cShininess);\n"
	"\t}\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1407);

// ******** End: VertShader.vsh ********

