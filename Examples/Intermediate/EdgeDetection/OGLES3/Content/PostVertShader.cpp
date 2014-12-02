// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PostVertShader.vsh ********

// File data
static const char _PostVertShader_vsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"layout (location = 0) in highp vec3\tinVertex;\t\t\t//Vertex coordinates\r\n"
	"layout (location = 1) in mediump vec2\tinTexCoord;\t\t//Texture coordinate passed to fragment.\r\n"
	"\r\n"
	"out mediump vec2 t1;\t\t\t\t\r\n"
	"\r\n"
	"#ifdef EDGE_DETECTION\r\n"
	"uniform mediump vec2 PixelSize;\t\t\t//Relative size of a pixel (in texels) for this program.\r\n"
	"out mediump vec2 t2;\t\t\t\t//Texture location for fragment directly above.\r\n"
	"out mediump vec2 t3;\t\t\t\t//Texture location for fragment directly to the right.\r\n"
	"#endif\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t//Pass through texture coordinates.\r\n"
	"\tt1 = inTexCoord;\r\n"
	"\r\n"
	"#ifdef EDGE_DETECTION\r\n"
	"\t// Sets texture coordinates for surrounding texels (up and right);\r\n"
	"\tt2 = vec2(inTexCoord.x, inTexCoord.y+PixelSize.y);\r\n"
	"\tt3 = vec2(inTexCoord.x+PixelSize.x, inTexCoord.y);\r\n"
	"#endif\r\n"
	"\r\n"
	"\t// Set vertex position.\r\n"
	"\tgl_Position = vec4(inVertex,  1.0);\r\n"
	"}\r\n";

// Register PostVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PostVertShader_vsh("PostVertShader.vsh", _PostVertShader_vsh, 840);

// ******** End: PostVertShader.vsh ********

