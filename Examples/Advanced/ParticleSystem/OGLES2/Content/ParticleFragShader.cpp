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
	"uniform sampler2D sTexture;\n"
	"\n"
	"varying mediump vec2 vTexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_FragColor = vec4(texture2D(sTexture, vTexCoord).rgb, vTexCoord.s);\n"
	"}\n";

// Register ParticleFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ParticleFragShader_fsh("ParticleFragShader.fsh", _ParticleFragShader_fsh, 149);

// ******** End: ParticleFragShader.fsh ********

