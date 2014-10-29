// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PostBloomFragShader.fsh ********

// File data
static const char _PostBloomFragShader_fsh[] = 
	"uniform sampler2D  sTexture;\n"
	"\n"
	"varying mediump vec2 TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = texture2D(sTexture, TexCoord);    \n"
	"}\n";

// Register PostBloomFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PostBloomFragShader_fsh("PostBloomFragShader.fsh", _PostBloomFragShader_fsh, 132);

// ******** End: PostBloomFragShader.fsh ********

