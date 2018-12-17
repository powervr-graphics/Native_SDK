/*!
\brief The main class that represents an Image (Texture).
\file PVRCore/texture/Texture.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/texture/TextureHeader.h"

namespace pvr {

/// <summary>Enumeration of the six faces of a Cube</summary>
enum class CubeFace : uint32_t
{
	PositiveX = 0, //!<+x
	NegativeX, //!<-x
	PositiveY, //!<+y
	NegativeY, //!<-y
	PositiveZ, //!<+z
	NegativeZ //!<-z
};

/// <summary>The dimension of an image.</summary>
enum class ImageType
{
	Image1D, //!< One-dimensional image
	Image2D, //!< Two-dimensional image
	Image3D, //!< Three-dimensional image
	Unallocated, //!< An image that has not been allocated yet
	Unknown, //!< An image of unknown dimensions
	Count = Image3D + 1
};

/// <summary>Enumeration of Texture dimensionalities.</summary>
enum class ImageViewType
{
	ImageView1D, //!< 1 dimensional Image View
	ImageView2D, //!< 2 dimensional Image View
	ImageView3D, //!< 3 dimensional Image View
	ImageView2DCube, //!< cube texture
	ImageView1DArray, //!< 1 dimensional Image View
	ImageView2DArray, //!< 2 dimensional Image View
	ImageView2DCubeArray, //!< 2 dimensional Image View
	ImageViewUnknown, //!< 3 dimensional Image View
};

/// <summary> Map an ImageViewType (2dCube etc) to its base type (1d/2d/3d)</summary>
/// <param name="viewtype">The ImageViewType</param>
/// <returns>The base type</returns>
inline ImageType imageViewTypeToImageBaseType(ImageViewType viewtype)
{
	switch (viewtype)
	{
	case ImageViewType::ImageView1D:
	case ImageViewType::ImageView1DArray:
		return ImageType::Image1D;

	case ImageViewType::ImageView2D:
	case ImageViewType::ImageView2DCube:
	case ImageViewType::ImageView2DArray:
	case ImageViewType::ImageView2DCubeArray:
		return ImageType::Image2D;

	case ImageViewType::ImageView3D:
		return ImageType::Image3D;

	default:
		return ImageType::Unallocated;
	}
}

/// <summary>Structure describes the number of array levels and mip levels an image contains</summary>
struct ImageLayersSize
{
	uint16_t numArrayLevels; //!< The number of array slices of the range
	uint16_t numMipLevels; //!< The number of mipmap levels of the range

	/// <summary>Constructor. All arguments optional.</summary>
	/// <param name="numArrayLevels">The number of array levels represented by this range. (Default 1).</param>
	/// <param name="numMipLevels">The number of mipmap levels represented by this range. (Default 1).</param>
	ImageLayersSize(uint16_t numArrayLevels = 1, uint8_t numMipLevels = 1) : numArrayLevels(numArrayLevels), numMipLevels(numMipLevels) {}
};

/// <summary>Represents an image format, including pixel format(channels/bits per channel), datatype and colorspace.
/// </summary>
struct ImageDataFormat
{
	PixelFormat format; //!< pixel format
	VariableType dataType; //!< datatype
	ColorSpace colorSpace; //!< colorspace, e.g lRGB

	/// <summary>Constructor. Creates a new ImageDataFormat. Default item is RGBA8888/UBYTE/lRGB</summary>
	/// <param name="format">The Pixel format. Default is RGBA_8888</param>
	/// <param name="dataType">The datatype. Default is UnsignedIntNormalized</param>
	/// <param name="colorSpace">The colorspace. Default is lRGB</param>
	explicit ImageDataFormat(const PixelFormat& format = PixelFormat::RGBA_8888(), VariableType dataType = VariableType::UnsignedByteNorm, ColorSpace colorSpace = ColorSpace::lRGB)
		: format(format), dataType(dataType), colorSpace(colorSpace)
	{}

	/// <summary>Equality operator. All content must be equal for it to return true.</summary>
	/// <param name="rhs">The right hand side of the operator</param>
	/// <returns>true if the right hand object is same as this.</returns>
	bool operator==(const ImageDataFormat& rhs) const
	{
		return (format == rhs.format && dataType == rhs.dataType && colorSpace == rhs.colorSpace);
	}

	/// <summary>Inequality operator. Equivalent to !(==)</summary>
	/// <param name="rhs">The right hand side of the operator</param>
	/// <returns>true if the right hand object is not same as this</returns>
	bool operator!=(const ImageDataFormat& rhs) const
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
	ImageStorageFormat(const PixelFormat& format = PixelFormat::RGBA_8888(), uint8_t numMipMapLevels = 1, ColorSpace colorSpace = ColorSpace::lRGB,
		VariableType dataType = VariableType::UnsignedByteNorm, uint8_t numSamples = 1)
		: ImageDataFormat(format, dataType, colorSpace), numMipMapLevels(numMipMapLevels), numSamples(numSamples)
	{}

	/// <summary>Constructor. Initializes to the provided values.</summary>
	/// <param name="dataFmt">The ImageDataFormat (will be copied as is)</param>
	/// <param name="numMipMapLevels">Number of mip levels</param>
	/// <param name="numSamples">number of samples</param>
	ImageStorageFormat(const ImageDataFormat& dataFmt, uint8_t numMipMapLevels = 1, uint8_t numSamples = 1)
		: ImageDataFormat(dataFmt), numMipMapLevels(numMipMapLevels), numSamples(numSamples)
	{}
};

/// <summary>Contains a 2d Integer size (width, height)</summary>
template<typename T>
struct GenericExtent2D
{
	T width; //!< Size along X axis
	T height; //!< Size along Y axis
			  /// <summary>Constructor by width and height</summary>
			  /// <param name="width">Horizontal size</param>
			  /// <param name="height">Vertical size</param>
	GenericExtent2D(T width = 0, T height = 0) : width(width), height(height) {}
};

/// <summary>Contains a 3D Integer size (width, height, depth)</summary>
template<typename Txy, typename Tz>
struct GenericExtent3D : public GenericExtent2D<Txy>
{
	Tz depth; //!< Size along Z axis
	GenericExtent3D() {}

	/// <summary>Constructor. Defaults to (1,1,1)</summary>
	/// <param name="width">Horizontal size (default 1)</param>
	/// <param name="height">Vertical size (default 1)</param>
	/// <param name="depth">Depth size (default 1)</param>
	GenericExtent3D(Txy width, Txy height, Tz depth = 1) : GenericExtent2D<Txy>(width, height), depth(depth) {}

	/// <summary>Constructor from GenericExtent2D)</summary>
	/// <param name="extent2D">Vertical and horizontal size</param>
	/// <param name="depth">Depth size (default 1)</param>
	GenericExtent3D(const GenericExtent2D<Txy>& extent2D, Tz depth = 1) : GenericExtent2D<Txy>(extent2D), depth(depth) {}
};

/// <summary>The GenericOffset2D contains a 16-bit 2D offset (offsetX, offsetY)</summary>
template<typename T>
struct GenericOffset2D
{
	T x; //!< offset in x axis
	T y; //!< offset in y axis
		 /// <summary>Constructor. Defaults to (0,0)</summary>
		 /// <param name="offsetX">Offset in the X direction. (Default 0)</param>
		 /// <param name="offsetY">Offset in the Y direction. (Default 0)</param>
	GenericOffset2D(T offsetX = 0, T offsetY = 0) : x(offsetX), y(offsetY) {}

	/// <summary>Sum this Offset with an Extent</summary>
	/// <param name="rhs">The right hand side of the additions</param>
	/// <returns>The result of this offset plus the extent rhs</returns>
	GenericOffset2D operator+(const GenericExtent2D<typename std::make_unsigned<T>::type>& rhs) const
	{
		return GenericOffset2D(*this) += rhs;
	}
	/// <summary>Add an Extent to this Offset</summary>
	/// <param name="rhs">The right hand side of the additions</param>
	/// <returns>This object, which now contains This offset plus the extent rhs</returns>
	GenericOffset2D& operator+=(const GenericExtent2D<typename std::make_unsigned<T>::type>& rhs)
	{
		x += rhs.width;
		y += rhs.height;
		return *this;
	}
};

/// <summary>The GenericOffset3D contains the offsets in 3 dimension (offsetX, offsetY, offsetZ)</summary>
template<typename Txy, typename Tz>
struct GenericOffset3D : public GenericOffset2D<Txy>
{
	Tz z; //!< offset in z axis
		  /// <summary>Constructor. Defaults to (0,0,0)</summary>
		  /// <param name="offsetX">Offset in the X direction. (Default 0)</param>
		  /// <param name="offsetY">Offset in the Y direction. (Default 0)</param>
		  /// <param name="offsetZ">Offset in the Z direction. (Default 0)</param>
	GenericOffset3D(Txy offsetX = 0, Txy offsetY = 0, Tz offsetZ = 0) : GenericOffset2D<Txy>(offsetX, offsetY), z(offsetZ) {}
	/// <summary>Sum this Offset with an Extent</summary>
	/// <param name="rhs">The right hand side of the additions</param>
	/// <returns>The result of this offset plus the extent rhs</returns>
	GenericOffset3D operator+(const GenericExtent3D<typename std::make_unsigned<Txy>::type, typename std::make_unsigned<Tz>::type>& rhs) const
	{
		return GenericOffset3D(*this) += rhs;
	}
	/// <summary>Add an Extent to this Offset</summary>
	/// <param name="rhs">The right hand side of the additions</param>
	/// <returns>This object, which now contains This offset plus the extent rhs</returns>
	GenericOffset3D& operator+=(const GenericExtent3D<typename std::make_unsigned<Txy>::type, typename std::make_unsigned<Tz>::type>& rhs)
	{
		GenericOffset2D<Txy>::x += rhs.width;
		GenericOffset2D<Txy>::y += rhs.height;
		z += rhs.height;
		return *this;
	}

	/// <summary>Constructor</summary>
	/// <param name="offsetXY">The 2D part of the offset</param>
	/// <param name="offsetZ">The depth of the offset. (Default 0)</param>
	GenericOffset3D(const GenericOffset2D<Txy>& offsetXY, Tz offsetZ = 0) : GenericOffset2D<Txy>(offsetXY), z(offsetZ) {}
};

/// <summary> A 2D, integer Offset typically used for Images</summary>
typedef GenericOffset2D<int32_t> Offset2D;
/// <summary> A 3D, integer Offset typically used for 3D Images</summary>
typedef GenericOffset3D<int32_t, int32_t> Offset3D;

/// <summary> A 2D, integer Extent typically used for Images</summary>
typedef GenericExtent2D<uint32_t> Extent2D;
/// <summary> A 3D, integer Extent  typically used for 3D Images</summary>
typedef GenericExtent3D<uint32_t, uint32_t> Extent3D;

/// <summary>Enumeration of the "aspect" (or "semantics") of an image: Color, Depth, Stencil.</summary>
enum class ImageAspectFlags : uint32_t
{
	Color = 0x1,
	Depth = 0x2,
	Stencil = 0x4,
	Metadata = 0x8,
	DepthAndStencil = Depth | Stencil,
};
/*! \brief Macro that defines all common bitwise operators for an enum-class */

inline ImageAspectFlags operator|(ImageAspectFlags lhs, ImageAspectFlags rhs)
{
	return static_cast<ImageAspectFlags>(static_cast<std::underlying_type<ImageAspectFlags>::type /**/>(lhs) | static_cast<std::underlying_type<ImageAspectFlags>::type /**/>(rhs));
}

inline void operator|=(ImageAspectFlags& lhs, ImageAspectFlags rhs)
{
	lhs = static_cast<ImageAspectFlags>(static_cast<std::underlying_type<ImageAspectFlags>::type /**/>(lhs) | static_cast<std::underlying_type<ImageAspectFlags>::type /**/>(rhs));
}

inline ImageAspectFlags operator&(ImageAspectFlags lhs, ImageAspectFlags rhs)
{
	return static_cast<ImageAspectFlags>(static_cast<std::underlying_type<ImageAspectFlags>::type /**/>(lhs) & static_cast<std::underlying_type<ImageAspectFlags>::type /**/>(rhs));
}

inline void operator&=(ImageAspectFlags& lhs, ImageAspectFlags rhs)
{
	lhs = static_cast<ImageAspectFlags>(static_cast<std::underlying_type<ImageAspectFlags>::type /**/>(lhs) & static_cast<std::underlying_type<ImageAspectFlags>::type /**/>(rhs));
}

/// <summary>Describes a single "layer" of an image: a single array layer of a single mip level, or the offset of a
/// layer range.</summary>
struct ImageSubresource
{
	ImageAspectFlags aspect; //!< The Aspect of the subresource (Color, Depth, Stencil, Depth&Stencil)
	uint16_t arrayLayerOffset; //!< The index of the array slice. In case of a range, the offset of the first layer.
	uint16_t mipLevelOffset; //!< The index of the mipmap level. In case of a range, the offset of the first mipmap level.
							 /// <summary>Constructor. All arguments optional.</summary>
							 /// <param name="mipLevelOffset">The index of the array slice. In case of a range, the offset of the first layer.
							 /// (Default 0)</param>
							 /// <param name="arrayLayerOffset">The index of the mipmap level. In case of a range, the offset of the first
							 /// mipmap level. (Default 0)</param>
							 /// <param name="aspectFlags">The aspect(s) of the subresource (Color/Depth/Stencil/DepthStencil)</param>
	ImageSubresource(ImageAspectFlags aspectFlags = ImageAspectFlags::Color, uint16_t mipLevelOffset = 0, uint16_t arrayLayerOffset = 0)
		: aspect(aspectFlags), arrayLayerOffset(arrayLayerOffset), mipLevelOffset(mipLevelOffset)
	{}
};

/// <summary>Represents a subresource range: A specified range of Array Layers and Mipmap levels of specific aspect of an
/// image</summary>
struct ImageSubresourceRange : public ImageLayersSize, public ImageSubresource
{
	/// <summary>Constructor</summary>
	ImageSubresourceRange() {}

	/// <summary>Constructor</summary>
	/// <param name="layersSize">Layers size</param>
	/// <param name="baseLayers">Base layers</param>
	ImageSubresourceRange(const ImageLayersSize& layersSize, const ImageSubresource& baseLayers) : ImageLayersSize(layersSize), ImageSubresource(baseLayers) {}
};

/// <summary>Represents a specific subresource layer: A specified Array Layer and Mipmap level of specific aspect of an
/// image</summary>
struct ImageSubresourceLayers : public ImageSubresource
{
	uint16_t numArrayLayers; //!< Number of array layers
							 /// <summary>Constructor</summary>
	ImageSubresourceLayers() : numArrayLayers(1) {}
	/// <summary>Constructor</summary>
	/// <param name="baseLayers">Base layers</param>
	/// <param name="numArrayLayers">Number of array layers</param>
	ImageSubresourceLayers(ImageSubresource baseLayers, uint16_t numArrayLayers) : ImageSubresource(baseLayers), numArrayLayers(numArrayLayers) {}
};

/// <summary>Represents a specific 3-D range in an image (an orthogonal cuboid anywhere in the image)</summary>
struct ImageRange : public Extent3D, public Offset3D
{
	/// <summary>ImageRange</summary>
	ImageRange() {}

	/// <summary>ImageRange</summary>
	/// <param name="extents"></param>
	/// <param name="offset"></param>
	ImageRange(const Extent3D& extents, const Offset3D& offset) : Extent3D(extents), Offset3D(offset) {}
};

/// <summary>Represents an image resolve operation</summary>
struct ImageResolveRange
{
	Offset3D srcOffset; //!< Source Region initial offset
	Offset3D dstOffset; //!< Destination Region initial offset
	Extent3D extent; //!< Size of the regions (as src must be equal to dst)
	ImageSubresourceLayers srcSubResource; //!< Source region subresource layers
	ImageSubresourceLayers dstSubResource; //!< Destination region subresource layers

	/// <summary>Constructor</summary>
	ImageResolveRange() {}

	/// <summary>Constructor</summary>
	/// <param name="srcOffset0">The source region's offset (bottom-left corner)</param>
	/// <param name="dstOffset0">The destination region's offset (bottom-left corner)</param>
	/// <param name="extent0">Size of both Source and Destination regions (as src must be equal to dst)</param>
	/// <param name="srcSubResource">Source region subresource layers</param>
	/// <param name="dstSubResource">Destination region subresource layers</param>
	ImageResolveRange(const Offset3D& srcOffset0, const Offset3D& dstOffset0, const Extent3D& extent0, const ImageSubresourceLayers& srcSubResource = ImageSubresourceLayers(),
		const ImageSubresourceLayers& dstSubResource = ImageSubresourceLayers())
	{
		srcOffset = srcOffset0;
		dstOffset = dstOffset0;
		extent = extent0;
		this->srcSubResource = srcSubResource;
		this->dstSubResource = dstSubResource;
	}
};

/// <summary>Describes a Compressed format. Compressed formats provide less information than the uncompressed format, as
/// they can only be accessed "black box".</summary>
struct CompressedImageDataFormat
{
	CompressedPixelFormat format; //!< compressed format
};

/// <summary>Describes a Compressed format. Compressed formats provide less information than the uncompressed format, as
/// they can only be accessed "black box".</summary>
struct ImageStorageFormatCompressed : public CompressedImageDataFormat
{
	int8_t numMipMapLevels; //!< number of mip levels
};

/// <summary>Enumerates the formats directly supported by the Framework.</summary>
enum class TextureFileFormat
{
	UNKNOWN = 0,
	KTX,
	DDX,
	PVR,
	TGA,
	BMP,
	DDS,
	JPEG
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
	std::vector<unsigned char> _pTextureData; // Pointer to texture data.
};

/// <summary>Infer the texture format from a filename.</summary>
/// <param name="assetname">The name of the asset, containing the extension.</param>
/// <returns>The TextureFileFormat if understood, otherwise TextureFileFormat::Unknown.</returns>
TextureFileFormat getTextureFormatFromFilename(const char* assetname);
} // namespace pvr
