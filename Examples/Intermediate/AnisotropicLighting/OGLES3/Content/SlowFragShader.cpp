// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SlowFragShader.fsh ********

// File data
static const char _SlowFragShader_fsh[] = 
	"#version 300 es\r\n"
	"in lowp vec3  DiffuseIntensity; \r\n"
	"in lowp vec3  SpecularIntensity;\r\n"
	"\r\n"
	"const lowp vec3 cBaseColor = vec3(0.9, 0.1, 0.1); \r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main() \r\n"
	"{ \r\n"
	"\toColour = vec4((cBaseColor * DiffuseIntensity) + SpecularIntensity, 1.0); \r\n"
	"}\r\n";

// Register SlowFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SlowFragShader_fsh("SlowFragShader.fsh", _SlowFragShader_fsh, 285);

// ******** End: SlowFragShader.fsh ********

