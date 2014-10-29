// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PreBloomFragShader.fsh ********

// File data
static const char _PreBloomFragShader_fsh[] = 
	"uniform sampler2D  sBloomMapping;\n"
	"\n"
	"uniform mediump float fBloomIntensity;\n"
	"\n"
	"varying mediump vec2 TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = texture2D(sBloomMapping, TexCoord) * fBloomIntensity;\n"
	"}\n";

// Register PreBloomFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PreBloomFragShader_fsh("PreBloomFragShader.fsh", _PreBloomFragShader_fsh, 196);

// ******** End: PreBloomFragShader.fsh ********

