// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: lattice.pfx ********

// File data
static const char _lattice_pfx[] = 
	"[HEADER]\n"
	"\tVERSION\t\t00.00.00.00\n"
	"\tDESCRIPTION Lattice Example\n"
	"\tCOPYRIGHT\tImagination Technologies Ltd.\n"
	"[/HEADER]\n"
	"\n"
	"[TEXTURES]\n"
	"[/TEXTURES]\n"
	"\n"
	"[VERTEXSHADER]\n"
	"\tNAME myVertShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tattribute vec4\tmyVertex;\n"
	"\t\tattribute vec3\tmyNormal;\n"
	"\t\tattribute vec2\tmyUV;\n"
	"\t\tuniform mat4\tmyMVPMatrix;\n"
	"\t\tuniform mat3\tmyModelViewIT;\n"
	"\n"
	"\t\tconst mediump vec3  LightDirection = vec3(0.0, 0.5, 0.5);\n"
	"\t\tconst mediump vec3  SurfaceColor = vec3(0.9, 0.7, 0.25);\n"
	"\t\tconst mediump vec4\tmyMaterial = vec4(0.5,0.5,5.0,0.8);\n"
	"\n"
	"\t\tvarying mediump vec3  Color;\n"
	"\t\tvarying mediump vec2  texCoord;\n"
	"\n"
	"\t\tvoid main(void)\n"
	"\t\t{\n"
	"\t\t\t// Passthrough UV cordinates\n"
	"\t\t\ttexCoord  = myUV.st;\n"
	"\t\t\t\n"
	"\t\t\t// transform position\n"
	"\t\t\tgl_Position    = myMVPMatrix * myVertex;\n"
	"\t\t\t\n"
	"\t\t\t// transform normal\n"
	"\t\t\tmediump vec3 tnorm   = normalize(myModelViewIT * myNormal);\n"
	"\t\t\t\n"
	"\t\t\t// Calsulate diffuse lighting\n"
	"\t\t\tmediump float DiffuseIntensity = dot(LightDirection, tnorm) * 0.5 + 0.5;\n"
	"\t\t\t\n"
	"\t\t\tmediump float SpecularIntensity = (DiffuseIntensity-myMaterial.w)* myMaterial.z;\n"
	"\t\t\tSpecularIntensity = max(SpecularIntensity,0.0);\n"
	"\n"
	"\t\t\t// Set the colour for the fragment shader\n"
	"\t\t\tColor = (SurfaceColor * DiffuseIntensity) + SpecularIntensity;\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/VERTEXSHADER]\n"
	"\n"
	"[FRAGMENTSHADER]\n"
	"\tNAME myFragShader\n"
	"\t[GLSL_CODE]\n"
	"\t\tvarying mediump vec3  Color;\n"
	"\t\tvarying mediump vec2  texCoord;\n"
	"\n"
	"\t\tconst mediump vec2  Scale = vec2(10, 10);\n"
	"\t\tconst mediump vec2  Threshold = vec2(0.13, 0.13);\n"
	"\n"
	"\n"
	"\t\tvoid main (void)\n"
	"\t\t{\n"
	"\t\t\tmediump float ss = fract(texCoord.s * Scale.s);\n"
	"\t\t\tmediump float tt = fract(texCoord.t * Scale.t);\n"
	"\n"
	"\t\t\tif ((ss > Threshold.s) && (tt > Threshold.t)) discard;\n"
	"\n"
	"\t\t\t\n"
	"\t\t\tgl_FragColor = vec4 (Color, 1.0);\n"
	"\t\t}\n"
	"\t[/GLSL_CODE]\n"
	"[/FRAGMENTSHADER]\n"
	"\n"
	"[EFFECT]\n"
	"\tNAME myEffect\n"
	"\n"
	"\tATTRIBUTE\tmyVertex\t\t\tPOSITION\n"
	"\tATTRIBUTE\tmyNormal\t\t\tNORMAL\n"
	"\tATTRIBUTE\tmyUV\t\t\t\tUV\n"
	"\tUNIFORM\t\tmyMVPMatrix\t\t\tWORLDVIEWPROJECTION\n"
	"\tUNIFORM\t\tmyModelViewIT\t\tWORLDVIEWIT\n"
	"\n"
	"\tVERTEXSHADER myVertShader\n"
	"\tFRAGMENTSHADER myFragShader\n"
	"[/EFFECT]\n";

// Register lattice.pfx in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_lattice_pfx("lattice.pfx", _lattice_pfx, 1933);

// ******** End: lattice.pfx ********

