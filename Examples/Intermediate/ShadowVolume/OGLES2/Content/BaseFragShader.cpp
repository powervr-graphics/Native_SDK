// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BaseFragShader.fsh ********

// File data
static const char _BaseFragShader_fsh[] = 
	"/*\n"
	"  Simple fragment shader:\n"
	"  - Single texturing modulated with vertex lighting\n"
	"*/\n"
	"\n"
	"uniform sampler2D sTexture;\n"
	"\n"
	"varying lowp    float LightIntensity;\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = vec4(texture2D(sTexture, TexCoord).rgb * LightIntensity, 1.0);\n"
	"}\n";

// Register BaseFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BaseFragShader_fsh("BaseFragShader.fsh", _BaseFragShader_fsh, 283);

// ******** End: BaseFragShader.fsh ********

