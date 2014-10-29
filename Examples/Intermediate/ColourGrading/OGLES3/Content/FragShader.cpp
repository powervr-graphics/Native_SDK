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
	"\r\n"
	"uniform  sampler2D     sTexture;\r\n"
	"uniform  mediump sampler3D\t\tsColourLUT;\r\n"
	"\r\n"
	"in mediump vec2 texCoords;\r\n"
	"layout(location = 0) out lowp vec4 oFragColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"    highp vec3 vCol = texture(sTexture, texCoords).rgb;\r\n"
	"\tlowp vec3 vAlteredCol = texture(sColourLUT, vCol.rgb).rgb;\r\n"
	"    oFragColour = vec4(vAlteredCol, 1.0);\r\n"
	"}\r\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 355);

// ******** End: FragShader.fsh ********

