// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: UIVertShader.vsh ********

// File data
static const char _UIVertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"#define VERTEX_ARRAY_UI\t0\r\n"
	"#define TEXCOORD_ARRAY_UI 1\r\n"
	"\r\n"
	"layout (location = VERTEX_ARRAY_UI) in highp vec2\tinVertex;\r\n"
	"layout (location = TEXCOORD_ARRAY_UI) in mediump vec2\tinTexCoord;\r\n"
	"\r\n"
	"uniform   highp   mat2  RotationMatrix;\r\n"
	"\r\n"
	"out   mediump vec2  vTexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\t\r\n"
	"\thighp vec2 vertex = RotationMatrix * inVertex;\r\n"
	"\tgl_Position = vec4(vertex, 0.0, 1.0);\t\r\n"
	"\tvTexCoord = inTexCoord;\r\n"
	"}\r\n";

// Register UIVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_UIVertShader_vsh("UIVertShader.vsh", _UIVertShader_vsh, 420);

// ******** End: UIVertShader.vsh ********

