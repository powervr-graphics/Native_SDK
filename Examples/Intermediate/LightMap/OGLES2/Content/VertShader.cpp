// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: VertShader.vsh ********

// File data
static const char _VertShader_vsh[] = 
	"attribute highp   vec3  inVertex;\n"
	"attribute mediump vec3  inNormal;\n"
	"attribute mediump vec2  inTexCoord;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform highp   mat4  ShadowProj;\n"
	"uniform mediump vec3  LightDirModel;\n"
	"uniform mediump vec3  EyePosModel;\n"
	"uniform mediump mat3  ModelWorld;\n"
	"\n"
	"varying mediump vec2   TexCoord;\n"
	"varying mediump vec3   ShadowCoord;\n"
	"varying mediump vec2   ReflectCoord;\n"
	"varying lowp    float  LightIntensity;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\t\t\n"
	"\t// Simple diffuse lighting\n"
	"\tLightIntensity = max(dot(inNormal, LightDirModel), 0.0);\n"
	"\t\n"
	"\t// Calculate eye direction in model space\n"
	"\tmediump vec3 eyeDir = normalize(inVertex - EyePosModel);\n"
	"\t\n"
	"\t// reflect eye direction over normal and transform to world space\n"
	"\tReflectCoord = vec2(ModelWorld * reflect(eyeDir, inNormal)) * 0.5 + 0.5;\n"
	"\t\n"
	"\tShadowCoord = (ShadowProj * vec4(inVertex, 1.0)).xyw;\n"
	"\tShadowCoord.xy += ShadowCoord.z;\n"
	"\tShadowCoord.z *= 2.0;\n"
	"\t\n"
	"\t// Pass through texcoords\n"
	"\tTexCoord = inTexCoord;\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1013);

// ******** End: VertShader.vsh ********

