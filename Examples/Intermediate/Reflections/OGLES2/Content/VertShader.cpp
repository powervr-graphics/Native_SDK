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
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform mediump mat3  ModelWorld;\n"
	"uniform mediump vec3  EyePosModel;\n"
	"\n"
	"varying mediump vec3  ReflectDir;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\t\n"
	"\t// Calculate eye direction in model space\n"
	"\tmediump vec3 eyeDir = normalize(inVertex - EyePosModel);\n"
	"\t\n"
	"\t// reflect eye direction over normal and transform to world space\n"
	"\tReflectDir = ModelWorld * reflect(eyeDir, inNormal);\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 519);

// ******** End: VertShader.vsh ********

