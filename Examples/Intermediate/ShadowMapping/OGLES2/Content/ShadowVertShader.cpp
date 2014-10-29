// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ShadowVertShader.vsh ********

// File data
static const char _ShadowVertShader_vsh[] = 
	"attribute highp vec3  inVertex;\n"
	"attribute highp vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp mat4 TexProjectionMatrix;\n"
	"uniform\thighp mat4 ProjectionMatrix;\n"
	"uniform highp mat4 ModelViewMatrix;\n"
	"uniform highp vec3 LightDirection;\n"
	"\n"
	"varying highp vec4 vProjCoord;\n"
	"varying mediump vec2 texCoord;\n"
	"varying lowp vec3 LightIntensity;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\thighp vec4 modelViewPos = ModelViewMatrix * vec4(inVertex, 1.0);\n"
	"\tgl_Position = ProjectionMatrix * modelViewPos;\n"
	"\tvProjCoord = TexProjectionMatrix * modelViewPos;\n"
	"\n"
	"\ttexCoord = inTexCoord;\n"
	"\t\n"
	"\t// Simple diffuse lighting in model space\n"
	"\tLightIntensity = vec3(dot(inNormal, -LightDirection));\n"
	"}\n";

// Register ShadowVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ShadowVertShader_vsh("ShadowVertShader.vsh", _ShadowVertShader_vsh, 653);

// ******** End: ShadowVertShader.vsh ********

