/*!*********************************************************************************************************************
\file         PVRAssets/Texture/Texture.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         The main class that represents an Image (Texture).
***********************************************************************************************************************/
#include "PVRAssets/Texture/TextureHeader.h"
#pragma once

namespace pvr {
namespace assets {

/*!**************************************************************************
\brief Enumerates the formats directly supported by PVRAssets.
****************************************************************************/
enum class TextureFileFormat {
	UNKNOWN = 0, KTX, DDX, PVR, TGA, BMP, DDS
};

/*!****************************************************************************************
\brief A 2D Texture asset, together with Information, Metadata and actual Pixel data. Only
       represents the actual data, not the API objects that may be created from it.
*******************************************************************************************/
class Texture : public TextureHeader
{
public:
	/*!***********************************************************************
	\brief     Construct a new empty texture.
	*************************************************************************/
	Texture();

	/*!***********************************************************************
	\brief      Create a texture using the information from a Texture header
				and copy the actual data from a provided pointer.
	\param[in]	sHeader A texture header describing the texture
	\param[in]	pData Pointer to memory containing the actual data.
	\description	Creates a new texture based on a texture header,
		pre-allocating the correct amount of memory. If data is supplied, it
		will be copied into memory. If the pointer contains less data than is
		dictated by the texture header, the behaviour is undefined.
	*************************************************************************/
	Texture(const TextureHeader& sHeader, const byte* pData = NULL);

	/*!***********************************************************************
	\brief      Create a texture using the information from a Texture header
	            and preallocate memory for its data.
	\param[in]	sHeader A texture header describing the texture
	\description	Creates a new texture based on a texture header,
		pre-allocating the correct amount of memory.
	*************************************************************************/
	void initializeWithHeader(const TextureHeader& sHeader);

	/*!***********************************************************************
	\param[in]			mipMapLevel
	\param[in]			arrayMember
	\param[in]			faceNumber
	\return			byte* Pointer to a location in the texture.
	\description	Returns a pointer into the texture's data.
		It is possible to specify an offSet to specific array members,
		faces and MIP Map levels.
	*************************************************************************/
	const byte* getDataPointer(uint32 mipMapLevel = 0, uint32 arrayMember = 0, uint32 faceNumber = 0) const;

	/*!***********************************************************************
	\param[in]			mipMapLevel
	\param[in]			arrayMember
	\param[in]			faceNumber
	\return			byte* Pointer to a location in the texture.
	\description	Returns a pointer into the texture's data.
		It is possible to specify an offSet to specific array members,
		faces and MIP Map levels.
	*************************************************************************/
	byte* getDataPointer(uint32 mipMapLevel = 0, uint32 arrayMember = 0, uint32 faceNumber = 0);


	byte* getPixelPointer(uint32 x, uint32 y, uint32 z = 0, uint32 mipMapLevel = 0, uint32 arrayMember = 0, uint32 faceNumber = 0)
	{
		uint8 pelsize = getPixelSize();
		size_t idx = (x + y * m_header.width * +z * m_header.width * m_header.height) * pelsize;
		return getDataPointer(mipMapLevel, arrayMember, faceNumber) + idx;
	}

	uint8 getPixelSize() const;

	types::ImageBaseType getDimension() const
	{
		return getDepth() > 1 ? types::ImageBaseType::Image3D : getHeight() > 1 ? types::ImageBaseType::Image2D : types::ImageBaseType::Image1D;
	}

	types::ImageAreaSize getTotalDimensions() const
	{
		return types::ImageAreaSize(getLayersSize(), getDimensions());
	}

	types::Extent3D getDimensions() const
	{
		return types::Extent3D((uint16)getWidth(), (uint16)getHeight(), (uint16)getDepth());
	}

	types::ImageLayersSize getLayersSize() const
	{
		return types::ImageLayersSize((uint16)(getNumberOfArrayMembers() * getNumberOfFaces()), (uint8)getNumberOfMIPLevels());
	}

	types::Extent3D getDimensions(uint32 miplevel) const
	{
		return types::Extent3D((uint16)getWidth(miplevel), (uint16)getHeight(miplevel), (uint16)getDepth(miplevel));
	}

	/*!***********************************************************************
	\return			const TextureHeader& Returns the header only for this texture.
	\description	Gets the header for this texture, allowing you to create a new
		texture based on this one with some changes. Useful for passing
		information about a texture without passing all of its data.
	*************************************************************************/
	const TextureHeader& getHeader() const;

	///////////////////////////////// File IO /////////////////////////////////

	/*!***********************************************************************
	\param[in]			uiPadding
	\description	When writing the texture out to a PVR file, it is often
		desirable to pad the meta data so that the start of the
		texture data aligns to a given boundary.
		This function pads to a boundary value equal to "uiPadding".
		For example Setting uiPadding=8 will align the start of the
		texture data to an 8 byte boundary.
		Note - this should be called immediately before saving as
		the value is worked out based on the current meta data size.
	*************************************************************************/
	void addPaddingMetaData(uint32 uiPadding);

private:
	std::vector<byte> m_pTextureData;		// Pointer to texture data.
};

/*!***********************************************************************
\brief      Infer the texture format from a filename.
\param[in]  assetname		The name of the asset, containing the extension.
\return		The TextureFileFormat if understood, otherwise TextureFileFormat::Unknown.
*************************************************************************/
TextureFileFormat getTextureFormatFromFilename(const char* assetname);

/*!***********************************************************************
\brief          Load a texture from a Stream to a texture file
\param[in]		textureStream	The stream containing the texture data.
\param[in]		type	The format of the texture.
\param[out]		outTex		The texture object where the texture will be stored.
\return			The error code for the operation.
*************************************************************************/
Result textureLoad(Stream::ptr_type textureStream, TextureFileFormat type, Texture& outTex);
}
}
