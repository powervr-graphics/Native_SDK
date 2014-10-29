// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PostFragShader.fsh ********

// File data
static const char _PostFragShader_fsh[] = 
	"#version 300 es\r\n"
	"\r\n"
	"uniform sampler2D  sColorBufferTexture;\t// Texture containing color info in rgb and an object ID value in the alpha channel.\r\n"
	"in mediump vec2 t1;\t\t\t\t// Texture coordinate for this fragment.\r\n"
	"\r\n"
	"//#define EDGE_DETECTION\r\n"
	"//#define INVERSE\t\t\tThese are for editing only, leave disabled as they are passed in by the main program\r\n"
	"//#define BLUR\r\n"
	"\r\n"
	"#ifdef EDGE_DETECTION\r\n"
	"in mediump vec2 t2;\t\t\t\t// Texel directly above this fragment's.\r\n"
	"in mediump vec2 t3;\t\t\t\t// Texel directly to the right of this fragment's.\r\n"
	"#endif\r\n"
	"\r\n"
	"const lowp vec3 black = vec3(0.0);\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Gets the colour from the render texture.\r\n"
	"\tlowp vec4 Color = texture(sColorBufferTexture, t1);\r\n"
	"\t\t\r\n"
	"\t// Temporary colour store to be written to, data transferred to gl_FragColor at the end.\r\n"
	"\tlowp vec3 newColor=Color.rgb;\r\n"
	"\t\r\n"
	"#ifdef EDGE_DETECTION\r\n"
	"\t// Reads in values from the colour texture, for two surrounding texels;\r\n"
	"\tlowp vec4 upFrag = texture(sColorBufferTexture, t2);\r\n"
	"\tlowp vec4 rightFrag = texture(sColorBufferTexture, t3);\r\n"
	"\t\r\n"
	"\t// If the object IDs covering this area differ, draw an edge\r\n"
	"\tif(upFrag.a-Color.a != 0.0 || rightFrag.a-Color.a != 0.0)\r\n"
	"\t{\r\n"
	"\t#ifdef INVERSE\r\n"
	"\t\t// Sets edge color to inverse of original colour.\r\n"
	"\t\tnewColor = 1.0-Color.rgb;\r\n"
	"\t#else \r\n"
	"\t#ifdef BLUR\r\n"
	"\t\t// Sets edge to a mixture of surrounding colours.\r\n"
	"\t\tnewColor = (Color.rgb*0.5+(upFrag.rgb+rightFrag.rgb)*0.25);\r\n"
	"\t\t\r\n"
	"\t#else\r\n"
	"\t\t// Sets edge to black\r\n"
	"\t\tnewColor = black;\r\n"
	"\t#endif\r\n"
	"\t#endif\r\n"
	"\t}\r\n"
	"\telse\r\n"
	"\t{\r\n"
	"\t#ifdef INVERSE\r\n"
	"\t\t//Sets non-edges to black\r\n"
	"\t\tnewColor=black;\r\n"
	"\t#endif\r\n"
	"\t}\r\n"
	"#endif\r\n"
	"\r\n"
	"\t// Finally assigns our out colour, with a default alpha value of 1.\r\n"
	"\toColour = vec4(newColor,1.0);\r\n"
	"}";

// Register PostFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PostFragShader_fsh("PostFragShader.fsh", _PostFragShader_fsh, 1725);

// ******** End: PostFragShader.fsh ********

