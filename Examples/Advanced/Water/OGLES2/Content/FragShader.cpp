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
	"uniform sampler2D\t\tNormalTex;\n"
	"uniform sampler2D\t\tReflectionTex;\n"
	"#ifdef ENABLE_REFRACTION\n"
	"\tuniform sampler2D\t\tRefractionTex;\n"
	"#endif\n"
	"uniform samplerCube NormalisationCubeMap;\n"
	"\n"
	"uniform mediump mat4\tModelViewMatrix;\n"
	"uniform lowp vec4\t\tWaterColour;\n"
	"uniform mediump float   RcpMaxFogDepth;\n"
	"uniform lowp vec4       FogColour;\n"
	"\n"
	"#ifdef ENABLE_DISTORTION\n"
	"\tuniform mediump float\tWaveDistortion;\n"
	"#endif\n"
	"uniform mediump vec2 \tRcpWindowSize;\n"
	"\n"
	"varying mediump vec2 \tBumpCoord0;\n"
	"varying mediump vec2 \tBumpCoord1;\n"
	"varying highp   vec3\tWaterToEye;\n"
	"varying mediump float\tWaterToEyeLength;\n"
	"\n"
	"void main()\n"
	"{\t\n"
	"\t// Calculate the tex coords of the fragment (using it's position on the screen), normal map is z-axis major.\n"
	"\tlowp vec3 vAccumulatedNormal = vec3(0.0,0.0,1.0);\n"
	"\tmediump vec2 vTexCoord = gl_FragCoord.xy * RcpWindowSize;\n"
	"\n"
	"\t// Test depth for fog\n"
	"\tlowp float fFogBlend = clamp(WaterToEyeLength * RcpMaxFogDepth, 0.0, 1.0);\n"
	"\t\n"
	"\t#ifdef ENABLE_DISTORTION\n"
	"\t\t// When distortion is enabled, use the normal map to calculate perturbation\n"
	"\t\tvAccumulatedNormal = texture2D(NormalTex, BumpCoord0).rgb;\n"
	"\t\tvAccumulatedNormal += texture2D(NormalTex, BumpCoord1).rgb;\n"
	"\t\tvAccumulatedNormal -= 1.0; // Same as * 2.0 - 2.0\n"
	"\t\n"
	"\t\tlowp vec2 vTmp = vAccumulatedNormal.xy;\n"
	"\t\t/* \t\n"
	"\t\t\tDivide by WaterToEyeLength to scale down the distortion\n"
	"\t\t \tof fragments based on their distance from the camera \n"
	"\t\t*/\n"
	"\t\tvTexCoord.xy -= vTmp * (WaveDistortion / WaterToEyeLength);\n"
	"\t#endif\n"
	"\n"
	"#ifdef ENABLE_REFRACTION\n"
	"\tlowp vec4 vReflectionColour = texture2D(ReflectionTex, vTexCoord);\n"
	"\tlowp vec4 vRefractionColour = texture2D(RefractionTex, vTexCoord);\n"
	"\t\n"
	"\t#ifdef ENABLE_FRESNEL\n"
	"\t\t// Calculate the Fresnel term to determine amount of reflection for each fragment\n"
	"\t\t\n"
	"\t\t// Use normalisation cube map instead of normalize() - See section 3.3.1 of white paper for more info\n"
	"\t\tlowp vec3 vWaterToEyeCube = textureCube(NormalisationCubeMap,WaterToEye).rgb * 2.0 - 1.0;\n"
	"\n"
	"\t\t//Normal map uses z-axis major instead of y-axis major, so we have to swizzle to switch the normal map's z and y axis.\n"
	"\t\tmediump float fEyeToNormalAngle = clamp(dot(vWaterToEyeCube,vAccumulatedNormal.xzy),0.0,1.0);\n"
	"\t\t\n"
	"\t\tmediump float fAirWaterFresnel = 1.0 - fEyeToNormalAngle;\n"
	"\t\tfAirWaterFresnel = pow(fAirWaterFresnel, 5.0);\n"
	"\t\tfAirWaterFresnel = (0.98 * fAirWaterFresnel) + 0.02;\t// R(0)-1 = ~0.98 , R(0)= ~0.02\n"
	"\t\tlowp float fTemp = fAirWaterFresnel;\n"
	"\t\t\n"
	"\t\t// Blend reflection and refraction\n"
	"\t\tlowp vec4 vFragColour = mix(vRefractionColour, vReflectionColour, fTemp);\n"
	"\t    gl_FragColor = mix(vFragColour, FogColour, fFogBlend);\n"
	"\t#else\n"
	"\t    lowp vec4 vFragColour = mix(vRefractionColour, vReflectionColour, 0.4);   // Constant mix\n"
	"\t    gl_FragColor = mix(vFragColour, FogColour, fFogBlend);\n"
	"\t#endif\n"
	"#else\n"
	"\tgl_FragColor = mix(texture2D(ReflectionTex, vTexCoord), FogColour, fFogBlend);\t\t\t\t\t// Reflection only\n"
	"#endif\n"
	"}\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 2820);

// ******** End: FragShader.fsh ********

