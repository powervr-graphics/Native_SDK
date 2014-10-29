// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: reflections.pfx ********

// File data
static const char _reflections_pfx[] = 
	"[HEADER]\n"
	"\tVERSION\t\t00.00.00.00\n"
	"\tDESCRIPTION Reflections from a Cubemap Example\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\n"
	"[/HEADER]\n"
	"\n"
	"[TEXTURES]\n"
	"\tFILE\tcubemap \t\tcubemap.pvr\t\t\tLINEAR-NEAREST-LINEAR\n"
	"[/TEXTURES]\n"
	"\n"
	"[VERTEXSHADER]\n"
	"\tNAME myVertShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tattribute highp vec4\tmyVertex;\n"
	"\t\tattribute mediump vec3\tmyNormal;\n"
	"\t\tuniform mediump mat4\tmyMVPMatrix;\n"
	"\t\tuniform mediump mat3\tmyModelViewIT;\n"
	"\t\tuniform mediump mat3\tmyViewIT;\n"
	"\t\tvarying mediump vec3\treflectVec;\n"
	"\t\tvoid main(void)\n"
	"\t\t{\n"
	"\t\t\tmediump vec3  EyeDir;\n"
	"\t\t\tmediump vec3  Normal;\n"
	"\n"
	"\t\t\tgl_Position = myMVPMatrix * myVertex;\n"
	"\t\t\tNormal = normalize(myModelViewIT * myNormal);\n"
	"\t\t\tEyeDir = -vec3(gl_Position);\n"
	"\t\t\treflectVec = normalize(myViewIT*reflect(EyeDir, Normal));\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/VERTEXSHADER]\n"
	"\n"
	"[FRAGMENTSHADER]\n"
	"\tNAME myFragShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tuniform samplerCube myCubeMap;\n"
	"\t\tvarying mediump vec3 reflectVec;\n"
	"\t\tvoid main (void)\n"
	"\t\t{\n"
	"\t\t\tmediump vec3 envColour = vec3(textureCube(myCubeMap, reflectVec));\n"
	"\t\t\tgl_FragColor = vec4 (envColour, 1.0);\n"
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
	"\tUNIFORM\t\tmyViewIT\t\t\tVIEWIT\n"
	"\tUNIFORM\t\tmyCubeMap\t\t\tTEXTURE\n"
	"\n"
	"\tTEXTURE 0 cubemap\n"
	"\n"
	"\tVERTEXSHADER myVertShader\n"
	"\tFRAGMENTSHADER myFragShader\n"
	"[/EFFECT]\n";

// Register reflections.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_reflections_pfx("reflections.pfx", _reflections_pfx, 1354);

// ******** End: reflections.pfx ********

