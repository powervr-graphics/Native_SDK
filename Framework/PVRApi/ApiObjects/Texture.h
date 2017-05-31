/*!
\brief The PVRApi basic Texture implementation.
\file PVRApi/ApiObjects/Texture.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRCore/Texture.h"
namespace pvr {
namespace api {
namespace impl {
class TextureView_;

/// <summary>The class powering any texture. Wraps the underlying API Texture object and represents the storage
/// bits of any texture. Is normally used through a TextureView object. Wrapped and accessed in a TextureStore
/// Reference Counted Framework object.</summary>
class TextureStore_
{
	friend class TextureView_;
	friend class ::pvr::api::impl::DescriptorSet_;
	TextureStore_& operator=(const TextureStore_&);//deleted
	TextureStore_(const TextureStore_&);//deleted
public:
	/// <summary>Get the width of this texture (number of columns of texels in the lowest mipmap)</summary>
	/// <returns>Texture width</returns>
	uint16 getWidth() const
	{
		return (uint16)_extents.width;
	}


	/// <summary>Get the height of this texture (number of rows of texels in the lowest mipmap)</summary>
	/// <returns>Texture height</returns>
	uint16 getHeight() const
	{
		return (uint16)_extents.height;
	}

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

	/// <summary>Return a const reference to the format of the texture</summary>
	/// <returns>The const reference to the ImageStorageFormat</returns>
	const ImageStorageFormat&  getFormat() const
	{
		return _format;
	}

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

	bool isTransient()const { return _isTransient; }

protected:
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
};


/// <summary>Base texture view class.</summary>
class TextureView_
{
	friend class ::pvr::api::impl::DescriptorSet_;
	friend class ::pvr::api::impl::Sampler_;
public:
	/// <summary>Get the dimensionality (1D/2D/3D) of this texture.</summary>
	/// <returns>The dimensionality of this texture</returns>
	types::ImageViewType getViewType()const
	{
		return viewtype;
	}

	/// <summary>Destructor (virtual)</summary>
	virtual ~TextureView_() {}

	/// <summary>INTERNAL. Use context->createTextureView or utils::textureUpload</summary>
	TextureView_(const TextureStore& texture, const native::HImageView_& view);

	/// <summary>INTERNAL. Use context->createTextureView or utils::textureUpload</summary>
	TextureView_(const TextureStore& texture = TextureStore());

	/// <summary>Get the underlying TextureStore object.</summary>
	/// <returns>The underlying TextureStore object.</returns>
	const TextureStore& getResource() const
	{
		return resource;
	}
	/// <summary>Get the underlying TextureStore object.</summary>
	/// <returns>The underlying TextureStore object.</returns>
	TextureStore& getResource()
	{
		return resource;
	}

	/// <summary>Query if this object contains a valid reference to an actual Texture.</summary>
	/// <returns>true if the texture is allocated (has an underlying image object), false otherwise.</returns>
	bool isAllocated()
	{
		return ((resource.isValid() && resource->isAllocated()) != 0);
	}

	/// <summary>Get the context that owns this object</summary>
	/// <returns>The context which owns this object</returns>
	IGraphicsContext& getContext()
	{
		return getResource()->getContext();
	}

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
};
}//namespace impl
}//namespace api
}//namespace pvr
