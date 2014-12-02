// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FragShader.fsh ********

// File data
static const char _FragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"uniform sampler2D  sBasetex;\r\n"
	"uniform sampler2D  sReflect;\r\n"
	"uniform sampler2D  sShadow;\r\n"
	"\r\n"
	"in mediump vec2   TexCoord;\r\n"
	"in mediump vec3   ShadowCoord;\r\n"
	"in mediump vec2   ReflectCoord;\r\n"
	"in lowp    float  LightIntensity;\r\n"
	"\r\n"
	"const lowp float  cReflect = 0.2;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tlowp vec3 baseColour = texture(sBasetex, TexCoord).rgb;\r\n"
	"\tbaseColour *= 0.2 + 0.8 * textureProj(sShadow, ShadowCoord).r * LightIntensity;\r\n"
	"\t\r\n"
	"\tlowp vec3 reflectColour = texture(sReflect, ReflectCoord).rgb;\r\n"
	"\r\n"
	"\toColour = vec4(baseColour +  reflectColour * cReflect, 1.0);\r\n"
	"}";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 616);

// ******** End: FragShader.fsh ********

