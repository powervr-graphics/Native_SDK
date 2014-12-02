// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: vertex_sine.pfx ********

// File data
static const char _vertex_sine_pfx[] = 
	"[HEADER]\n"
	"\tVERSION\t\t00.00.00.00\n"
	"\tDESCRIPTION \n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\n"
	"[/HEADER]\n"
	"\n"
	"[VERTEXSHADER] \n"
	"\tNAME \t\tMyVertexShader \n"
	"\n"
	"\t[GLSL_CODE]\n"
	"\n"
	"attribute mediump vec4\tmyVertex;\n"
	"attribute mediump vec3\tmyNormal;\n"
	"uniform mediump mat4\tmyWVPMatrix;\n"
	"uniform mediump float\tmyAnim;\n"
	"uniform mediump mat3\tmyWorldViewIT;\n"
	"const vec3 LightPosition = vec3(0.0,4.0,0.0);\n"
	"const vec3 SurfaceColor = vec3(0.7, 0.8, 0.4);\n"
	"const float scaleIn = 1.0;\n"
	"const float scaleOut = 0.1;\n"
	"varying highp vec4 Color;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"   \n"
	"\tvec3 normal = myNormal; \n"
	"\n"
	"\tfloat ripple = 3.0*cos(0.2*myVertex.y + (radians(5.0*myAnim*360.0)));\n"
	"\tfloat ripple2 = -0.5*sin(0.2*myVertex.y + (radians(5.0*myAnim*360.0)));\n"
	"\t\n"
	"\tvec3 vertex = myVertex.xyz + vec3(0,0.0, ripple);\n"
	"    gl_Position = myWVPMatrix * vec4(vertex,1.0);\n"
	"\n"
	"\tnormal = normalize(myWorldViewIT * (myNormal + vec3(0,0.0, ripple2)) );\n"
	"\t\n"
	"\tvec3 position = vec3(myWVPMatrix * vec4(vertex,1.0));\n"
	"    \tvec3 lightVec   = vec3(0.0,0.0,1.0);\n"
	"    \n"
	"    float diffuse   = max(dot(lightVec, normal), 0.0);\n"
	"\n"
	"    if (diffuse < 0.125)\n"
	"         diffuse = 0.125;\n"
	"         \n"
	"    Color = vec4(SurfaceColor * diffuse * 1.5, 1.0);\n"
	" }\n"
	"\n"
	"\t[/GLSL_CODE]\n"
	"[/VERTEXSHADER]\n"
	"    \n"
	"[FRAGMENTSHADER] \n"
	"\tNAME \t\tMyFragmentShader \n"
	"\n"
	"\t[GLSL_CODE]\n"
	"varying highp vec4 Color;\n"
	"\n"
	"void main (void)\n"
	"{\n"
	"    gl_FragColor = Color;\n"
	"}\n"
	"\t[/GLSL_CODE]\n"
	"[/FRAGMENTSHADER]\n"
	" \n"
	"[EFFECT] \n"
	"\tNAME \tmyEffect\n"
	"\tATTRIBUTE\tmyVertex\t\tPOSITION\n"
	"\tATTRIBUTE\tmyNormal\t\tNORMAL\n"
	"\tUNIFORM\t\tmyAnim\t\t\tANIMATION\n"
	"\tUNIFORM\t\tmyWorldViewIT\tWORLDVIEWIT\t\n"
	"\tUNIFORM\t\tmyWVPMatrix\t\tWORLDVIEWPROJECTION\n"
	"\t\n"
	"\tVERTEXSHADER MyVertexShader\n"
	"\tFRAGMENTSHADER MyFragmentShader\n"
	"[/EFFECT]\n";

// Register vertex_sine.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_vertex_sine_pfx("vertex_sine.pfx", _vertex_sine_pfx, 1609);

// ******** End: vertex_sine.pfx ********

