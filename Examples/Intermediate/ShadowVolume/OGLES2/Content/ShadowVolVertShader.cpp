// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ShadowVolVertShader.vsh ********

// File data
static const char _ShadowVolVertShader_vsh[] = 
	"/*\n"
	"\tThe vertex shader used for extruding the shadow volume along the light \n"
	"\tdirection. If inExtrude is > 0 then the vertex of the shadow volume is \n"
	"\textruded along the light direction by VolumeScale. If it is 0 then\n"
	"\tthe vertex position is calculated as normal.\n"
	"*/\n"
	"\n"
	"attribute highp vec3   inVertex;\n"
	"attribute lowp  float  inExtrude;\n"
	"\n"
	"uniform highp   mat4   MVPMatrix;\n"
	"uniform highp   vec3   LightPosModel;\n"
	"uniform mediump float  VolumeScale;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tif (inExtrude > 0.0)\n"
	"\t{\n"
	"\t\tmediump vec3 lightDir = normalize(inVertex - LightPosModel);\n"
	"\t\tmediump vec3 extrudedPos = inVertex + (VolumeScale * lightDir);\n"
	"\t\tgl_Position = MVPMatrix * vec4(extrudedPos, 1.0);\n"
	"\t}\n"
	"\telse\n"
	"\t{\n"
	"\t\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\t}\n"
	"}\n";

// Register ShadowVolVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ShadowVolVertShader_vsh("ShadowVolVertShader.vsh", _ShadowVolVertShader_vsh, 730);

// ******** End: ShadowVolVertShader.vsh ********

