// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ParaboloidVertShader.vsh ********

// File data
static const char _ParaboloidVertShader_vsh[] = 
	"#version 100\n"
	"\n"
	"uniform highp mat4 MVMatrix;\n"
	"uniform mediump vec3 LightDir;\n"
	"uniform mediump vec3 EyePos;\n"
	"uniform highp float Near;\n"
	"uniform highp float Far;\n"
	"\n"
	"attribute highp vec3 inVertex;\n"
	"attribute mediump vec3 inNormal;\n"
	"attribute highp vec2 inTexCoords;\n"
	"\n"
	"varying highp vec2 TexCoords;\n"
	"varying highp float LightIntensity;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position to the paraboloid's view space\n"
	"\tgl_Position = MVMatrix * vec4(inVertex, 1.0);\n"
	"\n"
	"\t// Store the distance\n"
	"\thighp float Distance = -gl_Position.z;\n"
	"\n"
	"\t// Calculate and set the X and Y coordinates\n"
	"\tgl_Position.xyz = normalize(gl_Position.xyz);\n"
	"\tgl_Position.xy /= 1.0 - gl_Position.z;\n"
	"\n"
	"\t// Calculate and set the Z and W coordinates\n"
	"\tgl_Position.z = ((Distance / Far) - 0.5) * 2.0;\n"
	"\tgl_Position.w = 1.0;\n"
	"\t\n"
	"\t// Pass through texture coordinates\n"
	"\tTexCoords = inTexCoords;\n"
	"\n"
	"\t// Calculate light intensity\n"
	"\t// Ambient\n"
	"\tLightIntensity = 0.4;\n"
	"\t\n"
	"\t// Diffuse\n"
	"\tLightIntensity += max(dot(inNormal, LightDir), 0.0) * 0.3;\n"
	"\n"
	"\t// Specular\n"
	"\tmediump vec3 EyeDir = normalize(EyePos - inVertex);\n"
	"\tLightIntensity += pow(max(dot(reflect(-LightDir, inNormal), EyeDir), 0.0), 5.0) * 0.8;\n"
	"}\n";

// Register ParaboloidVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ParaboloidVertShader_vsh("ParaboloidVertShader.vsh", _ParaboloidVertShader_vsh, 1114);

// ******** End: ParaboloidVertShader.vsh ********

