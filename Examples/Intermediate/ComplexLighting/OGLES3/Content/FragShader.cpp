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
	"uniform sampler2D  sTexture;\r\n"
	"\r\n"
	"in mediump vec2  TexCoord;\r\n"
	"in lowp    vec3  DiffuseLight;\r\n"
	"in lowp    vec3  SpecularLight;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"    lowp vec3 texColor  = vec3(texture(sTexture, TexCoord));\r\n"
	"\tlowp vec3 color = (texColor * DiffuseLight) + SpecularLight;\r\n"
	"\toColour = vec4(color, 1.0);\r\n"
	"}\r\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 366);

// ******** End: FragShader.fsh ********

