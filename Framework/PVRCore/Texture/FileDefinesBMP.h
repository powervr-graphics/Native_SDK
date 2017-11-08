/*!
\brief Defines used internally by the BMP reader.
\file PVRCore/Texture/FileDefinesBMP.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
//!\cond NO_DOXYGEN
namespace pvr {
namespace texture_bmp {
struct FileHeader
{
	uint16_t signature;
	uint32_t fileSize;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t pixelOffset;
};

struct CoreHeader
{
	uint32_t headerSize;
	uint16_t width;
	uint16_t height;
	uint16_t numPlanes;
	uint16_t bitsPerPixel;
};

struct InfoHeader1
{
	uint32_t headerSize;
	int32_t  width;
	int32_t  height;
	uint16_t numPlanes;
	uint16_t bitsPerPixel;
	uint32_t compressionType;
	uint32_t imageSize;
	int32_t  horizontalPixelsPerMeter;
	int32_t  verticalPixelsPerMeter;
	uint32_t numColorsInTable;
	uint32_t numImportantColors;
};

struct InfoHeader2 : public InfoHeader1   //Adobe Specific
{
	uint32_t redMask;
	uint32_t greenMask;
	uint32_t blueMask;
};

struct InfoHeader3 : public InfoHeader2   //Adobe Specific
{
	uint32_t alphaMask;
};

struct InfoHeader4 : public InfoHeader3
{
	int32_t   colorSpace;
	glm::ivec3 xyzEndPoints[3];
	uint32_t  gammaRed;
	uint32_t  gammaGreen;
	uint32_t  gammaBlue;
};

struct InfoHeader5 : public InfoHeader4
{
	uint32_t intent;
	uint32_t profileData;
	uint32_t profileSize;
	uint32_t	reserved;
};


namespace HeaderSize {
enum Enum
{
	File = 14,
	Core = 12,
	Core2 = 64,
	Info1 = 40,
	Info2 = 52,
	Info3 = 56,
	Info4 = 108,
	Info5 = 124
};
}

namespace CompressionMethod {
enum Enum
{
	None,
	RunLength8,
	RunLength4,
	Bitfields,
	JPEG,
	PNG,
	AlphaBitfields
};
}

namespace ColorSpace {
enum Enum
{
	CalibratedRGB = 0,          // Gamma correction values are supplied.
	sRGB = 0x42475273, // 'sRGB' in ASCII
	Windows = 0x206e6957, // 'Win ' in ASCII
	ProfileLinked = 0x4b4e494c, // 'LINK' in ASCII
	ProfileEmbedded = 0x4445424d  // 'MBED' in ASCII
};
}

static const uint16_t Identifier = 0x4d42; // 'B' 'M' in ASCII
}
}
//!\endcond