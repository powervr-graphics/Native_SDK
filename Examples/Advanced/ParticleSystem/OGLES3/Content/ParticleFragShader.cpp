// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ParticleFragShader.fsh ********

// File data
static const char _ParticleFragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"uniform sampler2D sTexture;\r\n"
	"\r\n"
	"in mediump float fTexCoord;\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\toColour = vec4(texture(sTexture, vec2(fTexCoord,.5)).rgb, 1);\r\n"
	"}\r\n";

// Register ParticleFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ParticleFragShader_fsh("ParticleFragShader.fsh", _ParticleFragShader_fsh, 210);

// ******** End: ParticleFragShader.fsh ********

