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
	"[HEADER]\t\n"
	"\tVERSION\t\t00.00.00.00\t\n"
	"\tDESCRIPTION Anisotropic Lighting Example\t\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\t\n"
	"[/HEADER]\n"
	"\n"
	"[TEXTURES]\n"
	"\tFILE\tanisotropicmap \tanisotropicmap.pvr\t\tLINEAR-LINEAR-LINEAR\n"
	"[/TEXTURES]\n"
	"\n"
	"[VERTEXSHADER]\n"
	"\tNAME myVertShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tattribute highp vec3\tmyVertex;\n"
	"\t\tattribute mediump vec3\tmyNormal;\n"
	"\t\tuniform mediump mat4\tmyMVPMatrix;\n"
	"\t\tuniform mediump mat3\tmyModelViewIT;\n"
	"\t\tconst mediump vec3\t\tmyLightDirection = vec3(0.577,0.577,0.577);\n"
	"\t\tconst mediump vec3\t\tmyEyePos = vec3(0.0,0.0,10.0);\n"
	"\t\tvarying mediump vec2\ttexCoordinate;\n"
	"\t\tvoid main(void)\n"
	"\t\t{\n"
	"\t\t\tgl_Position = myMVPMatrix * vec4(myVertex,1);\n"
	"\t\t\tmediump vec3 transNormal = normalize(myModelViewIT * myNormal);\n"
	"\t\t\t\n"
	"\t\t\tmediump vec3 vertexToEye = normalize(myEyePos - myVertex);\n"
	"\t\t\tmediump vec3 halfAngle = normalize(vertexToEye + myLightDirection);\n"
	"\t\t\t\n"
	"\t\t\ttexCoordinate.x = dot(myLightDirection, transNormal);\n"
	"\t\t\ttexCoordinate.y = dot(halfAngle, transNormal);\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/VERTEXSHADER]\n"
	"\n"
	"[FRAGMENTSHADER]\n"
	"\tNAME myFragShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tuniform sampler2D\t\tsampler2d;\n"
	"\t\tvarying mediump vec2\ttexCoordinate;\n"
	"\t\tvoid main (void)\n"
	"\t\t{\n"
	"\t\t\tmediump vec3 texColour = vec3(texture2D(sampler2d, texCoordinate));\n"
	"\t\t\tgl_FragColor = vec4(texColour, 1.0);\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/FRAGMENTSHADER]\n"
	"\n"
	"[EFFECT]\n"
	"\tNAME myEffect\n"
	"\n"
	"\tATTRIBUTE\tmyVertex\t\t\tPOSITION\t\n"
	"\tATTRIBUTE\tmyNormal\t\t\tNORMAL\n"
	"\tUNIFORM\t\tmyMVPMatrix\t\t\tWORLDVIEWPROJECTION\t\n"
	"\tUNIFORM\t\tmyModelViewIT\t\tWORLDVIEWIT\n"
	"\tUNIFORM\t\tsampler2d\t\t\tTEXTURE0\n"
	"\n"
	"\tTEXTURE\t\t0 anisotropicmap\n"
	"\n"
	"\tVERTEXSHADER myVertShader\n"
	"\tFRAGMENTSHADER myFragShader\n"
	"[/EFFECT]\n";

// Register anisotropic_lighting.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_anisotropic_lighting_pfx("anisotropic_lighting.pfx", _anisotropic_lighting_pfx, 1567);

// ******** End: anisotropic_lighting.pfx ********

