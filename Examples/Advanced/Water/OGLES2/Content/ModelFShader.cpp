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
	"#define ENABLE_TEXTURE\n"
	"#ifdef ENABLE_TEXTURE\n"
	"uniform sampler2D\t\tModelTexture;\n"
	"#ifdef ENABLE_SPECULAR\n"
	"uniform sampler2D\t\tModelTextureSpec;\n"
	"#endif\n"
	"#endif\n"
	"\n"
	"#ifdef ENABLE_FOG_DEPTH\n"
	"uniform lowp vec3 \t\tFogColour;\n"
	"uniform mediump float \tRcpMaxFogDepth;\n"
	"#endif\n"
	"\n"
	"#ifdef ENABLE_LIGHTING\n"
	"    uniform lowp vec3       EmissiveColour;\n"
	"    uniform lowp vec3       DiffuseColour;\n"
	"\tvarying lowp float\t\tLightIntensity;\n"
	"    #ifdef ENABLE_SPECULAR\n"
	"        uniform lowp vec3       SpecularColour;\n"
	"        varying mediump vec3    EyeDir;\n"
	"        varying mediump vec3    LightDir;\n"
	"\t    varying mediump vec3    Normal;\n"
	"    #endif\n"
	"#endif\n"
	"#ifdef ENABLE_TEXTURE\n"
	"\tvarying mediump vec2   \tTexCoord;\n"
	"#endif\n"
	"#ifdef ENABLE_FOG_DEPTH\n"
	"\tvarying mediump float \tVertexDepth;\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\t#ifdef ONLY_ALPHA\n"
	"\t\tgl_FragColor = vec4(vec3(0.5),0.0);\n"
	"\t#else\n"
	"\t\t#ifdef ENABLE_TEXTURE\n"
	"\t\t\t#ifdef ENABLE_FOG_DEPTH\t\t\n"
	"\t\t\t\t// Mix the object's colour with the fogging colour based on fragment's depth\n"
	"\t\t\t\tlowp vec3 vFragColour = texture2D(ModelTexture, TexCoord).rgb;\n"
	"\t\t\t\t\n"
	"\t\t\t\t// Perform depth test and clamp the values\n"
	"\t\t\t\tlowp float fFogBlend = clamp(VertexDepth * RcpMaxFogDepth, 0.0, 1.0);\n"
	"\t\t\t\t\n"
	"\t\t\t\t#ifdef ENABLE_LIGHTING\n"
	"\t\t\t\t\tvFragColour.rgb = mix(vFragColour.rgb * LightIntensity, FogColour.rgb, fFogBlend);\n"
	"\t\t\t\t#else\n"
	"\t\t\t\t\tvFragColour.rgb = mix(vFragColour.rgb, FogColour.rgb, fFogBlend);\n"
	"\t\t\t\t#endif\n"
	"                gl_FragColor = vec4(vFragColour,1.0);\n"
	"\t\t\t#else\n"
	"\t\t\t\t#ifdef ENABLE_LIGHTING\n"
	"\t                lowp vec3 vSpec = vec3(0.0);\n"
	"                    #ifdef ENABLE_SPECULAR\n"
	"\t                    lowp vec3 N        = normalize(Normal);\n"
	"\t                    lowp vec3 refl     = reflect(-LightDir, N);\n"
	"\t                    lowp float SpecPow = clamp(dot(-EyeDir, refl), 0.0, 1.0);\n"
	"\t                    SpecPow = pow(SpecPow, 4.0);\n"
	"                      \tvSpec = SpecularColour * texture2D(ModelTextureSpec, TexCoord).rgb * SpecPow;\n"
	"\t\t\t\t\t#endif\n"
	"\t                lowp vec4 vTex    = texture2D(ModelTexture, TexCoord);\n"
	"\t                lowp vec3 vDiff   = vTex.rgb * DiffuseColour * max(EmissiveColour, LightIntensity);\n"
	"\t\t\t\t\tgl_FragColor = vec4(vDiff + vSpec, vTex.a);\n"
	"\t\t\t\t#else\n"
	"\t\t\t\t\tgl_FragColor = vec4(texture2D(ModelTexture, TexCoord).rgb, 1.0);\n"
	"\t\t\t\t#endif\n"
	"\t\t\t#endif\n"
	"\t\t#else\n"
	"\t\t\t// Solid colour is used instead of texture colour\n"
	"\t\t\t#ifdef ENABLE_LIGHTING\n"
	"\t\t\t\tgl_FragColor = vec4(vec3(0.3,0.3,0.3)* LightIntensity, 1.0);\n"
	"\t\t\t#else\n"
	"\t\t\t\tgl_FragColor = vec4(vec3(0.3,0.3,0.3), 1.0);\t\n"
	"\t\t\t#endif\n"
	"\t\t#endif\n"
	"\t#endif\n"
	"}\n";

// Register ModelFShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ModelFShader_fsh("ModelFShader.fsh", _ModelFShader_fsh, 2475);

// ******** End: ModelFShader.fsh ********

