/*!*********************************************************************************************************************
\file         PVRAssets/Texture/TextureDefines.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Defines and constants used by assets Texture handling code.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"

namespace pvr {
namespace assets {
/*!***************************************************************************
\brief Constant on the Top mipmap level.
*****************************************************************************/
static const int32 c_pvrTextureTopMIPMap = 0;

/*!***************************************************************************
\brief Flag interpreted as All mipmap levels.
*****************************************************************************/
static const int32 c_pvrTextureAllMIPMaps = -1;
}

/*!***************************************************************************
\brief Enumeration of all known Compressed pixel formats.
*****************************************************************************/
enum class CompressedPixelFormat
{
	PVRTCI_2bpp_RGB,
	PVRTCI_2bpp_RGBA,
	PVRTCI_4bpp_RGB,
	PVRTCI_4bpp_RGBA,
	PVRTCII_2bpp,
	PVRTCII_4bpp,
	ETC1,
	DXT1,
	DXT2,
	DXT3,
	DXT4,
	DXT5,

	//These formats are identical to some DXT formats.
	BC1 = DXT1,
	BC2 = DXT3,
	BC3 = DXT5,

	//These are currently unsupported:
	BC4,
	BC5,
	BC6,
	BC7,

	//These are supported
	UYVY,
	YUY2,
	BW1bpp,
	SharedExponentR9G9B9E5,
	RGBG8888,
	GRGB8888,
	ETC2_RGB,
	ETC2_RGBA,
	ETC2_RGB_A1,
	EAC_R11,
	EAC_RG11,

	ASTC_4x4,
	ASTC_5x4,
	ASTC_5x5,
	ASTC_6x5,
	ASTC_6x6,
	ASTC_8x5,
	ASTC_8x6,
	ASTC_8x8,
	ASTC_10x5,
	ASTC_10x6,
	ASTC_10x8,
	ASTC_10x10,
	ASTC_12x10,
	ASTC_12x12,

	ASTC_3x3x3,
	ASTC_4x3x3,
	ASTC_4x4x3,
	ASTC_4x4x4,
	ASTC_5x4x4,
	ASTC_5x5x4,
	ASTC_5x5x5,
	ASTC_6x5x5,
	ASTC_6x6x5,
	ASTC_6x6x6,

	//Invalid value
	NumCompressedPFs
};


/*!***************************************************************************
\brief Enumeration of Datatypes.
*****************************************************************************/
enum class VariableType
{
	UnsignedByteNorm,
	SignedByteNorm,
	UnsignedByte,
	SignedByte,
	UnsignedShortNorm,
	SignedShortNorm,
	UnsignedShort,
	SignedShort,
	UnsignedIntegerNorm,
	SignedIntegerNorm,
	UnsignedInteger,
	SignedInteger,
	SignedFloat,
	Float = SignedFloat, //the name PVRFloat is now deprecated.
	UnsignedFloat,
	NumVarTypes
};
inline bool isVariableTypeSigned(VariableType item) { return (uint32)item < 11 ? (uint32)item & 1 : (uint32)item != 13; }
inline bool isVariableTypeNormalized(VariableType item) { return ((uint32)item < 10) && !((uint32)item & 2); }


namespace assets {

/*!****************************************************************************************
\brief Use this template class to generate a 4 channel PixelID.
\description Use this template class to generate a 4 channel PixelID (64-bit identifier for
       a pixel format used throughout PVR Assets from the channel information.
     Simply define the template parameters for your class and get the ID member.
     EXAMPLE USE: \code  uint64 myPixelID = GeneratePixelType4<'b','g','r','a',8,8,8,8>::ID; \endcode
\param C1Name The Name of the 1st channel (poss. values 'r','g','b','a','l',0)
\param C2Name The Name of the 2nd channel (poss. values 'r','g','b','a','l',0)
\param C3Name The Name of the 3rd channel (poss. values 'r','g','b','a','l',0)
\param C4Name The Name of the 4th channel (poss. values 'r','g','b','a','l',0)
\param C1Bits The number of bits of the 1st channel
\param C2Bits The number of bits of the 2nd channel
\param C3Bits The number of bits of the 3rd channel
\param C4Bits The number of bits of the 4th channel
********************************************************************************************/
template <char8 C1Name, char8 C2Name, char8 C3Name, char8 C4Name,
          uint8 C1Bits, uint8 C2Bits, uint8 C3Bits, uint8 C4Bits>
class GeneratePixelType4
{
public:
	static const uint64 ID =
	  (static_cast<uint64>(C1Name)      + (static_cast<uint64>(C2Name) << 8)  +
	   (static_cast<uint64>(C3Name) << 16) + (static_cast<uint64>(C4Name) << 24) +
	   (static_cast<uint64>(C1Bits) << 32) + (static_cast<uint64>(C2Bits) << 40) +
	   (static_cast<uint64>(C3Bits) << 48) + (static_cast<uint64>(C4Bits) << 56));
};

/*!****************************************************************************************
\brief Use this template class to generate a 3 channel PixelID.
\description Use this template class to generate a 3 channel PixelID (64-bit identifier for
       a pixel format used throughout PVR Assets from the channel information.
     Simply define the template parameters for your class and get the ID member.
     EXAMPLE USE: \code uint64 myPixelID = GeneratePixelType3<'r','g','b',8,8,8>::ID; \endcode
\param C1Name The Name of the 1st channel (poss. values 'r','g','b','a','l',0)
\param C2Name The Name of the 2nd channel (poss. values 'r','g','b','a','l',0)
\param C3Name The Name of the 3rd channel (poss. values 'r','g','b','a','l',0)
\param C1Bits The number of bits of the 1st channel
\param C2Bits The number of bits of the 2nd channel
\param C3Bits The number of bits of the 3rd channel
********************************************************************************************/
template <char8 C1Name, char8 C2Name, char8 C3Name, uint8 C1Bits, uint8 C2Bits, uint8 C3Bits>
class GeneratePixelType3
{
public:
	static const uint64 ID = (static_cast<uint64>(C1Name) + (static_cast<uint64>(C2Name) << 8) + (static_cast<uint64>(C3Name) << 16) +
	                          (static_cast<uint64>(0) << 24) + (static_cast<uint64>(C1Bits) << 32) + (static_cast<uint64>(C2Bits) << 40) +
	                          (static_cast<uint64>(C3Bits) << 48) + (static_cast<uint64>(0) << 56));
};

/*!****************************************************************************************
\brief Use this template class to generate a 2 channel PixelID.
\description Use this template class to generate a 2 channel PixelID (64-bit identifier for
       a pixel format used throughout PVR Assets from the channel information.
     Simply define the template parameters for your class and get the ID member.
     EXAMPLE USE: \code  uint64 myPixelID = GeneratePixelType2<'r', 'a', 8, 8>::ID; \endcode
\param C1Name The Name of the 1st channel (poss. values 'r','g','a','l',0)
\param C2Name The Name of the 2nd channel (poss. values 'r','g','a','l',0)
\param C1Bits The number of bits of the 1st channel
\param C2Bits The number of bits of the 2nd channel
********************************************************************************************/
template <char8 C1Name, char8 C2Name, uint8 C1Bits, uint8 C2Bits>
class GeneratePixelType2
{
public:
	static const uint64 ID =
	  (static_cast<uint64>(C1Name) + (static_cast<uint64>(C2Name) << 8) + (static_cast<uint64>(0) << 16) +
	   (static_cast<uint64>(0) << 24) + (static_cast<uint64>(C1Bits) << 32) + (static_cast<uint64>(C2Bits) << 40) +
	   (static_cast<uint64>(0) << 48) + (static_cast<uint64>(0) << 56));
};

/*!****************************************************************************************
\brief Use this template class to generate a 1 channel PixelID.
\description Use this template class to generate a 1 channel PixelID (64-bit identifier for
       a pixel format used throughout PVR Assets from the channel information.
     Simply define the template parameters for your class and get the ID member.
     EXAMPLE USE:  \code uint64 myPixelID = GeneratePixelType1<'r',8>::ID; \endcode
\param C1Name The Name of the 1st channel (poss. values 'r','a','l',0)
\param C1Bits The number of bits of the 1st channel
********************************************************************************************/
template <char8 C1Name, uint8 C1Bits>
class GeneratePixelType1
{
public:
	static const uint64 ID = (static_cast<uint64>(C1Name) + (static_cast<uint64>(0) << 8) + (static_cast<uint64>(0) << 16) +
	                          (static_cast<uint64>(0) << 24) + (static_cast<uint64>(C1Bits) << 32) + (static_cast<uint64>(0) << 40) +
	                          (static_cast<uint64>(0) << 48) + (static_cast<uint64>(0) << 56));
};

/*!*******************************************************************************************************************************
\brief  Calculate the offset of a 2D texture
\return Return the offset of a 2D texture
\param  x Offset X
\param  y Offset Y
\param  width Texture width
**********************************************************************************************************************************/
inline uint64 textureOffset2D(uint64 x, uint64 y, uint64 width) { return ((x) + (y * width)); }

/*!*******************************************************************************************************************************
\brief  calculate the offset of 3D texture
\return Return the offset of a 3D texture
\param  x Offset X
\param  y Offset Y
\param  z Offset Z
\param  width Texture Width
\param  height Texture Height
**********************************************************************************************************************************/
inline uint64 textureOffset3D(uint64 x, uint64 y, uint64 z, uint64 width, uint64 height)
{
	return ((x) + (y * width) + (z * width * height));
}


/*!***************************************************************************
* Legacy constants (V1/V2)
*****************************************************************************/
const uint32 PVRTEX_MIPMAP      = (1 << 8);   ///< Has mip map levels. DEPRECATED.
const uint32 PVRTEX_TWIDDLE     = (1 << 9);   ///< Is twiddled. DEPRECATED.
const uint32 PVRTEX_BUMPMAP     = (1 << 10);    ///< Has normals encoded for a bump map. DEPRECATED.
const uint32 PVRTEX_TILING      = (1 << 11);    ///< Is bordered for tiled pvr. DEPRECATED.
const uint32 PVRTEX_CUBEMAP     = (1 << 12);    ///< Is a cubemap/skybox. DEPRECATED.
const uint32 PVRTEX_FALSEMIPCOL   = (1 << 13);    ///< Are there false colored MIP levels. DEPRECATED.
const uint32 PVRTEX_VOLUME      = (1 << 14);    ///< Is this a volume texture. DEPRECATED.
const uint32 PVRTEX_ALPHA     = (1 << 15);    ///< v2.1. Is there transparency info in the texture. DEPRECATED.
const uint32 PVRTEX_VERTICAL_FLIP = (1 << 16);    ///< v2.1. Is the texture vertically flipped. DEPRECATED.

const uint32 PVRTEX_PIXELTYPE   = 0xff;     ///< Pixel type is always in the last 16bits of the flags. DEPRECATED.
const uint32 PVRTEX_IDENTIFIER    = 0x21525650; ///< The pvr identifier is the characters 'P','V','R'. DEPRECATED.

const uint32 PVRTEX_V1_HEADER_SIZE  = 44;     ///< Old header size was 44 for identification purposes. DEPRECATED.

const uint32 PVRTC2_MIN_TEXWIDTH  = 16;     ///< DEPRECATED.
const uint32 PVRTC2_MIN_TEXHEIGHT = 8;      ///< DEPRECATED.
const uint32 PVRTC4_MIN_TEXWIDTH  = 8;      ///< DEPRECATED.
const uint32 PVRTC4_MIN_TEXHEIGHT = 8;      ///< DEPRECATED.
const uint32 ETC_MIN_TEXWIDTH   = 4;      ///< DEPRECATED.
const uint32 ETC_MIN_TEXHEIGHT    = 4;      ///< DEPRECATED.
const uint32 DXT_MIN_TEXWIDTH   = 4;      ///< DEPRECATED.
const uint32 DXT_MIN_TEXHEIGHT    = 4;      ///< DEPRECATED.
}
}
