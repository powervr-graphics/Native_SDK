// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ModelFShader.fsh ********

// File data
static const char _ModelFShader_fsh[] = 
	"#version 300 es\r\n"
	"#define ENABLE_TEXTURE\r\n"
	"#ifdef ENABLE_TEXTURE\r\n"
	"uniform sampler2D\t\tModelTexture;\r\n"
	"#ifdef ENABLE_SPECULAR\r\n"
	"uniform sampler2D\t\tModelTextureSpec;\r\n"
	"#endif\r\n"
	"#endif\r\n"
	"\r\n"
	"#ifdef ENABLE_FOG_DEPTH\r\n"
	"uniform lowp vec3 \t\tFogColour;\r\n"
	"uniform mediump float \tRcpMaxFogDepth;\r\n"
	"#endif\r\n"
	"\r\n"
	"#ifdef ENABLE_LIGHTING\r\n"
	"    uniform lowp vec3       EmissiveColour;\r\n"
	"    uniform lowp vec3       DiffuseColour;\r\n"
	"\tin lowp float\t\t\tLightIntensity;\r\n"
	"\t#ifdef ENABLE_SPECULAR\r\n"
	"        uniform lowp vec3       SpecularColour;\r\n"
	"        in mediump vec3    EyeDir;\r\n"
	"        in mediump vec3    LightDir;\r\n"
	"\t    in mediump vec3    Normal;\r\n"
	"    #endif\r\n"
	"#endif\r\n"
	"#ifdef ENABLE_TEXTURE\r\n"
	"\tin mediump vec2   \tTexCoord;\r\n"
	"#endif\r\n"
	"#ifdef ENABLE_FOG_DEPTH\r\n"
	"\tin mediump float \tVertexDepth;\r\n"
	"#endif\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\t\r\n"
	"\t#ifdef ONLY_ALPHA\r\n"
	"\t\toColour = vec4(vec3(0.5),0.0);\r\n"
	"\t#else\r\n"
	"\t\t#ifdef ENABLE_TEXTURE\r\n"
	"\t\t\t#ifdef ENABLE_FOG_DEPTH\t\t\r\n"
	"\t\t\t\t// Mix the object's colour with the fogging colour based on fragment's depth\r\n"
	"\t\t\t\tlowp vec3 vFragColour = texture(ModelTexture, TexCoord).rgb;\r\n"
	"\t\t\t\t\r\n"
	"\t\t\t\t// Perform depth test and clamp the values\r\n"
	"\t\t\t\tlowp float fFogBlend = clamp(VertexDepth * RcpMaxFogDepth, 0.0, 1.0);\r\n"
	"\t\t\t\t\r\n"
	"\t\t\t\t#ifdef ENABLE_LIGHTING\r\n"
	"\t\t\t\t\toColour.rgb = mix(vFragColour.rgb * LightIntensity, FogColour.rgb, fFogBlend);\r\n"
	"\t\t\t\t#else\r\n"
	"\t\t\t\t\toColour.rgb = mix(vFragColour.rgb, FogColour.rgb, fFogBlend);\r\n"
	"\t\t\t\t#endif\r\n"
	"                    oColour.a = 1.0;\r\n"
	"\t\t\t#else\r\n"
	"\t\t\t\t#ifdef ENABLE_LIGHTING\r\n"
	"\t\t\t\t\tlowp vec3 vSpec = vec3(0.0);\r\n"
	"                    #ifdef ENABLE_SPECULAR\r\n"
	"\t                    lowp vec3 N        = normalize(Normal);\r\n"
	"\t                    lowp vec3 refl     = reflect(-LightDir, N);\r\n"
	"\t                    lowp float SpecPow = clamp(dot(-EyeDir, refl), 0.0, 1.0);\r\n"
	"\t                    SpecPow = pow(SpecPow, 4.0);\r\n"
	"                      \tvSpec = SpecularColour * texture(ModelTextureSpec, TexCoord).rgb * SpecPow;\r\n"
	"\t\t\t\t\t#endif\r\n"
	"\t                lowp vec4 vTex    = texture(ModelTexture, TexCoord);\r\n"
	"\t                lowp vec3 vDiff   = vTex.rgb * DiffuseColour * max(EmissiveColour, LightIntensity);\r\n"
	"\t\t\t\t\toColour = vec4(vDiff + vSpec, vTex.a);\r\n"
	"\t\t\t\t#else\r\n"
	"\t\t\t\t\toColour = vec4(texture(ModelTexture, TexCoord).rgb, 1.0);\r\n"
	"\t\t\t\t#endif\r\n"
	"\t\t\t#endif\r\n"
	"\t\t#else\r\n"
	"\t\t\t// Solid colour is used instead of texture colour\r\n"
	"\t\t\t#ifdef ENABLE_LIGHTING\r\n"
	"\t\t\t\toColour = vec4(vec3(0.3,0.3,0.3)* LightIntensity, 1.0);\r\n"
	"\t\t\t#else\r\n"
	"\t\t\t\toColour = vec4(vec3(0.3,0.3,0.3), 1.0);\t\r\n"
	"\t\t\t#endif\r\n"
	"\t\t#endif\r\n"
	"\t#endif\r\n"
	"}\r\n";

// Register ModelFShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ModelFShader_fsh("ModelFShader.fsh", _ModelFShader_fsh, 2515);

// ******** End: ModelFShader.fsh ********

