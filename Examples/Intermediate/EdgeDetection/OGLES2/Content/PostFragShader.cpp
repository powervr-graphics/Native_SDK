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
	"uniform sampler2D  sColorBufferTexture;\t// Texture containing color info in rgb and an object ID value in the alpha channel.\n"
	"varying highp vec2 t1;\t\t\t\t// Texture coordinate for this fragment.\n"
	"//#define EDGE_DETECTION\n"
	"//#define INVERSE\t\t\tThese are for editing only, leave disabled as they are passed in by the main program\n"
	"//#define BLUR\n"
	"\n"
	"#ifdef EDGE_DETECTION\n"
	"varying highp vec2 t2;\t\t\t\t// Texel directly above this fragment's.\n"
	"varying highp vec2 t3;\t\t\t\t// Texel directly to the right of this fragment's.\n"
	"#endif\n"
	"\n"
	"const lowp vec3 black = vec3(0.0);\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Gets the color from the render texture.\n"
	"\tlowp vec4 Color = texture2D(sColorBufferTexture, t1);\n"
	"\t\t\n"
	"\t// Temporary color store to be written to, data transferred to gl_FragColor at the end.\n"
	"\tlowp vec3 newColor=Color.rgb;\n"
	"\t\n"
	"#ifdef EDGE_DETECTION\n"
	"\t// Reads in values from the color texture, for two surrounding texels;\n"
	"\tlowp vec4 upFrag = texture2D(sColorBufferTexture, t2);\n"
	"\tlowp vec4 rightFrag = texture2D(sColorBufferTexture, t3);\n"
	"\t\n"
	"\t// If the object IDs covering this area differ, draw an edge\n"
	"\tif(upFrag.a-Color.a!=0.0 || rightFrag.a-Color.a!=0.0)\n"
	"\t{\n"
	"\t#ifdef INVERSE\n"
	"\t\t// Sets edge color to inverse of original color.\n"
	"\t\tnewColor=1.0-Color.rgb;\n"
	"\t#else \n"
	"\t#ifdef BLUR\n"
	"\t\t// Sets edge to a mixture of surrounding colors.\n"
	"\t\t//newColor=((2.0*Color.rgb+upFrag.rgb+rightFrag.rgb)*0.25);\n"
	"\t\t\n"
	"\t\t//newColor=((Color.rgb+upFrag.rgb)*0.25+(Color.rgb+rightFrag.rgb)*0.25);\n"
	"\t\tnewColor=(Color.rgb*0.5+(upFrag.rgb+rightFrag.rgb)*0.25);\n"
	"\t\t\n"
	"\t\t\n"
	"\t#else\n"
	"\t\t// Sets edge to black\n"
	"\t\tnewColor=black;\n"
	"\t#endif\n"
	"\t#endif\n"
	"\t}\n"
	"\telse\n"
	"\t{\n"
	"\t#ifdef INVERSE\n"
	"\t\t//Sets non-edges to black\n"
	"\t\tnewColor=black;\n"
	"\t#endif\n"
	"\t}\n"
	"#endif\n"
	"\n"
	"\t// Finally assigns gl_FragColor, with a default alpha value of 1.\n"
	"\tgl_FragColor=vec4(newColor,1.0);\n"
	"}";

// Register PostFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PostFragShader_fsh("PostFragShader.fsh", _PostFragShader_fsh, 1748);

// ******** End: PostFragShader.fsh ********

