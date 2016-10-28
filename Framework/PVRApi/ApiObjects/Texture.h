/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\Texture.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         The PVRApi basic Texture implementation.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRAssets/PixelFormat.h"
namespace pvr {
namespace api {

/*!*********************************************************************************************************************
\brief Represents an image format, including pixel format(channels/bits per channel), datatype and colorspace.
***********************************************************************************************************************/
struct ImageDataFormat
{
	PixelFormat format;//!< pixel format
	VariableType dataType;//!< datatype
	types::ColorSpace colorSpace;//!< colorspace, e.g lRGB

	/*!*******************************************************************************************
	\param format The Pixel format
	\param dataType The datatype (e.g. UnsignedIntNormalized)
	\param colorSpace The colorspace e.g lRGB/sRGB
	**********************************************************************************************/
	ImageDataFormat(const PixelFormat& format = PixelFormat::RGBA_8888,
	                VariableType dataType = VariableType::UnsignedByteNorm,
	                types::ColorSpace colorSpace = types::ColorSpace::lRGB) :
		format(format), dataType(dataType), colorSpace(colorSpace) { }

	/*!*******************************************************************************************
	\return true if the right hand object is same as this.
	**********************************************************************************************/
	bool operator ==(const ImageDataFormat& rhs)const
	{
		return (format == rhs.format && dataType == rhs.dataType &&
		        colorSpace == rhs.colorSpace);
	}

	/*!*******************************************************************************************
	\return true if the right hand object is not same as this
	**********************************************************************************************/
	bool operator !=(const ImageDataFormat& rhs)const
	{
		return !(*this == rhs);
	}
};

/*!*******************************************************************************************
\brief ImageStorageFormat extends the ImageDataFormat with mipmaps and number of Samples.
**********************************************************************************************/
struct ImageStorageFormat : public ImageDataFormat
{
	int8 mipmapLevels;//< number of mip levels
	pvr::uint8 numSamples;//< number of samples

	/*!*******************************************************************************************
	\brief Constructor. Initializes to the provided values.
	\param format Pixel format
	\param mipmapLevels Number of mip levels
	\param colorSpace Color space (linear RGB or sRGB)
	\param dataType The DataType
	\param numSamples number of samples
	**********************************************************************************************/
	ImageStorageFormat(const PixelFormat& format = PixelFormat::RGBA_8888, int8 mipmapLevels = 1,
	                   types::ColorSpace colorSpace = types::ColorSpace::lRGB,
	                   VariableType dataType = VariableType::UnsignedByteNorm, pvr::uint8 numSamples = 1)
		: ImageDataFormat(format, dataType, colorSpace),
		  mipmapLevels(mipmapLevels), numSamples(numSamples) {}
};

/*!************************************************************************************************************
\brief Class used by texture update functions. Represents an area of the texture to be updated. Default value:
width=1, height=1, (depth=1), offsetx=0,offsety=0,offsetz=0, arrayslice=0, cubeface=0, miplevel=0
***************************************************************************************************************/
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

	/*!************************************************************************************************************
	\brief Construct an empty texture area object
	**************************************************************************************************************/
	TextureArea() : width(1), height(1), depth(1),
		offsetx(0), offsety(0), offsetz(0),
		compressedSize(0), cubeFace(0), mipLevel(0)
	{
	}

	/*!************************************************************************************************************
	\brief Construct a texture area representing the most common case (zero-offset for a 2d uncompressed texture,
	z-dimension 1)
	\param width The size of the textureArea along the x-direction, in texels  (default 1)
	\param height The size of the textureArea along the y-direction, in texels  (default 1)
	**************************************************************************************************************/
	TextureArea(uint32 width, uint32 height) : width(width), height(height), depth(1),
		offsetx(0), offsety(0), offsetz(0),
		compressedSize(0), cubeFace(0), mipLevel(0)
	{
	}

	/*!************************************************************************************************************
	\brief Sets the size of a compressed texture. For a compressed texture, the dimensions cannot be used directly.
	\param compressedSize The size, in bytes, of the texture.
	***************************************************************************************************************/
	void setCompressedSize(uint32 compressedSize)
	{
		this->compressedSize = compressedSize;
	}

	/*!************************************************************************************************************
	\brief Set the basic dimensions of the texture area (width/x,height/y,depth/z) in texels
	\param width The size of the textureArea along the x-direction, in texels
	\param height The size of the textureArea along the y-direction, in texels
	\param depth The size of the textureArea along the z-direction (default 1),in texels
	***************************************************************************************************************/
	void setDimensions(uint32 width, uint32 height, uint32 depth = 1)
	{
		this->width = width;
		this->height = height;
		this->depth = depth;
	}

	/*!************************************************************************************************************
	\brief Set the basic dimensions of the texture area, in pixels (width/x,height/y,depth/z)
	\param size A 3d unsigned int vector containing the size of the area in each dimension, in texels
	(x=width, y=height, z=depth)
	***************************************************************************************************************/
	void setDimensions(const glm::uvec3& size = glm::uvec3(1, 1, 1))
	{
		this->width = size.x;
		this->height = size.y;
		this->depth = size.z;
	}

	/*!************************************************************************************************************
	\brief Set the offset of the texture area is the distance along each direction from (0,0,0), in texels
	\param offsetx The distance along the x-axis of the leftmost extent of the area from the left
	\param offsety The distance along the y-axis of the topmost extent of the area from the top
	\param offsetz The distance along the z-axis of the foremost extent of the area from the front
	***************************************************************************************************************/
	void setOffset(uint32 offsetx, uint32 offsety, uint32 offsetz = 0)
	{
		this->offsetx = offsetx;
		this->offsety = offsety;
		this->offsetz = offsetz;
	}

	/*!************************************************************************************************************
	\brief Set the offset of the texture area is the distance along each direction from (0,0,0), in texels.
	Initial value (initial value 0,0,0)
	\param offset The offset of the area from the start described as a vector of unsigned integers
	***************************************************************************************************************/
	void setOffset(const glm::uvec3& offset = glm::uvec3(0, 0, 0))
	{
		this->width = offset.x;
		this->height = offset.y;
		this->depth = offset.z;
	}

	/*!************************************************************************************************************
	\brief Set the mipmap level that the area represents (initial value 0)
	\param mipLevel The mipmap level that the area represents
	***************************************************************************************************************/
	void setMipLevel(uint32 mipLevel)
	{
		this->mipLevel = (uint8)mipLevel;
	}

	/*!************************************************************************************************************
	\brief Set the array slice of an array texture that the TextureArea represents (initial value 0)
	\param arrayIndex The array slice that the area represents
	***************************************************************************************************************/
	void setArraySlice(uint32 arrayIndex)
	{
		this->arrayIndex = (uint16)arrayIndex;
	}

	/*!************************************************************************************************************
	\brief Set the Cube face of a Cube Texture that the area represents (initial value CubeFacePositiveX)
	\param cubeFace The cube face that the area represents
	***************************************************************************************************************/
	void setCubeFace(types::CubeFace cubeFace)
	{
		this->cubeFace = (uint8)cubeFace;
	}
};
/*!*******************************************************************************************
\brief Describes a Compressed format. Compressed formats provide less information than the
uncompressed format, as they can only be accessed "black box".
**********************************************************************************************/
struct CompressedImageDataFormat
{
	CompressedPixelFormat format;//!< compressed format
};

/*!*******************************************************************************************
\brief Describes a Compressed format. Compressed formats provide less information than the
uncompressed format, as they can only be accessed "black box".
**********************************************************************************************/
struct ImageStorageFormatCompressed : public CompressedImageDataFormat
{
	int8 mipmapLevels;//!< number of mip levels
};


class BindDescriptorSets;
namespace impl {
class TextureView_;

/*!*******************************************************************************************
\brief The class powering any texture. Wraps the underlying API Texture object and represents
the storage bits of any texture. Is normally used through a TextureView object.
Wrapped and accessed in a TextureStore Reference Counted Framework object.
**********************************************************************************************/
class TextureStore_
{
	friend class TextureView_;
	friend class ::pvr::api::impl::DescriptorSet_;
	friend class ::pvr::api::BindDescriptorSets;
	TextureStore_& operator=(const TextureStore_&);//deleted
	TextureStore_(const TextureStore_&);//deleted
public:
	/*!*******************************************************************************************
	\return Texture width
	**********************************************************************************************/
	uint16 getWidth() const
	{
		return (uint16)extents.width;
	}


	/*!*******************************************************************************************
	\return Texture height
	**********************************************************************************************/
	uint16 getHeight() const
	{
		return (uint16)extents.height;
	}

	/*!*******************************************************************************************
	\return Texture depth
	**********************************************************************************************/
	uint16 getDepth() const
	{
		return (uint16)extents.depth;
	}

	/*!*******************************************************************************************
	\return The number of array layers of this texture
	**********************************************************************************************/
	uint16 getNumArrayLayers() const
	{
		return layersSize.numArrayLevels;
	}

	/*!*******************************************************************************************
	\return The number of array layers of this texture
	**********************************************************************************************/
	uint16 getNumMipLevels() const
	{
		return layersSize.numMipLevels;
	}

	/*!*******************************************************************************************
	\brief Return the basic dimensioning of the texture (1D/2D/3D).
	\return The TextureDimension
	**********************************************************************************************/
	types::ImageBaseType getImageBaseType() const
	{
		return imageBaseType;
	}

	/*!*******************************************************************************************
	\brief Return a const reference to the format of the texture
	\return The const reference to the ImageStorageFormat
	**********************************************************************************************/
	const ImageStorageFormat&  getFormat() const
	{
		return format;
	}

	/*!*******************************************************************************************
	\brief Returns a native handle to the texture. Calling this function will require including
	the API-specific headers.
	\return An HTexture (thin smart pointer wrapper) over the underlying API object
	**********************************************************************************************/
	native::HTexture_& getNativeObject();

	/*!*******************************************************************************************
	\brief Returns a native handle to the texture. Calling this function will require including
	the API-specific headers.
	\return An HTexture (thin smart pointer wrapper) over the underlying API object
	**********************************************************************************************/
	const native::HTexture_& getNativeObject() const;

	/*!*******************************************************************************************
	\brief Check if this texture is allocated.
	\return true if the texture is allocated. Otherwise, the texture is empty and must be constructed.
	**********************************************************************************************/
	bool isAllocated() const;

	/*!*******************************************************************************************
	\brief Destructor. Will properly release all resources held by this object.
	**********************************************************************************************/
	virtual ~TextureStore_();

	/*!
	   \brief Allocate 2D texture. Only valid once.
	   \param format Image format
	   \param width Width
	   \param height Height
	   \param usage Image usage
	   \param newLayout New layout the image to be transformed in to. Default: Preintialized.
	 */
	void allocate2D(const ImageStorageFormat& format, uint32 width, uint32 height,
	                types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled,
	                types::ImageLayout newLayout = types::ImageLayout::Preinitialized);

	/*!
	   \brief Allocate 2D multisample texture. Only valid once.
	   \param format Texture format
	   \param width width
	   \param height height
	   \param samples Number of samples
	   \param usage Image usage
	   \param newLayout New layout the image to be transformed in to. Default: Preintialized.
	 */
	void allocate2DMS(
	  const ImageStorageFormat& format, uint32 width, uint32 height, types::SampleCount samples,
	  types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled,
	  types::ImageLayout newLayout = types::ImageLayout::Preinitialized);

	/*!
	   \brief Allocate 2DArray multisample texture. Only valid once.
	   \param format Texture format
	   \param width width
	   \param height height
	   \param arraySize Array size
	   \param samples Number of samples
	   \param usage Image usage
	   \param newLayout New layout the image to be transformed in to. Default: Preintialized.
	 */
	void allocate2DArrayMS(
	  const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize, types::SampleCount samples,
	  types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled,
	  types::ImageLayout newLayout = types::ImageLayout::Preinitialized);


	/*!************************************************************************************************************
	\brief Initialize a transient texture 2D with the specified parameters. Only valid once.
	***************************************************************************************************************/
	void allocateTransient(const ImageStorageFormat& format, uint32 width, uint32 height);

	/*!************************************************************************************************************
	\brief Initialize a storage texture 2D with the specified parameters. Only valid once.
	***************************************************************************************************************/
	void allocateStorage(const ImageStorageFormat& format, uint32 width, uint32 height);

	/*!************************************************************************************************************
	\brief Initialize the texture with the specified parameters. Only valid once.
	***************************************************************************************************************/
	void allocate2DCube(const ImageStorageFormat& format, uint32 width, uint32 height,
	                    types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled,
	                    types::ImageLayout newLayout = types::ImageLayout::Preinitialized);

	/*!************************************************************************************************************
	\brief Initialize the texture with the specified parameters. Only valid once.
	***************************************************************************************************************/
	void allocate2DArray(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize,
	                     types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled,
	                     types::ImageLayout newLayout = types::ImageLayout::Preinitialized);

	/*!************************************************************************************************************
	\brief Initialize the texture with the specified parameters. Only valid once.
	***************************************************************************************************************/
	void allocate3D(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth,
	                types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled,
	                types::ImageLayout newLayout = types::ImageLayout::Preinitialized);

	/*!************************************************************************************************************
	\brief Update the data of the texture. DOES NOT WORK WITH COMPRESSED TEXTURES.
	\param data Pointer to the memory which will be copied by the texture
	\param format The format of "data" pointer
	\param area A TextureArea object describing the area of the texture that will be updated by this call
	***************************************************************************************************************/
	void update(const void* data, const ImageDataFormat& format, const TextureArea& area);

	/*!*******************************************************************************************
	\return The reference to the context which owns this texture
	**********************************************************************************************/
	IGraphicsContext& getContext()
	{
		return *context;
	}

	/*!*******************************************************************************************
	\return The const reference to the context which owns this texture
	**********************************************************************************************/
	const IGraphicsContext& getContext()const
	{
		return *context;
	}

	bool is2DCubeMap()const
	{
		return isCubeMap;
	}
protected:
	TextureStore_(GraphicsContext& context, bool isCubeMap = false, types::ImageBaseType imageBaseType = types::ImageBaseType::Unallocated):
		context(context), isCubeMap(isCubeMap), imageBaseType(imageBaseType) {}

	TextureStore_() : isCubeMap(false) {}

	GraphicsContext       context;
	ImageStorageFormat      format;
	bool            isCubeMap;
	types::Extent3D       extents;
	types::ImageLayersSize      layersSize;
	types::ImageBaseType  imageBaseType;
	types::SampleCount  samplesCount;
};


/*!************************************************************************************************************
\brief Base texture view class.
***************************************************************************************************************/
class TextureView_
{
	friend class ::pvr::api::impl::DescriptorSet_;
	friend class ::pvr::api::impl::Sampler_;
protected:
	types::ImageViewType    viewtype;
	TextureStore          resource;//!<Texture view implementations access the underlying texture through this
	/*!************************************************************************************************************
	\brief DescriptorSets use this function to bind the texture to a texture unit.
	***************************************************************************************************************/
public:
	/*!************************************************************************************************************
	\brief Get the dimensionality of this texture.
	***************************************************************************************************************/
	types::ImageViewType getViewType()const
	{
		return viewtype;
	}

	/*!*******************************************************************************************
	\return dtor.
	**********************************************************************************************/
	virtual ~TextureView_() {}

	/*!*******************************************************************************************
	\return The reference to the native textrue view object
	**********************************************************************************************/
	const native::HImageView_& getNativeObject()const;

	/*!*******************************************************************************************
	\return The const reference to the native texture view object
	**********************************************************************************************/
	native::HImageView_& getNativeObject();

	/*!************************************************************************************************************
	\brief INTERNAL. Use context->createTextureView or utils::textureUpload
	\param texture
	\param view
	***************************************************************************************************************/
	TextureView_(const TextureStore& texture, const native::HImageView_& view);

	/*!************************************************************************************************************
	\brief INTERNAL. Use context->createTextureView or utils::textureUpload
	***************************************************************************************************************/
	TextureView_(const TextureStore& texture = TextureStore());

	/*!************************************************************************************************************
	\brief Get the underlying TextureStore object.
	***************************************************************************************************************/
	const TextureStore& getResource() const
	{
		return resource;
	}
	/*!************************************************************************************************************
	\brief Get the underlying TextureStore object.
	***************************************************************************************************************/
	TextureStore& getResource()
	{
		return resource;
	}

	/*!************************************************************************************************************
	\brief Query if this object contains a valid reference to an actual Texture.
	***************************************************************************************************************/
	bool isAllocated()
	{
		return (resource.isValid() && resource->isAllocated());
	}

	/*!*******************************************************************************************
	\return The reference to the context which owns this object
	**********************************************************************************************/
	IGraphicsContext& getContext()
	{
		return getResource()->getContext();
	}

	/*!*******************************************************************************************
	\return The const reference to the context which owns this object
	**********************************************************************************************/
	const IGraphicsContext& getContext()const
	{
		return getResource()->getContext();
	}

};
}//namespace impl
}//namespace api
}//namespace pvr
