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
	"uniform sampler2D  sReflectTex;\n"
	"uniform sampler2D  sNormalMap;\n"
	"\n"
	"uniform bool  bHighDetail;\n"
	"\n"
	"varying mediump vec3  EyeDirection;\n"
	"varying mediump vec2  TexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tif (bHighDetail)\n"
	"\t{\n"
	"\t\t// Get the normal direction per pixel from the normal map\n"
	"\t\t// The tNormal vector is defined in surface local coordinates (tangent space).\n"
	"\t\tmediump vec3 normal = texture2D(sNormalMap, TexCoord).rgb * 2.0 - 1.0;\n"
	"\t\t// reflect(): For the incident vector I and surface orientation N, returns the reflection direction:\n"
	"\t\t// I - 2 * dot(N, I) * N, N must already be normalized in order to achieve the desired result.\n"
	"\t\tmediump vec3 reflectDir = reflect(-EyeDirection, normal);\n"
	"\t\tmediump vec2 reflectCoord = (reflectDir.xy) * 0.5 + 0.5;\n"
	"\t\n"
	"\t\t// Look-up in the 2D texture using the normal map disturbance\n"
	"\t\tgl_FragColor = texture2D(sReflectTex, reflectCoord);\n"
	"\t}\n"
	"\telse\n"
	"\t{\n"
	"\t\tgl_FragColor = texture2D(sReflectTex, TexCoord);\n"
	"\t}\n"
	"}\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 918);

// ******** End: FragShader.fsh ********

