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
	types::ColorSpace colorSpace;//!< colorspace, e.g lRGB

	/// <summary>Constructor. Creates a new ImageDataFormat. Default item is RGBA8888/UBYTE/lRGB</summary>
	/// <param name="format">The Pixel format. Default is RGBA_8888</param>
	/// <param name="dataType">The datatype. Default is UnsignedIntNormalized</param>
	/// <param name="colorSpace">The colorspace. Default is lRGB</param>
	ImageDataFormat(const PixelFormat& format = PixelFormat::RGBA_8888,
	                VariableType dataType = VariableType::UnsignedByteNorm,
	                types::ColorSpace colorSpace = types::ColorSpace::lRGB) :
		format(format), dataType(dataType), colorSpace(colorSpace) { }

	/// <summary>Equality operator. All content must be equal for it to return true.</summary>
	/// <returns>true if the right hand object is same as this.</returns>
	bool operator ==(const ImageDataFormat& rhs)const
	{
		return (format == rhs.format && dataType == rhs.dataType &&
		        colorSpace == rhs.colorSpace);
	}

	/// <summary>Inequality operator. Equivalent to !(==)</summary>
	/// <returns>true if the right hand object is not same as this</returns>
	bool operator !=(const ImageDataFormat& rhs)const
	{
		return !(*this == rhs);
	}
};

/// <summary>Extends the ImageDataFormat with mipmaps and number of Samples.</summary>
struct ImageStorageFormat : public ImageDataFormat
{
	uint8 mipmapLevels;//< number of mip levels
	pvr::uint8 numSamples;//< number of samples

	/// <summary>Constructor. Initializes to the provided values.</summary>
	/// <param name="format">Pixel format</param>
	/// <param name="mipmapLevels">Number of mip levels</param>
	/// <param name="colorSpace">Color space (linear RGB or sRGB)</param>
	/// <param name="dataType">The DataType</param>
	/// <param name="numSamples">number of samples</param>
	ImageStorageFormat(const PixelFormat& format = PixelFormat::RGBA_8888, uint8 mipmapLevels = 1,
	                   types::ColorSpace colorSpace = types::ColorSpace::lRGB,
	                   VariableType dataType = VariableType::UnsignedByteNorm, pvr::uint8 numSamples = 1)
		: ImageDataFormat(format, dataType, colorSpace),
		  mipmapLevels(mipmapLevels), numSamples(numSamples) {}

	ImageStorageFormat(const ImageDataFormat& dataFmt, uint8 mipMapLevels = 1, uint8 numSamples = 1) :
		ImageDataFormat(dataFmt), mipmapLevels(mipMapLevels), numSamples(numSamples) {}


};

/// <summary>Class used by texture update functions. Represents an area of the texture to be updated. Default value:
/// width=1, height=1, (depth=1), offsetx=0,offsety=0,offsetz=0, arrayslice=0, cubeface=0, miplevel=0</summary>
struct TextureArea
{
	uint32 width;  //< ! X-axis size (width) of the area of the texture to update. Default 1. At least 1.
	uint32 height; //< ! Y-axis size (height) of the area of the texture to update. Default 1. At least 1.
	union
	{
		uint32 depth;  //< ! Z-axis size (depth) of the area of the texture to update. Default 1. At least 1. IGNORED for 2D Textures.
		uint16 arraySize;      //<! Number of array slices of the area. IGNORED for non-array textures.
	};
	uint32 offsetx; //< ! x-coordinate of the start point of the area of the texture to update. Default 0.
	uint32 offsety; //< ! y-coordinate of the start point of the area of the texture to update. Default 0.
	union
	{
		uint32 offsetz; //< ! z-coordinate of the start point of the area of the texture to update. IGNORED for 2D Textures. Default 0.
		uint16 arrayIndex;     //<! Array index of the starting array slice of the area. IGNORED for non-array textures.
	};
	uint32 compressedSize; //<! Size of the actual data that will be provided for updating a compressed texture. IGNORED for uncompressed textures.
	uint8 cubeFace;        //<! Which face of the Cube texture to update. IGNORED for non-cube textures.
	uint8 mipLevel;        //<! Which mipmap level of the texture to update. Default 0;

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
	TextureArea(uint32 width, uint32 height) : width(width), height(height), depth(1),
		offsetx(0), offsety(0), offsetz(0),
		compressedSize(0), cubeFace(0), mipLevel(0)
	{
	}

	/// <summary>Sets the size of a compressed texture. For a compressed texture, the dimensions cannot be used
	/// directly.</summary>
	/// <param name="compressedSize">The size, in bytes, of the texture.</param>
	void setCompressedSize(uint32 compressedSize)
	{
		this->compressedSize = compressedSize;
	}

	/// <summary>Set the basic dimensions of the texture area (width/x,height/y,depth/z) in texels</summary>
	/// <param name="width">The size of the textureArea along the x-direction, in texels</param>
	/// <param name="height">The size of the textureArea along the y-direction, in texels</param>
	/// <param name="depth">The size of the textureArea along the z-direction (default 1),in texels</param>
	void setDimensions(uint32 width, uint32 height, uint32 depth = 1)
	{
		this->width = width;
		this->height = height;
		this->depth = depth;
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

	/// <summary>Set the offset of the texture area is the distance along each direction from (0,0,0), in texels
	/// </summary>
	/// <param name="offsetx">The distance along the x-axis of the leftmost extent of the area from the left</param>
	/// <param name="offsety">The distance along the y-axis of the topmost extent of the area from the top</param>
	/// <param name="offsetz">The distance along the z-axis of the foremost extent of the area from the front</param>
	void setOffset(uint32 offsetx, uint32 offsety, uint32 offsetz = 0)
	{
		this->offsetx = offsetx;
		this->offsety = offsety;
		this->offsetz = offsetz;
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
	/// <param name="mipLevel">The mipmap level that the area represents</param>
	void setMipLevel(uint32 mipLevel)
	{
		this->mipLevel = (uint8)mipLevel;
	}

	/// <summary>Set the array slice of an array texture that the TextureArea represents (initial value 0)</summary>
	/// <param name="arrayIndex">The array slice that the area represents</param>
	void setArraySlice(uint32 arrayIndex)
	{
		this->arrayIndex = (uint16)arrayIndex;
	}

	/// <summary>Set the Cube face of a Cube Texture that the area represents (initial value CubeFacePositiveX)
	/// </summary>
	/// <param name="cubeFace">The cube face that the area represents</param>
	void setCubeFace(types::CubeFace cubeFace)
	{
		this->cubeFace = (uint8)cubeFace;
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
	int8 mipmapLevels;//!< number of mip levels
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
	Texture(const TextureHeader& sHeader, const byte* pData = NULL);

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
	const byte* getDataPointer(uint32 mipMapLevel = 0, uint32 arrayMember = 0, uint32 faceNumber = 0) const;

	/// <summary>Returns a pointer into the raw texture's data. Can be offset to a specific array member, face and/or MIP
	/// Map levels.</summary>
	/// <param name="mipMapLevel">The mip map level for which to get the data pointer (default 0)</param>
	/// <param name="arrayMember">The array member for which to get the data pointer (default 0)</param>
	/// <param name="faceNumber">The face for which to get the data pointer (default 0)</param>
	/// <returns>Raw pointer to a location in the texture.</returns>
	/// <remarks>The data is contiguous so that the entire texture (all mips, array members and faces) can always be
	/// accessed from any pointer.</remarks>
	byte* getDataPointer(uint32 mipMapLevel = 0, uint32 arrayMember = 0, uint32 faceNumber = 0);


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
	/// accessed from any pointer. Equivalent to getDataPointer(mipMapLevel, arrayMember, faceNumber) + [byte offset
	/// of pixel (x,y,z)]</remarks>
	byte* getPixelPointer(uint32 x, uint32 y, uint32 z = 0, uint32 mipMapLevel = 0, uint32 arrayMember = 0, uint32 faceNumber = 0)
	{
		uint8 pelsize = getPixelSize();
		size_t idx = (x + y * _header.width * +z * _header.width * _header.height) * pelsize;
		return getDataPointer(mipMapLevel, arrayMember, faceNumber) + idx;
	}

	/// <summary>Get the number of bits size of each pixel in the texture. Not accurate for many compressed textures
	/// (e.g. ASTC)</summary>
	/// <returns>he number of bits size of each pixel in the texture. May return zero for some compressed formats.
	/// </returns>
	uint8 getPixelSize() const;

	/// <summary>Return the base dimensioning type of the image (3D, 2D, 1D).</summary>
	/// <returns>The base dimensioning type of the image (3D, 2D, 1D).</returns>
	types::ImageBaseType getDimension() const
	{
		return getDepth() > 1 ? types::ImageBaseType::Image3D : getHeight() > 1 ? types::ImageBaseType::Image2D : types::ImageBaseType::Image1D;
	}

	/// <summary>Return the entire size of the image as an ImageAreaSize (height, width, depth, miplevels, arraylayers)
	/// </summary>
	/// <returns>The entire size of the image as an ImageAreaSize (height, width, depth, miplevels, arraylayers)
	/// </returns>
	types::ImageAreaSize getTotalDimensions() const
	{
		return types::ImageAreaSize(getLayersSize(), getDimensions());
	}

	/// <summary>Return the texture's layer layout (miplevels, arraylevels). Faces are considered array levels, so a cube
	/// array has array x face array levels.</summary>
	/// <returns>The texture's layer layout (miplevels, arraylevels)</returns>
	types::ImageLayersSize getLayersSize() const
	{
		return types::ImageLayersSize((uint16)(getNumberOfArrayMembers() * getNumberOfFaces()), (uint8)getNumberOfMIPLevels());
	}

	/// <summary>Return the texture's dimensions as a 3D extent (height, width, depth).</summary>
	/// <returns>Return the texture's dimensions as a 3D extent (height, width, depth)</returns>
	types::Extent3D getDimensions(uint32 miplevel = 0) const
	{
		return types::Extent3D((uint16)getWidth(miplevel), (uint16)getHeight(miplevel), (uint16)getDepth(miplevel));
	}

	/// <summary>This function pads to a boundary value equal to "uiPadding". For example, setting alignment=8 will
	/// align the start of the texture data to an 8 byte boundary.</summary>
	/// <param name="alignment">The final alignment of the metadata</param>
	/// <remarks>When writing the texture out to a PVR file, it is often desirable to pad the meta data so that the
	/// start of the texture data aligns to a given boundary. Note - this should be called immediately before saving
	/// (in any case, before adding any metadata) as the value is worked out based on the current meta data size.
	/// </remarks>
	void addPaddingMetaData(uint32 alignment);

private:
	std::vector<byte> _pTextureData;    // Pointer to texture data.
};

/// <summary>Infer the texture format from a filename.</summary>
/// <param name="assetname">The name of the asset, containing the extension.</param>
/// <returns>The TextureFileFormat if understood, otherwise TextureFileFormat::Unknown.</returns>
TextureFileFormat getTextureFormatFromFilename(const char* assetname);
}
