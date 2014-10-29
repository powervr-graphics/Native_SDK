// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: SpriteShaderF.fsh ********

// File data
static const char _SpriteShaderF_fsh[] = 
	"#version 300 es\r\n"
	"uniform sampler2D Texture;\r\n"
	"\r\n"
	"in highp vec2 TexCoord;\r\n"
	"in lowp vec4 RGBA;\r\n"
	"\r\n"
	"layout (location = 0) out lowp vec4 oColour;\r\n"
	"\r\n"
	"void main()\r\n"
	"{\r\n"
	"\tlowp vec4 rgba = texture(Texture, TexCoord) * RGBA;\r\n"
	"#ifdef DISPLAY_SPRITE_ALPHA\r\n"
	"\tif(rgba.a < 0.1)\r\n"
	"\t{\r\n"
	"\t\trgba.a = 0.6;\r\n"
	"\t\trgba.rgb = vec3(1.0, 0.0, 0.0);\r\n"
	"\t}\r\n"
	"#endif\t\r\n"
	"\toColour = rgba;\r\n"
	"}";

// Register SpriteShaderF.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SpriteShaderF_fsh("SpriteShaderF.fsh", _SpriteShaderF_fsh, 348);

// ******** End: SpriteShaderF.fsh ********

