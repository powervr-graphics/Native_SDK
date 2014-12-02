// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SkyboxFShader.fsh ********

// File data
static const char _SkyboxFShader_fsh[] = 
	"uniform samplerCube CubeMap;\n"
	"\n"
	"uniform lowp vec4 FogColour;\n"
	"uniform mediump float RcpMaxFogDepth;\n"
	"\n"
	"#ifdef ENABLE_DISCARD_CLIP\n"
	"uniform bool ClipPlaneBool;\n"
	"#endif\n"
	"varying mediump vec3 EyeDir;\n"
	"varying mediump float VertexHeight;\n"
	"#ifdef ENABLE_DISCARD_CLIP\n"
	"varying highp float ClipDist;\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t#ifdef ENABLE_DISCARD_CLIP\n"
	"\t\t// Reject fragments behind the clip plane\n"
	"\t\tif(ClipDist < 0.0)\n"
	"\t\t{\n"
	"\t\t\tdiscard; // Too slow for hardware. Left as an example of how not to do this!\n"
	"\t\t}\n"
	"\t#endif\n"
	"\t\n"
	"\t// Mix the object's colour with the fogging colour based on fragment's depth\n"
	"\tlowp vec3 vFragColour = textureCube(CubeMap, EyeDir).rgb;\n"
	"\t\n"
	"\t// Test depth\n"
	"\tlowp float fFogBlend = 1.0 - clamp(VertexHeight * RcpMaxFogDepth, 0.0, 1.0);\n"
	"\tvFragColour.rgb = mix(vFragColour.rgb, FogColour.rgb, fFogBlend);\n"
	"\t\t\n"
	"\tgl_FragColor = vec4(vFragColour.rgb, 1.0);\n"
	"}";

// Register SkyboxFShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SkyboxFShader_fsh("SkyboxFShader.fsh", _SkyboxFShader_fsh, 842);

// ******** End: SkyboxFShader.fsh ********

