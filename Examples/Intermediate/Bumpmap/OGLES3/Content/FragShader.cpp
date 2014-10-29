// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FragShader.fsh ********

// File data
static const char _FragShader_fsh[] = 
	"#version 300 es\r\n"
	"uniform sampler2D  sBaseTex;\r\n"
	"uniform sampler2D  sNormalMap;\r\n"
	"\t\t\r\n"
	"in lowp    vec3  LightVec;\r\n"
	"in mediump vec2  TexCoord;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// read the per-pixel normal from the normal map and expand to [-1, 1]\r\n"
	"\tlowp vec3 normal = texture(sNormalMap, TexCoord).rgb * 2.0 - 1.0;\r\n"
	"\t\r\n"
	"\t// linear interpolations of normals may cause shortened normals and thus\r\n"
	"\t// visible artifacts on low-poly models.\r\n"
	"\t// We omit the normalization here for performance reasons\r\n"
	"\t\r\n"
	"\t// calculate diffuse lighting as the cosine of the angle between light\r\n"
	"\t// direction and surface normal (both in surface local/tangent space)\r\n"
	"\t// We don't have to clamp to 0 here because the framebuffer write will be clamped\r\n"
	"\tlowp float lightIntensity = dot(LightVec, normal);\r\n"
	"\r\n"
	"\t// read base texture and modulate with light intensity\r\n"
	"\tlowp vec3 texColor = texture(sBaseTex, TexCoord).rgb;\t\r\n"
	"\toColour = vec4(texColor * lightIntensity, 1.0);\r\n"
	"}\r\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 981);

// ******** End: FragShader.fsh ********

