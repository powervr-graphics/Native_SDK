// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: DefaultFragShader.fsh ********

// File data
static const char _DefaultFragShader_fsh[] = 
	"#version 100\n"
	"\n"
	"uniform sampler2D s2DMap;\n"
	"\n"
	"varying highp vec2 TexCoords;\n"
	"varying highp float LightIntensity;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Sample texture and shade fragment\n"
	"\tgl_FragColor = vec4(LightIntensity * texture2D(s2DMap, TexCoords).rgb, 1.0);\n"
	"}\n";

// Register DefaultFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_DefaultFragShader_fsh("DefaultFragShader.fsh", _DefaultFragShader_fsh, 240);

// ******** End: DefaultFragShader.fsh ********

