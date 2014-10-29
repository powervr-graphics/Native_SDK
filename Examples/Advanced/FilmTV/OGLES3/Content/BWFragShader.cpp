// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BWFragShader.fsh ********

// File data
static const char _BWFragShader_fsh[] = 
	"#version 300 es\r\n"
	"uniform sampler2D  sTexture;\r\n"
	"\r\n"
	"in lowp    float LightIntensity;\r\n"
	"in mediump vec2  TexCoord;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tlowp vec3 fCol = texture(sTexture, TexCoord).rgb;\r\n"
	"\tlowp float fAvg = (fCol.r + fCol.g + fCol.b) / 3.0;\r\n"
	"    oColour.rgb = vec3(fAvg * LightIntensity);\r\n"
	"    oColour.a = 1.0;\r\n"
	"}\r\n";

// Register BWFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BWFragShader_fsh("BWFragShader.fsh", _BWFragShader_fsh, 356);

// ******** End: BWFragShader.fsh ********

