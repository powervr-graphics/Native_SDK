// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: anisotropic_lighting.pfx ********

// File data
static const char _anisotropic_lighting_pfx[] = 
	"[HEADER]\t\r\n"
	"\tVERSION\t\t00.00.00.00\t\r\n"
	"\tDESCRIPTION Anisotropic Lighting Example\t\r\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\t\r\n"
	"[/HEADER]\r\n"
	"\r\n"
	"[TEXTURES]\r\n"
	"\tFILE\tanisotropicmap \tanisotropicmap.pvr\t\tLINEAR-LINEAR-LINEAR\r\n"
	"[/TEXTURES]\r\n"
	"\r\n"
	"[VERTEXSHADER]\r\n"
	"\tNAME myVertShader\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\r\n"
	"\t\tlayout (location = 0) in highp vec3\tmyVertex;\r\n"
	"\t\tlayout (location = 1) in highp vec3\tmyNormal;\r\n"
	"\r\n"
	"\t\tuniform mediump mat4\tmyMVPMatrix;\r\n"
	"\t\tuniform mediump mat3\tmyModelViewIT;\r\n"
	"\t\tconst mediump vec3\t\tmyLightDirection = vec3(0.577,0.577,0.577);\r\n"
	"\t\tconst mediump vec3\t\tmyEyePos = vec3(0.0,0.0,10.0);\r\n"
	"\t\tout mediump vec2\t\ttexCoordinate;\r\n"
	"\r\n"
	"\t\tvoid main(void)\r\n"
	"\t\t{\r\n"
	"\t\t\tgl_Position = myMVPMatrix * vec4(myVertex,1);\r\n"
	"\t\t\tmediump vec3 transNormal = normalize(myModelViewIT * myNormal);\r\n"
	"\t\t\t\r\n"
	"\t\t\tmediump vec3 vertexToEye = normalize(myEyePos - myVertex);\r\n"
	"\t\t\tmediump vec3 halfAngle = normalize(vertexToEye + myLightDirection);\r\n"
	"\t\t\t\r\n"
	"\t\t\ttexCoordinate.x = dot(myLightDirection, transNormal);\r\n"
	"\t\t\ttexCoordinate.y = dot(halfAngle, transNormal);\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/VERTEXSHADER]\r\n"
	"\r\n"
	"[FRAGMENTSHADER]\r\n"
	"\tNAME myFragShader\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\r\n"
	"\t\tuniform sampler2D\t\tsampler2d;\r\n"
	"\t\tin mediump vec2\ttexCoordinate;\r\n"
	"\r\n"
	"\t\tlayout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"\t\tvoid main (void)\r\n"
	"\t\t{\r\n"
	"\t\t\tmediump vec3 texColour = vec3(texture(sampler2d, texCoordinate));\r\n"
	"\t\t\toColour = vec4(texColour, 1.0);\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/FRAGMENTSHADER]\r\n"
	"\r\n"
	"[EFFECT]\r\n"
	"\tNAME myEffect\r\n"
	"\r\n"
	"\tATTRIBUTE\tmyVertex\t\t\tPOSITION\t\r\n"
	"\tATTRIBUTE\tmyNormal\t\t\tNORMAL\r\n"
	"\tUNIFORM\t\tmyMVPMatrix\t\t\tWORLDVIEWPROJECTION\t\r\n"
	"\tUNIFORM\t\tmyModelViewIT\t\tWORLDVIEWIT\r\n"
	"\tUNIFORM\t\tsampler2d\t\t\tTEXTURE0\r\n"
	"\r\n"
	"\tTEXTURE\t\t0 anisotropicmap\r\n"
	"\r\n"
	"\tVERTEXSHADER myVertShader\r\n"
	"\tFRAGMENTSHADER myFragShader\r\n"
	"[/EFFECT]\r\n";

// Register anisotropic_lighting.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_anisotropic_lighting_pfx("anisotropic_lighting.pfx", _anisotropic_lighting_pfx, 1739);

// ******** End: anisotropic_lighting.pfx ********

