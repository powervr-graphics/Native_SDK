// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: VertShader.vsh ********

// File data
static const char _VertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY\t0\r\n"
	"#define NORMAL_ARRAY\t1\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY) in highp vec3 inVertex;\r\n"
	"layout (location = NORMAL_ARRAY) in highp vec3 inNormal;\r\n"
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
	"\tgl_Position = uModelViewProjectionMatrix * vec4(inVertex, 1.0);\r\n"
	"\tvNormal = uModelViewITMatrix * inNormal;\r\n"
	"\r\n"
	"\thighp vec3 position = (uModelViewMatrix * vec4(inVertex, 1.0)).xyz;\r\n"
	"\tvLightDirection = uLightPosition - position;\r\n"
	"}\r\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 666);

// ******** End: VertShader.vsh ********

