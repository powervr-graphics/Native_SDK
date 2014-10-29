// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: lattice.pfx ********

// File data
static const char _lattice_pfx[] = 
	"[HEADER]\r\n"
	"\tVERSION\t\t00.00.00.00\r\n"
	"\tDESCRIPTION Lattice Example\r\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\r\n"
	"[/HEADER]\r\n"
	"\r\n"
	"[TEXTURES]\r\n"
	"[/TEXTURES]\r\n"
	"\r\n"
	"[VERTEXSHADER]\r\n"
	"\tNAME myVertShader\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\r\n"
	"\t\tlayout (location = 0) in highp vec4\tmyVertex;\r\n"
	"\t\tlayout (location = 1) in highp vec3\tmyNormal;\r\n"
	"\t\tlayout (location = 2) in mediump vec2 myUV;\r\n"
	"\r\n"
	"\t\tuniform mat4\tmyMVPMatrix;\r\n"
	"\t\tuniform mat3\tmyModelViewIT;\r\n"
	"\r\n"
	"\t\tconst mediump vec3  LightDirection = vec3(0.0, 0.5, 0.5);\r\n"
	"\t\tconst mediump vec3  SurfaceColor = vec3(0.9, 0.7, 0.25);\r\n"
	"\t\tconst mediump vec4\tmyMaterial = vec4(0.5,0.5,5.0,0.8);\r\n"
	"\r\n"
	"\t\tout mediump vec3  Colour;\r\n"
	"\t\tout mediump vec2  texCoord;\r\n"
	"\r\n"
	"\t\tvoid main(void)\r\n"
	"\t\t{\r\n"
	"\t\t\t// Passthrough UV cordinates\r\n"
	"\t\t\ttexCoord  = myUV.st;\r\n"
	"\t\t\t\r\n"
	"\t\t\t// transform position\r\n"
	"\t\t\tgl_Position    = myMVPMatrix * myVertex;\r\n"
	"\t\t\t\r\n"
	"\t\t\t// transform normal\r\n"
	"\t\t\tmediump vec3 tnorm   = normalize(myModelViewIT * myNormal);\r\n"
	"\t\t\t\r\n"
	"\t\t\t// Calsulate diffuse lighting\r\n"
	"\t\t\tmediump float DiffuseIntensity = dot(LightDirection, tnorm) * 0.5 + 0.5;\r\n"
	"\t\t\t\r\n"
	"\t\t\tmediump float SpecularIntensity = (DiffuseIntensity-myMaterial.w)* myMaterial.z;\r\n"
	"\t\t\tSpecularIntensity = max(SpecularIntensity,0.0);\r\n"
	"\r\n"
	"\t\t\t// Set the colour for the fragment shader\r\n"
	"\t\t\tColour = (SurfaceColor * DiffuseIntensity) + SpecularIntensity;\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/VERTEXSHADER]\r\n"
	"\r\n"
	"[FRAGMENTSHADER]\r\n"
	"\tNAME myFragShader\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\r\n"
	"\t\tin mediump vec3  Colour;\r\n"
	"\t\tin mediump vec2  texCoord;\r\n"
	"\r\n"
	"\t\tconst mediump vec2 Scale = vec2(10, 10);\r\n"
	"\t\tconst mediump vec2 Threshold = vec2(0.13, 0.13);\r\n"
	"\r\n"
	"\t\tlayout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"\t\tvoid main (void)\r\n"
	"\t\t{\r\n"
	"\t\t\tmediump float ss = fract(texCoord.s * Scale.s);\r\n"
	"\t\t\tmediump float tt = fract(texCoord.t * Scale.t);\r\n"
	"\r\n"
	"\t\t\tif ((ss > Threshold.s) && (tt > Threshold.t)) discard;\r\n"
	"\r\n"
	"\t\t\toColour = vec4 (Colour, 1.0);\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/FRAGMENTSHADER]\r\n"
	"\r\n"
	"[EFFECT]\r\n"
	"\tNAME myEffect\r\n"
	"\r\n"
	"\tATTRIBUTE\tmyVertex\t\t\tPOSITION\r\n"
	"\tATTRIBUTE\tmyNormal\t\t\tNORMAL\r\n"
	"\tATTRIBUTE\tmyUV\t\t\t\tUV\r\n"
	"\tUNIFORM\t\tmyMVPMatrix\t\t\tWORLDVIEWPROJECTION\r\n"
	"\tUNIFORM\t\tmyModelViewIT\t\tWORLDVIEWIT\r\n"
	"\r\n"
	"\tVERTEXSHADER myVertShader\r\n"
	"\tFRAGMENTSHADER myFragShader\r\n"
	"[/EFFECT]\r\n";

// Register lattice.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_lattice_pfx("lattice.pfx", _lattice_pfx, 2147);

// ******** End: lattice.pfx ********

