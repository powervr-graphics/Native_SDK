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
	"\tDESCRIPTION OGLES3TextureArray\r\n"
	"\tCOPYRIGHT\tImagination Technologies\r\n"
	"[/HEADER]\r\n"
	"\r\n"
	"[TEXTURE]\r\n"
	"\tNAME\t\t\ttextureArray\r\n"
	"\tPATH\t\t\ttextureArray.pvr\r\n"
	"\tMINIFICATION\t\tLINEAR\r\n"
	"\tMAGNIFICATION\tLINEAR\r\n"
	"\tMIPMAP\t\t\t\tNEAREST\r\n"
	"\tWRAP_S\t\t\t\tREPEAT\r\n"
	"\tWRAP_T\t\t\t\tREPEAT\r\n"
	"[/TEXTURE]\r\n"
	"\r\n"
	"//\r\n"
	"// Effect to render the scene using a cube shadow map\r\n"
	"//\r\n"
	"\r\n"
	"[EFFECT] \r\n"
	"\tNAME \tRenderTextureArray\r\n"
	"\t\t\r\n"
	"\t// GLOBALS UNIFORMS\t\t\r\n"
	"\tUNIFORM sTextureArray\t\tTEXTURE0\r\n"
	"\t\r\n"
	"\t// ATTRIBUTES\r\n"
	"\tATTRIBUTE inVertex\t\t\tPOSITION\r\n"
	"\tATTRIBUTE\tinTexCoord\t\tUV\r\n"
	"\tATTRIBUTE\tinTexIndex\t\tCUSTOMSEMANTIC_TEXINDEX\r\n"
	"\t\t\r\n"
	"\t// SHADERS\r\n"
	"\tVERTEXSHADER   VertexShader\r\n"
	"\tFRAGMENTSHADER FragmentShader\r\n"
	"[/EFFECT]\r\n"
	"\r\n"
	"[VERTEXSHADER]\r\n"
	"\tNAME \t\tVertexShader\r\n"
	"\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\r\n"
	"\t\t#define VERTEX_ARRAY    0\r\n"
	"\t\t#define TEXCOORD_ARRAY  1\r\n"
	"\t\t#define TEXINDEX_ARRAY  2\r\n"
	"\r\n"
	"\t\tlayout (location = VERTEX_ARRAY)   in highp   vec2  inVertex;\r\n"
	"\t\tlayout (location = TEXCOORD_ARRAY) in mediump vec2  inTexCoord;\r\n"
	"\t\tlayout (location = TEXINDEX_ARRAY) in mediump float inTexIndex;\r\n"
	"\r\n"
	"\t\tout mediump vec3 vTexCoord;\r\n"
	"\r\n"
	"\t\tvoid main() \r\n"
	"\t\t{ \t\t\r\n"
	"\t\t\tgl_Position = vec4(inVertex, 0.0, 1.0);\t\t\t\r\n"
	"\t\t\tvTexCoord.st = inTexCoord;\t\r\n"
	"\t\t\tvTexCoord.p = inTexIndex;\r\n"
	"\t\t} \r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/VERTEXSHADER]\r\n"
	"\r\n"
	"    \r\n"
	"[FRAGMENTSHADER] \r\n"
	"\tNAME \t\tFragmentShader \r\n"
	"\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t  #version 300 es\r\n"
	"\r\n"
	"\t  uniform mediump sampler2DArray sTextureArray;\r\n"
	"\r\n"
	"\t  in mediump vec3 vTexCoord;\r\n"
	"\r\n"
	"\t  layout(location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"\t  void main()\r\n"
	"\t  {\r\n"
	"\t      oColour = texture(sTextureArray, vTexCoord);\t\r\n"
	"\t  }\t\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/FRAGMENTSHADER]\r\n";

// Register effect.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_effect_pfx("effect.pfx", _effect_pfx, 1587);

// ******** End: effect.pfx ********

