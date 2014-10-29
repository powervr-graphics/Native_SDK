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
	"#version 300 es\r\n"
	"uniform samplerCube CubeMap;\r\n"
	"\r\n"
	"uniform lowp vec4 FogColour;\r\n"
	"uniform mediump float RcpMaxFogDepth;\r\n"
	"\r\n"
	"#ifdef ENABLE_DISCARD_CLIP\r\n"
	"uniform bool ClipPlaneBool;\r\n"
	"#endif\r\n"
	"in mediump vec3 EyeDir;\r\n"
	"in mediump float VertexHeight;\r\n"
	"#ifdef ENABLE_DISCARD_CLIP\r\n"
	"in highp float ClipDist;\r\n"
	"#endif\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t#ifdef ENABLE_DISCARD_CLIP\r\n"
	"\t\t// Reject fragments behind the clip plane\r\n"
	"\t\tif(ClipDist < 0.0)\r\n"
	"\t\t{\r\n"
	"\t\t\tdiscard; // Too slow for hardware. Left as an example of how not to do this!\r\n"
	"\t\t}\r\n"
	"\t#endif\r\n"
	"\t\r\n"
	"\t// Mix the object's colour with the fogging colour based on fragment's depth\r\n"
	"\tlowp vec3 vFragColour = texture(CubeMap, EyeDir).rgb;\r\n"
	"\t\t\r\n"
	"\t// Test depth\r\n"
	"\tlowp float fFogBlend = 1.0 - clamp(VertexHeight * RcpMaxFogDepth, 0.0, 1.0);\r\n"
	"\tvFragColour.rgb = mix(vFragColour.rgb, FogColour.rgb, fFogBlend);\r\n"
	"\t\t\t\r\n"
	"\toColour = vec4(vFragColour.rgb, 1.0);\r\n"
	"}\r\n";

// Register SkyboxFShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SkyboxFShader_fsh("SkyboxFShader.fsh", _SkyboxFShader_fsh, 919);

// ******** End: SkyboxFShader.fsh ********

