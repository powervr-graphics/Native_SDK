// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: simple.pfx ********

// File data
static const char _simple_pfx[] = 
	"[HEADER]\n"
	"\tVERSION\t\t00.00.00.00\n"
	"\tDESCRIPTION Simple Texture Example\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\n"
	"[/HEADER]\n"
	"\n"
	"[TEXTURES]\n"
	"\tFILE base \t\t\tbase.pvr\t\t\tLINEAR-LINEAR-LINEAR\n"
	"[/TEXTURES]\n"
	"\n"
	"[VERTEXSHADER]\n"
	"\tNAME myVertShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tattribute highp vec4\tmyVertex;\n"
	"\t\tattribute mediump vec2\tmyUV;\n"
	"\t\tuniform mediump mat4\tmyMVPMatrix;\n"
	"\t\tvarying mediump vec2 texCoord;\n"
	"\n"
	"\t\tvoid main(void)\n"
	"\t\t{\n"
	"\t\t\tgl_Position = myMVPMatrix * myVertex;\n"
	"\t\t\ttexCoord = myUV.st;\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/VERTEXSHADER]\n"
	"\n"
	"[FRAGMENTSHADER]\n"
	"\tNAME myFragShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tuniform sampler2D sampler2d;\n"
	"\t\tvarying mediump vec2 texCoord;\n"
	"\n"
	"\t\tvoid main(void)\n"
	"\t\t{\n"
	"\t\t\tgl_FragColor = texture2D(sampler2d,texCoord);\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/FRAGMENTSHADER]\n"
	"\n"
	"[EFFECT]\n"
	"\tNAME myEffect\n"
	"\n"
	"\tATTRIBUTE\tmyVertex\t\t\tPOSITION\n"
	"\tATTRIBUTE\tmyUV\t\t\t\tUV\n"
	"\tUNIFORM\t\tmyMVPMatrix\t\t\tWORLDVIEWPROJECTION\n"
	"\tUNIFORM\t\tsampler2d\t\t\tTEXTURE0\n"
	"\n"
	"\tTEXTURE 0 base\n"
	"\n"
	"\tVERTEXSHADER myVertShader\n"
	"\tFRAGMENTSHADER myFragShader\n"
	"[/EFFECT]\n";

// Register simple.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_simple_pfx("simple.pfx", _simple_pfx, 950);

// ******** End: simple.pfx ********

