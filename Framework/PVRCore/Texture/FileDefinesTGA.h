/*!
\brief Defines used internally by the TGA reader.
\file PVRCore/Texture/FileDefinesTGA.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/CoreIncludes.h"
//!\cond NO_DOXYGEN
namespace pvr {
namespace texture_tga {
struct FileHeader
{
	uint8 identSize;          // size of ID field that follows 18 byte header (0 usually)
	uint8 colorMapType;      // type of color map 0=none, 1=has palette
	uint8 imageType;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed
	int16 colorMapStart;     // first color map entry in palette
	int16 colorMapLength;    // number of colors in palette
	uint8 colorMapBits;      // number of bits per palette entry 15,16,24,32
	int16 xStart;             // image x origin
	int16 yStart;             // image y origin
	int16 width;              // image width in pixels
	int16 height;             // image height in pixels
	uint8 bits;               // image bits per pixel 8,15,16,24,32
	uint8 descriptor;         // image descriptor bits (vh flip bits)
};

namespace ColorMap {
enum Enum
{
	None,
	Paletted
};
};

namespace ImageType {
enum Enum
{
	None,
	Indexed,
	RGB,
	GreyScale,
	RunLengthNone = 8,
	RunLengthIndexed,
	RunLengthRGB,
	RunLengthGreyScale,
	RunLengthHuffmanDelta = 32,
	RunLengthHuffmanDeltaFourPassQuadTree
};
};

enum DescriptorFlag
{
	DescriptorFlagAlpha = 8
};

// Expected size of a header in file
static const uint32 ExpectedHeaderSize = 18;

}
}
//!\endcond