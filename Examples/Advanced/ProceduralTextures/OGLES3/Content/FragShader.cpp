// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FragShader.fsh ********

// File data
static const char _FragShader_fsh[] = 
	"uniform sampler2D    sTexture;\r\n"
	"uniform sampler2D    sColourSpline;\r\n"
	"\r\n"
	"uniform mediump float uColourSplineIndex;\r\n"
	"\r\n"
	"varying mediump vec2  vTexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tlowp vec4 fn = texture2D(sTexture, vTexCoord);\r\n"
	"\t\r\n"
	"#if defined(FN0)\r\n"
	"\tmediump float val = fn.x;\r\n"
	"#elif  defined(FN1)\r\n"
	"\tmediump float val = fn.y;\r\n"
	"#elif  defined(FN2)\r\n"
	"\tmediump float val = fn.z;\r\n"
	"#elif  defined(FN3)\r\n"
	"\tmediump float val = fn.w;\r\n"
	"#elif  defined(FN1_MINUS_FN0)\r\n"
	"\tmediump float val = fn.y - fn.x;\r\n"
	"#elif  defined(FN2_MINUS_FN1)\r\n"
	"\tmediump float val = fn.z * -1.5 - fn.y;\r\n"
	"#elif defined(SUM_FN0_FN1_FN2)\r\n"
	"\tmediump float val = fn.x * -1.0 + fn.y * 0.3 + fn.z * -0.03;\r\n"
	"#endif\r\n"
	"\r\n"
	"\tlowp vec3 colour = texture2D(sColourSpline, vec2(val, uColourSplineIndex)).rgb;\r\n"
	"\tgl_FragColor = vec4(colour, 1.0);\r\n"
	"}\r\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 778);

// ******** End: FragShader.fsh ********

