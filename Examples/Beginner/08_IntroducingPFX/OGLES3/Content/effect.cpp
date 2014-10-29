// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: effect.pfx ********

// File data
static const char _effect_pfx[] = 
	"[HEADER]\r\n"
	"\tVERSION\t\t01.00.00.00\r\n"
	"\tDESCRIPTION texture example\r\n"
	"\tCOPYRIGHT\tImg Tec\r\n"
	"[/HEADER]\r\n"
	"\r\n"
	"[TEXTURES] \r\n"
	"\tFILE basetex \tBasetex.pvr\t\tLINEAR-LINEAR-LINEAR REPEAT-REPEAT\r\n"
	"\tFILE reflection Reflection.pvr\tLINEAR-LINEAR-LINEAR REPEAT-REPEAT\r\n"
	"[/TEXTURES]\r\n"
	"\r\n"
	"[VERTEXSHADER]\r\n"
	"\tNAME \t\tVertexShader\r\n"
	"\r\n"
	"\t// LOAD GLSL AS CODE\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\t\tlayout (location = 0) in highp vec4\tinVertex;\r\n"
	"\t\tlayout (location = 1) in mediump vec3\tinNormal;\r\n"
	"\t\tlayout (location = 2) in mediump vec2\tinTexCoord;\r\n"
	"\r\n"
	"\t\tuniform highp   mat4  WVPMatrix;\r\n"
	"\t\tuniform mediump mat3  WorldViewIT;\r\n"
	"\t\tuniform mediump vec3  LightDirection;\r\n"
	"\t\tuniform highp   mat4  CustomMatrix;\r\n"
	"\r\n"
	"\t\tout lowp    float  DiffuseIntensity;\r\n"
	"\t\tout mediump vec2   TexCoord;\r\n"
	"\t\tout mediump vec2   EnvMapCoord;\r\n"
	"\r\n"
	"\t\tvoid main()\r\n"
	"\t\t{\r\n"
	"\t\t\tgl_Position = WVPMatrix * CustomMatrix * inVertex;\r\n"
	"\t\t\tmediump vec3 transNormal = normalize(WorldViewIT * inNormal);\r\n"
	"\t\t\tDiffuseIntensity = 0.5 + dot(transNormal, normalize(LightDirection)) * 0.5;\r\n"
	"\t\t\tTexCoord = inTexCoord;\r\n"
	"\t\t\tEnvMapCoord = 0.5 + transNormal.xy * 0.5;\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/VERTEXSHADER]\r\n"
	"    \r\n"
	"[FRAGMENTSHADER] \r\n"
	"\tNAME \t\tFragmentShader \r\n"
	"\r\n"
	"\t// LOAD GLSL AS CODE\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\t\tuniform sampler2D  sBaseTex;\r\n"
	"\t\tuniform sampler2D  sEnvMap;\r\n"
	"\t\t\r\n"
	"\t\tin lowp    float  DiffuseIntensity;\r\n"
	"\t\tin mediump vec2   TexCoord;\r\n"
	"\t\tin mediump vec2   EnvMapCoord;\r\n"
	"\r\n"
	"\t\tlayout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"\t\tvoid main()\r\n"
	"\t\t{\r\n"
	"\t\t\tlowp vec3 envColor = 0.5 * texture(sEnvMap, EnvMapCoord).rgb;\r\n"
	"\t\t\toColour.rgb = texture(sBaseTex, TexCoord).rgb * (DiffuseIntensity + envColor);\r\n"
	"\t\t\toColour.a = 1.0;\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/FRAGMENTSHADER]\r\n"
	" \r\n"
	"[EFFECT] \r\n"
	"\tNAME \tEffect\r\n"
	"\t\t\r\n"
	"\t// GLOBALS UNIFORMS\r\n"
	"\tUNIFORM WorldViewIT \t\tWORLDVIEWIT\r\n"
	"\tUNIFORM WVPMatrix \t\t\tWORLDVIEWPROJECTION\r\n"
	"\tUNIFORM\tLightDirection\t\tLIGHTDIREYE\r\n"
	"\tUNIFORM\tsBaseTex\t\t\tTEXTURE0\r\n"
	"\tUNIFORM\tsEnvMap\t\t\t\tTEXTURE1\r\n"
	"\tUNIFORM CustomMatrix\t\tMYCUSTOMSCALE\r\n"
	"\r\n"
	"\t// ATTRIBUTES\r\n"
	"\tATTRIBUTE \tinVertex\tPOSITION\r\n"
	"\tATTRIBUTE\tinNormal\tNORMAL\r\n"
	"\tATTRIBUTE\tinTexCoord\tUV\r\n"
	"\r\n"
	"\tVERTEXSHADER   VertexShader\r\n"
	"\tFRAGMENTSHADER FragmentShader\r\n"
	"\tTEXTURE 0 basetex\r\n"
	"\tTEXTURE 1 reflection\r\n"
	"[/EFFECT]\r\n";

// Register effect.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_effect_pfx("effect.pfx", _effect_pfx, 2154);

// ******** End: effect.pfx ********

