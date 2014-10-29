// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SceneVertShader.vsh ********

// File data
static const char _SceneVertShader_vsh[] = 
	"attribute highp vec4  inVertex;\r\n"
	"attribute highp vec3  inNormal;\r\n"
	"attribute highp vec2  inTexCoord;\r\n"
	"\r\n"
	"uniform highp mat4   MVPMatrix;\r\n"
	"uniform highp vec3   LightDirection;\r\n"
	"uniform highp float  MaterialBias;\r\n"
	"uniform highp float  MaterialScale;\r\n"
	"\r\n"
	"varying lowp vec3  DiffuseLight;\r\n"
	"varying lowp vec3  SpecularLight;\r\n"
	"varying mediump vec2  TexCoord;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tgl_Position = MVPMatrix * inVertex;\r\n"
	"\t\r\n"
	"\tDiffuseLight = vec3(max(dot(inNormal, LightDirection), 0.0));\r\n"
	"\tSpecularLight = vec3(max((DiffuseLight.x - MaterialBias) * MaterialScale, 0.0));\r\n"
	"\t\r\n"
	"\tTexCoord = inTexCoord;\r\n"
	"}\r\n";

// Register SceneVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SceneVertShader_vsh("SceneVertShader.vsh", _SceneVertShader_vsh, 588);

// ******** End: SceneVertShader.vsh ********

