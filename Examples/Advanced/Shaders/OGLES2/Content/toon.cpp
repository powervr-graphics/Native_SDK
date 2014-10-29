// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: toon.pfx ********

// File data
static const char _toon_pfx[] = 
	"[HEADER]\n"
	"\tVERSION\t\t00.00.00.00\n"
	"\tDESCRIPTION Toon Example\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\n"
	"[/HEADER]\n"
	"\n"
	"[TEXTURES]\n"
	"[/TEXTURES]\n"
	"\n"
	"[VERTEXSHADER]\n"
	"\tNAME myVertShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tattribute mediump vec4\tmyVertex;\n"
	"\t\tattribute mediump vec3\tmyNormal;\n"
	"\t\tattribute mediump vec4\tmyUV;\n"
	"\t\tuniform mediump mat4\tmyMVPMatrix;\n"
	"\t\tuniform mediump mat3\tmyModelViewIT;\n"
	"\t\tvarying mediump float\tintensity;\n"
	"\n"
	"\t\tvoid main(void)\n"
	"\t\t{\n"
	"\t\t\tmediump vec3 Normal;\n"
	"\t\t\tgl_Position = myMVPMatrix * myVertex;\n"
	"\t\t\tNormal = myModelViewIT * myNormal;\n"
	"\t\t\tintensity = abs( dot(vec3(0,0,1),normalize(Normal)) );\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/VERTEXSHADER]\n"
	"\n"
	"[FRAGMENTSHADER]\n"
	"\tNAME myFragShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tvarying mediump float intensity;\n"
	"\t\t\n"
	"\t\tvoid main (void)\n"
	"\t\t{\n"
	"\t\t\tmediump vec4 color;\t\n"
	"\t\t\tif (intensity > 0.95)\n"
	"\t\t\t\tcolor = vec4(1.0,0.5,0.5,1.0);\n"
	"\t\t\telse if (intensity > 0.5)\n"
	"\t\t\t\tcolor = vec4(0.6,0.3,0.3,1.0);\n"
	"\t\t\telse if (intensity > 0.25)\n"
	"\t\t\t\tcolor = vec4(0.4,0.2,0.2,1.0);\n"
	"\t\t\telse\n"
	"\t\t\t\tcolor = vec4(0.2,0.1,0.1,1.0);\n"
	" \t\t\tgl_FragColor = color;\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/FRAGMENTSHADER]\n"
	"\n"
	"[EFFECT]\n"
	"\tNAME myEffect\n"
	"\n"
	"\tATTRIBUTE\tmyVertex\t\t\tPOSITION\n"
	"\tATTRIBUTE\tmyNormal\t\t\tNORMAL\n"
	"\tUNIFORM\t\tmyMVPMatrix\t\t\tWORLDVIEWPROJECTION\n"
	"\tUNIFORM\t\tmyModelViewIT\t\tWORLDVIEWIT\n"
	"\n"
	"\tVERTEXSHADER myVertShader\n"
	"\tFRAGMENTSHADER myFragShader\n"
	"[/EFFECT]\n";

// Register toon.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_toon_pfx("toon.pfx", _toon_pfx, 1271);

// ******** End: toon.pfx ********

