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
	"uniform highp   mat4   ModelViewProjMatrix;\n"
	"uniform highp   vec3   LightDirection;\n"
	"uniform mediump float  VolumeScale;\n"
	"\n"
	"uniform lowp    vec4   FlatColour;\n"
	"varying lowp    vec4   vColour;\n"
	"\n"
	"void main()\n"
	"{\n"
	"/*\n"
	"\tmediump vec3 extrudedPos = inVertex + (VolumeScale * LightDirection) * inExtrude;\n"
	"\tgl_Position = ModelViewProjMatrix * vec4(extrudedPos, 1.0);\n"
	"\t*/\n"
	"\t\n"
	"\tvColour = FlatColour;\n"
	"\n"
	"\tif (inExtrude > 0.0)\n"
	"\t{\n"
	"\t\tmediump vec3 extrudedPos = inVertex + (VolumeScale * LightDirection);\n"
	"\t\tgl_Position = ModelViewProjMatrix * vec4(extrudedPos, 1.0);\n"
	"\t}\n"
	"\telse\n"
	"\t{\n"
	"\t\tgl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);\n"
	"\t}\n"
	"}\n";

// Register ShadowVolVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ShadowVolVertShader_vsh("ShadowVolVertShader.vsh", _ShadowVolVertShader_vsh, 949);

// ******** End: ShadowVolVertShader.vsh ********

