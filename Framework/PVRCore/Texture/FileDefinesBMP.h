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
	uint16	signature;
	uint32	fileSize;
	uint16	reserved1;
	uint16	reserved2;
	uint32	pixelOffset;
};

struct CoreHeader
{
	uint32	headerSize;
	uint16	width;
	uint16	height;
	uint16	numberOfPlanes;
	uint16	bitsPerPixel;
};

struct InfoHeader1
{
	uint32 headerSize;
	int32  width;
	int32  height;
	uint16 numberOfPlanes;
	uint16 bitsPerPixel;
	uint32 compressionType;
	uint32 imageSize;
	int32  horizontalPixelsPerMeter;
	int32  verticalPixelsPerMeter;
	uint32 numberOfColorsInTable;
	uint32 numberOfImportantColors;
};

struct InfoHeader2 : public InfoHeader1   //Adobe Specific
{
	uint32 redMask;
	uint32 greenMask;
	uint32 blueMask;
};

struct InfoHeader3 : public InfoHeader2   //Adobe Specific
{
	uint32 alphaMask;
};

struct InfoHeader4 : public InfoHeader3
{
	int32   colorSpace;
	glm::ivec3 xyzEndPoints[3];
	uint32  gammaRed;
	uint32  gammaGreen;
	uint32  gammaBlue;
};

struct InfoHeader5 : public InfoHeader4
{
	uint32 intent;
	uint32 profileData;
	uint32 profileSize;
	uint32	reserved;
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

static const uint16 Identifier = 0x4d42; // 'B' 'M' in ASCII
}
}
//!\endcond