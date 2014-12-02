// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: ModelVShader.vsh ********

// File data
static const char _ModelVShader_vsh[] = 
	"#define ENABLE_TEXTURE\n"
	"attribute highp vec3 \tinVertex;\n"
	"attribute highp vec3\tinNormal;\n"
	"attribute highp vec2\tinTexCoord;\n"
	"\n"
	"#ifdef ENABLE_PERTURB_VTX\n"
	"    uniform highp float fTime;\n"
	"#endif\n"
	"\n"
	"uniform highp mat4\t\tMVPMatrix;\n"
	"uniform highp mat4\t\tModelMatrix;\n"
	"uniform mediump vec3\tLightDirection;\n"
	"#ifdef ENABLE_FOG_DEPTH\n"
	"uniform mediump float\tWaterHeight;\t\t//Assume water always lies on the y-axis\n"
	"#endif\n"
	"\n"
	"#ifdef ENABLE_LIGHTING\n"
	"\tvarying lowp float\t\tLightIntensity;\n"
	"\n"
	"    #ifdef ENABLE_SPECULAR\n"
	"        uniform mediump vec3    EyePos;\n"
	"\n"
	"        varying mediump vec3    EyeDir;\n"
	"        varying mediump vec3    LightDir;\n"
	"        varying mediump vec3    Normal;\n"
	"    #endif\n"
	"#endif\n"
	"#ifdef ENABLE_TEXTURE\n"
	"\tvarying mediump vec2 \tTexCoord;\n"
	"#endif\n"
	"#ifdef ENABLE_FOG_DEPTH\n"
	"\tvarying mediump float\tVertexDepth;\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Convert each vertex into projection-space and output the value\n"
	"\thighp vec4 vInVertex   = vec4(inVertex, 1.0);\n"
	"\tmediump vec3 vInNormal = vec3(inNormal);\n"
	"#ifdef ENABLE_PERTURB_VTX\n"
	"\tlowp float fStr      = inTexCoord.x * 0.7;\n"
	"\tmediump float fDroop = 2.0 * inTexCoord.x;\n"
	"\tvInVertex.y += fStr * sin(fTime + vInVertex.x);\n"
	"\tvInVertex.x += fStr * sin(fTime + vInVertex.x);\n"
	"\tvInVertex.z += fDroop*fDroop;\n"
	"\tvInNormal.x += fStr * cos(fTime + vInVertex.x) / 2.0;\n"
	"    vInNormal.z += fStr * sin(fTime + vInVertex.x) / 2.0;\n"
	"\tvInNormal = normalize(vInNormal);\n"
	"#endif\n"
	"\tgl_Position = MVPMatrix * vInVertex;\n"
	"\t\n"
	"\t#ifdef ENABLE_TEXTURE\n"
	"\t\tTexCoord = inTexCoord;\n"
	"\t#endif\n"
	"\t\n"
	"\t#ifdef ENABLE_FOG_DEPTH\n"
	"\t\t// Calculate the vertex's distance under water surface. This assumes clipping has removed all objects above the water\n"
	"\t\tmediump float vVertexHeight = (ModelMatrix * vec4(inVertex,1.0)).y;\n"
	"\t\tVertexDepth = WaterHeight - vVertexHeight;\n"
	"\t#endif\n"
	"\t\n"
	"\t#ifdef ENABLE_LIGHTING\n"
	"\t    // Simple diffuse lighting in world space\n"
	"\t    lowp vec3 N = normalize((ModelMatrix * vec4(vInNormal, 0.0)).xyz);\n"
	"\t    lowp vec3 L = normalize(LightDirection);\n"
	"\t    LightIntensity = 0.3 + max(0.0, dot(N, -L));\n"
	"\t\t#ifdef ENABLE_SPECULAR\n"
	"\t\t\tLightDir       = L;\n"
	"\t\t\tNormal         = N;\n"
	"\t    \tEyeDir         = normalize(EyePos - (ModelMatrix * vInVertex).xyz);\n"
	"    \t#endif\n"
	"\t#endif\n"
	"}\n";

// Register ModelVShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_ModelVShader_vsh("ModelVShader.vsh", _ModelVShader_vsh, 2136);

// ******** End: ModelVShader.vsh ********

