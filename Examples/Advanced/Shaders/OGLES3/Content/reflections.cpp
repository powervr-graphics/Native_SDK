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
	"[HEADER]\r\n"
	"\tVERSION\t\t00.00.00.00\r\n"
	"\tDESCRIPTION Reflections from a Cubemap Example\r\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\r\n"
	"[/HEADER]\r\n"
	"\r\n"
	"[TEXTURES]\r\n"
	"\tFILE\tcubemap \t\tcubemap.pvr\t\t\tLINEAR-NEAREST-LINEAR\r\n"
	"[/TEXTURES]\r\n"
	"\r\n"
	"[VERTEXSHADER]\r\n"
	"\tNAME myVertShader\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\r\n"
	"\t\tlayout (location = 0) in highp vec4\tmyVertex;\r\n"
	"\t\tlayout (location = 1) in mediump vec3 myNormal;\r\n"
	"\r\n"
	"\t\tuniform mediump mat4\tmyMVPMatrix;\r\n"
	"\t\tuniform mediump mat3\tmyModelViewIT;\r\n"
	"\t\tuniform mediump mat3\tmyViewIT;\r\n"
	"\t\tout\t\tmediump vec3\treflectVec;\r\n"
	"\r\n"
	"\t\tvoid main(void)\r\n"
	"\t\t{\r\n"
	"\t\t\tmediump vec3  EyeDir;\r\n"
	"\t\t\tmediump vec3  Normal;\r\n"
	"\r\n"
	"\t\t\tgl_Position = myMVPMatrix * myVertex;\r\n"
	"\t\t\tNormal = normalize(myModelViewIT * myNormal);\r\n"
	"\t\t\tEyeDir = -vec3(gl_Position);\r\n"
	"\t\t\treflectVec = normalize(myViewIT*reflect(EyeDir, Normal));\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/VERTEXSHADER]\r\n"
	"\r\n"
	"[FRAGMENTSHADER]\r\n"
	"\tNAME myFragShader\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\r\n"
	"\t\tuniform samplerCube myCubeMap;\r\n"
	"\t\tin mediump vec3 reflectVec;\r\n"
	"\r\n"
	"\t\tlayout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"\t\tvoid main (void)\r\n"
	"\t\t{\r\n"
	"\t\t\tmediump vec3 envColour = vec3(texture(myCubeMap, reflectVec));\r\n"
	"\t\t\toColour = vec4 (envColour, 1.0);\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/FRAGMENTSHADER]\r\n"
	"\r\n"
	"[EFFECT]\r\n"
	"\tNAME myEffect\r\n"
	"\r\n"
	"\tATTRIBUTE\tmyVertex\t\t\tPOSITION\r\n"
	"\tATTRIBUTE\tmyNormal\t\t\tNORMAL\r\n"
	"\tUNIFORM\t\tmyMVPMatrix\t\t\tWORLDVIEWPROJECTION\r\n"
	"\tUNIFORM\t\tmyModelViewIT\t\tWORLDVIEWIT\r\n"
	"\tUNIFORM\t\tmyViewIT\t\t\tVIEWIT\r\n"
	"\tUNIFORM\t\tmyCubeMap\t\t\tTEXTURE\r\n"
	"\r\n"
	"\tTEXTURE 0 cubemap\r\n"
	"\r\n"
	"\tVERTEXSHADER myVertShader\r\n"
	"\tFRAGMENTSHADER myFragShader\r\n"
	"[/EFFECT]\r\n";

// Register reflections.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_reflections_pfx("reflections.pfx", _reflections_pfx, 1525);

// ******** End: reflections.pfx ********

