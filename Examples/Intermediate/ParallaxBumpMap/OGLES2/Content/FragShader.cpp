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
	"uniform sampler2D\tbasemap;\n"
	"uniform sampler2D\tnormalmap;\n"
	"uniform sampler2D\theightmap;\n"
	"\n"
	"varying lowp vec3\tlightDir;\n"
	"varying lowp vec3\tviewDir;\n"
	"varying lowp vec2\ttexCoord;\n"
	"\n"
	"void main (void)\n"
	"{\n"
	"\t// Normalise the directions in tangent space\n"
	"\tlowp vec3 vLightDir = normalize(lightDir);\n"
	"\t\n"
	"\t// Initial texture read\n"
	"\t// Calculate how far we're shifting by (using parallax scale).\n"
	"\tlowp float fDepth = texture2D(heightmap, texCoord).x;\n"
	"\t\n"
	"\t// Set the UV Coord appropriately\n"
	"\tlowp vec2 vTexCoord = texCoord + (fDepth * viewDir.xy);\n"
	"\t\n"
	"\t// Base map Lookup\n"
	"\tlowp vec3 texColour = texture2D(basemap, vTexCoord).rgb;\n"
	"\t\n"
	"\t// Now do everything else, diffuse, ambient etc.\n"
	"\tlowp vec3 vNormal = (texture2D(normalmap, vTexCoord).rbg)*2.0-1.0;\n"
	"\t\t\n"
	"\t// diffuse lighting\n"
	"\tlowp float diffIntensity = max(dot(vLightDir, vNormal), 0.0);\t\n"
	"\t\n"
	"\t// calculate actual colour\n"
	"\tlowp vec3 colour = vec3(diffIntensity) * texColour;\n"
	"\n"
	"\tgl_FragColor = vec4(colour, 1.0);\n"
	"}";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 927);

// ******** End: FragShader.fsh ********

