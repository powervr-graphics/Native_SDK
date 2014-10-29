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
	"#version 300 es\r\n"
	"uniform sampler2D  sTexture;\r\n"
	"\r\n"
	"uniform lowp vec3  FogColor;\r\n"
	"\r\n"
	"in mediump vec2  TexCoord;\r\n"
	"in lowp    vec3  DiffuseLight;\r\n"
	"in lowp    vec3  FogIntensity;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Get color from the texture and modulate with diffuse lighting\r\n"
	"    lowp vec3 texColour  = texture(sTexture, TexCoord).rgb;\r\n"
	"    lowp vec3 colour = texColour * DiffuseLight;\r\n"
	"\t\r\n"
	"\t// interpolate the fog colour with the texture-diffuse colour using the \r\n"
	"\t// fog intensity calculated in the vertex shader\r\n"
	"\tcolour = mix(FogColor, colour, FogIntensity);\r\n"
	"\toColour = vec4(colour, 1.0);\r\n"
	"}\r\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 628);

// ******** End: FragShader.fsh ********

