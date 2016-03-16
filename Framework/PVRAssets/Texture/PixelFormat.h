/*!*********************************************************************************************************************
\file         PVRAssets/Texture/PixelFormat.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the definition of the PixelFormat class used throughout the PowerVR Framework.
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/Texture/TextureDefines.h"
namespace pvr {
/*!********************************************************************************************************
\brief	The PixelFormat class fully defines a Pixel Format (channels, format, compression, bit width etc.).
**********************************************************************************************************/
class PixelFormat
{
public:
	/*!********************************************************************************************************
	\brief	64 bit integer representation as 32 lower bits and 32 higher bits
	**********************************************************************************************************/
	struct LowHigh
	{
		uint32	Low;
		uint32	High;
	};

	/*!***********************************************************************
	\brief	Default Constructor. Creates an empty pixeltype.
	*************************************************************************/
	PixelFormat() {}

	/*!***********************************************************************
	\param[in]	Type Pixel format type
	\return		Return a new PixelFormat
	\brief	Initializes a new pixel type from a 64 bit integer value.
	*************************************************************************/
	PixelFormat(uint64 Type): m_format(Type) { }

	/*!***********************************************************************
	\param[in]			C1Name channel 1 name
	\param[in]			C2Name channel 2 name
	\param[in]			C3Name channel 3 name
	\param[in]			C4Name channel 4 name
	\param[in]			C1Bits number of bits in channel 1
	\param[in]			C2Bits number of bits in channel 2
	\param[in]			C3Bits number of bits in channel 3
	\param[in]			C4Bits number of bits in channel 4
	\brief 		Construct a Pixel format from the given channels which takes up to 4 characters (CnName) and 4 values (CnBits).
				Any unused channels should be set to 0.
	\description For example: PixelFormat('r','g','b',0,8,8,8,0);
	*************************************************************************/
	PixelFormat(uint8 C1Name, uint8 C2Name, uint8 C3Name, uint8 C4Name,
	            uint8 C1Bits, uint8 C2Bits, uint8 C3Bits, uint8 C4Bits) :
		m_format(C1Name, C2Name, C3Name, C4Name, C1Bits, C2Bits, C3Bits, C4Bits) {}

	/*!***********************************************************************************************************
	\brief Returns the "content", or "name" of a channel, as a character. (normally r,g,b,a,d,s,l,i)
	\param channel The zero-indexed channel of the texture(0, 1, 2, 3)
	\return		   Return a character describing the channel contents
	\description   For example, the format d24s8 would return 'd' for channel:0, 's' for channel:1, NULL otherwise
	*************************************************************************************************************/
	char getChannelContent(uint8 channel)
	{
		if (channel >= 4) { return 0; }
		return m_format.m_pixelTypeChar[channel];
	}

	/*!***********************************************************************************************************
	\brief Get the width of the specified channel
	\param channel The zero-indexed channel of the texture(0, 1, 2, 3)
	\return Return The number of bits the specified channel takes up.
	*************************************************************************************************************/
	uint8 getChannelBits(uint8 channel)
	{
		if (channel > 4) { return 0; }
		return m_format.m_pixelTypeChar[channel + 4];
	}

	/*!***********************************************************************************************************
	\brief Get the number of channels in the format.
	\return	Return the number of channels in the format.
	*************************************************************************************************************/
	uint8 getNumberOfChannels()
	{
		return
		  m_format.m_pixelTypeChar[7] ? 4 :
		  m_format.m_pixelTypeChar[6] ? 3 :
		  m_format.m_pixelTypeChar[5] ? 2 :
		  m_format.m_pixelTypeChar[4] ? 1 : 0;
	}

	/*!***********************************************************************************************************
	\brief Returns true if the format is a "normal" compressed format, i.e. the format is not regular (channel type/
	bitrate combination), but excludes some special packed formats that are not compressed, such as shared exponent
	formats.
	*************************************************************************************************************/
	uint8 isCompressedFormat()
	{
		return (m_format.Part.High == 0) && (m_format.Part.Low != CompressedPixelFormat::SharedExponentR9G9B9E5);
	}

	/*!***********************************************************************************************************
	\brief Returns if the format is some kind of directly supported format that is not regular (i.e. channel type/
	channel bitrate combination). I.e. returns true if the format is any of the formats described in the supported
	"compressed" formats enumeration.
	*************************************************************************************************************/
	uint8 isIrregularFormat()
	{
		return m_format.Part.High == 0;
	}

	/*!*******************************************************************************************************************************
	\brief	Get the pixel type id
	\return	Return the pixel type id
	**********************************************************************************************************************************/
	uint64 getPixelTypeId()const { return m_format.m_pixelTypeID; }

	/*!*******************************************************************************************************************************
	\brief	Get a const pointer to the pixel type char
	\return	Return a const pointer to the pixel type char
	**********************************************************************************************************************************/
	const uint8* getPixelTypeChar()const { return m_format.m_pixelTypeChar; }

	/*!*******************************************************************************************************************************
	\brief	Get a pointer to the pixel type char
	\return	Return a pointer to the pixel type char
	**********************************************************************************************************************************/
	uint8* getPixelTypeChar() { return m_format.m_pixelTypeChar; }

	/*!*******************************************************************************************************************************
	\brief	Get the pixel format's low and high part
	\return	Return pixel format's low and high part
	**********************************************************************************************************************************/
	LowHigh getPart()const { return m_format.Part; }

	/*!*******************************************************************************************************************************
	\brief	Get the number of bits per pixel
	\return	Return the number of bits per pixel
	**********************************************************************************************************************************/
	uint8 getBitsPerPixel() const
	{
		return m_format.m_pixelTypeChar[4] + m_format.m_pixelTypeChar[5] + m_format.m_pixelTypeChar[6] + m_format.m_pixelTypeChar[7];
	}

	/*!*******************************************************************************************************************************
	\brief	operator==, validate if a given pixel format is same as this.
	\return	Return true if the given pixel format is same
	\param	rhs pixel format to compare
	**********************************************************************************************************************************/
	bool operator ==(const PixelFormat& rhs)const {	return getPixelTypeId() == rhs.getPixelTypeId(); }

	/*!*******************************************************************************************************************************
	\brief	operator!=, validate if a give pixel format is not same as this.
	\return	Return true if the given pixel format is not same
	\param	rhs pixel format to compare
	**********************************************************************************************************************************/
	bool operator !=(const PixelFormat& rhs)const {	return !(*this == rhs);  }

private:

	union PixelFormatImpl
	{
		/*!***********************************************************************
			\return		A new PixelFormat
			\description	Creates an empty pixeltype.
			*************************************************************************/
		PixelFormatImpl() : m_pixelTypeID(0) { }

		/*!***********************************************************************
			\param[in]			Type
			\return		A new PixelFormat
			\description	Initializes a new pixel type from a 64 bit integer value.
			*************************************************************************/
		PixelFormatImpl(uint64 Type) : m_pixelTypeID(Type) { }

		/************************************************************************
			\param[in]			C1Name
			\param[in]			C2Name
			\param[in]			C3Name
			\param[in]			C4Name
			\param[in]			C1Bits
			\param[in]			C2Bits
			\param[in]			C3Bits
			\param[in]			C4Bits
			\return		A new PixelFormat
			\description	Takes up to 4 characters (CnName) and 4 values (CnBits)
			to create a new PixelFormat. Any unused channels should be set to 0.
			For example: PixelFormat('r','g','b',0,8,8,8,0);
		*************************************************************************/
		PixelFormatImpl(uint8 C1Name, uint8 C2Name, uint8 C3Name, uint8 C4Name,
		                uint8 C1Bits, uint8 C2Bits, uint8 C3Bits, uint8 C4Bits)
		{
			m_pixelTypeChar[0] = C1Name;
			m_pixelTypeChar[1] = C2Name;
			m_pixelTypeChar[2] = C3Name;
			m_pixelTypeChar[3] = C4Name;
			m_pixelTypeChar[4] = C1Bits;
			m_pixelTypeChar[5] = C2Bits;
			m_pixelTypeChar[6] = C3Bits;
			m_pixelTypeChar[7] = C4Bits;
		}


		LowHigh Part;
		uint64	m_pixelTypeID;
		uint8	m_pixelTypeChar[8];

	};
	PixelFormatImpl	m_format;


public:
	// List of common used pixel formats
	static const PixelFormat Intensity8; //!< Intensity8
	static const PixelFormat RGBA_8888; //!< R8 G8 B8 A8
	static const PixelFormat RGBA_32;//!< R32 G32 B32 A32

	static const PixelFormat RG_88;//!< R8 G8
	static const PixelFormat R_32;//!< R32
	static const PixelFormat RGB_565;//!< R5 G6 B5
	static const PixelFormat RGB_888;//!<
	static const PixelFormat RGBA_4444;//!<	R4 G4 B4 A4
	static const PixelFormat RGBA_5551;//!<	R5 G5 B5 A1

	static const PixelFormat Depth8;//!< Depth8
	static const PixelFormat Depth16;//!< Depth16
	static const PixelFormat Depth24;//!< Depth24
	static const PixelFormat Depth32;//!< Depth32
	static const PixelFormat Depth24Stencil8;//!< Depth24 ,Stencil8
	static const PixelFormat Depth32Stencil8;//!< Depth32 ,Stencil8
	static const PixelFormat Stencil8;//!< Stencil8
	static const PixelFormat Unknown;//!< Unknown
};
}

//
//
//namespace pvr {
//struct DataFormat
//{
//public:
//	enum InterpretationType
//	{
//		SFLOAT = 0, UFLOAT = 1, SINT = 2, UINT = 3, SSCALED = 4, USCALED = 5, UNUSED = 6, SPECIAL1 = 7
//	};
//	//IF ALL HIGH BITS (>=32) ARE ZERO:	//Special compressed type. Low bits an enumeration.
//
//	//IF SOME HIGH BITS (>=32) ARE NOT ZERO
//	// 0 -  4 : Channel 0 type (character a-z packed into 5 bits. 0 means not present)
//	// 5 -  9 : Channel 1 type (character a-z packed into 5 bits. 0 means not present)
//	//10 - 14 : Channel 2 type (character a-z packed into 5 bits. 0 means not present)
//	//15 - 19 : Channel 3 type (character a-z packed into 5 bits. 0 means not present)
//	//20 - 23 : InterpretationType
//	//24 - 32 : RESERVED. must be 0.
//	//32 - 39 : Channel 0 width (0-255)
//	//40 - 47 : Channel 1 width (0-255)
//	//48 - 55 : Channel 2 width (0-255)
//	//56 - 63 : Channel 3 width (0-255)
//
//	uint64 dataFormat; // 0-4:C1, 5-9:C2, 10-14:C3, 15-19: C5, 20-23: Scaling, 25-31:NONE, 32-63: Channel width
//
//	//LOW_5_BITS_SET = 31, bits 6-7 set: 96
//	bool isSpecialFormat() { return !(dataFormat & 0xFFFFFFFF00000000ull); }
//	uint8 getChannelType0() { return (dataFormat & 31) | 96 ; } //Unpacks an english character without case, from 5 bits into an ASCII character
//	uint8 getChannelType1() { return ((dataFormat >> 5) & 31) | 96 ; }
//	uint8 getChannelType2() { return ((dataFormat >> 10) & 31) | 96; }
//	uint8 getChannelType3() { return ((dataFormat >> 15) & 31) | 96; }
//	uint8 getChannelType(uint8 index) { return ((dataFormat >> (index * 5)) & 31) | 96; }
//	uint8 getChannelWidth0() { return (dataFormat >> 32) & 0xFF; }
//	uint8 getChannelWidth1() { return (dataFormat >> 40) & 0xFF; }
//	uint8 getChannelWidth2() { return (dataFormat >> 48) & 0xFF; }
//	uint8 getChannelWidth3() { return (dataFormat >> 56) & 0xFF; }
//	uint8 getChannelWidth(uint8 index) { return (dataFormat >> (32 + index * 8)) & 0xFF; }
//	InterpretationType getInterpretation() { return (InterpretationType)((dataFormat >> 20) & 7); }
//	bool isUniform() { return (InterpretationType)((dataFormat >> 24) & 1); }
//
//	uint8 getUniformWidth() {}
//
//	void setChannelType0(uint8 channelType)
//	{
//		dataFormat &= 0xFFFFFFFFFFFFFFE0ull; //~31
//		dataFormat |= packChannelType(channelType);
//	}
//	void setChannelType1(uint8 channelType)
//	{
//		dataFormat &= 0xFFFFFFFFFFFFFC1Full; //~(31<<5)
//		dataFormat |= (packChannelType(channelType) << 5);
//	}
//	void setChannelType2(uint8 channelType)
//	{
//		dataFormat &= 0xFFFFFFFFFFFF83FFull; //~31(31<<10)
//		dataFormat |= (packChannelType(channelType) << 10);
//	}
//	void setChannelType3(uint8 channelType)
//	{
//		dataFormat &= 0xFFFFFFFFFFF07FFFull; //~31(31<<15)
//		dataFormat |= (packChannelType(channelType) << 15);
//	}
//	void setChannelType(uint8 channelIndex, uint8 channelType)
//	{
//		dataFormat &= ~(31ull << (channelIndex * 5));
//		dataFormat |= (packChannelType(channelType) << (channelIndex * 5));
//	}
//	void setChannelWidth0(uint8 channelWidth)
//	{
//		dataFormat &= 0xFFFFFF00FFFFFFFFull; //~31
//		dataFormat |= (uint64(channelWidth) << 32);
//	}
//	void setChannelWidth1(uint8 channelWidth)
//	{
//		dataFormat &= 0xFFFF00FFFFFFFFFFull; //~(31<<5)
//		dataFormat |= (uint64(channelWidth) << 40);
//	}
//	void setChannelWidth2(uint8 channelWidth)
//	{
//		dataFormat &= 0xFF00FFFFFFFFFFFFull; //~31(31<<10)
//		dataFormat |= (uint64(channelWidth) << 48);
//	}
//	void setChannelWidth3(uint8 channelWidth)
//	{
//		dataFormat &= 0x00FFFFFFFFFFFFFFull; //~31(31<<15)
//		dataFormat |= (uint64(channelWidth) << 56);
//	}
//	void setChannelWidth(uint8 channelIndex, uint8 channelWidth)
//	{
//		dataFormat &= ~(0xFFull << (32 + channelIndex * 8));
//		dataFormat |= (uint64(channelWidth) << (32 + channelIndex * 8));
//	}
//
//	uint8 getNumberOfChannels()
//	{
//		return dataFormat >> 32 ? dataFormat >> 40 ? dataFormat >> 48 ? dataFormat >> 56 ? 4 : 3 : 2 : 1 : 0;
//	}
//
//	DataFormat(InterpretationType type, uint8 c0type, uint8 c0width)
//	{
//		dataFormat = packChannelType(c0type) | (uint64(c0width) << 32) | type << 20;
//	}
//	DataFormat(uint8 c0type, uint8 c0width, uint8 c1type, uint8 c1width, InterpretationType type)
//	{
//		dataFormat = packChannelType(c0type) | (uint64(c0width) << 32) |
//		             (packChannelType(c1type) << 5) | (uint64(c1width) << 40) | type << 20;
//	}
//	DataFormat(InterpretationType type, uint8 c0type, uint8 c0width, uint8 c1type, uint8 c1width, uint8 c2type, uint8 c2width)
//	{
//		dataFormat = packChannelType(c0type) | (uint64(c0width) << 32) | (packChannelType(c1type) << 5) | (uint64(c1width) << 40) |
//		             (packChannelType(c2type) << 10) | (uint64(c2width) << 48) | type << 20;
//	}
//	DataFormat(InterpretationType type, uint8 c0type, uint8 c0width, uint8 c1type, uint8 c1width, uint8 c2type, uint8 c2width, uint8 c3type, uint8 c3width)
//	{
//		dataFormat = packChannelType(c0type) | (uint64(c0width) << 32) | (packChannelType(c1type) << 5) | (uint64(c1width) << 40) |
//		             (packChannelType(c2type) << 10) | (uint64(c2width) << 48) | (packChannelType(c3type) << 15) | (uint64(c2width) << 56) | type << 20;
//	}
//	private:
//		uint32 packChannelType(uint8 channelType)
//		{
//			return channelType & 31;
//		}
//};
//
//}
//
