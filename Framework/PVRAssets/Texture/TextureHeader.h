/*!*********************************************************************************************************************
\file         PVRAssets/Texture/TextureHeader.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Information about an Image asset, excluding the actual image pixels and custom metadata.
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/PixelFormat.h"
#include "PVRAssets/Texture/MetaData.h"
#include "PVRAssets/Texture/TextureDefines.h"

namespace pvr {
namespace assets {

/*!********************************************************************************************************************
\brief    A class mirroring the PVR Texture container format header, and which can in general represent any Texture
         asset. Contains accessors functions to facilitate using the Texture data in application code.
**********************************************************************************************************************/
class TextureHeader
{
public:
	// V3 Header Identifiers.
	/*!********************************************************************************************************************
	\brief    This header stores everything that you would ever need to load (but not necessarily use) a texture's
	         data accurately, but no more.  Data that is provided but is not needed to read the data is stored in the
			 Metadata section (See TextureHeaderWithMetadata). Correct use of the texture may rely on meta data, but
			 accurate data loading can be done through the standard header alone.
	**********************************************************************************************************************/
	struct Header
	{
		enum
		{
			PVRv3 = 0x03525650, //!< PVR format v3 identifier
			PVRv3Reversed = 0x50565203, //!< PVR format v3 reversed identifier

			// PVR header flags.
			CompressedFlag = (1 << 0), //!< Compressed format flag
			PremultipliedFlag = (1 << 1), //!< Premultiplied flag
			SizeOfHeader = 52
		};


		uint32 flags;            //!< Various format flags.
		PixelFormat pixelFormat;      //!< The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
		types::ColorSpace colorSpace;      //!< The Color Space of the texture, currently either linear RGB or sRGB.
		VariableType channelType;      //!< Variable type that the channel is stored in. Supports signed/unsigned int/short/byte/float.
		uint32 height;           //!< Height of the texture.
		uint32 width;            //!< Width of the texture.
		uint32 depth;            //!< Depth of the texture. (Z-slices)
		uint32 numberOfSurfaces; //!< Number of members in a Texture Array.
		uint32 numberOfFaces;    //!< Number of faces in a Cube Map. Maybe be a value other than 6.
		uint32 mipMapCount;      //!< Number of MIP Maps in the texture - NB: Includes top level.
		uint32 metaDataSize;     //!< Size of the accompanying meta data.
		Header() : flags(0), pixelFormat(0), colorSpace(types::ColorSpace::lRGB), channelType(VariableType::UnsignedByteNorm),
			height(1), width(1), depth(1), numberOfSurfaces(1), numberOfFaces(1), mipMapCount(1), metaDataSize(0)
		{}
	};

protected:
	Header          m_header;  //Texture header as laid out in a file.
	std::map<uint32, std::map<uint32, TextureMetaData>> m_metaDataMap; //Map of all the meta data stored for a texture.
public:
	/*!***********************************************************************
	\brief	Default constructor for a TextureHeader. Returns an empty header.
	*************************************************************************/
	TextureHeader();

	/*!*******************************************************************************************************************************
	\brief	Copy constructor
	\param	rhs Copy from
	**********************************************************************************************************************************/
	TextureHeader(const TextureHeader& rhs)
	{
		//Copy the header over.
		m_header = rhs.m_header;
		m_metaDataMap = rhs.m_metaDataMap;
	}

	/*!*******************************************************************************************************************************
	\brief	Construct this from the given file header
	\param	header File Header used to construct this
	**********************************************************************************************************************************/
	TextureHeader(Header& header);

	/*!*******************************************************************************************************************************
	\brief	Construct this from file header and meta data
	\param	fileHeader File header to construct from
	\param	metaDataCount Number of meta data in the array
	\param	metaData Array of meta data
	**********************************************************************************************************************************/
	TextureHeader(Header fileHeader, uint32 metaDataCount, TextureMetaData* metaData);


	/*!*******************************************************************************************************************************
	\brief	Constructor.
	\param	pixelFormat Pixel format of the texture
	\param	width Texture width
	\param	height Texture height
	\param	depth Texture depth
	\param	mipMapCount Number of mipmap in the texture
	\param	colorSpace Texture color space (e.g sRGB, lRGB)
	\param	channelType Texture channel type
	\param	numberOfSurfaces Number of surfaces the texture has
	\param	numberOfFaces Number of faces the texture has
	\param	flags Additional provided flags
	\param	metaData Texture meta data
	\param	metaDataSize Texture meta data size
	**********************************************************************************************************************************/
	TextureHeader(PixelFormat pixelFormat, uint32 width, uint32 height, uint32 depth = 1, uint32 mipMapCount = 1,
	              types::ColorSpace colorSpace = types::ColorSpace::lRGB, VariableType channelType = VariableType::UnsignedByteNorm,
	              uint32 numberOfSurfaces = 1, uint32 numberOfFaces = 1, uint32 flags = 0, TextureMetaData* metaData = NULL, uint32 metaDataSize = 0);

	/*!***********************************************************************
	\param[in]	rhs copy the information from
	\return	Return this header.
	\brief	Copy the contents and information of another header into this one.
	*************************************************************************/
	TextureHeader& operator=(const TextureHeader& rhs)
	{
		//If it equals itself, return early.
		if (&rhs == this) { return *this; }

		//Copy the header over.
		m_header = rhs.m_header;
		m_metaDataMap = rhs.m_metaDataMap;

		//Return
		return *this;
	}

	/*!***********************************************************************
	\return	Return the file header.
	\brief	Gets the file header structure.
	*************************************************************************/
	Header getFileHeader() const
	{
		return m_header;
	}

	/*!***********************************************************************
	\return	Return the file header.
	\brief	Gets the file header access.
	*************************************************************************/
	Header& getFileHeaderAccess()
	{
		return m_header;
	}

	/*!***********************************************************************
	\return			Return a 64-bit pixel type ID.
	\description	Gets the pixel type ID of the texture.
	*************************************************************************/
	PixelFormat getPixelFormat() const
	{
		return PixelFormat(m_header.pixelFormat);
	}

	/*!***********************************************************************
	\return			Return number of bits per pixel
	\description	Gets the bits per pixel of the texture format.
	*************************************************************************/
	uint32 getBitsPerPixel() const;

	/*!***********************************************************************
	\param[out]	minX	Minimum width of the texture format.
	\param[out]	minY	Minimum height of the texture format.
	\param[out]	minZ	Minimum depth of the texture format.
	\brief	Get the minimum dimensions that the texture format of this header can be.
	*************************************************************************/
	void getMinDimensionsForFormat(uint32& minX, uint32& minY, uint32& minZ) const;

	/*!***********************************************************************
	\return	Return the ColorSpace enum representing color space.
	\brief	Get the color space of the texture.
	*************************************************************************/
	types::ColorSpace getColorSpace() const
	{
		return static_cast<types::ColorSpace>(m_header.colorSpace);
	}

	/*!***********************************************************************
	\return	Return the  enum representing the type of the texture.
	\brief Get the channel type that the texture's data is stored in.
	*************************************************************************/
	VariableType getChannelType() const
	{
		return static_cast<VariableType>(m_header.channelType);
	}

	/*!***********************************************************************
	\param[in]	uiMipLevel	MIP level that user is interested in.
	\return		Return the width of the specified MIP-Map level.
	\brief	Gets the width of the user specified MIP-Map level for the texture.
	*************************************************************************/
	uint32 getWidth(uint32 uiMipLevel = c_pvrTextureTopMIPMap) const
	{
		//If MipLevel does not exist, return no uiDataSize.
		if (uiMipLevel > m_header.mipMapCount)
		{
			return 0;
		}
		return std::max<uint32>(m_header.width >> uiMipLevel, 1);
	}

	/*!***********************************************************************
	\param[in]		axis	The axis to examine.
	\return			Return the orientation of the axis.
	\description	Gets the data orientation for this texture.
	*************************************************************************/
	TextureMetaData::AxisOrientation getOrientation(TextureMetaData::Axis axis) const;

	/*!***********************************************************************
	\param[in]		uiMipLevel	MIP level that user is interested in.
	\return			Return the Height of the specified MIP-Map level.
	\description	Gets the height of the user specified MIP-Map
	level for the texture.
	*************************************************************************/
	uint32 getHeight(uint32 uiMipLevel = c_pvrTextureTopMIPMap) const
	{
		//If MipLevel does not exist, return no uiDataSize.
		if (uiMipLevel > m_header.mipMapCount)
		{
			return 0;
		}
		return std::max<uint32>(m_header.height >> uiMipLevel, 1);
	}

	/*!***********************************************************************
	\param[in]		mipLevel	MIP level that user is interested in.
	\return			Return the depth of the specified MIP-Map level.
	\description	Gets the depth of the user specified MIP-Map level for the texture.
	*************************************************************************/
	uint32 getDepth(uint32 mipLevel = c_pvrTextureTopMIPMap) const
	{
		//If MipLevel does not exist, return no uiDataSize.
		if (mipLevel > m_header.mipMapCount) {  return 0;    }
		return std::max<uint32>(m_header.depth >> mipLevel, 1);
	}

	/*!***********************************************************************
	\param[in] mipMapLevel	Specifies a MIP level to check, 'c_pvrTextureAllMIPMapLevels'
						can be passed to Get the size of all MIP levels.
	\param[in] allSurfaces	The Size of all surfaces is calculated if true, only a single surface if false.
	\param[in] allFaces	The	Size of all faces is calculated if true, only a single face if false.
	\return	Return the size in PIXELS of the specified texture area.
	\brief	Gets the size in PIXELS of the texture, given various input parameters.
	\description User can retrieve the total size of either all surfaces or a single surface,
				 all faces or a single face and all MIP-Maps or a single specified MIP level. All of these
	*************************************************************************/
	uint32 getTextureSize(int32 mipMapLevel = c_pvrTextureAllMIPMaps, bool allSurfaces = true, bool allFaces = true) const
	{
		return (uint32)(((uint64)8 * (uint64)getDataSize(mipMapLevel, allSurfaces, allFaces)) / (uint64)getBitsPerPixel());
	}

	/*!***********************************************************************
	\param[in]	mipLevel Specifies a mip level to check, 'c_pvrTextureAllMIPMapLevels' can be passed to Get
						 the size of all MIP levels.
	\param[in]	allSurfaces	The Size of all surfaces is calculated if true, only a single surface if false.
	\param[in]	allFaces	The Size of all faces is calculated if true, only a single face if false.
	\return		Return the size in BYTES of the specified texture area.
	\brief			Gets the size in BYTES of the texture, given various input parameters.
	\description    User can retrieve the size of either all surfaces or a single surface,
					all faces or a single face and all MIP-Maps or a single specified MIP level.
	*************************************************************************/
	uint32 getDataSize(int32 mipLevel = c_pvrTextureAllMIPMaps, bool allSurfaces = true, bool allFaces = true) const;

	/*!*******************************************************************************************************************************
	\brief	Get a offset in the data
	\return	Return data offset
	\param	mipMapLevel The mip map level of the offset
	\param	arrayMember The array index of the offset
	\param	face The face of the offset
	**********************************************************************************************************************************/
	ptrdiff_t getDataOffset(uint32 mipMapLevel = 0, uint32 arrayMember = 0, uint32 face = 0) const;

	/*!***********************************************************************
	\return	Return the number of array members in this texture.
	\brief	Gets the number of array members stored in this texture.
	*************************************************************************/
	uint32 getNumberOfArrayMembers() const
	{
		return m_header.numberOfSurfaces;
	}

	/*!***********************************************************************
	\return	Return a direct pointer to the MetaData map.
	\brief Get a pointer directly to the Meta Data Map, to allow users to read out data.
	*************************************************************************/
	const std::map<uint32, std::map<uint32, TextureMetaData>>* getMetaDataMap() const
	{
		return &m_metaDataMap;
	}


	/*!***********************************************************************
	\return	Return the number of MIP-Map levels in this texture.
	\brief Gets the number of MIP-Map levels stored in this texture.
	*************************************************************************/
	uint32 getNumberOfMIPLevels() const
	{
		return m_header.mipMapCount;
	}

	/*!***********************************************************************
	\return	Return the number of faces in this texture.
	\brief Gets the number of faces stored in this texture.
	*************************************************************************/
	uint32 getNumberOfFaces() const
	{
		return m_header.numberOfFaces;
	}

	/*!***********************************************************************
	\return	Returns cube map order.
	\brief	Gets the cube map face order.
	\description Returned string will be in the form "ZzXxYy" with capitals
				 representing positive and small letters representing negative. I.e. Z=Z-Positive,
				 z=Z-Negative.
	*************************************************************************/
	const std::string getCubeMapOrder() const;

	/*!***********************************************************************
	\return	Return true if it is file compressed.
	\brief	Returns whether or not the texture is compressed using
			PVRTexLib's FILE compression - this is independent of any texture compression.
	*************************************************************************/
	bool isFileCompressed() const
	{
		return (m_header.flags & Header::CompressedFlag) != 0;
	}

	/*!***********************************************************************
	\return	Return true if texture is premultiplied.
	\brief	Check whether or not the texture's color has been pre-multiplied by the alpha values.
	*************************************************************************/
	bool isPreMultiplied() const
	{
		return (m_header.flags & Header::PremultipliedFlag) != 0;
	}

	/*!***********************************************************************
	\return	Return the size, in bytes, of the meta data stored in the header.
	\brief	Get the total size of the meta data stored in the header.
	This includes the size of all information stored in all CPVRMetaDataBlocks.
	*************************************************************************/
	uint32 getMetaDataSize() const
	{
		return m_header.metaDataSize;
	}

	/*!***********************************************************************
	\param[out]	outD3dFormat Returned d3d format
	\return	Return true on success, returns false if it cannot find a suitable type
	\brief	Gets the Direct3D equivalent format enumeration for this texture.
	*************************************************************************/
	bool getDirect3DFormat(uint32& outD3dFormat) const;

	/*!***********************************************************************
	\param[out] notAlpha Return whether the \p outDxgiFormat is has alpha or not.
	\param[out]	outDxgiFormat Returned dxgi format
	\return	Return true on success, returns false if it cannot find a suitable type
	\brief Gets the DirectXGI equivalent format enumeration for this texture.
	*************************************************************************/
	bool getDirectXGIFormat(uint32& outDxgiFormat, bool& notAlpha) const;

	/*!***********************************************************************
	\param[in]	uPixelFormat The format of the pixel.
	\brief Sets the pixel format for this texture.
	*************************************************************************/
	void setPixelFormat(PixelFormat uPixelFormat)
	{
		m_header.pixelFormat = uPixelFormat.getPixelTypeId();
	}

	/*!***********************************************************************
	\param[in]	colorSpace	A color space of the texture.
	\brief	Sets the color space for this texture. Default is lRGB.
	*************************************************************************/
	void setColorSpace(types::ColorSpace colorSpace)
	{
		m_header.colorSpace = colorSpace;
	}

	/*!***********************************************************************
	\param[in]	channelType	Texture's channel type
	\description Sets the channel type of this texture.
	*************************************************************************/
	void setChannelType(VariableType channelType)
	{
		m_header.channelType = channelType;
	}

	/*!***********************************************************************
	\param[in]	bumpScale	Floating point "height" value to scale the bump map.
	\param[in]	bumpOrder	Up to 4 character string, with values x,y,z,h in
							some combination.
	\brief	Sets a texture's bump map data.
	\description For \p bumpOrder Not all values need to be present.
				 Denotes channel order; x,y,z refer to the corresponding axes,
				 h indicates presence of the original height map. It is possible
				 to have only some of these values rather than all.
				 For example if 'h' is present alone it will be considered a height map.
				 The values should be presented in RGBA order, regardless
				 of the texture format, so a zyxh order in a bgra texture
				 should still be passed as 'xyzh'. Capitals are allowed.
				 Any character stored here that is not one of x,y,z,h
				 or a NULL character	will be ignored when PVRTexLib
				 reads the data,	but will be preserved. This is useful
				 if you wish to define a custom data channel for instance.
				 In these instances PVRTexLib will assume it is simply
				 color data.
	*************************************************************************/
	void setBumpMap(float bumpScale, string bumpOrder);

	/*!*******************************************************************************************************************************
	\brief	Check if this texture is bumpmap
	\return	Return true if the texture is bumpmap
	**********************************************************************************************************************************/
	bool isBumpMap()const;

	/*!***********************************************************************
	\param[in]		glInternalFormat Opengl internal format
	\param[in]		glFormat Opengl format
	\param[in]		glType Opengl type
	\return			Return true if the format is valid.
	\brief	Sets the format of the texture to PVRTexLib's internal representation
			of the openGL/ES format. The internal format may be sized or unsized.
	*************************************************************************/
	bool setopenGLFormat(uint32 glInternalFormat, uint32 glFormat, uint32 glType);

	/*!***********************************************************************
	\return			Return true if the format is valid or not.
	\brief	Sets the format of the texture to PVRTexLib's internal
					representation of the DirectXGI format.
	*************************************************************************/
	bool setDirectXGIFormat(uint32 dxgiFormat);

	/*!***********************************************************************
	\return			Return true if the format is valid or not.
	\brief	Sets the format of the texture to PVRTexLib's internal
					representation of the Direct3D format.
	*************************************************************************/
	bool setDirect3DFormat(uint32 d3dFormat);

	/*!***********************************************************************
	\param[in]	newWidth	The new width.
	\brief Sets the texture  width.
	*************************************************************************/
	void setWidth(uint32 newWidth)
	{
		m_header.width = newWidth;
	}

	/*!***********************************************************************
	\param[in]	newHeight	The new height.
	\brief	Sets the texture height.
	*************************************************************************/
	void setHeight(uint32 newHeight)
	{
		m_header.height = newHeight;
	}

	/*!***********************************************************************
	\param[in]	newDepth	The new depth.
	\brief	Sets the texture depth.
	*************************************************************************/
	void setDepth(uint32 newDepth)
	{
		m_header.depth = newDepth;
	}

	/*!***********************************************************************
	\param[in]	newNumMembers	The new number of members in this array.
	\brief	Sets the number of arrays in this texture
	*************************************************************************/
	void setNumberOfArrayMembers(uint32 newNumMembers)
	{
		m_header.numberOfSurfaces = newNumMembers;
	}

	/*!***********************************************************************
	\param[in]	newNumMIPLevels		New number of MIP-Map levels.
	\brief	Sets the number of MIP-Map levels in this texture.
	*************************************************************************/
	void setNumberOfMIPLevels(uint32 newNumMIPLevels)
	{
		m_header.mipMapCount = newNumMIPLevels;
	}

	/*!***********************************************************************
	\param[in]	newNumFaces New number of faces for this texture.
	\brief Sets the number of faces stored in this texture.
	*************************************************************************/
	void setNumberOfFaces(uint32 newNumFaces)
	{
		m_header.numberOfFaces = newNumFaces;
	}

	/*!***********************************************************************
	\param[in]	axisOrientation Specifying axis and orientation.
	\brief Sets the data orientation for a given axis in this texture.
	*************************************************************************/
	void setOrientation(TextureMetaData::AxisOrientation axisOrientation);

	/*!***********************************************************************
	\param[in]	cubeMapOrder	Up to 6 character string, with values
				x,X,y,Y,z,Z in some combination.
	\brief		Sets a texture's bump map data.
	\description for \p cubmapOrder Not all values need to be present. Denotes face
				 order; Capitals refer to positive axis positions and small letters refer to
				 negative axis positions. E.g. x=X-Negative, X=X-Positive.
				 It is possible to have only some of these values rather than all, as
				 long as they are NULL terminated. NB: Values past the 6th character are not read.
	*************************************************************************/
	void setCubeMapOrder(std::string cubeMapOrder);

	/*!***********************************************************************
	\param[in]	isFileCompressed	Sets file compression to true/false.
	\brief	Sets whether or not the texture is compressed using PVRTexLib's FILE
			compression - this is independent of any texture compression.
			Currently unsupported.
	*************************************************************************/
	void setIsFileCompressed(bool isFileCompressed)
	{
		if (isFileCompressed)
		{
			m_header.flags |= Header::CompressedFlag;
		}
		else
		{
			m_header.flags &= !Header::CompressedFlag;
		}
	}

	/*!***********************************************************************
	\param	isPreMultiplied	Sets if texture is premultiplied.
	\brief	Sets whether or not the texture's color has been
			pre-multiplied by the alpha values.
	*************************************************************************/
	void setIsPreMultiplied(bool isPreMultiplied)
	{
		if (isPreMultiplied)
		{
			m_header.flags |= Header::PremultipliedFlag;
		}
		else
		{
			m_header.flags &= !Header::PremultipliedFlag;
		}
	}

	/*!***********************************************************************
	\param[in]	metaData	Meta data block to be added.
	\brief Adds an arbitrary piece of meta data.
	*************************************************************************/
	void addMetaData(const TextureMetaData& metaData);
};
}
}
