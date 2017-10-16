/*!
\brief Defines used internally by the XNB reader.
\file PVRCore/Texture/FileDefinesXNB.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
//!\cond NO_DOXYGEN
namespace pvr
{
	namespace texture_xnb
	{
	struct FileHeader
	{
		uint8  identifier[3];
		uint8  platform;
		uint8  version;
		uint8  flags;
		uint32 fileSize;
	};

	struct Texture2DHeader
	{
		int32 format;
		uint32 width;
		uint32 height;
		uint32 mipMapCount;
	};

	struct Texture3DHeader
	{
		int32 format;
		uint32 width;
		uint32 height;
		uint32 depth;
		uint32 mipMapCount;
	};

	struct TextureCubeHeader
	{
		int32 format;
		uint32 size;
		uint32 mipMapCount;
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

		NumberOfXNBFormats
	};

	// Magic identifier
	static const uint8  c_identifier[] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

	// Reference number to verify endianness of a file
	static const uint32 c_endianReference = 0x04030201;

	// Expected size of a header in file
	static const uint32 c_expectedHeaderSize = 10;

}
}
//!\endcond