// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: EffectVertShader.vsh ********

// File data
static const char _EffectVertShader_vsh[] = 
	"#version 100\n"
	"\n"
	"#ifdef REFRACT\n"
	"#ifdef CHROMATIC\n"
	"const lowp vec3 Eta = vec3(0.85, 0.87, 0.89);\n"
	"#else\n"
	"const lowp float Eta = 0.87;\n"
	"#endif\n"
	"#endif\n"
	"\n"
	"#if defined(REFLECT) && defined(REFRACT)\n"
	"const lowp float FresnelBias = 0.3;\n"
	"const lowp float FresnelScale = 0.7;\n"
	"const lowp float FresnelPower = 1.5;\n"
	"#endif\n"
	"\n"
	"uniform highp mat4 MVPMatrix;\n"
	"uniform mediump mat3 MMatrix;\n"
	"uniform mediump vec3 EyePos;\n"
	"\n"
	"attribute highp vec3 inVertex;\n"
	"attribute mediump vec3 inNormal;\n"
	"\n"
	"#ifdef REFLECT\n"
	"varying mediump vec3 ReflectDir;\n"
	"#endif\n"
	"\n"
	"#ifdef REFRACT\n"
	"#ifdef CHROMATIC\n"
	"varying mediump vec3 RefractDirRed;\n"
	"varying mediump vec3 RefractDirGreen;\n"
	"varying mediump vec3 RefractDirBlue;\n"
	"#else\n"
	"varying mediump vec3 RefractDir;\n"
	"#endif\n"
	"#endif\n"
	"\n"
	"#if defined(REFLECT) && defined(REFRACT)\n"
	"varying highp float ReflectFactor;\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\t\n"
	"\t// Calculate view direction in model space\n"
	"\tmediump vec3 ViewDir = normalize(inVertex - EyePos);\n"
	"\n"
	"#ifdef REFLECT\t\n"
	"\t// Reflect view direction and transform to world space\n"
	"\tReflectDir = MMatrix * reflect(ViewDir, inNormal);\n"
	"#endif\n"
	"\n"
	"#ifdef REFRACT\n"
	"#ifdef CHROMATIC\n"
	"\t// Refract view direction and transform to world space\n"
	"\tRefractDirRed = MMatrix * refract(ViewDir, inNormal, Eta.r);\n"
	"\tRefractDirGreen = MMatrix * refract(ViewDir, inNormal, Eta.g);\n"
	"\tRefractDirBlue = MMatrix * refract(ViewDir, inNormal, Eta.b);\n"
	"#else\n"
	"\tRefractDir = MMatrix * refract(ViewDir, inNormal, Eta);\n"
	"#endif\n"
	"#endif\n"
	"\n"
	"#if defined(REFLECT) && defined(REFRACT)\n"
	"\t// Calculate the reflection factor\n"
	"\tReflectFactor = FresnelBias + FresnelScale * pow(1.0 + dot(ViewDir, inNormal), FresnelPower);\n"
	"\tReflectFactor = clamp(ReflectFactor, 0.0, 1.0);\n"
	"#endif\n"
	"}\n";

// Register EffectVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_EffectVertShader_vsh("EffectVertShader.vsh", _EffectVertShader_vsh, 1696);

// ******** End: EffectVertShader.vsh ********

