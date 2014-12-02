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
	"varying mediump vec2  TexCoord;\n"
	"varying lowp    vec3  DiffuseLight;\n"
	"varying lowp    vec3  SpecularLight;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    lowp vec3 texColor  = vec3(texture2D(sTexture, TexCoord));\n"
	"\tlowp vec3 color = (texColor * DiffuseLight) + SpecularLight;\n"
	"\tgl_FragColor = vec4(color, 1.0);\n"
	"}\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 311);

// ******** End: FragShader.fsh ********

