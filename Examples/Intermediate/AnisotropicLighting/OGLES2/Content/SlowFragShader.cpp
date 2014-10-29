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
	"varying lowp vec3  DiffuseIntensity; \n"
	"varying lowp vec3  SpecularIntensity;\n"
	"\n"
	"const lowp vec3 cBaseColor = vec3(0.9, 0.1, 0.1); \n"
	" \n"
	"void main() \n"
	"{ \n"
	"\tgl_FragColor = vec4((cBaseColor * DiffuseIntensity) + SpecularIntensity, 1.0); \n"
	"}\n";

// Register SlowFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SlowFragShader_fsh("SlowFragShader.fsh", _SlowFragShader_fsh, 229);

// ******** End: SlowFragShader.fsh ********

