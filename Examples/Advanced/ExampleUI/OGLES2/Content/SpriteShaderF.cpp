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
	"uniform sampler2D Texture;\n"
	"\n"
	"varying highp vec2 TexCoord;\n"
	"varying lowp vec4 RGBA;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tlowp vec4 rgba = texture2D(Texture, TexCoord) * RGBA;\n"
	"#ifdef DISPLAY_SPRITE_ALPHA\n"
	"\tif(rgba.a < 0.1)\n"
	"\t{\n"
	"\t\trgba.a = 0.6;\n"
	"\t\trgba.rgb = vec3(1.0, 0.0, 0.0);\n"
	"\t}\n"
	"#endif\t\n"
	"\tgl_FragColor = rgba;\n"
	"}";

// Register SpriteShaderF.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_SpriteShaderF_fsh("SpriteShaderF.fsh", _SpriteShaderF_fsh, 284);

// ******** End: SpriteShaderF.fsh ********

