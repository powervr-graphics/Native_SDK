// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FloorVertShader.vsh ********

// File data
static const char _FloorVertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"in highp vec3 inVertex;\r\n"
	"in highp vec3 inNormal;\r\n"
	"\r\n"
	"uniform highp mat4  uModelViewMatrix;\r\n"
	"uniform highp mat3  uModelViewITMatrix;\r\n"
	"uniform highp mat4  uModelViewProjectionMatrix;\r\n"
	"\r\n"
	"uniform highp vec3  uLightPosition;\r\n"
	"\r\n"
	"out highp vec3  vNormal;\r\n"
	"out highp vec3  vLightDirection;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\thighp vec4 position   = (uModelViewMatrix * vec4(inVertex, 1.0));\r\n"
	"\r\n"
	"\tgl_Position = uModelViewProjectionMatrix * vec4(inVertex, 1.0);\r\n"
	"\t\r\n"
	"\tvNormal         = uModelViewITMatrix * inNormal;\r\n"
	"\tvLightDirection = uLightPosition - position.xyz;\r\n"
	"}\r\n";

// Register FloorVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FloorVertShader_vsh("FloorVertShader.vsh", _FloorVertShader_vsh, 563);

// ******** End: FloorVertShader.vsh ********

