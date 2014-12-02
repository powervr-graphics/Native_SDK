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
	"#version 300 es\r\n"
	"\r\n"
	"layout (location = 0) in highp vec3\tinVertex;\r\n"
	"layout (location = 1) in mediump vec3\tinNormal;\r\n"
	"layout (location = 2) in mediump vec2\tinTexCoord;\r\n"
	"\r\n"
	"uniform highp   mat4  MVPMatrix;\r\n"
	"uniform highp   mat4  ShadowProj;\r\n"
	"uniform mediump vec3  LightDirModel;\r\n"
	"uniform mediump vec3  EyePosModel;\r\n"
	"uniform mediump mat3  ModelWorld;\r\n"
	"\r\n"
	"out mediump vec2   TexCoord;\r\n"
	"out mediump vec3   ShadowCoord;\r\n"
	"out mediump vec2   ReflectCoord;\r\n"
	"out lowp    float  LightIntensity;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\t// Transform position\r\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\r\n"
	"\t\t\r\n"
	"\t// Simple diffuse lighting\r\n"
	"\tLightIntensity = max(dot(inNormal, LightDirModel), 0.0);\r\n"
	"\t\r\n"
	"\t// Calculate eye direction in model space\r\n"
	"\tmediump vec3 eyeDir = normalize(inVertex - EyePosModel);\r\n"
	"\t\r\n"
	"\t// reflect eye direction over normal and transform to world space\r\n"
	"\tReflectCoord = vec2(ModelWorld * reflect(eyeDir, inNormal)) * 0.5 + 0.5;\r\n"
	"\t\r\n"
	"\tShadowCoord = (ShadowProj * vec4(inVertex, 1.0)).xyw;\r\n"
	"\tShadowCoord.xy += ShadowCoord.z;\r\n"
	"\tShadowCoord.z *= 2.0;\r\n"
	"\t\r\n"
	"\t// Pass through texcoords\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1091);

// ******** End: VertShader.vsh ********

