// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: DefaultVertShader.vsh ********

// File data
static const char _DefaultVertShader_vsh[] = 
	"#version 100\n"
	"\n"
	"uniform highp mat4 MVPMatrix;\n"
	"uniform mediump vec3 LightDir;\n"
	"uniform highp vec3 EyePos;\n"
	"\n"
	"attribute highp vec3 inVertex;\n"
	"attribute mediump vec3 inNormal;\n"
	"attribute highp vec2 inTexCoords;\n"
	"\n"
	"varying highp vec2 TexCoords;\n"
	"varying highp float LightIntensity;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\t\n"
	"\t// Pass through texture coordinates\n"
	"\tTexCoords = inTexCoords;\n"
	"\n"
	"\t// Calculate light intensity\n"
	"\t// Ambient\n"
	"\tLightIntensity = 0.4;\n"
	"\t\n"
	"\t// Diffuse\n"
	"\tLightIntensity += max(dot(inNormal, LightDir), 0.0) * 0.3;\n"
	"\n"
	"\t// Specular\n"
	"\tmediump vec3 EyeDir = normalize(EyePos - inVertex);\n"
	"\tLightIntensity += pow(max(dot(reflect(-LightDir, inNormal), EyeDir), 0.0), 5.0) * 0.8;\n"
	"}\n";

// Register DefaultVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_DefaultVertShader_vsh("DefaultVertShader.vsh", _DefaultVertShader_vsh, 716);

// ******** End: DefaultVertShader.vsh ********

