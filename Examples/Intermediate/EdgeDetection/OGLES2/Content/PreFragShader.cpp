// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PreFragShader.fsh ********

// File data
static const char _PreFragShader_fsh[] = 
	"uniform lowp vec4 inColor;\t// Color and ID passed in from vertex.\n"
	"void main()\n"
	"{\n"
	"\t// Simply assigns the color and ID number of the object it renders.\n"
	"\tgl_FragColor = inColor;\n"
	"}\n"
	"\n";

// Register PreFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PreFragShader_fsh("PreFragShader.fsh", _PreFragShader_fsh, 177);

// ******** End: PreFragShader.fsh ********

