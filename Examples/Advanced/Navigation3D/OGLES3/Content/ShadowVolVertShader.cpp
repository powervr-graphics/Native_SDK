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
	"#version 300 es\r\n"
	"\r\n"
	"/*\r\n"
	"\tThe vertex shader used for extruding the shadow volume along the light \r\n"
	"\tdirection. If inExtrude is > 0 then the vertex of the shadow volume is \r\n"
	"\textruded along the light direction by VolumeScale. If it is 0 then\r\n"
	"\tthe vertex position is calculated as normal.\r\n"
	"*/\r\n"
	"\r\n"
	"layout (location = 0) in highp vec3\tinVertex;\r\n"
	"layout (location = 1) in lowp float\tinExtrude;\r\n"
	"\r\n"
	"uniform highp   mat4   ModelViewProjMatrix;\r\n"
	"uniform highp   vec3   LightDirection;\r\n"
	"uniform mediump float  VolumeScale;\r\n"
	"\r\n"
	"uniform lowp    vec4   FlatColour;\r\n"
	"out lowp    vec4   vColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"/*\r\n"
	"\tmediump vec3 extrudedPos = inVertex + (VolumeScale * LightDirection) * inExtrude;\r\n"
	"\tgl_Position = ModelViewProjMatrix * vec4(extrudedPos, 1.0);\r\n"
	"\t*/\r\n"
	"\t\r\n"
	"\tvColour = FlatColour;\r\n"
	"\r\n"
	"\tif (inExtrude > 0.0)\r\n"
	"\t{\r\n"
	"\t\tmediump vec3 extrudedPos = inVertex + (VolumeScale * LightDirection);\r\n"
	"\t\tgl_Position = ModelViewProjMatrix * vec4(extrudedPos, 1.0);\r\n"
	"\t}\r\n"
	"\telse\r\n"
	"\t{\r\n"
	"\t\tgl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);\r\n"
	"\t}\r\n"
	"}\r\n";

// Register ShadowVolVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ShadowVolVertShader_vsh("ShadowVolVertShader.vsh", _ShadowVolVertShader_vsh, 1026);

// ******** End: ShadowVolVertShader.vsh ********

