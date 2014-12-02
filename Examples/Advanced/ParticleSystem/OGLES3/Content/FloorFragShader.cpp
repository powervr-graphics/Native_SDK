// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FloorFragShader.fsh ********

// File data
static const char _FloorFragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"in highp vec3 vNormal;\r\n"
	"in highp vec3 vLightDirection;\r\n"
	"\r\n"
	"out lowp vec4 fragColor;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\thighp float inv_lightdist = 1.0f / length(vLightDirection);\r\n"
	"\thighp float diffuse = max(dot(normalize(vNormal), vLightDirection * inv_lightdist), 0.0);\r\n"
	"\t\t\r\n"
	"\tfragColor = vec4(vec3(diffuse), 1.0);\r\n"
	"}\r\n";

// Register FloorFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FloorFragShader_fsh("FloorFragShader.fsh", _FloorFragShader_fsh, 322);

// ******** End: FloorFragShader.fsh ********

