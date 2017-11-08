/*!
\brief The main class that represents an Image (Texture).
\file PVRCore/Texture.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Texture/TextureHeader.h"

namespace pvr {

/// <summary>Represents an image format, including pixel format(channels/bits per channel), datatype and colorspace.
/// </summary>
struct ImageDataFormat
{
	PixelFormat format;//!< pixel format
	VariableType dataType;//!< datatype
	ColorSpace colorSpace;//!< colorspace, e.g lRGB

	/// <summary>Constructor. Creates a new ImageDataFormat. Default item is RGBA8888/UBYTE/lRGB</summary>
	/// <param name="format">The Pixel format. Default is RGBA_8888</param>
	/// <param name="dataType">The datatype. Default is UnsignedIntNormalized</param>
	/// <param name="colorSpace">The colorspace. Default is lRGB</param>
	ImageDataFormat(const PixelFormat& format = PixelFormat::RGBA_8888,
	                VariableType dataType = VariableType::UnsignedByteNorm,
	                ColorSpace colorSpace = ColorSpace::lRGB) :
		format(format), dataType(dataType), colorSpace(colorSpace) { }

	/// <summary>Equality operator. All content must be equal for it to return true.</summary>
	/// <param name="rhs">The right hand side of the operator</param>
	/// <returns>true if the right hand object is same as this.</returns>
	bool operator ==(const ImageDataFormat& rhs)const
	{
		return (format == rhs.format && dataType == rhs.dataType &&
		        colorSpace == rhs.colorSpace);
	}

	/// <summary>Inequality operator. Equivalent to !(==)</summary>
	/// <param name="rhs">The right hand side of the operator</param>
	/// <returns>true if the right hand object is not same as this</returns>
	bool operator !=(const ImageDataFormat& rhs)const
	{
		return !(*this == rhs);
	}
};

/// <summary>Extends the ImageDataFormat with mipmaps and number of Samples.</summary>
struct ImageStorageFormat : public ImageDataFormat
{
	uint8_t numMipMapLevels; //!<  number of mip levels
	uint8_t numSamples; //!<  number of samples

	/// <summary>Constructor. Initializes to the provided values.</summary>
	/// <param name="format">Pixel format</param>
	/// <param name="numMipMapLevels">Number of mip levels</param>
	/// <param name="colorSpace">Color space (linear RGB or sRGB)</param>
	/// <param name="dataType">The DataType</param>
	/// <param name="numSamples">number of samples</param>
	ImageStorageFormat(const PixelFormat& format = PixelFormat::RGBA_8888, uint8_t numMipMapLevels = 1,
	                   ColorSpace colorSpace = ColorSpace::lRGB,
	                   VariableType dataType = VariableType::UnsignedByteNorm, uint8_t numSamples = 1)
		: ImageDataFormat(format, dataType, colorSpace),
		  numMipMapLevels(numMipMapLevels), numSamples(numSamples) {}

	/// <summary>Constructor. Initializes to the provided values.</summary>
	/// <param name="dataFmt">The ImageDataFormat (will be copied as is)</param>
	/// <param name="numMipMapLevels">Number of mip levels</param>
	/// <param name="numSamples">number of samples</param>
	ImageStorageFormat(const ImageDataFormat& dataFmt, uint8_t numMipMapLevels = 1, uint8_t numSamples = 1) :
		ImageDataFormat(dataFmt), numMipMapLevels(numMipMapLevels), numSamples(numSamples) {}


};

/// <summary>Class used by texture update functions. Represents an area of the texture to be updated. Default value:
/// width=1, height=1, (depth=1), offsetx=0,offsety=0,offsetz=0, arrayslice=0, cubeface=0, miplevel=0</summary>
struct TextureArea
{
	uint32_t width;  //!< X-axis size (width) of the area of the texture to update. Default 1. At least 1.
	uint32_t height; //!< Y-axis size (height) of the area of the texture to update. Default 1. At least 1.
	/// <summary>Unnamed union for storing the 3rd SIZE coodinate: either Depth axis or Array Size</summary>
	union
	{
		uint32_t depth;  //!< Z-axis size (depth) of the area of the texture to update. Default 1. At least 1. IGNORED for 2D Textures.
		uint16_t arraySize; //!< Number of array slices of the area. IGNORED for non-array textures.
	};
	uint32_t offsetx; //!< x-coordinate of the start point of the area of the texture to update. Default 0.
	uint32_t offsety; //!< y-coordinate of the start point of the area of the texture to update. Default 0.
	/// <summary>Unnamed union for storing the 3rd OFFSET coodinate: either Offset along Z or Array Index offset</summary>
	union
	{
		uint32_t offsetz; //!< z-coordinate of the start point of the area of the texture to update. IGNORED for 2D Textures. Default 0.
		uint16_t arrayIndex; //!< Array index of the starting array slice of the area. IGNORED for non-array textures.
	};
	uint32_t compressedSize; //!< Size of the actual data that will be provided for updating a compressed texture. IGNORED for uncompressed textures.
	uint8_t cubeFace;        //!< Which face of the Cube texture to update. IGNORED for non-cube textures.
	uint8_t mipLevel;        //!< Which mipmap level of the texture to update. Default 0;

	/// <summary>Construct an empty texture area object</summary>
	TextureArea() : width(1), height(1), depth(1),
		offsetx(0), offsety(0), offsetz(0),
		compressedSize(0), cubeFace(0), mipLevel(0)
	{
	}

	/// <summary>Construct a texture area representing the most common case (zero-offset for a 2d uncompressed texture,
	/// z-dimension 1)</summary>
	/// <param name="width">The size of the textureArea along the x-direction, in texels (default 1)</param>
	/// <param name="height">The size of the textureArea along the y-direction, in texels (default 1)</param>
	TextureArea(uint32_t width, uint32_t height) : width(width), height(height), depth(1),
		offsetx(0), offsety(0), offsetz(0),
		compressedSize(0), cubeFace(0), mipLevel(0)
	{
	}

	/// <summary>Sets the size of a compressed texture. For a compressed texture, the dimensions cannot be used
	/// directly.</summary>
	/// <param name="mycompressedSize">The size, in bytes, of the texture.</param>
	void setCompressedSize(uint32_t mycompressedSize)
	{
		this->compressedSize = mycompressedSize;
	}

	/// <summary>Set the basic dimensions of the texture area (width/x,height/y,depth/z) in texels</summary>
	/// <param name="mywidth">The size of the textureArea along the x-direction, in texels</param>
	/// <param name="myheight">The size of the textureArea along the y-direction, in texels</param>
	/// <param name="mydepth">The size of the textureArea along the z-direction (default 1),in texels</param>
	void setDimensions(uint32_t mywidth, uint32_t myheight, uint32_t mydepth = 1)
	{
		this->width = mywidth;
		this->height = myheight;
		this->depth = mydepth;
	}

	/// <summary>Set the basic dimensions of the texture area, in pixels (width/x,height/y,depth/z)</summary>
	/// <param name="size">A 3d unsigned int vector containing the size of the area in each dimension, in texels
	/// (x=width, y=height, z=depth)</param>
	void setDimensions(const glm::uvec3& size = glm::uvec3(1, 1, 1))
	{
		this->width = size.x;
		this->height = size.y;
		this->depth = size.z;
	}

	/// <summary>Set the offset of the texture area is the distance along each direction from (0,0,0), in texels</summary>
	/// <param name="myoffsetx">The distance along the x-axis of the leftmost extent of the area from the left</param>
	/// <param name="myoffsety">The distance along the y-axis of the topmost extent of the area from the top</param>
	/// <param name="myoffsetz">The distance along the z-axis of the foremost extent of the area from the front</param>
	void setOffset(uint32_t myoffsetx, uint32_t myoffsety, uint32_t myoffsetz = 0)
	{
		this->offsetx = myoffsetx;
		this->offsety = myoffsety;
		this->offsetz = myoffsetz;
	}

	/// <summary>Set the offset of the texture area is the distance along each direction from (0,0,0), in texels.
	/// Initial value (initial value 0,0,0)</summary>
	/// <param name="offset">The offset of the area from the start described as a vector of unsigned integers</param>
	void setOffset(const glm::uvec3& offset = glm::uvec3(0, 0, 0))
	{
		this->width = offset.x;
		this->height = offset.y;
		this->depth = offset.z;
	}

	/// <summary>Set the mipmap level that the area represents (initial value 0)</summary>
	/// <param name="mymipLevel">The mipmap level that the area represents</param>
	void setMipLevel(uint32_t mymipLevel)
	{
		this->mipLevel = static_cast<uint8_t>(mymipLevel);
	}

	/// <summary>Set the array slice of an array texture that the TextureArea represents (initial value 0)</summary>
	/// <param name="myarrayIndex">The array slice that the area represents</param>
	void setArraySlice(uint32_t myarrayIndex)
	{
		this->arrayIndex = static_cast<uint16_t>(myarrayIndex);
	}

	/// <summary>Set the Cube face of a Cube Texture that the area represents (initial value CubeFacePositiveX)
	/// </summary>
	/// <param name="mycubeFace">The cube face that the area represents</param>
	void setCubeFace(CubeFace mycubeFace)
	{
		this->cubeFace = static_cast<uint8_t>(mycubeFace);
	}
};
/// <summary>Describes a Compressed format. Compressed formats provide less information than the uncompressed format, as
/// they can only be accessed "black box".</summary>
struct CompressedImageDataFormat
{
	CompressedPixelFormat format;//!< compressed format
};

/// <summary>Describes a Compressed format. Compressed formats provide less information than the uncompressed format, as
/// they can only be accessed "black box".</summary>
struct ImageStorageFormatCompressed : public CompressedImageDataFormat
{
	int8_t numMipMapLevels;//!< number of mip levels
};

/// <summary>Enumerates the formats directly supported by the Framework.</summary>
enum class TextureFileFormat
{
	UNKNOWN = 0, KTX, DDX, PVR, TGA, BMP, DDS
};

/// <summary>A 2D Texture asset, together with Information, Metadata and actual Pixel data. Only represents the
/// actual data, not the API objects that may be created from it.</summary>
class Texture : public TextureHeader
{
public:
	/// <summary>Construct a new empty texture.</summary>
	Texture();

	/// <summary>Create a texture using the information from a Texture header and copy the actual data from a provided
	/// pointer.</summary>
	/// <param name="sHeader">A texture header describing the texture</param>
	/// <param name="pData">Pointer to memory containing the actual data.</param>
	/// <remarks>Creates a new texture based on a texture header, pre-allocating the correct amount of memory. If data is
	/// supplied, it will be copied into memory. If the pointer contains less data than is dictated by the texture
	/// header, the behaviour is undefined.</remarks>
	Texture(const TextureHeader& sHeader, const char* pData = NULL);

	/// <summary>Create a texture using the information from a Texture header and preallocate memory for its data.
	/// </summary>
	/// <param name="sHeader">A texture header describing the texture</param>
	/// <remarks>Creates a new texture based on a texture header, pre-allocating the correct amount of memory.</remarks>
	void initializeWithHeader(const TextureHeader& sHeader);

	/// <summary>Returns a (const) pointer into the raw texture's data. Can be offset to a specific array member, face
	/// and/or MIP Map levels.</summary>
	/// <param name="mipMapLevel">The mip map level to get a pointer to (default 0)</param>
	/// <param name="arrayMember">The array member to get a pointer to (default 0)</param>
	/// <param name="faceNumber">The cube face to get a pointer to (default 0)</param>
	/// <returns>Const raw pointer to a location in the texture.</returns>
	/// <remarks>The data is contiguous so that the entire texture (all mips, array members and faces) can always be
	/// accessed from any pointer.</remarks>
	const unsigned char* getDataPointer(uint32_t mipMapLevel = 0, uint32_t arrayMember = 0, uint32_t faceNumber = 0) const;

	/// <summary>Returns a pointer into the raw texture's data. Can be offset to a specific array member, face and/or MIP
	/// Map levels.</summary>
	/// <param name="mipMapLevel">The mip map level for which to get the data pointer (default 0)</param>
	/// <param name="arrayMember">The array member for which to get the data pointer (default 0)</param>
	/// <param name="faceNumber">The face for which to get the data pointer (default 0)</param>
	/// <returns>Raw pointer to a location in the texture.</returns>
	/// <remarks>The data is contiguous so that the entire texture (all mips, array members and faces) can always be
	/// accessed from any pointer.</remarks>
	unsigned char* getDataPointer(uint32_t mipMapLevel = 0, uint32_t arrayMember = 0, uint32_t faceNumber = 0);


	/// <summary>Returns a pointer into the raw texture's data, offset to a specific pixel. DOES NOT WORK FOR COMPRESSED
	/// TEXTURES.</summary>
	/// <param name="x">The x position of the pointer</param>
	/// <param name="y">The y position of the pointer</param>
	/// <param name="z">The z position of the pointer (default 0)</param>
	/// <param name="mipMapLevel">The mip map level for which to get the data pointer (default 0)</param>
	/// <param name="arrayMember">The array member for which to get the data pointer (default 0)</param>
	/// <param name="faceNumber">The face for which to get the data pointer (default 0)</param>
	/// <returns>Raw pointer to a location in the texture.</returns>
	/// <remarks>The data is contiguous so that the entire texture (all mips, array members and faces) can always be
	/// accessed from any pointer. Equivalent to getDataPointer(mipMapLevel, arrayMember, faceNumber) + [char offset
	/// of pixel (x,y,z)]</remarks>
	unsigned char* getPixelPointer(uint32_t x, uint32_t y, uint32_t z = 0, uint32_t mipMapLevel = 0, uint32_t arrayMember = 0, uint32_t faceNumber = 0)
	{
		uint8_t pelsize = getPixelSize();
		size_t idx = (x + y * _header.width * +z * _header.width * _header.height) * pelsize;
		return getDataPointer(mipMapLevel, arrayMember, faceNumber) + idx;
	}

	/// <summary>Get the number of bytes size of each pixel in the texture. Not accurate for many compressed textures
	/// (e.g. ASTC)</summary>
	/// <returns>he number of bytes size of each pixel in the texture. May return zero for some compressed formats.
	/// </returns>
	uint8_t getPixelSize() const;

	/// <summary>Return the base dimensioning type of the image (3D, 2D, 1D).</summary>
	/// <returns>The base dimensioning type of the image (3D, 2D, 1D).</returns>
	ImageType getDimension() const
	{
		return getDepth() > 1 ? ImageType::Image3D : getHeight() > 1 ? ImageType::Image2D : ImageType::Image1D;
	}

	/// <summary>Return the texture's layer layout (miplevels, arraylevels). Faces are considered array levels, so a cube
	/// array has array x face array levels.</summary>
	/// <returns>The texture's layer layout (miplevels, arraylevels)</returns>
	ImageLayersSize getLayersSize() const
	{
		return ImageLayersSize(static_cast<uint16_t>(getNumArrayMembers() * getNumFaces()), static_cast<uint8_t>(getNumMipMapLevels()));
	}

	/// <summary>Return the texture's dimensions as a 3D extent (height, width, depth).</summary>
	/// <param name="miplevel">(Default 0) The mip level for which to get the dimensions</param>
	/// <returns>Return the texture's dimensions as a 3D extent (height, width, depth)</returns>
	Extent3D getDimensions(uint32_t miplevel = 0) const
	{
		return Extent3D(static_cast<uint16_t>(getWidth(miplevel)), static_cast<uint16_t>(getHeight(miplevel)), static_cast<uint16_t>(getDepth(miplevel)));
	}

	/// <summary>This function pads to a boundary value equal to "uiPadding". For example, setting alignment=8 will
	/// align the start of the texture data to an 8 char boundary.</summary>
	/// <param name="alignment">The final alignment of the metadata</param>
	/// <remarks>When writing the texture out to a PVR file, it is often desirable to pad the meta data so that the
	/// start of the texture data aligns to a given boundary. Note - this should be called immediately before saving
	/// (in any case, before adding any metadata) as the value is worked out based on the current meta data size.
	/// </remarks>
	void addPaddingMetaData(uint32_t alignment);

private:
	std::vector<unsigned char> _pTextureData;    // Pointer to texture data.
};

/// <summary>Infer the texture format from a filename.</summary>
/// <param name="assetname">The name of the asset, containing the extension.</param>
/// <returns>The TextureFileFormat if understood, otherwise TextureFileFormat::Unknown.</returns>
TextureFileFormat getTextureFormatFromFilename(const char* assetname);
}
