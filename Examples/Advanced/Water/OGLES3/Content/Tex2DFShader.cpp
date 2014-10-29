// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: Tex2DFShader.fsh ********

// File data
static const char _Tex2DFShader_fsh[] = 
	"#version 300 es\r\n"
	"uniform sampler2D Texture;\r\n"
	"\r\n"
	"in highp vec2 TexCoord;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\toColour = texture(Texture, TexCoord);\r\n"
	"}";

// Register Tex2DFShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_Tex2DFShader_fsh("Tex2DFShader.fsh", _Tex2DFShader_fsh, 179);

// ******** End: Tex2DFShader.fsh ********

