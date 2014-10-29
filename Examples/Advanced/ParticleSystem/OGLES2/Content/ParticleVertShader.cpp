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
	"attribute highp vec3  inPosition;\n"
	"attribute highp float inLifespan;\n"
	"\n"
	"uniform highp mat4  uModelViewProjectionMatrix;\n"
	"\n"
	"varying mediump vec2 vTexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = uModelViewProjectionMatrix * vec4(inPosition, 1.0);\n"
	"\tgl_PointSize = 3.0;\n"
	"\tfloat scale = clamp(inLifespan / 20.0, 0.0, 1.0);\n"
	"\tscale = scale * scale;\n"
	"\tvTexCoord.st = vec2(scale, scale);\n"
	"}\n";

// Register ParticleVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ParticleVertShader_vsh("ParticleVertShader.vsh", _ParticleVertShader_vsh, 366);

// ******** End: ParticleVertShader.vsh ********

