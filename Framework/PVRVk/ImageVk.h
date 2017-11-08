/*!
\brief The PVRVk Image class and related classes (SwapchainImage, ImageView).
\file PVRVk/ImageVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/DeviceMemoryVk.h"

namespace pvrvk {
namespace impl {
/// <summary>TextureStoreVk_ implementation that wraps the Vulkan Texture object</summary>
class Image_
{
public:
	DECLARE_NO_COPY_SEMANTICS(Image_)

	/// <summary>Return a reference to the format of the texture</summary>
	/// <returns>The reference to the ImageStorageFormat</returns>
	VkFormat  getFormat()const { return _format; }

	/// <summary>Set this texture layer(const)</summary>
	/// <returns>layerSize Texture layer size</returns>
	const ImageLayersSize& getLayers() const { return (ImageLayersSize&)_extents; }

	/// <summary>Return number of samples(const)</summary>
	/// <returns>VkSampleCountFlags</returns>
	VkSampleCountFlags getNumSamples()const { return _numSamples; }

	/// <summary>Check if this texture is a cubemap</summary>
	/// <returns>true if the texture is a cubemap, otherwise false</returns>
	bool isCubeMap()const
	{
		return (_createFlags & VkImageCreateFlags::e_CUBE_COMPATIBLE_BIT) == VkImageCreateFlags::e_CUBE_COMPATIBLE_BIT;
	}

	/// <summary>Return true if this is transient image</summary>
	/// <returns>bool</returns>
	bool isTransient()const { return _isTransient; }

	/// <summary>Check if this texture is allocated.</summary>
	/// <returns>true if the texture is allocated. Otherwise, the texture is empty and must be constructed.</returns>
	bool isAllocated() const { return (_vkImage != VK_NULL_HANDLE); }

	/// <summary>Get the width of this texture (number of columns of texels in the lowest mipmap)</summary>
	/// <returns>Texture width</returns>
	uint16_t getWidth() const
	{
		return static_cast<uint16_t>(_extents.width);
	}

	/// <summary>Get the height of this texture (number of rows of texels in the lowest mipmap)</summary>
	/// <returns>Texture height</returns>
	uint16_t getHeight() const
	{
		return static_cast<uint16_t>(_extents.height);
	}

	/// <summary>Get the depth of this texture (number of (non-array) layers of texels in the lowest mipmap)
	/// </summary>
	/// <returns>Texture depth</returns>
	uint16_t getDepth() const
	{
		return static_cast<uint16_t>(_extents.depth);
	}

	/// <summary>Get the number of array layers of this texture</summary>
	/// <returns>The number of array layers of this texture</returns>
	uint16_t getNumArrayLayers() const
	{
		return static_cast<uint16_t>(_extents.numArrayLevels);
	}

	/// <summary>Get the number of MipMap levels of this texture</summary>
	/// <returns>The number of MipMap levels layers of this texture</returns>
	uint16_t getNumMipMapLevels() const
	{
		return static_cast<uint16_t>(_extents.numMipLevels);
	}

	/// <summary>Get the basic dimensioning of the texture (1D/2D/3D).</summary>
	/// <returns>The TextureDimension</returns>
	VkImageType getImageType() const
	{
		return _imageType;
	}

	/// <summary>Get vulkan object(const)</summary>
	/// <returns>const VkImage&</returns>
	const VkImage& getNativeObject()const { return _vkImage; }

	/// <summary>Get the device which owns this resource</summary>
	/// <returns>DeviceWeakPtr</returns>
	DeviceWeakPtr getDevice()const { return _device; }

	/// <summary>Get image create flags (const)</summary>
	/// <returns>VkImageCreateFlags</returns>
	VkImageCreateFlags getCreateFlags()const { return _createFlags; }

	/// <summary>Bind memory this image.</summary>
	/// <param name="memory"></param>
	/// <returns></returns>
	bool bindMemoryNonSparse(DeviceMemory memory)
	{
		if ((getCreateFlags() & (VkImageCreateFlags::e_SPARSE_ALIASED_BIT | VkImageCreateFlags::e_SPARSE_BINDING_BIT |
		                         VkImageCreateFlags::e_SPARSE_RESIDENCY_BIT)) != 0)
		{
			Log("Cannot bind memory to sparse image");
			return false;
		}
		if (_memory.isValid())
		{
			Log("Cannot bind the memory block to this Image. A memory block is already bound");
			return false;
		}

		if (vk::BindImageMemory(_device->getNativeObject(), _vkImage, memory->getNativeObject(), 0) != VkResult::e_SUCCESS)
		{
			Log("Failed to bind a memory block to this image");
			return false;
		}
		_memory = memory;
		return true;
	}
	const VkMemoryRequirements& getMemoryRequirement()const
	{
		return _memReqs;
	}
protected:
	/// <summary>Destructor. Will properly release all resources held by this object.</summary>
	virtual ~Image_();

	/// <summary>Constructor. Use to wrap a preexisting, underlying texture object.</summary>
	/// <param name="context">The GraphicsContext where this Texture will belong</param>
	/// <remarks>NOTE: This object will take ownership of the passed texture object, destroying it in its destructor.
	/// </remarks>
	Image_(const DeviceWeakPtr& context) :
		_vkImage(VK_NULL_HANDLE), _device(context), _format(VkFormat::e_UNDEFINED)
	{
	}

	/// <summary>Constructor (from all enements)</summary>
	/// <param name="device">Device which owns this resource</param>
	/// <param name="image">Vulkan image handle</param>
	/// <param name="imageType">image type</param>
	/// <param name="format">image format</param>
	/// <param name="size">Image size, layers and mipmap levels</param>
	/// <param name="numSamples">Number of samples</param>
	/// <param name="isCubeMap">Is cubemap image</param>
	/// <param name="boundDeviceMemory">Bound device memory(Only for non-sparse image)</param>
	Image_(
	  const DeviceWeakPtr& device, VkImage image, VkImageType imageType,
	  VkFormat format, const ImageAreaSize& size, VkSampleCountFlags numSamples,
	  bool isCubeMap, DeviceMemory boundDeviceMemory) :
		_vkImage(image), _device(device), _format(format),
		_extents(size), _numSamples(numSamples), _memory(boundDeviceMemory), _imageType(imageType), _createFlags(
		  isCubeMap ? VkImageCreateFlags::e_CUBE_COMPATIBLE_BIT : (VkImageCreateFlags)0)
	{
	}

	VkFormat _format;//!< image format
	bool _isTransient;//!< Is this image transient
	VkImageUsageFlags _usage;//!< Image usage flags
	ImageAreaSize _extents;//!< image dimension, array levels and mipmap levels
	VkImageType _imageType;//!< Image type (e.g 1d, 2d, 3d)
	VkSampleCountFlags _numSamples;//!< Number of samples
	VkImage _vkImage; //!< Vulkan image handle
	VkMemoryRequirements _memReqs; //!< Image specific memory requirements
	DeviceMemory _memory;//!< Device memory bound to this image (Only for non sparse image).
	VkImageCreateFlags _createFlags;//!< image create flags

private:

	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	bool init(VkImageType imageType, const ImageAreaSize& size, VkFormat format,
	          VkImageUsageFlags usage, VkImageCreateFlags createFlags, VkSampleCountFlags samples,
	          bool sharingExclusive, const uint32_t* queueFamilyIndices, uint32_t numQueueFamilyIndices);

	DeviceWeakPtr _device;
};

/// <summary>The Specialized image for swapchain</summary>
class SwapchainImage_ : public Image_
{
public:
	DECLARE_NO_COPY_SEMANTICS(SwapchainImage_)
private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
	friend class ::pvrvk::impl::Swapchain_;
	virtual ~SwapchainImage_();
	bool init(const VkImage& swapchainImage, const VkFormat& format,
	          const ImageAreaSize& size, const VkImageUsageFlags& usage);

	SwapchainImage_(const DeviceWeakPtr& context) : Image_(context)
	{}
};

/// <summary>TextureViewVk_ implementation that wraps the Vulkan Texture View object</summary>
class ImageView_
{
public:
	DECLARE_NO_COPY_SEMANTICS(ImageView_)

	/// <summary>Get the underlying TextureStore object.</summary>
	/// <returns>The underlying TextureStore object.</returns>
	const Image& getImage() const
	{
		return _resource;
	}
	/// <summary>Get the underlying TextureStore object.</summary>
	/// <returns>The underlying TextureStore object.</returns>
	Image& getImage()
	{
		return _resource;
	}

	/// <summary>Query if this object contains a valid reference to an actual Texture.</summary>
	/// <returns>true if the texture is allocated (has an underlying image object), false otherwise.</returns>
	bool isAllocated()const
	{
		return ((_resource.isValid() && _resource->isAllocated()) != 0);
	}

	/// <summary>Get vulkan object(const)</summary>
	/// <returns>const VkImageView&</returns>
	const VkImageView& getNativeObject()const { return _vkImageView; }

	/// <summary>Get vulkan object</summary>
	/// <returns>VkImageView&</returns>
	VkImageViewType getViewType()const { return _viewType; }

private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	bool init(const Image& image, VkImageViewType viewType, VkFormat format,
	          const ImageSubresourceRange& range, ComponentMapping swizzleChannels);

	ImageView_() : _viewType(VkImageViewType::e_MAX_ENUM), _vkImageView(VK_NULL_HANDLE) {}

	/// <summary>dtor</summary>
	~ImageView_() { destroy(); }

	/// <summary>Destroy this textrue view object</summary>
	void destroy();

	VkImageViewType   _viewType;
	Image           _resource;//!<Texture view implementations access the underlying texture through this
	VkImageView     _vkImageView;
};
}// namespace impl
}// namespace pvrvk
