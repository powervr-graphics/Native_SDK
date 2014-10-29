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
	"attribute highp\t  vec3 inVertex;\t\t//Vertex coordinates\n"
	"attribute highp vec2 inTexCoord;\t\t//Texture coordinates in.\n"
	"varying   highp vec2 t1;\t\t\t\t//Texture coordinate passed to fragment.\n"
	"\n"
	"#ifdef EDGE_DETECTION\n"
	"uniform mediump vec2 PixelSize;\t\t\t//Relative size of a pixel (in texels) for this program.\n"
	"varying highp vec2 t2;\t\t\t\t//Texture location for fragment directly above.\n"
	"varying highp vec2 t3;\t\t\t\t//Texture location for fragment directly to the right.\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t//Pass through texture coordinates.\n"
	"\tt1 = inTexCoord;\n"
	"\n"
	"#ifdef EDGE_DETECTION\n"
	"\t// Sets texture coordinates for surrounding texels (up and right);\n"
	"\tt2 = vec2(inTexCoord.x, inTexCoord.y+PixelSize.y);\n"
	"\tt3 = vec2(inTexCoord.x+PixelSize.x, inTexCoord.y);\n"
	"#endif\n"
	"\n"
	"\t// Set vertex position.\n"
	"\tgl_Position = vec4(inVertex,  1.0);\n"
	"\n"
	"}\n";

// Register PostVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PostVertShader_vsh("PostVertShader.vsh", _PostVertShader_vsh, 798);

// ******** End: PostVertShader.vsh ********

