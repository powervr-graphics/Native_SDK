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
	"uniform sampler2D  sBaseTex;\n"
	"uniform sampler2D  sNormalMap;\n"
	"\t\t\n"
	"varying lowp    vec3  LightVec;\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// read the per-pixel normal from the normal map and expand to [-1, 1]\n"
	"\tlowp vec3 normal = texture2D(sNormalMap, TexCoord).rgb * 2.0 - 1.0;\n"
	"\t\n"
	"\t// linear interpolations of normals may cause shortened normals and thus\n"
	"\t// visible artifacts on low-poly models.\n"
	"\t// We omit the normalization here for performance reasons\n"
	"\t\n"
	"\t// calculate diffuse lighting as the cosine of the angle between light\n"
	"\t// direction and surface normal (both in surface local/tangent space)\n"
	"\t// We don't have to clamp to 0 here because the framebuffer write will be clamped\n"
	"\tlowp float lightIntensity = dot(LightVec, normal);\n"
	"\n"
	"\t// read base texture and modulate with light intensity\n"
	"\tlowp vec3 texColor = texture2D(sBaseTex, TexCoord).rgb;\t\n"
	"\tgl_FragColor = vec4(texColor * lightIntensity, 1.0);\n"
	"}\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 911);

// ******** End: FragShader.fsh ********

