// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: phong_lighting.pfx ********

// File data
static const char _phong_lighting_pfx[] = 
	"[HEADER]\n"
	"\tVERSION\t\t00.00.00.00\n"
	"\tDESCRIPTION Phong Lighting Example\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\n"
	"[/HEADER]\n"
	"\n"
	"[TEXTURES]\n"
	"[/TEXTURES]\n"
	"\n"
	"[VERTEXSHADER]\n"
	"\tNAME myVertShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tattribute highp vec4\tmyVertex;\n"
	"\t\tattribute mediump vec3\tmyNormal;\n"
	"\t\tuniform mediump mat4\tmyMVPMatrix;\n"
	"\t\tuniform mediump mat3\tmyModelViewIT;\n"
	"\t\tuniform mediump mat4\tmyModelView;\n"
	"\n"
	"\t\tconst mediump vec4\tLightSourcePosition = vec4(-1.0, 3.0, -2.0, 0.0);\n"
	"\t\tconst mediump vec4\tambient = vec4(0.2,0.2,0.2,1.0); \n"
	"\t\tconst mediump vec4\tambientGlobal = vec4(0.2,0.2,0.2,1.0);\n"
	"\t\tconst mediump vec4\tdiffuse = vec4(0.3,0.4,0.1,1.0); \n"
	"\t\t \n"
	"\t\tvarying mediump vec4 color; \n"
	"\n"
	"\t\tvoid main()\n"
	"\t\t{\n"
	"\t\t\tvec4 ecPos;\n"
	"\t\t\tmediump vec3 viewV,ldir;\n"
	"\t\t\tmediump float NdotL,NdotHV;\n"
	"\t\t\tmediump vec3 normal,lightDir,halfVector;\n"
	"\t\t\t\n"
	"\t\t\t// Transform the position\n"
	"\t\t\tgl_Position = myMVPMatrix * myVertex;\n"
	"\t\t\t\n"
	"\t\t\t// Transform the normal\n"
	"\t\t\tnormal = normalize(myModelViewIT * myNormal);\n"
	"\t\t\t\n"
	"\t\t\t// Compute the light's direction \n"
	"\t\t\tecPos = myModelView * myVertex;\n"
	"\t\t\tlightDir = normalize(vec3(LightSourcePosition-ecPos));\n"
	"\n"
	"\t\t\thalfVector = normalize(lightDir + vec3(ecPos));\n"
	"\t\t\t\t\n"
	"\t\t\t/* The ambient terms have been separated since one of them */\n"
	"\t\t\t/* suffers attenuation */\n"
	"\t\t\tcolor = ambientGlobal; \n"
	"\t\t\t\n"
	"\t\t\t/* compute the dot product between normal and normalized lightdir */\n"
	"\t\t\tNdotL = abs(dot(normal,lightDir));\n"
	"\n"
	"\t\t\tif (NdotL > 0.0) \n"
	"\t\t\t{\n"
	"\t\t\t\tcolor += (diffuse * NdotL + ambient);\t\n"
	"\t\t\t\t\n"
	"\t\t\t\tNdotHV = abs(dot(normal,halfVector));\n"
	"\t\t\t\tcolor += vec4(0.6,0.2,0.4,1.0) * pow(NdotHV,50.0);\n"
	"\t\t\t}\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/VERTEXSHADER]\n"
	"\n"
	"[FRAGMENTSHADER]\n"
	"\tNAME myFragShader\n"
	"\t[GLSL_CODE]\n"
	"\n"
	"\tvarying mediump vec4 color;\n"
	"\n"
	"\t\tvoid main()\n"
	"\t\t{\n"
	"\t\t\tgl_FragColor = color;\n"
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
	"\tUNIFORM\t\tmyModelView\t\t\tWORLDVIEW\n"
	"\tUNIFORM\t\tmyModelViewIT\t\tWORLDVIEWIT\n"
	"\n"
	"\tVERTEXSHADER myVertShader\n"
	"\tFRAGMENTSHADER myFragShader\n"
	"[/EFFECT]\n";

// Register phong_lighting.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_phong_lighting_pfx("phong_lighting.pfx", _phong_lighting_pfx, 2001);

// ******** End: phong_lighting.pfx ********

