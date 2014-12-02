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
	"[HEADER]\r\n"
	"\tVERSION\t\t00.00.00.00\r\n"
	"\tDESCRIPTION \r\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\r\n"
	"[/HEADER]\r\n"
	"\r\n"
	"[VERTEXSHADER] \r\n"
	"\tNAME \t\tMyVertexShader \r\n"
	"\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\r\n"
	"\t\tlayout (location = 0) in mediump vec4 myVertex;\r\n"
	"\t\tlayout (location = 1) in mediump vec3 myNormal;\r\n"
	"\r\n"
	"\t\tuniform mediump mat4\tmyWVPMatrix;\r\n"
	"\t\tuniform mediump float\tmyAnim;\r\n"
	"\t\tuniform mediump mat3\tmyWorldViewIT;\r\n"
	"\t\tconst vec3 LightPosition = vec3(0.0,4.0,0.0);\r\n"
	"\t\tconst vec3 SurfaceColor = vec3(0.7, 0.8, 0.4);\r\n"
	"\t\tconst float scaleIn = 1.0;\r\n"
	"\t\tconst float scaleOut = 0.1;\r\n"
	"\t\tout highp vec4 Colour;\r\n"
	"\r\n"
	"\t\tvoid main(void)\r\n"
	"\t\t{\r\n"
	"   \r\n"
	"\t\t\tvec3 normal = myNormal; \r\n"
	"\r\n"
	"\t\t\tfloat ripple = 3.0*cos(0.2*myVertex.y + (radians(5.0*myAnim*360.0)));\r\n"
	"\t\t\tfloat ripple2 = -0.5*sin(0.2*myVertex.y + (radians(5.0*myAnim*360.0)));\r\n"
	"\t\r\n"
	"\t\t\tvec3 vertex = myVertex.xyz + vec3(0,0.0, ripple);\r\n"
	"\t\t\tgl_Position = myWVPMatrix * vec4(vertex,1.0);\r\n"
	"\r\n"
	"\t\t\tnormal = normalize(myWorldViewIT * (myNormal + vec3(0,0.0, ripple2)) );\r\n"
	"\t\r\n"
	"\t\t\tvec3 position = vec3(myWVPMatrix * vec4(vertex,1.0));\r\n"
	"    \t\t\tvec3 lightVec   = vec3(0.0,0.0,1.0);\r\n"
	"    \r\n"
	"\t\t\tfloat diffuse   = max(dot(lightVec, normal), 0.0);\r\n"
	"\r\n"
	"\t\t\tif (diffuse < 0.125)\r\n"
	"\t\t\t\t diffuse = 0.125;\r\n"
	"         \r\n"
	"\t\t\tColour = vec4(SurfaceColor * diffuse * 1.5, 1.0);\r\n"
	"\t\t }\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/VERTEXSHADER]\r\n"
	"    \r\n"
	"[FRAGMENTSHADER] \r\n"
	"\tNAME \t\tMyFragmentShader \r\n"
	"\r\n"
	"\t[GLSL_CODE]\r\n"
	"\t\t#version 300 es\r\n"
	"\t\tin highp vec4 Colour;\r\n"
	"\r\n"
	"\t\tlayout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"\t\tvoid main (void)\r\n"
	"\t\t{\r\n"
	"\t\t\toColour = Colour;\r\n"
	"\t\t}\r\n"
	"\t[/GLSL_CODE]\r\n"
	"[/FRAGMENTSHADER]\r\n"
	" \r\n"
	"[EFFECT] \r\n"
	"\tNAME \tmyEffect\r\n"
	"\tATTRIBUTE\tmyVertex\t\tPOSITION\r\n"
	"\tATTRIBUTE\tmyNormal\t\tNORMAL\r\n"
	"\tUNIFORM\t\tmyAnim\t\t\tANIMATION\r\n"
	"\tUNIFORM\t\tmyWorldViewIT\tWORLDVIEWIT\t\r\n"
	"\tUNIFORM\t\tmyWVPMatrix\t\tWORLDVIEWPROJECTION\r\n"
	"\t\r\n"
	"\tVERTEXSHADER MyVertexShader\r\n"
	"\tFRAGMENTSHADER MyFragmentShader\r\n"
	"[/EFFECT]\r\n";

// Register vertex_sine.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_vertex_sine_pfx("vertex_sine.pfx", _vertex_sine_pfx, 1829);

// ******** End: vertex_sine.pfx ********

