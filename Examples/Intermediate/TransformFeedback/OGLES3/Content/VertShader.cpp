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
	"#define POSITION_ARRAY\t\t0\r\n"
	"#define VELOCITY_ARRAY\t\t1\r\n"
	"#define ATTRIBUTES_ARRAY\t2\r\n"
	"\r\n"
	"layout (location = POSITION_ARRAY)   in highp   vec3 inPosition;\r\n"
	"layout (location = VELOCITY_ARRAY)   in highp   vec3 inVelocity;\r\n"
	"layout (location = ATTRIBUTES_ARRAY) in highp   vec3 inAttributes;\r\n"
	"\r\n"
	"uniform highp mat4 ViewProjMatrix;\r\n"
	"\r\n"
	"out highp float varTimeToLive;\r\n"
	"\r\n"
	"void main() \r\n"
	"{ \t\t\r\n"
	"\tgl_Position = ViewProjMatrix * vec4(inPosition, 1.0);\r\n"
	"\tgl_PointSize = gl_Position.z * 2.0;\r\n"
	"\tvarTimeToLive = inAttributes.x;\t\r\n"
	"} \r\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 530);

// ******** End: VertShader.vsh ********

