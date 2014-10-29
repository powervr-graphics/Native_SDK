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
	"layout (location = 0) in highp vec3\tvertPos;\r\n"
	"layout (location = 1) in highp vec3\tvertNormal;\r\n"
	"layout (location = 2) in highp vec2\tvertUV;\r\n"
	"layout (location = 3) in highp vec3\tvertTangent;\r\n"
	"\r\n"
	"uniform highp mat4\tmModelView;\r\n"
	"uniform highp mat4\tmModelViewProj;\r\n"
	"uniform highp mat3\tmNormal;\r\n"
	"uniform highp vec3\tvLightEyeSpacePos;\r\n"
	"\r\n"
	"out lowp  vec3\tlightDir;\r\n"
	"out lowp  vec3\tviewDir;\r\n"
	"out lowp  vec2\ttexCoord;\r\n"
	"\r\n"
	"const lowp float fParallaxScale = 0.065;\r\n"
	"\r\n"
	"void main(void)\r\n"
	"{\t\r\n"
	"\t// Create a Matrix to transform from eye space to tangent space\r\n"
	"\t// Start by calculating the normal, tangent and binormal.\r\n"
	"\thighp vec3 n = normalize(mNormal * vertNormal);\r\n"
	"\thighp vec3 t = normalize(mNormal * vertTangent);\r\n"
	"\thighp vec3 b = cross(n,t);\r\n"
	"\r\n"
	"\t// Create the matrix from the above\r\n"
	"\thighp mat3 mEyeToTangent = mat3( t.x, b.x, n.x,\r\n"
	"\t\t\t\t\t\t\t   t.y, b.y, n.y,\r\n"
	"\t\t\t\t\t\t\t   t.z, b.z, n.z);\r\n"
	"\t\r\n"
	"\t// Write gl_pos\r\n"
	"\thighp vec4 tempPos = vec4(vertPos, 1.0);\t\t\t\t   \r\n"
	"\tgl_Position = mModelViewProj * tempPos;\r\n"
	"\t\r\n"
	"\t// Translate the view direction into Tangent Space\r\n"
	"\t// Translate the position into eye space\r\n"
	"\ttempPos = mModelView * tempPos;\r\n"
	"\t// Get the vector from the eye to the surface, this is the inverse of tempPos\r\n"
	"\tviewDir = tempPos.xyz;\r\n"
	"\t// Then translate that into Tangent Space (multiplied by parallax scale as only has to\r\n"
	"\t// be done once per surface, not per fragment)\r\n"
	"\tviewDir = normalize(mEyeToTangent * viewDir) * fParallaxScale;\r\n"
	"\t\r\n"
	"\t// Translate the light dir from eye space into Tangent Space\r\n"
	"\tlightDir = normalize(vLightEyeSpacePos - tempPos.xyz);\r\n"
	"\t\r\n"
	"\t// Finally set the texture co-ords\r\n"
	"\ttexCoord = vertUV;\r\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1631);

// ******** End: VertShader.vsh ********

