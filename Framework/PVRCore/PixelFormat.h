/*!
\brief Contains the definition of the PixelFormat class used throughout the PowerVR Framework.
\file         PVRCore/PixelFormat.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Base/Types.h"
namespace pvr {
/// <summary>Enumeration of all known Compressed pixel formats.</summary>
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


/// <summary>Enumeration of Datatypes.</summary>
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


/// <summary>The PixelFormat class fully defines a Pixel Format (channels, format, compression, bit width etc.).
/// </summary>
class PixelFormat
{
public:
	/// <summary>64 bit integer representation as 32 lower bits and 32 higher bits</summary>
	struct LowHigh
	{
		uint32  Low;
		uint32  High;
	};

	/// <summary>Default Constructor. Creates an empty pixeltype.</summary>
	PixelFormat() {}

	/// <summary>Initializes a new pixel type from a 64 bit integer value.</summary>
	/// <param name="type">Pixel format type</param>
	/// <returns>Return a new PixelFormat</returns>
	PixelFormat(uint64 type): _format(type) { }

	/// <summary>Initializes a new pixel type from a CompressedPixelFormat type.</summary>
	/// <param name="type">Compressed Pixel Format type</param>
	/// <returns>Return a new PixelFormat</returns>
	PixelFormat(CompressedPixelFormat type) : _format((uint64)type) {}


	/// <summary>Construct a Pixel format from the given channels which takes up to 4 characters (CnName) and 4 values
	/// (CnBits). Any unused channels should be set to 0.</summary>
	/// <param name="C1Name">channel 1 name</param>
	/// <param name="C2Name">channel 2 name</param>
	/// <param name="C3Name">channel 3 name</param>
	/// <param name="C4Name">channel 4 name</param>
	/// <param name="C1Bits">number of bits in channel 1</param>
	/// <param name="C2Bits">number of bits in channel 2</param>
	/// <param name="C3Bits">number of bits in channel 3</param>
	/// <param name="C4Bits">number of bits in channel 4</param>
	/// <remarks>For example: PixelFormat('r','g','b',0,8,8,8,0);</remarks>
	PixelFormat(uint8 C1Name, uint8 C2Name, uint8 C3Name, uint8 C4Name,
	            uint8 C1Bits, uint8 C2Bits, uint8 C3Bits, uint8 C4Bits) :
		_format(C1Name, C2Name, C3Name, C4Name, C1Bits, C2Bits, C3Bits, C4Bits) {}

	/// <summary>Returns the "content", or "name" of a channel, as a character. (normally r,g,b,a,d,s,l,i)</summary>
	/// <param name="channel">The zero-indexed channel of the texture(0, 1, 2, 3)</param>
	/// <returns>Return a character describing the channel contents</returns>
	/// <remarks>For example, the format d24s8 would return 'd' for channel:0, 's' for channel:1, NULL otherwise
	/// </remarks>
	char getChannelContent(uint8 channel)
	{
		if (channel >= 4) { return 0; }
		return _format._pixelTypeChar[channel];
	}

	/// <summary>Get the width of the specified channel</summary>
	/// <param name="channel">The zero-indexed channel of the texture(0, 1, 2, 3)</param>
	/// <returns>Return The number of bits the specified channel takes up.</returns>
	uint8 getChannelBits(uint8 channel)
	{
		if (channel >= 4) { return 0; }
		return _format._pixelTypeChar[channel + 4];
	}

	/// <summary>Get the number of channels in the format.</summary>
	/// <returns>Return the number of channels in the format.</returns>
	uint8 getNumberOfChannels()
	{
		return
		  _format._pixelTypeChar[7] ? 4 :
		  _format._pixelTypeChar[6] ? 3 :
		  _format._pixelTypeChar[5] ? 2 :
		  _format._pixelTypeChar[4] ? 1 : 0;
	}

	/// <summary>Returns true if the format is a "normal" compressed format, i.e. the format is not regular (channel type/
	/// bitrate combination), but excludes some special packed formats that are not compressed, such as shared
	/// exponent formats.</summary>
	uint8 isCompressedFormat()
	{
		return (_format.Part.High == 0) && (_format.Part.Low != (uint32)CompressedPixelFormat::SharedExponentR9G9B9E5);
	}

	/// <summary>Returns if the format is some kind of directly supported format that is not regular (i.e. channel type/
	/// channel bitrate combination). I.e. returns true if the format is any of the formats described in the supported
	/// "compressed" formats enumeration.</summary>
	uint8 isIrregularFormat()
	{
		return _format.Part.High == 0;
	}

	/// <summary>Get the pixel type id</summary>
	/// <returns>Return the pixel type id</returns>
	uint64 getPixelTypeId()const { return _format._pixelTypeID; }

	/// <summary>Get a const pointer to the pixel type char</summary>
	/// <returns>Return a const pointer to the pixel type char</returns>
	const uint8* getPixelTypeChar()const { return _format._pixelTypeChar; }

	/// <summary>Get a pointer to the pixel type char</summary>
	/// <returns>Return a pointer to the pixel type char</returns>
	uint8* getPixelTypeChar() { return _format._pixelTypeChar; }

	/// <summary>Get the pixel format's low and high part</summary>
	/// <returns>Return pixel format's low and high part</returns>
	LowHigh getPart()const { return _format.Part; }

	/// <summary>Get the number of bits per pixel</summary>
	/// <returns>Return the number of bits per pixel</returns>
	uint8 getBitsPerPixel() const
	{
		return _format._pixelTypeChar[4] + _format._pixelTypeChar[5] + _format._pixelTypeChar[6] + _format._pixelTypeChar[7];
	}

	/// <summary>operator==, validate if a given pixel format is same as this.</summary>
	/// <param name="rhs">pixel format to compare</param>
	/// <returns>Return true if the given pixel format is same</returns>
	bool operator ==(const PixelFormat& rhs)const { return getPixelTypeId() == rhs.getPixelTypeId(); }

	/// <summary>operator!=, validate if a give pixel format is not same as this.</summary>
	/// <param name="rhs">pixel format to compare</param>
	/// <returns>Return true if the given pixel format is not same</returns>
	bool operator !=(const PixelFormat& rhs)const { return !(*this == rhs);  }

private:

	union PixelFormatImpl
	{
		/// <summary>Creates an empty pixeltype.</summary>
		/// <returns>A new PixelFormat</returns>
		PixelFormatImpl() : _pixelTypeID(0) { }

		/// <summary>Initializes a new pixel type from a 64 bit integer value.</summary>
		/// <param name="type">The pixel format represented as a 64 bit integer. The value is the same as if you get
		/// _pixelTypeID of this class.</param>
		/// <returns>A new PixelFormat</returns>
		PixelFormatImpl(uint64 type) : _pixelTypeID(type) { }

		/************************************************************************
		\brief  Takes up to 4 characters (CnName) and 4 values (CnBits)
		to create a new PixelFormat. Any unused channels should be set to 0.
		For example: PixelFormat('r','g','b',0,8,8,8,0);
		  \param[in]      C1Name
		  \param[in]      C2Name
		  \param[in]      C3Name
		  \param[in]      C4Name
		  \param[in]      C1Bits
		  \param[in]      C2Bits
		  \param[in]      C3Bits
		  \param[in]      C4Bits
		  \return   A new PixelFormat
		*************************************************************************/
		PixelFormatImpl(uint8 C1Name, uint8 C2Name, uint8 C3Name, uint8 C4Name,
		                uint8 C1Bits, uint8 C2Bits, uint8 C3Bits, uint8 C4Bits)
		{
			_pixelTypeChar[0] = C1Name;
			_pixelTypeChar[1] = C2Name;
			_pixelTypeChar[2] = C3Name;
			_pixelTypeChar[3] = C4Name;
			_pixelTypeChar[4] = C1Bits;
			_pixelTypeChar[5] = C2Bits;
			_pixelTypeChar[6] = C3Bits;
			_pixelTypeChar[7] = C4Bits;
		}


		LowHigh Part;
		uint64  _pixelTypeID;
		uint8 _pixelTypeChar[8];

	};
	PixelFormatImpl _format;


public:
	// List of common used pixel formats
	static const PixelFormat Intensity8; //!< Intensity8

	static const PixelFormat RGB_888;//!<
	static const PixelFormat RGBA_8888; //!< R8 G8 B8 A8

    static const PixelFormat R_8;//!< R8

    static const PixelFormat R_16;//!< R16

	static const PixelFormat R_32;//!< R32

    static const PixelFormat RG_1616;//!< R32
	static const PixelFormat RG_3232;//!< R32
	static const PixelFormat RGB_323232;//!< R32 G32 B32
	static const PixelFormat RGBA_32323232;//!< R32 G32 B32 A32

	static const PixelFormat RGBA_16161616;//!< R16 G16 B16 A16








	static const PixelFormat RG_88;//!< R8 G8

	static const PixelFormat RGB_565;//!< R5 G6 B5

	static const PixelFormat RGBA_4444;//!< R4 G4 B4 A4
	static const PixelFormat RGBA_5551;//!< R5 G5 B5 A1

	static const PixelFormat BGR_888;//!<
	static const PixelFormat BGRA_8888;//!<

    static const PixelFormat ABGR_8888;//!<

	static const PixelFormat Depth8;//!< Depth8
	static const PixelFormat Depth16;//!< Depth16
	static const PixelFormat Depth24;//!< Depth24
	static const PixelFormat Depth32;//!< Depth32
	static const PixelFormat Depth16Stencil8;//!< Depth16 ,Stencil8
	static const PixelFormat Depth24Stencil8;//!< Depth24 ,Stencil8
	static const PixelFormat Depth32Stencil8;//!< Depth32 ,Stencil8
	static const PixelFormat Stencil8;//!< Stencil8

	static const PixelFormat L_32;// !< Luminance32
	static const PixelFormat LA_1616;// !< Luminance16, Alpha16
	static const PixelFormat LA_3232;// !< Luminance32, Alpha32

	static const PixelFormat Unknown;//!< Unknown
};


/// <summary>Use this template class to generate a 4 channel PixelID.</summary>
/// <param name="C1Name">The Name of the 1st channel (poss. values 'r','g','b','a','l',0)</param>
/// <param name="C2Name">The Name of the 2nd channel (poss. values 'r','g','b','a','l',0)</param>
/// <param name="C3Name">The Name of the 3rd channel (poss. values 'r','g','b','a','l',0)</param>
/// <param name="C4Name">The Name of the 4th channel (poss. values 'r','g','b','a','l',0)</param>
/// <param name="C1Bits">The number of bits of the 1st channel</param>
/// <param name="C2Bits">The number of bits of the 2nd channel</param>
/// <param name="C3Bits">The number of bits of the 3rd channel</param>
/// <param name="C4Bits">The number of bits of the 4th channel</param>
/// <remarks>Use this template class to generate a 4 channel PixelID (64-bit identifier for a pixel format used
/// throughout PVR Assets from the channel information. Simply define the template parameters for your class and
/// get the ID member. EXAMPLE USE: <code>uint64 myPixelID = GeneratePixelType4&lt;'b','g','r','a',8,8,8,8&gt;::ID;
/// </code></remarks>
template <char8 C1Name, char8 C2Name, char8 C3Name, char8 C4Name,
          uint8 C1Bits, uint8 C2Bits, uint8 C3Bits, uint8 C4Bits>
class GeneratePixelType4
{
public:
	static const uint64 ID =
	  (static_cast<uint64>(C1Name) + (static_cast<uint64>(C2Name) << 8) +
	   (static_cast<uint64>(C3Name) << 16) + (static_cast<uint64>(C4Name) << 24) +
	   (static_cast<uint64>(C1Bits) << 32) + (static_cast<uint64>(C2Bits) << 40) +
	   (static_cast<uint64>(C3Bits) << 48) + (static_cast<uint64>(C4Bits) << 56));
};

/// <summary>Use this template class to generate a 3 channel PixelID.</summary>
/// <param name="C1Name">The Name of the 1st channel (poss. values 'r','g','b','a','l',0)</param>
/// <param name="C2Name">The Name of the 2nd channel (poss. values 'r','g','b','a','l',0)</param>
/// <param name="C3Name">The Name of the 3rd channel (poss. values 'r','g','b','a','l',0)</param>
/// <param name="C1Bits">The number of bits of the 1st channel</param>
/// <param name="C2Bits">The number of bits of the 2nd channel</param>
/// <param name="C3Bits">The number of bits of the 3rd channel</param>
/// <remarks>Use this template class to generate a 3 channel PixelID (64-bit identifier for a pixel format used
/// throughout PVR Assets from the channel information. Simply define the template parameters for your class and
/// get the ID member. EXAMPLE USE: <code>uint64 myPixelID = GeneratePixelType3&lt;'r','g','b',8,8,8&gt;::ID; </code>
/// </remarks>
template <char8 C1Name, char8 C2Name, char8 C3Name, uint8 C1Bits, uint8 C2Bits, uint8 C3Bits>
class GeneratePixelType3
{
public:
	static const uint64 ID = (static_cast<uint64>(C1Name) + (static_cast<uint64>(C2Name) << 8) + (static_cast<uint64>(C3Name) << 16) +
	                          (static_cast<uint64>(0) << 24) + (static_cast<uint64>(C1Bits) << 32) + (static_cast<uint64>(C2Bits) << 40) +
	                          (static_cast<uint64>(C3Bits) << 48) + (static_cast<uint64>(0) << 56));
};

/// <summary>Use this template class to generate a 2 channel PixelID.</summary>
/// <param name="C1Name">The Name of the 1st channel (poss. values 'r','g','a','l',0)</param>
/// <param name="C2Name">The Name of the 2nd channel (poss. values 'r','g','a','l',0)</param>
/// <param name="C1Bits">The number of bits of the 1st channel</param>
/// <param name="C2Bits">The number of bits of the 2nd channel</param>
/// <remarks>Use this template class to generate a 2 channel PixelID (64-bit identifier for a pixel format used
/// throughout PVR Assets from the channel information. Simply define the template parameters for your class and
/// get the ID member. EXAMPLE USE: <code>uint64 myPixelID = GeneratePixelType2&lt;'r', 'a', 8, 8&gt;::ID; </code>
/// </remarks>
template <char8 C1Name, char8 C2Name, uint8 C1Bits, uint8 C2Bits>
class GeneratePixelType2
{
public:
	static const uint64 ID =
	  (static_cast<uint64>(C1Name) + (static_cast<uint64>(C2Name) << 8) + (static_cast<uint64>(0) << 16) +
	   (static_cast<uint64>(0) << 24) + (static_cast<uint64>(C1Bits) << 32) + (static_cast<uint64>(C2Bits) << 40) +
	   (static_cast<uint64>(0) << 48) + (static_cast<uint64>(0) << 56));
};

/// <summary>Use this template class to generate a 1 channel PixelID.</summary>
/// <param name="C1Name">The Name of the 1st channel (poss. values 'r','a','l',0)</param>
/// <param name="C1Bits">The number of bits of the 1st channel</param>
/// <remarks>Use this template class to generate a 1 channel PixelID (64-bit identifier for a pixel format used
/// throughout PVR Assets from the channel information. Simply define the template parameters for your class and
/// get the ID member. EXAMPLE USE: <code>uint64 myPixelID = GeneratePixelType1&lt;'r',8&gt;::ID; </code></remarks>
template <char8 C1Name, uint8 C1Bits>
class GeneratePixelType1
{
public:
	static const uint64 ID = (static_cast<uint64>(C1Name) + (static_cast<uint64>(0) << 8) + (static_cast<uint64>(0) << 16) +
	                          (static_cast<uint64>(0) << 24) + (static_cast<uint64>(C1Bits) << 32) + (static_cast<uint64>(0) << 40) +
	                          (static_cast<uint64>(0) << 48) + (static_cast<uint64>(0) << 56));
};

}
