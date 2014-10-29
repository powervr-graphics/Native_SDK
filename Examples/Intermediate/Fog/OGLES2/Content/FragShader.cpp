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
	"uniform lowp vec3  FogColor;\n"
	"\n"
	"varying mediump vec2  TexCoord;\n"
	"varying lowp    vec3  DiffuseLight;\n"
	"varying lowp    vec3  FogIntensity;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Get color from the texture and modulate with diffuse lighting\n"
	"    lowp vec3 texColor  = texture2D(sTexture, TexCoord).rgb;\n"
	"    lowp vec3 color = texColor * DiffuseLight;\n"
	"\t\n"
	"\t// interpolate the fog color with the texture-diffuse color using the \n"
	"\t// fog intensity calculated in the vertex shader\n"
	"\tcolor = mix(FogColor, color, FogIntensity);\n"
	"\tgl_FragColor = vec4(color, 1.0);\n"
	"}\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 558);

// ******** End: FragShader.fsh ********

