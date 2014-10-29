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
	"uniform sampler2D  sBasetex;\n"
	"uniform sampler2D  sReflect;\n"
	"uniform sampler2D  sShadow;\n"
	"\n"
	"varying mediump vec2   TexCoord;\n"
	"varying mediump vec3   ShadowCoord;\n"
	"varying mediump vec2   ReflectCoord;\n"
	"varying lowp    float  LightIntensity;\n"
	"\n"
	"const lowp float  cReflect = 0.2;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tlowp vec3 baseColor = texture2D(sBasetex, TexCoord).rgb;\n"
	"\tbaseColor *= 0.2 + 0.8 * texture2DProj(sShadow, ShadowCoord).r * LightIntensity;\n"
	"\t\n"
	"\tlowp vec3 reflectColor = texture2D(sReflect, ReflectCoord).rgb;\n"
	"\n"
	"\tgl_FragColor = vec4(baseColor +  reflectColor * cReflect, 1.0);\n"
	"}";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 556);

// ******** End: FragShader.fsh ********

