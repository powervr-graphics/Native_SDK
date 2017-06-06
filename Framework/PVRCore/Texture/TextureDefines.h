/*!
\brief Defines and constants used by assets Texture handling code.
\file PVRCore/Texture/TextureDefines.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Defines.h"

namespace pvr {
/// <summary>Calculate the offset of a 2D texture</summary>
/// <param name="x">Offset X</param>
/// <param name="y">Offset Y</param>
/// <param name="width">Texture width</param>
/// <returns>Return the offset of a 2D texture</returns>
inline uint64 textureOffset2D(uint64 x, uint64 y, uint64 width) { return ((x) + (y * width)); }

/// <summary>calculate the offset of 3D texture</summary>
/// <param name="x">Offset X</param>
/// <param name="y">Offset Y</param>
/// <param name="z">Offset Z</param>
/// <param name="width">Texture Width</param>
/// <param name="height">Texture Height</param>
/// <returns>Return the offset of a 3D texture</returns>
inline uint64 textureOffset3D(uint64 x, uint64 y, uint64 z, uint64 width, uint64 height)
{
	return ((x) + (y * width) + (z * width * height));
}


//Legacy constants (V1/V2)
enum
{
	PVRTEX_MIPMAP = (1 << 8),   ///< Has mip map levels. DEPRECATED.
	PVRTEX_TWIDDLE = (1 << 9),   ///< Is twiddled. DEPRECATED.
	PVRTEX_BUMPMAP = (1 << 10),     ///< Has normals encoded for a bump map. DEPRECATED.
	PVRTEX_TILING = (1 << 11),    ///< Is bordered for tiled pvr. DEPRECATED.
	PVRTEX_CUBEMAP = (1 << 12), ///< Is a cubemap/skybox. DEPRECATED.}
	PVRTEX_FALSEMIPCOL = (1 << 13),    ///< Are there false colored MIP levels. DEPRECATED.
	PVRTEX_VOLUME = (1 << 14),    ///< Is this a volume texture. DEPRECATED.
	PVRTEX_ALPHA = (1 << 15),    ///< v2.1. Is there transparency info in the texture. DEPRECATED.
	PVRTEX_VERTICAL_FLIP = (1 << 16),    ///< v2.1. Is the texture vertically flipped. DEPRECATED.
	PVRTEX_PIXELTYPE = 0xff,     ///< Pixel type is always in the last 16bits of the flags. DEPRECATED.
	PVRTEX_IDENTIFIER = 0x21525650, ///< The pvr identifier is the characters 'P','V','R'. DEPRECATED.
	PVRTEX_V1_HEADER_SIZE = 44,     ///< Old header size was 44 for identification purposes. DEPRECATED.
	PVRTC2_MIN_TEXWIDTH = 16,     ///< DEPRECATED.
	PVRTC2_MIN_TEXHEIGHT = 8,      ///< DEPRECATED.
	PVRTC4_MIN_TEXWIDTH = 8,      ///< DEPRECATED.
	PVRTC4_MIN_TEXHEIGHT = 8,      ///< DEPRECATED.
};
}