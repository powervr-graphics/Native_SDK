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
	"[HEADER]\n"
	"\tVERSION\t\t01.00.00.00\n"
	"\tDESCRIPTION texture example\n"
	"\tCOPYRIGHT\tImg Tec\n"
	"[/HEADER]\n"
	"\n"
	"[TEXTURES] \n"
	"\tFILE basetex \tBasetex.pvr\t\tLINEAR-LINEAR-LINEAR REPEAT-REPEAT\n"
	"\tFILE reflection Reflection.pvr\tLINEAR-LINEAR-LINEAR REPEAT-REPEAT\n"
	"[/TEXTURES]\n"
	"\n"
	"[VERTEXSHADER]\n"
	"\tNAME \t\tVertexShader\n"
	"\n"
	"\t// LOAD GLSL AS CODE\n"
	"\t[GLSL_CODE]\n"
	"\t\tattribute highp   vec4  inVertex;\n"
	"\t\tattribute mediump vec3  inNormal;\n"
	"\t\tattribute mediump vec2  inTexCoord;\n"
	"\n"
	"\t\tuniform highp   mat4  WVPMatrix;\n"
	"\t\tuniform mediump mat3  WorldViewIT;\n"
	"\t\tuniform mediump vec3  LightDirection;\n"
	"\t\tuniform highp   mat4  CustomMatrix;\n"
	"\n"
	"\t\tvarying lowp    float  DiffuseIntensity;\n"
	"\t\tvarying mediump vec2   TexCoord;\n"
	"\t\tvarying mediump vec2   EnvMapCoord;\n"
	"\n"
	"\t\tvoid main()\n"
	"\t\t{\n"
	"\t\t\tgl_Position = WVPMatrix * CustomMatrix * inVertex;\n"
	"\t\t\tmediump vec3 transNormal = normalize(WorldViewIT * inNormal);\n"
	"\t\t\tDiffuseIntensity = 0.5 + dot(transNormal, normalize(LightDirection)) * 0.5;\n"
	"\t\t\tTexCoord = inTexCoord;\n"
	"\t\t\tEnvMapCoord = 0.5 + transNormal.xy * 0.5;\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/VERTEXSHADER]\n"
	"    \n"
	"[FRAGMENTSHADER] \n"
	"\tNAME \t\tFragmentShader \n"
	"\n"
	"\t// LOAD GLSL AS CODE\n"
	"\t[GLSL_CODE]\n"
	"\t\tuniform sampler2D  sBaseTex;\n"
	"\t\tuniform sampler2D  sEnvMap;\n"
	"\t\t\n"
	"\t\tvarying lowp    float  DiffuseIntensity;\n"
	"\t\tvarying mediump vec2   TexCoord;\n"
	"\t\tvarying mediump vec2   EnvMapCoord;\n"
	"\n"
	"\t\tvoid main()\n"
	"\t\t{\n"
	"\t\t\tlowp vec3 envColor = 0.5 * texture2D(sEnvMap, EnvMapCoord).rgb;\n"
	"\t\t\tgl_FragColor.rgb = texture2D(sBaseTex, TexCoord).rgb * (DiffuseIntensity + envColor);\n"
	"\t\t\tgl_FragColor.a = 1.0;\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/FRAGMENTSHADER]\n"
	" \n"
	"[EFFECT] \n"
	"\tNAME \tEffect\n"
	"\t\t\n"
	"\t// GLOBALS UNIFORMS\n"
	"\tUNIFORM WorldViewIT \t\tWORLDVIEWIT\n"
	"\tUNIFORM WVPMatrix \t\t\tWORLDVIEWPROJECTION\n"
	"\tUNIFORM\tLightDirection\t\tLIGHTDIREYE\n"
	"\tUNIFORM\tsBaseTex\t\t\tTEXTURE0\n"
	"\tUNIFORM\tsEnvMap\t\t\t\tTEXTURE1\n"
	"\tUNIFORM CustomMatrix\t\tMYCUSTOMSCALE\n"
	"\n"
	"\t// ATTRIBUTES\n"
	"\tATTRIBUTE \tinVertex\tPOSITION\n"
	"\tATTRIBUTE\tinNormal\tNORMAL\n"
	"\tATTRIBUTE\tinTexCoord\tUV\n"
	"\n"
	"\tVERTEXSHADER   VertexShader\n"
	"\tFRAGMENTSHADER FragmentShader\n"
	"\tTEXTURE 0 basetex\n"
	"\tTEXTURE 1 reflection\n"
	"[/EFFECT]\n";

// Register effect.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_effect_pfx("effect.pfx", _effect_pfx, 1985);

// ******** End: effect.pfx ********

