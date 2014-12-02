// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BlurVertShader.vsh ********

// File data
static const char _BlurVertShader_vsh[] = 
	"// Blur filter kernel shader\n"
	"//\n"
	"// 0  1  2  3  4\n"
	"// x--x--X--x--x    <- original filter kernel\n"
	"//   y---X---y      <- filter kernel abusing the hardware texture filtering\n"
	"//       |\n"
	"//      texel center\n"
	"//\n"
	"// \n"
	"// Using hardware texture filtering, the amount of samples can be\n"
	"// reduced to three. To calculate the offset, use this formula:\n"
	"// d = w1 / (w1 + w2),  whereas w1 and w2 denote the filter kernel weights\n"
	"\n"
	"attribute highp   vec3  inVertex;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform mediump float  TexelOffsetX;\n"
	"uniform mediump float  TexelOffsetY;\n"
	"\n"
	"varying mediump vec2  TexCoord0;\n"
	"varying mediump vec2  TexCoord1;\n"
	"varying mediump vec2  TexCoord2;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Pass through vertex\n"
	"\tgl_Position = vec4(inVertex, 1.0);\n"
	"\t\n"
	"\t// Calculate texture offsets and pass through\t\n"
	"\tmediump vec2 offset = vec2(TexelOffsetX, TexelOffsetY);\n"
	"  \n"
	"    TexCoord0 = inTexCoord - offset;\n"
	"    TexCoord1 = inTexCoord;\n"
	"    TexCoord2 = inTexCoord + offset;    \n"
	"\n"
	"}";

// Register BlurVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BlurVertShader_vsh("BlurVertShader.vsh", _BlurVertShader_vsh, 954);

// ******** End: BlurVertShader.vsh ********

