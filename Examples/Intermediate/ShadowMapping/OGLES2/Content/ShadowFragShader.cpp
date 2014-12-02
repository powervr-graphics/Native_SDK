// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ShadowFragShader.fsh ********

// File data
static const char _ShadowFragShader_fsh[] = 
	"#ifdef USE_SHADOW_SAMPLERS\n"
	"#\textension GL_EXT_shadow_samplers : require\n"
	"    uniform sampler2DShadow sShadow;\n"
	"#else   \n"
	"    uniform highp sampler2D sShadow;\n"
	"#endif\n"
	"\n"
	"uniform sampler2D       sTexture;\n"
	"\n"
	"varying highp vec4      vProjCoord;\n"
	"varying mediump vec2    texCoord;\n"
	"varying lowp vec3       LightIntensity;\n"
	"\n"
	"void main ()\n"
	"{\t\n"
	"    const lowp float fAmbient = 0.4;\n"
	"    \n"
	"#ifdef USE_SHADOW_SAMPLERS\n"
	"    // Don't completely darken the shadowed areas, assume some ambient light\n"
	"    highp float shadowVal = shadow2DProjEXT(sShadow, vProjCoord) * 0.6 + fAmbient;\n"
	"#else\n"
	"    // Subtract a small magic number to account for floating-point error\n"
	"    highp float comp = (vProjCoord.z / vProjCoord.w) - 0.03;\n"
	"\thighp float depth = texture2DProj(sShadow, vProjCoord).r;\n"
	"    lowp float shadowVal = comp <= depth ? 1.0 : fAmbient;\n"
	"#endif\n"
	"\tlowp vec3 color = texture2D(sTexture, texCoord).rgb * LightIntensity * shadowVal;\n"
	"\tgl_FragColor = vec4(color, 1.0);\n"
	"}\n"
	"\n";

// Register ShadowFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ShadowFragShader_fsh("ShadowFragShader.fsh", _ShadowFragShader_fsh, 939);

// ******** End: ShadowFragShader.fsh ********

