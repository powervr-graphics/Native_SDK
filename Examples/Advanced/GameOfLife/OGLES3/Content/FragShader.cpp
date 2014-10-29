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
	"uniform sampler2D    sTexture;\r\n"
	"\r\n"
	"in highp vec2 TexCoord;\r\n"
	"\r\n"
	"out highp vec4 fragColor;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"    mediump float smp = texture(sTexture, TexCoord).x;\r\n"
	"\t//Just giving it a little color.\r\n"
	"\tconst lowp vec3 hue = vec3(.5, 1, .5);\r\n"
	"\t//const lowp vec3 hue = vec3(.9, .26, 1.);\r\n"
	"\tfragColor = vec4(hue * smp, 1.0); \r\n"
	"}\r\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 340);

// ******** End: FragShader.fsh ********

