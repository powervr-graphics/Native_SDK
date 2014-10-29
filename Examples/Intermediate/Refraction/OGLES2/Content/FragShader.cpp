// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FragShader.fsh ********

// File data
static const char _FragShader_fsh[] = 
	"uniform sampler2D  sTexture;\n"
	"\n"
	"varying lowp    float  SpecularIntensity;\n"
	"varying mediump vec2   RefractCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tlowp vec3 refractColor = texture2D(sTexture, RefractCoord).rgb;\t\n"
	"\tgl_FragColor = vec4(refractColor + SpecularIntensity, 1.0);\n"
	"}";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 252);

// ******** End: FragShader.fsh ********

