/*!
\brief The PVRApi basic Texture implementation.
\file PVRApi/ApiObjects/Texture.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
<<<<<<< HEAD
#include "PVRAssets/PixelFormat.h"
=======
#include "PVRCore/Texture.h"
>>>>>>> 1776432f... 4.3
namespace pvr {
namespace api {
namespace impl {
class TextureView_;

/// <summary>The class powering any texture. Wraps the underlying API Texture object and represents the storage
/// bits of any texture. Is normally used through a TextureView object. Wrapped and accessed in a TextureStore
/// Reference Counted Framework object.</summary>
class TextureStore_
{
<<<<<<< HEAD
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
=======
	friend class TextureView_;
	friend class ::pvr::api::impl::DescriptorSet_;
	TextureStore_& operator=(const TextureStore_&);//deleted
	TextureStore_(const TextureStore_&);//deleted
public:
	/// <summary>Get the width of this texture (number of columns of texels in the lowest mipmap)</summary>
	/// <returns>Texture width</returns>
	uint16 getWidth() const
>>>>>>> 1776432f... 4.3
	{
		return (uint16)_extents.width;
	}

<<<<<<< HEAD
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
=======

	/// <summary>Get the height of this texture (number of rows of texels in the lowest mipmap)</summary>
	/// <returns>Texture height</returns>
	uint16 getHeight() const
	{
		return (uint16)_extents.height;
	}
>>>>>>> 1776432f... 4.3

	/// <summary>Get the depth of this texture (number of (non-array) layers of texels in the lowest mipmap)
	/// </summary>
	/// <returns>Texture depth</returns>
	uint16 getDepth() const
	{
		return (uint16)_extents.depth;
	}

	/// <summary>Get the number of array layers of this texture</summary>
	/// <returns>The number of array layers of this texture</returns>
	uint16 getNumArrayLayers() const
	{
		return _layersSize.numArrayLevels;
	}

	/// <summary>Get the number of MipMap levels of this texture</summary>
	/// <returns>The number of MipMap levels layers of this texture</returns>
	uint16 getNumMipLevels() const
	{
		return _layersSize.numMipLevels;
	}

	/// <summary>Get the basic dimensioning of the texture (1D/2D/3D).</summary>
	/// <returns>The TextureDimension</returns>
	types::ImageBaseType getImageBaseType() const
	{
		return _imageBaseType;
	}

<<<<<<< HEAD
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
=======
	/// <summary>Return a const reference to the format of the texture</summary>
	/// <returns>The const reference to the ImageStorageFormat</returns>
	const ImageStorageFormat&  getFormat() const
	{
		return _format;
	}
>>>>>>> 1776432f... 4.3

	/// <summary>Check if this texture is allocated.</summary>
	/// <returns>true if the texture is allocated. Otherwise, the texture is empty and must be constructed.</returns>
	bool isAllocated() const { return isAllocated_(); }

	/// <summary>Destructor. Will properly release all resources held by this object.</summary>
	virtual ~TextureStore_() {}

	/// <summary>Allocate 2D texture. Only valid once.</summary>
	/// <param name="format">Image format</param>
	/// <param name="width">Width</param>
	/// <param name="height">Height</param>
	/// <param name="usage">Image usage</param>
	/// <param name="newLayout">New layout the image to be transformed in to. Default: Preintialized.</param>
	void allocate2D(const ImageStorageFormat& format, uint32 width, uint32 height,
	                types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled,
	                types::ImageLayout newLayout = types::ImageLayout::Preinitialized)
	{ allocate2D_(format, width, height, usage, newLayout); }

	/// <summary>Allocate 2D multisample texture. Only valid once.</summary>
	/// <param name="format">Texture format</param>
	/// <param name="width">Texture width</param>
	/// <param name="height">Texture height</param>
	/// <param name="usage">Image usage</param>
	/// <param name="newLayout">New layout the image to be transformed in to. Default: Preintialized.</param>
	void allocate2DMS(
	  const ImageStorageFormat& format, uint32 width, uint32 height,
	  types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled,
	  types::ImageLayout newLayout = types::ImageLayout::Preinitialized)
	{ allocate2DMS_(format, width, height, usage, newLayout); }


	/// <summary>Allocate 2DArray multisample texture. Only valid once.</summary>
	/// <param name="format">Texture format</param>
	/// <param name="width">width</param>
	/// <param name="height">height</param>
	/// <param name="arraySize">Array size</param>
	/// <param name="usage">Image usage</param>
	/// <param name="newLayout">New layout the image to be transformed in to. Default: Preintialized.</param>
	void allocate2DArrayMS(
	  const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize,
	  types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled,
	  types::ImageLayout newLayout = types::ImageLayout::Preinitialized)
	{ allocate2DArrayMS_(format, width, height, arraySize, usage, newLayout); }

	/// <summary>Allocate a transient 2D texture. A transient texture is one that will be used "logistically" between
	/// subpasses/draws but the implementation is encouraged to NOT allocate memory for. It is invalid to call this
	/// function on an already allocated texture.</summary>
	/// <param name="format">Texture format</param>
	/// <param name="width">Texture width</param>
	/// <param name="usage">Bitwise-or'ed allowed usage flags for this texture</param>
	/// <param name="imageLayout">Initial layout to which the image will be created</param>
	/// <remarks>A typical use for a transient attachment, is a G-Buffer.</remarks>
	void allocateTransient(const ImageStorageFormat& format, uint32 width, uint32 height,
	                       types::ImageUsageFlags usage = types::ImageUsageFlags::ColorAttachment |
	                           types::ImageUsageFlags::InputAttachment | types::ImageUsageFlags::TransientAttachment,
	                       types::ImageLayout imageLayout = types::ImageLayout::ColorAttachmentOptimal)
	{ allocateTransient_(format, width, height, usage, imageLayout); }

	/// <summary>Initialize a storage texture 2D. A storage texture is one that whose texels will be accessed directly
	/// through image load/store and not through a sampler.It is invalid to call this function on an already allocated
	/// texture.</summary>
	/// <param name="format">Texture format</param>
	/// <param name="width">Texture width</param>
	/// <param name="height">Texture height</param>
	void allocateStorage(const ImageStorageFormat& format, uint32 width, uint32 height)
	{ allocateStorage_(format, width, height); }

	/// <summary>Initialize a cubemap texture. It is invalid to call this function on an already allocated texture.
	/// </summary>
	/// <param name="format">Texture format</param>
	/// <param name="width">Texture width</param>
	/// <param name="height">Texture height</param>
	/// <param name="usage">All allowed usage flags</param>
	/// <param name="initialLayout">The initial Layout of the image.</param>
	void allocate2DCube(const ImageStorageFormat& format, uint32 width, uint32 height,
	                    types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled | types::ImageUsageFlags::TransferDest,
	                    types::ImageLayout initialLayout = types::ImageLayout::Preinitialized)
	{ allocate2DCube_(format, width, height, usage, initialLayout); }

	/// <summary>Initialize a 2D Array texture. It is invalid to call this function on an already allocated texture.
	/// </summary>
	/// <param name="format">Texture format</param>
	/// <param name="width">Texture width</param>
	/// <param name="height">Texture height</param>
	/// <param name="arraySize">Number of array slices</param>
	/// <param name="usage">All allowed usage flags</param>
	/// <param name="initialLayout">The initial Layout of the image.</param>
	void allocate2DArray(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize,
	                     types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled | types::ImageUsageFlags::TransferDest,
	                     types::ImageLayout initialLayout = types::ImageLayout::Preinitialized)
	{ allocate2DArray_(format, width, height, arraySize, usage, initialLayout); }

	/// <summary>Initialize a 3D texture. It is invalid to call this function on an already allocated texture.</summary>
	/// <param name="format">Texture format</param>
	/// <param name="width">Texture width</param>
	/// <param name="height">Texture height</param>
	/// <param name="depth">Number of array slices</param>
	/// <param name="usage">All allowed usage flags</param>
	/// <param name="initialLayout">The initial Layout of the image.</param>
	void allocate3D(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth,
	                types::ImageUsageFlags usage = types::ImageUsageFlags::Sampled | types::ImageUsageFlags::TransferDest,
	                types::ImageLayout initialLayout = types::ImageLayout::Preinitialized)
	{ allocate3D_(format, width, height, depth, usage, initialLayout); }


	/// <summary>Update the data of the texture. DOES NOT WORK WITH COMPRESSED TEXTURES.</summary>
	/// <param name="data">Pointer to the memory which will be copied by the texture</param>
	/// <param name="format">The format of "data" pointer</param>
	/// <param name="area">A TextureArea object describing the area of the texture that will be updated by this call
	/// </param>
	void update(const void* data, const ImageDataFormat& format, const TextureArea& area)
	{ update_(data, format, area); }

	/// <summary>Get a reference to the context which owns this texture</summary>
	/// <returns>The reference to the context which owns this texture</returns>
	IGraphicsContext& getContext()
	{
		return *_context;
	}

	/// <summary>Get a reference to the context which owns this texture</summary>
	/// <returns>The const reference to the context which owns this texture</returns>
	const IGraphicsContext& getContext()const
	{
		return *_context;
	}

	/// <summary>Check if this texture is a cubemap</summary>
	/// <returns>true if the texture is a cubemap, otherwise false</returns>
	bool is2DCubeMap()const
	{
		return _isCubeMap;
	}

<<<<<<< HEAD
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
=======
	bool isTransient()const { return _isTransient; }
>>>>>>> 1776432f... 4.3

	bool is2DCubeMap()const
	{
		return isCubeMap;
	}
protected:
<<<<<<< HEAD
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
=======
	TextureStore_(const GraphicsContext& context, bool isCubeMap = false, types::ImageBaseType imageBaseType = types::ImageBaseType::Unallocated):
		_context(context), _isCubeMap(isCubeMap), _imageBaseType(imageBaseType), _isTransient(false) {}

	TextureStore_() : _isCubeMap(false), _isTransient(false) {}


	virtual bool isAllocated_() const = 0;
	virtual void allocate2D_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout finalLayout) = 0;
	virtual void allocate2DMS_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout finalLayout) = 0;
	virtual void allocate2DArrayMS_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize, types::ImageUsageFlags usage, types::ImageLayout finalLayout) = 0;
	virtual void allocateTransient_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout finalLayout) = 0;
	virtual void allocateStorage_(const ImageStorageFormat& format, uint32 width, uint32 height) = 0;
	virtual void allocate2DCube_(const ImageStorageFormat& format, uint32 width, uint32 height, types::ImageUsageFlags usage, types::ImageLayout finalLayout) = 0;
	virtual void allocate2DArray_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize, types::ImageUsageFlags usage, types::ImageLayout finalLayout) = 0;
	virtual void allocate3D_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth, types::ImageUsageFlags usage, types::ImageLayout finalLayout) = 0;
	virtual void update_(const void* data, const ImageDataFormat& format, const TextureArea& area) = 0;



	GraphicsContext _context;
	ImageStorageFormat _format;
	bool _isCubeMap;
	bool _isTransient;
	types::Extent3D _extents;
	types::ImageLayersSize _layersSize;
	types::ImageBaseType _imageBaseType;
	types::SampleCount _samplesCount;
>>>>>>> 1776432f... 4.3
};


/// <summary>Base texture view class.</summary>
class TextureView_
{
	friend class ::pvr::api::impl::DescriptorSet_;
	friend class ::pvr::api::impl::Sampler_;
<<<<<<< HEAD
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
=======
public:
	/// <summary>Get the dimensionality (1D/2D/3D) of this texture.</summary>
	/// <returns>The dimensionality of this texture</returns>
>>>>>>> 1776432f... 4.3
	types::ImageViewType getViewType()const
	{
		return viewtype;
	}

<<<<<<< HEAD
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
=======
	/// <summary>Destructor (virtual)</summary>
	virtual ~TextureView_() {}

	/// <summary>INTERNAL. Use context->createTextureView or utils::textureUpload</summary>
	TextureView_(const TextureStore& texture, const native::HImageView_& view);

	/// <summary>INTERNAL. Use context->createTextureView or utils::textureUpload</summary>
	TextureView_(const TextureStore& texture = TextureStore());

	/// <summary>Get the underlying TextureStore object.</summary>
	/// <returns>The underlying TextureStore object.</returns>
>>>>>>> 1776432f... 4.3
	const TextureStore& getResource() const
	{
		return resource;
	}
<<<<<<< HEAD
	/*!************************************************************************************************************
	\brief Get the underlying TextureStore object.
	***************************************************************************************************************/
=======
	/// <summary>Get the underlying TextureStore object.</summary>
	/// <returns>The underlying TextureStore object.</returns>
>>>>>>> 1776432f... 4.3
	TextureStore& getResource()
	{
		return resource;
	}

<<<<<<< HEAD
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
=======
	/// <summary>Query if this object contains a valid reference to an actual Texture.</summary>
	/// <returns>true if the texture is allocated (has an underlying image object), false otherwise.</returns>
	bool isAllocated()
	{
		return ((resource.isValid() && resource->isAllocated()) != 0);
	}

	/// <summary>Get the context that owns this object</summary>
	/// <returns>The context which owns this object</returns>
	IGraphicsContext& getContext()
>>>>>>> 1776432f... 4.3
	{
		return getResource()->getContext();
	}

<<<<<<< HEAD
=======
	/// <summary>Get the context that owns this object</summary>
	/// <returns>The context which owns this object</returns>
	const IGraphicsContext& getContext()const
	{
		return getResource()->getContext();
	}

protected:
	types::ImageViewType    viewtype;
	TextureStore          resource;//!<Texture view implementations access the underlying texture through this
private:
>>>>>>> 1776432f... 4.3
};
}//namespace impl
}//namespace api
}//namespace pvr
