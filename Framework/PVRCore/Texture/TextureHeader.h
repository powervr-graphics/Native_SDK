/*!
\brief Information about an Image asset, excluding the actual image pixels and custom metadata.
\file PVRCore/Texture/TextureHeader.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/CoreIncludes.h"
#include "PVRCore/PixelFormat.h"
#include "PVRCore/Texture/MetaData.h"

namespace pvr {
/// <summary>Flag interpreted as All mipmap levels.</summary>
enum { pvrTextureAllMIPMaps = -1 };

/// <summary>A class mirroring the PVR Texture container format header, and which can in general represent any
/// Texture asset. Contains accessors functions to facilitate using the Texture data in application code.
/// </summary>
class TextureHeader
{
public:
	// V3 Header Identifiers.
	/// <summary>This header stores everything that you would ever need to load (but not necessarily use) a texture's
	/// data accurately, but no more. Data that is provided but is not needed to read the data is stored in the
	/// Metadata section (See TextureHeaderWithMetadata). Correct use of the texture may rely on meta data, but
	/// accurate data loading can be done through the standard header alone.</summary>
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
	Header _header;  //Texture header as laid out in a file.
	std::map<uint32, std::map<uint32, TextureMetaData>> _metaDataMap; //Map of all the meta data stored for a texture.
public:
	/// <summary>Default constructor for a TextureHeader. Returns an empty header.</summary>
	TextureHeader();

	/// <summary>Copy constructor</summary>
	/// <param name="rhs">Copy from</param>
	TextureHeader(const TextureHeader& rhs)
	{
		//Copy the header over.
		_header = rhs._header;
		_metaDataMap = rhs._metaDataMap;
	}

	/// <summary>Construct this from the given file header</summary>
	/// <param name="header">File Header used to construct this</param>
	TextureHeader(Header& header);

	/// <summary>Construct this from file header and meta data</summary>
	/// <param name="fileHeader">File header to construct from</param>
	/// <param name="metaDataCount">Number of meta data in the array</param>
	/// <param name="metaData">Array of meta data</param>
	TextureHeader(Header fileHeader, uint32 metaDataCount, TextureMetaData* metaData);


	/// <summary>Constructor.</summary>
	/// <param name="pixelFormat">Pixel format of the texture</param>
	/// <param name="width">Texture width</param>
	/// <param name="height">Texture height</param>
	/// <param name="depth">Texture depth</param>
	/// <param name="mipMapCount">Number of mipmap in the texture</param>
	/// <param name="colorSpace">Texture color space (e.g sRGB, lRGB)</param>
	/// <param name="channelType">Texture channel type</param>
	/// <param name="numberOfSurfaces">Number of surfaces the texture has</param>
	/// <param name="numberOfFaces">Number of faces the texture has</param>
	/// <param name="flags">Additional provided flags</param>
	/// <param name="metaData">Texture meta data</param>
	/// <param name="metaDataSize">Texture meta data size</param>
	TextureHeader(PixelFormat pixelFormat, uint32 width, uint32 height, uint32 depth = 1, uint32 mipMapCount = 1,
	              types::ColorSpace colorSpace = types::ColorSpace::lRGB, VariableType channelType = VariableType::UnsignedByteNorm,
	              uint32 numberOfSurfaces = 1, uint32 numberOfFaces = 1, uint32 flags = 0, TextureMetaData* metaData = NULL, uint32 metaDataSize = 0);

	/// <summary>Gets the file header structure.</summary>
	/// <returns>Return the file header.</returns>
	const Header& getHeader() const
	{
		return _header;
	}

	/// <summary>Gets the file header access.</summary>
	/// <returns>Return the file header.</returns>
	Header& getHeaderAccess()
	{
		return _header;
	}

	/// <summary>Gets the pixel type ID of the texture.</summary>
	/// <returns>Return a 64-bit pixel type ID.</returns>
	PixelFormat getPixelFormat() const
	{
		return PixelFormat(_header.pixelFormat);
	}

	/// <summary>Gets the bits per pixel of the texture format.</summary>
	/// <returns>Return number of bits per pixel</returns>
	uint32 getBitsPerPixel() const;

	/// <summary>Get the minimum dimensions that the texture format of this header can be.</summary>
	/// <param name="minX">Minimum width of the texture format.</param>
	/// <param name="minY">Minimum height of the texture format.</param>
	/// <param name="minZ">Minimum depth of the texture format.</param>
	void getMinDimensionsForFormat(uint32& minX, uint32& minY, uint32& minZ) const;

	/// <summary>Get the color space of the texture.</summary>
	/// <returns>Return the ColorSpace enum representing color space.</returns>
	types::ColorSpace getColorSpace() const
	{
		return static_cast<types::ColorSpace>(_header.colorSpace);
	}

	/// <summary>Get the channel type that the texture's data is stored in.</summary>
	/// <returns>Return the enum representing the type of the texture.</returns>
	VariableType getChannelType() const
	{
		return static_cast<VariableType>(_header.channelType);
	}

	/// <summary>Gets the width of the user specified MIP-Map level for the texture.</summary>
	/// <param name="uiMipLevel">MIP level that user is interested in.</param>
	/// <returns>Return the width of the specified MIP-Map level.</returns>
	uint32 getWidth(uint32 uiMipLevel = 0) const
	{
		//If MipLevel does not exist, return no uiDataSize.
		if (uiMipLevel > _header.mipMapCount)
		{
			return 0;
		}
		return std::max<uint32>(_header.width >> uiMipLevel, 1);
	}

	/// <summary>Gets the data orientation for this texture.</summary>
	/// <param name="axis">The axis to examine.</param>
	/// <returns>Return the orientation of the axis.</returns>
	TextureMetaData::AxisOrientation getOrientation(TextureMetaData::Axis axis) const;

	/// <summary>Gets the height of the user specified MIP-Map level for the texture.</summary>
	/// <param name="uiMipLevel">MIP level that user is interested in.</param>
	/// <returns>Return the Height of the specified MIP-Map level.</returns>
	uint32 getHeight(uint32 uiMipLevel = 0) const
	{
		//If MipLevel does not exist, return no uiDataSize.
		if (uiMipLevel > _header.mipMapCount)
		{
			return 0;
		}
		return std::max<uint32>(_header.height >> uiMipLevel, 1);
	}

	/// <summary>Gets the depth of the user specified MIP-Map level for the texture.</summary>
	/// <param name="mipLevel">MIP level that user is interested in.</param>
	/// <returns>Return the depth of the specified MIP-Map level.</returns>
	uint32 getDepth(uint32 mipLevel = 0) const
	{
		//If MipLevel does not exist, return no uiDataSize.
		if (mipLevel > _header.mipMapCount) {  return 0;    }
		return std::max<uint32>(_header.depth >> mipLevel, 1);
	}

	/// <summary>Gets the size in PIXELS of the texture, given various input parameters.</summary>
	/// <param name="mipMapLevel">Specifies a MIP level to check, 'c_pvrTextureAllMIPMapLevels' can be passed to Get the
	/// size of all MIP levels.</param>
	/// <param name="allSurfaces">The Size of all surfaces is calculated if true, only a single surface if false.
	/// </param>
	/// <param name="allFaces">The Size of all faces is calculated if true, only a single face if false.</param>
	/// <returns>Return the size in PIXELS of the specified texture area.</returns>
	/// <remarks>User can retrieve the total size of either all surfaces or a single surface, all faces or a single
	/// face and all MIP-Maps or a single specified MIP level. All of these</remarks>
	uint32 getTextureSize(int32 mipMapLevel = pvrTextureAllMIPMaps, bool allSurfaces = true, bool allFaces = true) const
	{
		return (uint32)(((uint64)8 * (uint64)getDataSize(mipMapLevel, allSurfaces, allFaces)) / (uint64)getBitsPerPixel());
	}

	/// <summary>Gets the size in BYTES of the texture, given various input parameters.</summary>
	/// <param name="mipLevel">Specifies a mip level to check, 'c_pvrTextureAllMIPMapLevels' can be passed to Get the size
	/// of all MIP levels.</param>
	/// <param name="allSurfaces">The Size of all surfaces is calculated if true, only a single surface if false.
	/// </param>
	/// <param name="allFaces">The Size of all faces is calculated if true, only a single face if false.</param>
	/// <returns>Return the size in BYTES of the specified texture area.</returns>
	/// <remarks>User can retrieve the size of either all surfaces or a single surface, all faces or a single face and
	/// all MIP-Maps or a single specified MIP level.</remarks>
	uint32 getDataSize(int32 mipLevel = pvrTextureAllMIPMaps, bool allSurfaces = true, bool allFaces = true) const;

	/// <summary>Get a offset in the data</summary>
	/// <param name="mipMapLevel">The mip map level of the offset</param>
	/// <param name="arrayMember">The array index of the offset</param>
	/// <param name="face">The face of the offset</param>
	/// <returns>Return data offset</returns>
	ptrdiff_t getDataOffset(uint32 mipMapLevel = 0, uint32 arrayMember = 0, uint32 face = 0) const;

	/// <summary>Gets the number of array members stored in this texture.</summary>
	/// <returns>Return the number of array members in this texture.</returns>
	uint32 getNumberOfArrayMembers() const
	{
		return _header.numberOfSurfaces;
	}

	/// <summary>Get a pointer directly to the Meta Data Map, to allow users to read out data.</summary>
	/// <returns>Return a direct pointer to the MetaData map.</returns>
	const std::map<uint32, std::map<uint32, TextureMetaData>>* getMetaDataMap() const
	{
		return &_metaDataMap;
	}


	/// <summary>Gets the number of MIP-Map levels stored in this texture.</summary>
	/// <returns>Return the number of MIP-Map levels in this texture.</returns>
	uint32 getNumberOfMIPLevels() const
	{
		return _header.mipMapCount;
	}

	/// <summary>Gets the number of faces stored in this texture.</summary>
	/// <returns>Return the number of faces in this texture.</returns>
	uint32 getNumberOfFaces() const
	{
		return _header.numberOfFaces;
	}

	/// <summary>Gets the cube map face order.</summary>
	/// <returns>Returns cube map order.</returns>
	/// <remarks>Returned string will be in the form "ZzXxYy" with capitals representing positive and small letters
	/// representing negative. I.e. Z=Z-Positive, z=Z-Negative.</remarks>
	const std::string getCubeMapOrder() const;

	/// <summary>Returns whether or not the texture is compressed using PVRTexLib's FILE compression - this is independent
	/// of any texture compression.</summary>
	/// <returns>Return true if it is file compressed.</returns>
	bool isFileCompressed() const
	{
		return (_header.flags & Header::CompressedFlag) != 0;
	}

	/// <summary>Check whether or not the texture's color has been pre-multiplied by the alpha values.</summary>
	/// <returns>Return true if texture is premultiplied.</returns>
	bool isPreMultiplied() const
	{
		return (_header.flags & Header::PremultipliedFlag) != 0;
	}

	/// <summary>Get the total size of the meta data stored in the header. This includes the size of all information
	/// stored in all CPVRMetaDataBlocks.</summary>
	/// <returns>Return the size, in bytes, of the meta data stored in the header.</returns>
	uint32 getMetaDataSize() const
	{
		return _header.metaDataSize;
	}

	/// <summary>Gets the Direct3D equivalent format enumeration for this texture.</summary>
	/// <param name="outD3dFormat">Returned d3d format</param>
	/// <returns>Return true on success, returns false if it cannot find a suitable type</returns>
	bool getDirect3DFormat(uint32& outD3dFormat) const;

	/// <summary>Gets the DirectXGI equivalent format enumeration for this texture.</summary>
	/// <param name="notAlpha">Return whether the <paramref name="outDxgiFormat"/>is has alpha or not.</param>
	/// <param name="outDxgiFormat">Returned dxgi format</param>
	/// <returns>Return true on success, returns false if it cannot find a suitable type</returns>
	bool getDirectXGIFormat(uint32& outDxgiFormat, bool& notAlpha) const;

	/// <summary>Sets the pixel format for this texture.</summary>
	/// <param name="uPixelFormat">The format of the pixel.</param>
	void setPixelFormat(PixelFormat uPixelFormat)
	{
		_header.pixelFormat = uPixelFormat.getPixelTypeId();
	}

	/// <summary>Sets the color space for this texture. Default is lRGB.</summary>
	/// <param name="colorSpace">A color space of the texture.</param>
	void setColorSpace(types::ColorSpace colorSpace)
	{
		_header.colorSpace = colorSpace;
	}

	/// <summary>Sets the channel type of this texture.</summary>
	/// <param name="channelType">Texture's channel type</param>
	void setChannelType(VariableType channelType)
	{
		_header.channelType = channelType;
	}

	/// <summary>Sets a texture's bump map data.</summary>
	/// <param name="bumpScale">Floating point "height" value to scale the bump map.</param>
	/// <param name="bumpOrder">Up to 4 character string, with values x,y,z,h in some combination.</param>
	/// <remarks>For <paramref name="bumpOrder"/>Not all values need to be present. Denotes channel order; x,y,z refer
	/// to the corresponding axes, h indicates presence of the original height map. It is possible to have only some
	/// of these values rather than all. For example if 'h' is present alone it will be considered a height map. The
	/// values should be presented in RGBA order, regardless of the texture format, so a zyxh order in a bgra texture
	/// should still be passed as 'xyzh'. Capitals are allowed. Any character stored here that is not one of x,y,z,h
	/// or a NULL character will be ignored when PVRTexLib reads the data, but will be preserved. This is useful if
	/// you wish to define a custom data channel for instance. In these instances PVRTexLib will assume it is simply
	/// color data.</remarks>
	void setBumpMap(float bumpScale, string bumpOrder);

	/// <summary>Check if this texture is bumpmap</summary>
	/// <returns>Return true if the texture is bumpmap</returns>
	bool isBumpMap()const;

	/// <summary>Sets the texture width.</summary>
	/// <param name="newWidth">The new width.</param>
	void setWidth(uint32 newWidth)
	{
		_header.width = newWidth;
	}

	/// <summary>Sets the texture height.</summary>
	/// <param name="newHeight">The new height.</param>
	void setHeight(uint32 newHeight)
	{
		_header.height = newHeight;
	}

	/// <summary>Sets the texture depth.</summary>
	/// <param name="newDepth">The new depth.</param>
	void setDepth(uint32 newDepth)
	{
		_header.depth = newDepth;
	}

	/// <summary>Sets the number of arrays in this texture</summary>
	/// <param name="newNumMembers">The new number of members in this array.</param>
	void setNumberOfArrayMembers(uint32 newNumMembers)
	{
		_header.numberOfSurfaces = newNumMembers;
	}

	/// <summary>Sets the number of MIP-Map levels in this texture.</summary>
	/// <param name="newNumMIPLevels">New number of MIP-Map levels.</param>
	void setNumberOfMIPLevels(uint32 newNumMIPLevels)
	{
		_header.mipMapCount = newNumMIPLevels;
	}

	/// <summary>Sets the number of faces stored in this texture.</summary>
	/// <param name="newNumFaces">New number of faces for this texture.</param>
	void setNumberOfFaces(uint32 newNumFaces)
	{
		_header.numberOfFaces = newNumFaces;
	}

	/// <summary>Sets the data orientation for a given axis in this texture.</summary>
	/// <param name="axisOrientation">Specifying axis and orientation.</param>
	void setOrientation(TextureMetaData::AxisOrientation axisOrientation);

	/// <summary>Sets a texture's bump map data.</summary>
	/// <param name="cubeMapOrder">Up to 6 character string, with values x,X,y,Y,z,Z in some combination.</param>
	/// <remarks>for <paramref name="cubmapOrder"/>Not all values need to be present. Denotes face order; Capitals
	/// refer to positive axis positions and small letters refer to negative axis positions. E.g. x=X-Negative,
	/// X=X-Positive. It is possible to have only some of these values rather than all, as long as they are NULL
	/// terminated. NB: Values past the 6th character are not read.</remarks>
	void setCubeMapOrder(std::string cubeMapOrder);

	/// <summary>Sets whether or not the texture is compressed using PVRTexLib's FILE compression - this is independent
	/// of any texture compression. Currently unsupported.</summary>
	/// <param name="isFileCompressed">Sets file compression to true/false.</param>
	void setIsFileCompressed(bool isFileCompressed)
	{
		if (isFileCompressed)
		{
			_header.flags |= Header::CompressedFlag;
		}
		else
		{
			_header.flags &= !Header::CompressedFlag;
		}
	}

	/// <summary>Sets whether or not the texture's color has been pre-multiplied by the alpha values.</summary>
	/// <param name="isPreMultiplied">Sets if texture is premultiplied.</param>
	void setIsPreMultiplied(bool isPreMultiplied)
	{
		if (isPreMultiplied)
		{
			_header.flags |= Header::PremultipliedFlag;
		}
		else
		{
			_header.flags &= !Header::PremultipliedFlag;
		}
	}

	/// <summary>Adds an arbitrary piece of meta data.</summary>
	/// <param name="metaData">Meta data block to be added.</param>
	void addMetaData(const TextureMetaData& metaData);
};
}