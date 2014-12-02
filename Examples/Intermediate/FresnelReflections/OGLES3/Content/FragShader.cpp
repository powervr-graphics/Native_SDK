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
	"uniform sampler2D  sBaseTex;\r\n"
	"uniform sampler2D  sReflectTex;\r\n"
	"\r\n"
	"in mediump vec2   ReflectCoord;\r\n"
	"in mediump vec2   TexCoord;\r\n"
	"in lowp    float  ReflectRatio;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tlowp vec3 baseColour = vec3(texture(sBaseTex, TexCoord));\r\n"
	"\tlowp vec3 reflection = vec3(texture(sReflectTex, ReflectCoord));\r\n"
	"\tlowp vec3 colour = mix(baseColour, reflection, ReflectRatio);\r\n"
	"\toColour = vec4(colour, 1.0);\r\n"
	"}\r\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 470);

// ******** End: FragShader.fsh ********

