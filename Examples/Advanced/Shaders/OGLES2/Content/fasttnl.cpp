// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: fasttnl.pfx ********

// File data
static const char _fasttnl_pfx[] = 
	"[HEADER]\n"
	"\tVERSION\t\t\t00.00.00.00\n"
	"\tDESCRIPTION\t\tFast Transformation and Lighting Example\n"
	"\tCOPYRIGHT\t\tImagination Technologies Ltd.\n"
	"[/HEADER]\n"
	"\n"
	"[TEXTURES]\n"
	"\tFILE base \tbase.pvr\t\tLINEAR-LINEAR-LINEAR\n"
	"[/TEXTURES]\n"
	"\n"
	"[VERTEXSHADER]\n"
	"\tNAME myVertShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tattribute highp vec3\tmyVertex;\n"
	"\t\tattribute mediump vec3\tmyNormal;\n"
	"\t\tattribute mediump vec2\tmyUV;\n"
	"\t\tuniform mediump mat4\tmyMVPMatrix;\n"
	"\t\tuniform mediump mat3\tmyModelViewIT;\n"
	"\t\tconst mediump vec3\t\tmyLightDirection = vec3(0.6,0.6,0.6);\n"
	"\t\tconst mediump vec4\t\tmyMaterial = vec4(0.5,0.5,2.5,0.8);\n"
	"\t\tvarying lowp float\t\tDiffuseIntensity;\n"
	"\t\tvarying lowp float\t\tSpecularIntensity;\n"
	"\t\tvarying lowp vec2\t\ttexCoordinate;\n"
	"\t\tvoid main(void)\n"
	"\t\t{\n"
	"\t\t\t// Transform the position into clipping-space.\n"
	"\t\t\tgl_Position = myMVPMatrix * vec4(myVertex,1);\t\t\t\t\t\t\t\n"
	"\t\t\tmediump vec3 normal = normalize(myModelViewIT * myNormal);\n"
	"\t\t\tDiffuseIntensity = dot(normal, myLightDirection); \n"
	"\t\t\t// Substract and multiply DiffuseIntensity by Specular Bias (w) and Scale (z) to have a 'decent' looking specular effect\n"
	"\t\t\t// See code below for an explanation of how these parameters are calculated\n"
	"\t\t\tSpecularIntensity = max((DiffuseIntensity - myMaterial.w) * myMaterial.z, 0.0); \n"
	"\t\t\t// Pass the UV co-ordinates\n"
	"\t\t\ttexCoordinate = myUV.st;\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/VERTEXSHADER]\n"
	"\n"
	"[FRAGMENTSHADER]\n"
	"\tNAME myFragShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tuniform sampler2D sampler2d;\n"
	"\t\tvarying lowp float\tDiffuseIntensity;\n"
	"\t\tvarying lowp float\tSpecularIntensity;\n"
	"\t\tvarying lowp vec2\ttexCoordinate;\n"
	"\t\tvoid main (void)\n"
	"\t\t{\n"
	"\t\t\tlowp vec3 texColour  = vec3 (texture2D(sampler2d, texCoordinate));\n"
	"\t\t\tlowp vec3 finalColour = (texColour * DiffuseIntensity) + SpecularIntensity;\n"
	"\t\t\tgl_FragColor = vec4(finalColour, 1.0);\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/FRAGMENTSHADER]\n"
	"\n"
	"[EFFECT]\n"
	"\tNAME myEffect\n"
	"\n"
	"\tATTRIBUTE\tmyVertex\t\t\tPOSITION\n"
	"\tATTRIBUTE\tmyNormal\t\t\tNORMAL\n"
	"\tATTRIBUTE\tmyUV\t\t\t\tUV\n"
	"\tUNIFORM\t\tmyMVPMatrix\t\t\tWORLDVIEWPROJECTION\n"
	"\tUNIFORM\t\tmyModelViewIT\t\tWORLDVIEWIT\n"
	"\tUNIFORM\t\tsampler2d\t\t\tTEXTURE0\n"
	"\n"
	"\tTEXTURE 0 base\n"
	"\n"
	"\tVERTEXSHADER myVertShader\n"
	"\tFRAGMENTSHADER myFragShader\n"
	"[/EFFECT]\n";

// Register fasttnl.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_fasttnl_pfx("fasttnl.pfx", _fasttnl_pfx, 2035);

// ******** End: fasttnl.pfx ********

