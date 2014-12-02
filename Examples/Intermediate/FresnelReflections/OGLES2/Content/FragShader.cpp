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
	"uniform sampler2D  sBaseTex;\n"
	"uniform sampler2D  sReflectTex;\n"
	"\n"
	"varying mediump vec2   ReflectCoord;\n"
	"varying mediump vec2   TexCoord;\n"
	"varying lowp    float  ReflectRatio;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tlowp vec3 baseColor = vec3(texture2D(sBaseTex, TexCoord));\n"
	"\tlowp vec3 reflection = vec3(texture2D(sReflectTex, ReflectCoord));\n"
	"\tlowp vec3 color = mix(baseColor, reflection, ReflectRatio);\n"
	"\tgl_FragColor = vec4(color, 1.0);\n"
	"}\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 409);

// ******** End: FragShader.fsh ********

