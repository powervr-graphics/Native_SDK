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
	"\r\n"
	"uniform sampler2D\tbasemap;\r\n"
	"uniform sampler2D\tnormalmap;\r\n"
	"uniform sampler2D\theightmap;\r\n"
	"\r\n"
	"in lowp vec3 lightDir;\r\n"
	"in lowp vec3 viewDir;\r\n"
	"in lowp vec2 texCoord;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main (void)\r\n"
	"{\r\n"
	"\t// Normalise the directions in tangent space\r\n"
	"\tlowp vec3 vLightDir = normalize(lightDir);\r\n"
	"\t\r\n"
	"\t// Initial texture read\r\n"
	"\t// Calculate how far we're shifting by (using parallax scale).\r\n"
	"\tlowp float fDepth = texture(heightmap, texCoord).x;\r\n"
	"\t\r\n"
	"\t// Set the UV Coord appropriately\r\n"
	"\tlowp vec2 vTexCoord = texCoord + (fDepth * viewDir.xy);\r\n"
	"\t\r\n"
	"\t// Base map Lookup\r\n"
	"\tlowp vec3 texColour = texture(basemap, vTexCoord).rgb;\r\n"
	"\t\r\n"
	"\t// Now do everything else, diffuse, ambient etc.\r\n"
	"\tlowp vec3 vNormal = (texture(normalmap, vTexCoord).rbg)*2.0-1.0;\r\n"
	"\t\t\r\n"
	"\t// diffuse lighting\r\n"
	"\tlowp float diffIntensity = max(dot(vLightDir, vNormal), 0.0);\t\r\n"
	"\t\r\n"
	"\t// calculate actual colour\r\n"
	"\tlowp vec3 colour = vec3(diffIntensity) * texColour;\r\n"
	"\r\n"
	"\toColour = vec4(colour, 1.0);\r\n"
	"}";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 1001);

// ******** End: FragShader.fsh ********

