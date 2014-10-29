// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PostBloomVertShader.vsh ********

// File data
static const char _PostBloomVertShader_vsh[] = 
	"attribute highp   vec2  inVertex;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"varying mediump vec2   TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    // Pass through vertex\n"
	"\tgl_Position = vec4(inVertex, 0.0, 1.0);\n"
	"\t\n"
	"\t// Pass through texcoords\n"
	"\tTexCoord = inTexCoord;\n"
	"}\n";

// Register PostBloomVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PostBloomVertShader_vsh("PostBloomVertShader.vsh", _PostBloomVertShader_vsh, 242);

// ******** End: PostBloomVertShader.vsh ********

