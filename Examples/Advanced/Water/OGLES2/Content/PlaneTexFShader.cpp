// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: PlaneTexFShader.fsh ********

// File data
static const char _PlaneTexFShader_fsh[] = 
	"uniform sampler2D\t\tTexture;\n"
	"\n"
	"uniform mediump vec2 \tRcpWindowSize;\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\tmediump vec2 vTexCoord = gl_FragCoord.xy * RcpWindowSize;\n"
	"\tgl_FragColor = texture2D(Texture, vTexCoord);\n"
	"}\n";

// Register PlaneTexFShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_PlaneTexFShader_fsh("PlaneTexFShader.fsh", _PlaneTexFShader_fsh, 190);

// ******** End: PlaneTexFShader.fsh ********

