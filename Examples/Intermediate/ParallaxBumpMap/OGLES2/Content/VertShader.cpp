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
	"attribute highp vec3\tvertPos;\n"
	"attribute highp vec3\tvertNormal;\n"
	"attribute highp vec2\tvertUV;\n"
	"attribute highp vec3\tvertTangent;\n"
	"\n"
	"uniform highp mat4\tmModelView;\n"
	"uniform highp mat4\tmModelViewProj;\n"
	"uniform highp mat3\tmNormal;\n"
	"uniform highp vec3\tvLightEyeSpacePos;\n"
	"\n"
	"varying lowp  vec3\tlightDir;\n"
	"varying lowp  vec3\tviewDir;\n"
	"varying lowp  vec2\ttexCoord;\n"
	"\n"
	"const lowp float fParallaxScale = 0.065;\n"
	"\n"
	"void main(void)\n"
	"{\t\n"
	"\t// Create a Matrix to transform from eye space to tangent space\n"
	"\t// Start by calculating the normal, tangent and binormal.\n"
	"\thighp vec3 n = normalize(mNormal * vertNormal);\n"
	"\thighp vec3 t = normalize(mNormal * vertTangent);\n"
	"\thighp vec3 b = cross(n,t);\n"
	"\n"
	"\t// Create the matrix from the above\n"
	"\thighp mat3 mEyeToTangent = mat3( t.x, b.x, n.x,\n"
	"\t\t\t\t\t\t\t   t.y, b.y, n.y,\n"
	"\t\t\t\t\t\t\t   t.z, b.z, n.z);\n"
	"\t\n"
	"\t// Write gl_pos\n"
	"\thighp vec4 tempPos = vec4(vertPos, 1.0);\t\t\t\t   \n"
	"\tgl_Position = mModelViewProj * tempPos;\n"
	"\t\n"
	"\t// Translate the view direction into Tangent Space\n"
	"\t// Translate the position into eye space\n"
	"\ttempPos = mModelView * tempPos;\n"
	"\t// Get the vector from the eye to the surface, this is the inverse of tempPos\n"
	"\tviewDir = tempPos.xyz;\n"
	"\t// Then translate that into Tangent Space (multiplied by parallax scale as only has to\n"
	"\t// be done once per surface, not per fragment)\n"
	"\tviewDir = normalize(mEyeToTangent * viewDir) * fParallaxScale;\n"
	"\t\n"
	"\t// Translate the light dir from eye space into Tangent Space\n"
	"\tlightDir = normalize(vLightEyeSpacePos - tempPos.xyz);\n"
	"\t\n"
	"\t// Finally set the texture co-ords\n"
	"\ttexCoord = vertUV;\n"
	"}";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1517);

// ******** End: VertShader.vsh ********

