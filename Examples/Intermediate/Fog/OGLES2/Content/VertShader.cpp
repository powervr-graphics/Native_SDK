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
	"attribute highp vec4  inVertex;\n"
	"attribute highp vec3  inNormal;\n"
	"attribute highp vec2  inTexCoord;\n"
	"\n"
	"uniform highp   mat4  MVPMatrix;\n"
	"uniform highp   mat4  ModelViewMatrix;\n"
	"uniform highp vec3  LightDirection;\n"
	"// fog uniforms\n"
	"uniform lowp    int    iFogMode;\n"
	"uniform highp float  FogDensity;\n"
	"uniform highp float  FogEnd;\n"
	"uniform highp float  FogRcpEndStartDiff;\n"
	"\n"
	"varying mediump vec2  TexCoord;\n"
	"varying lowp    vec3  DiffuseLight;\n"
	"varying lowp    vec3  FogIntensity;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// transform position to view space as we need the distance to the eye for fog\n"
	"\thighp vec3 viewPos = vec3(ModelViewMatrix * inVertex);\n"
	"\thighp float eyeDist = length(viewPos);\n"
	"\t\n"
	"\t// transform vertex position\n"
	"\tgl_Position = MVPMatrix * inVertex;\n"
	"\t\n"
	"\t// texcoords pass through\n"
	"\tTexCoord = inTexCoord;\n"
	"\n"
	"\t// calculate lighting\n"
	"\t// We use a directional light with direction given in model space\n"
	"\tlowp float DiffuseIntensity = dot(inNormal, normalize(LightDirection));\n"
	"\t\n"
	"\t// clamp negative values and add some ambient light\n"
	"\tDiffuseLight = vec3(max(DiffuseIntensity, 0.0) * 0.5 + 0.5);\n"
	"\n"
	"\t\n"
	"\t// select fog function. 1 is linear, 2 is exponential, 3 is exponential squared, 0 is no fog.\n"
	"\thighp float fogIntensity = 1.0;\n"
	"\tif(iFogMode == 1)\n"
	"\t{\n"
	"\t\tfogIntensity = (FogEnd - eyeDist) * FogRcpEndStartDiff;\n"
	"\t}\n"
	"\telse if(iFogMode >= 2)\n"
	"\t{\n"
	"\t\thighp float scaledDist = eyeDist * FogDensity;\n"
	"\t\tif (iFogMode == 3)\n"
	"\t\t{\n"
	"\t\t\tscaledDist *= scaledDist;\n"
	"\t\t}\n"
	"\t\tfogIntensity = exp2(-scaledDist);\n"
	"\n"
	"\t}\n"
	"\n"
	"\t// clamp the intensity within a valid range\n"
	"\tFogIntensity = vec3(clamp(fogIntensity, 0.0, 1.0));\n"
	"}\n";

// Register VertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_VertShader_vsh("VertShader.vsh", _VertShader_vsh, 1547);

// ******** End: VertShader.vsh ********

