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
	"\tDESCRIPTION OGLES3CubeShadowMapping\r\n"
	"\tCOPYRIGHT\tImagination Technologies\r\n"
	"[/HEADER]\r\n"
	"\r\n"
	"//\r\n"
	"// Effect to render the scene using a cube shadow map\r\n"
	"//\r\n"
	"\r\n"
	"[EFFECT] \r\n"
	"\tNAME \tRenderSceneInstanced\r\n"
	"\t\t\t\r\n"
	"\t// GLOBALS UNIFORMS\r\n"
	"\tUNIFORM uWorldViewProjMatrix\tWORLDVIEWPROJECTION\r\n"
	"\tUNIFORM uWorldITMatrix\t\t\t\tWORLDIT\r\n"
	"\tUNIFORM uLightDirection\t\t\tLIGHTDIRWORLD\r\n"
	"\tUNIFORM uModelColours\t\t\t\tMATERIALCOLORDIFFUSE\r\n"
	"\t\r\n"
	"\t// CUSTOM SEMANTICS\r\n"
	"\tUNIFORM uInstancesPerRow\t\t\tCUSTOMSEMANTIC_INSTANCESPERROW\r\n"
	"\t\r\n"
	"\t// ATTRIBUTES\r\n"
	"\tATTRIBUTE\tinVertex\t\tPOSITION\r\n"
	"\tATTRIBUTE\tinNormal\t\tNORMAL\r\n"
	"\t\r\n"
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
	"\t\t#define VERTEX_ARRAY  0\r\n"
	"\t\t#define NORMAL_ARRAY  1\r\n"
	"\r\n"
	"\t\tlayout (location = VERTEX_ARRAY) in highp vec3 inVertex;\r\n"
	"\t\tlayout (location = NORMAL_ARRAY) in highp vec3 inNormal;\r\n"
	"\r\n"
	"\t\tuniform highp mat4  uWorldViewProjMatrix;\r\n"
	"\t\tuniform highp mat3  uWorldITMatrix;\r\n"
	"\t\tuniform highp vec3  uLightDirection;\r\n"
	"\t\tuniform highp float uInstancesPerRow;\r\n"
	"\t\tuniform lowp  vec3  uModelColours[6];\r\n"
	"\r\n"
	"\t\tout lowp vec3 vDiffuseColour;\r\n"
	"\r\n"
	"\t\tvoid main()\r\n"
	"\t\t{\r\n"
	"\t\t\thighp float fInstanceID = float(gl_InstanceID);\r\n"
	"\t\t\thighp float row = floor(fInstanceID / uInstancesPerRow);\r\n"
	"\t\t\thighp float column = fInstanceID - row * uInstancesPerRow;\r\n"
	"\t\t\thighp vec3 offset = vec3(column, 0.0, row) * 40.0;\r\n"
	"\t\r\n"
	"\t\t\t// Transform position\r\n"
	"\t\t\tgl_Position = uWorldViewProjMatrix * vec4(inVertex + offset, 1.0);\t\r\n"
	"\r\n"
	"\t\t\t// Simple diffuse lighting in model space\r\n"
	"\t\t\thighp vec3 normal = uWorldITMatrix * inNormal;\r\n"
	"\t\t\thighp float LightIntensity = dot(normal, uLightDirection);\t\r\n"
	"\t\t\tlowp int colourindex = int(mod(fInstanceID, 6.0));\r\n"
	"\t\t\tvDiffuseColour = uModelColours[colourindex] * LightIntensity;\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/VERTEXSHADER]\r\n"
	"\r\n"
	"    \r\n"
	"[FRAGMENTSHADER] \r\n"
	"\tNAME \t\tFragmentShader \r\n"
	"\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\t\t\r\n"
	"\t\tin lowp vec3 vDiffuseColour;\r\n"
	"\t\t\r\n"
	"\t\tlayout(location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"\t\tvoid main()\r\n"
	"\t\t{\r\n"
	"\t\t\toColour = vec4(vDiffuseColour, 1.0);\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/FRAGMENTSHADER]\r\n";

// Register effect.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_effect_pfx("effect.pfx", _effect_pfx, 2130);

// ******** End: effect.pfx ********

