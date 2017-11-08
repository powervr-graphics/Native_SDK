/*!
\brief Defines used internally by the XNB reader.
\file PVRCore/Texture/FileDefinesXNB.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
//!\cond NO_DOXYGEN
namespace pvr {
namespace texture_xnb {
struct FileHeader
{
	uint8_t  identifier[3];
	uint8_t  platform;
	uint8_t  version;
	uint8_t  flags;
	uint32_t fileSize;
};

struct Texture2DHeader
{
	int32_t format;
	uint32_t width;
	uint32_t height;
	uint32_t numMipMaps;
};

struct Texture3DHeader
{
	int32_t format;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t numMipMaps;
};

struct TextureCubeHeader
{
	int32_t format;
	uint32_t size;
	uint32_t numMipMaps;
};

enum Flags
{
	e_fileCompressed = 0x80
};

enum PixelFormat
{
	FormatRGBA,
	FormatBGR565,
	FormatBGRA5551,
	FormatBGRA4444,
	FormatDXT1,
	FormatDXT3,
	FormatDXT5,
	FormatNormalizedByte2,
	FormatNormalizedByte4,
	FormatRGBA1010102,
	FormatRG32,
	FormatRGBA64,
	FormatAlpha8,
	FormatSingle,
	FormatVector2,
	FormatVector4,
	FormatHalfSingle,
	FormatHalfVector2,
	FormatHalfVector4,
	FormatHDRBlendable,

	NumXNBFormats
};

// Magic identifier
static const uint8_t  c_identifier[] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

// Reference number to verify endianness of a file
static const uint32_t c_endianReference = 0x04030201;

// Expected size of a header in file
static const uint32_t c_expectedHeaderSize = 10;

}
}
//!\endcond