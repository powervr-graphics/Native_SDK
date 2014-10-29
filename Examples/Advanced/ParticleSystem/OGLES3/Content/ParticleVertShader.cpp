// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ParticleVertShader.vsh ********

// File data
static const char _ParticleVertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define POSITION_ARRAY\t0\r\n"
	"#define LIFESPAN_ARRAY\t1\r\n"
	"\r\n"
	"layout (location = POSITION_ARRAY) in highp vec3  inPosition;\r\n"
	"layout (location = LIFESPAN_ARRAY) in highp float inLifespan;\r\n"
	"\r\n"
	"uniform highp mat4 uModelViewProjectionMatrix;\r\n"
	"\r\n"
	"out mediump float fTexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tgl_Position = uModelViewProjectionMatrix * vec4(inPosition, 1.0);\r\n"
	"\tgl_PointSize = 1.5;\r\n"
	"\tfTexCoord = inLifespan;\r\n"
	"}\r\n";

// Register ParticleVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ParticleVertShader_vsh("ParticleVertShader.vsh", _ParticleVertShader_vsh, 418);

// ******** End: ParticleVertShader.vsh ********

