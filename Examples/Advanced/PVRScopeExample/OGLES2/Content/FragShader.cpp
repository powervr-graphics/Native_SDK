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
	"uniform sampler2D  sThicknessTex;\n"
	"\n"
	"uniform highp float  MinThickness;\n"
	"uniform highp float  MaxVariation;\n"
	"\n"
	"varying mediump float  CosViewAngle;\n"
	"varying mediump float  LightIntensity;\n"
	"varying mediump vec2   TexCoord;\n"
	"\n"
	"// We use wave numbers (k) for the iridescence effect, given as\n"
	"//   k =  2 * pi / wavelength in nm.\n"
	"const highp float  PI = 3.141592654;\n"
	"const highp vec3   cRgbK = 2.0 * PI * vec3(1.0/475.0, 1.0/510.0, 1.0/650.0);\n"
	"\n"
	"void main()\n"
	"{\n"
	"\thighp float thickness = texture2D(sThicknessTex, TexCoord).r * MaxVariation + MinThickness;\n"
	"\thighp float delta = (thickness / LightIntensity) + (thickness / CosViewAngle);\n"
	"\tlowp vec3 color = cos(delta * cRgbK) * LightIntensity;\n"
	"\tgl_FragColor = vec4(color, 1.0);\n"
	"}";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 710);

// ******** End: FragShader.fsh ********

