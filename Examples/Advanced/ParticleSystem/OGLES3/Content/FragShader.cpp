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
	"in highp vec3 vNormal;\r\n"
	"in highp vec3 vLightDirection;\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\thighp float inv_lightdist = 1.0 / length(vLightDirection);\r\n"
	"\thighp float diffuse = max(dot(normalize(vNormal), vLightDirection * inv_lightdist), 0.0);\r\n"
	"\toColour = vec4(vec3(diffuse) * inv_lightdist * 10.0, 1.0);\r\n"
	"}\r\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 356);

// ******** End: FragShader.fsh ********

