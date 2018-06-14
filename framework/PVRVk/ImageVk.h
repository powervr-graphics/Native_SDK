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
/// <summary>Image creation descriptor.</summary>
struct ImageCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	ImageCreateInfo()
		: _flags(ImageCreateFlags::e_NONE), _imageType(ImageType::e_2D), _extent(Extent3D()), _numMipLevels(1), _numArrayLayers(1), _numSamples(SampleCountFlags::e_1_BIT),
		  _format(Format::e_UNDEFINED), _sharingMode(SharingMode::e_EXCLUSIVE), _usageFlags(ImageUsageFlags::e_NONE), _initialLayout(ImageLayout::e_UNDEFINED),
		  _tiling(ImageTiling::e_OPTIMAL), _numQueueFamilyIndices(0), _queueFamilyIndices(nullptr)
	{}

	/// <summary>Constructor</summary>
	/// <param name="imageType">Image creation type (ImageType)</param>
	/// <param name="format">Image creation format (Format)</param>
	/// <param name="extent">Image creation extent (Extent3D)</param>
	/// <param name="usage">Image creation usage (ImageUsageFlags)</param>
	/// <param name="flags">Image creation flags (ImageCreateFlags)</param>
	/// <param name="numMipLevels">The number of Image mip map levels</param>
	/// <param name="numArrayLayers">The number of Image array layers</param>
	/// <param name="samples">The number of Image samples (SampleCountFlags)</param>
	/// <param name="tiling">The Image tiling mode (ImageTiling)</param>
	/// <param name="sharingMode">The Image sharing mode (SharingMode)</param>
	/// <param name="initialLayout">The initial layout of the Image (ImageLayout)</param>
	/// <param name="queueFamilyIndices">A pointer to a list of queue family indices specifying the list of queue families which can make use of the image</param>
	/// <param name="numQueueFamilyIndices">The number of queue family indices pointed to by queueFamilyIndices</param>
	ImageCreateInfo(ImageType imageType, pvrvk::Format format, const pvrvk::Extent3D& extent, pvrvk::ImageUsageFlags usage,
		pvrvk::ImageCreateFlags flags = pvrvk::ImageCreateFlags::e_NONE, uint32_t numMipLevels = 1, uint32_t numArrayLayers = 1,
		pvrvk::SampleCountFlags samples = pvrvk::SampleCountFlags::e_1_BIT, ImageTiling tiling = ImageTiling::e_OPTIMAL, SharingMode sharingMode = SharingMode::e_EXCLUSIVE,
		ImageLayout initialLayout = ImageLayout::e_UNDEFINED, const uint32_t* queueFamilyIndices = nullptr, uint32_t numQueueFamilyIndices = 0)
		: _flags(flags), _imageType(imageType), _extent(extent), _numMipLevels(numMipLevels), _numArrayLayers(numArrayLayers), _numSamples(samples), _format(format),
		  _sharingMode(sharingMode), _usageFlags(usage), _initialLayout(initialLayout), _tiling(tiling), _numQueueFamilyIndices(numQueueFamilyIndices),
		  _queueFamilyIndices(queueFamilyIndices)
	{}

	/// <summary>Get Image creation Flags</summary>
	/// <returns>Image creation flags (ImageCreateFlags)</returns>
	inline ImageCreateFlags getFlags() const
	{
		return _flags;
	}
	/// <summary>Set PVRVk Image creation flags</summary>
	/// <param name="flags">Flags to use for creating a PVRVk image</param>
	inline void setFlags(ImageCreateFlags flags)
	{
		this->_flags = flags;
	}

	/// <summary>Get Image creation type</summary>
	/// <returns>Image creation type (ImageType)</returns>
	inline ImageType getImageType() const
	{
		return _imageType;
	}
	/// <summary>Set PVRVk Image creation image type</summary>
	/// <param name="imageType">ImageType to use for creating a PVRVk image</param>
	inline void setImageType(ImageType imageType)
	{
		this->_imageType = imageType;
	}

	/// <summary>Get Extent</summary>
	/// <returns>Extent</returns>
	inline Extent3D getExtent() const
	{
		return _extent;
	}
	/// <summary>Set PVRVk Image creation image extents</summary>
	/// <param name="extent">extent to use for creating a PVRVk image</param>
	inline void setExtent(Extent3D extent)
	{
		this->_extent = extent;
	}

	/// <summary>Get the number of Image mip map levels</summary>
	/// <returns>Image mip map levels</returns>
	inline uint32_t getNumMipLevels() const
	{
		return _numMipLevels;
	}
	/// <summary>Set the number of mipmap levels for PVRVk Image creation</summary>
	/// <param name="numMipLevels">The number of mipmap levels to use for creating a PVRVk image</param>
	inline void setNumMipLevels(uint32_t numMipLevels)
	{
		this->_numMipLevels = numMipLevels;
	}

	/// <summary>Get the number of Image array layers</summary>
	/// <returns>Image array layers</returns>
	inline uint32_t getNumArrayLayers() const
	{
		return _numArrayLayers;
	}
	/// <summary>Set the number of array layers for PVRVk Image creation</summary>
	/// <param name="numArrayLayers">The number of array layers to use for creating a PVRVk image</param>
	inline void setNumArrayLayers(uint32_t numArrayLayers)
	{
		this->_numArrayLayers = numArrayLayers;
	}

	/// <summary>Get the number of samples taken for the Image</summary>
	/// <returns>Image number of samples (SampleCountFlags)</returns>
	inline SampleCountFlags getNumSamples() const
	{
		return _numSamples;
	}
	/// <summary>Set the number of samples for PVRVk Image creation</summary>
	/// <param name="numSamples">The number of samples to use for creating a PVRVk image</param>
	inline void setNumSamples(SampleCountFlags numSamples)
	{
		this->_numSamples = numSamples;
	}

	/// <summary>Get Image format</summary>
	/// <returns>Image format (Format)</returns>
	inline Format getFormat() const
	{
		return _format;
	}
	/// <summary>Set the Image format for PVRVk Image creation</summary>
	/// <param name="format">The image format to use for creating a PVRVk image</param>
	inline void setFormat(Format format)
	{
		this->_format = format;
	}

	/// <summary>Get Image sharing mode</summary>
	/// <returns>Image sharing mode (SharingMode)</returns>
	inline SharingMode getSharingMode() const
	{
		return _sharingMode;
	}
	/// <summary>Set the Image sharing mode for PVRVk Image creation</summary>
	/// <param name="sharingMode">The image sharing mode to use for creating a PVRVk image</param>
	inline void setSharingMode(SharingMode sharingMode)
	{
		this->_sharingMode = sharingMode;
	}

	/// <summary>Get Image usage flags</summary>
	/// <returns>Image usage flags (ImageUsageFlags)</returns>
	inline ImageUsageFlags getUsageFlags() const
	{
		return _usageFlags;
	}
	/// <summary>Set the Image usage flags for PVRVk Image creation</summary>
	/// <param name="usageFlags">The image usage flags to use for creating a PVRVk image</param>
	inline void setUsageFlags(ImageUsageFlags usageFlags)
	{
		this->_usageFlags = usageFlags;
	}

	/// <summary>Get Image initial layout</summary>
	/// <returns>Image initial layout (ImageLayout)</returns>
	inline ImageLayout getInitialLayout() const
	{
		return _initialLayout;
	}
	/// <summary>Set the Image initial layout for PVRVk Image creation</summary>
	/// <param name="initialLayout">The image initial layout to use for creating a PVRVk image</param>
	inline void setInitialLayout(ImageLayout initialLayout)
	{
		this->_initialLayout = initialLayout;
	}

	/// <summary>Get Image tiling mode</summary>
	/// <returns>Image initial tiling mode (ImageTiling)</returns>
	inline ImageTiling getTiling() const
	{
		return _tiling;
	}
	/// <summary>Set the Image tiling for PVRVk Image creation</summary>
	/// <param name="tiling">The image tiling to use for creating a PVRVk image</param>
	inline void setTiling(ImageTiling tiling)
	{
		this->_tiling = tiling;
	}

	/// <summary>Get the number of queue family inidices for the image</summary>
	/// <returns>The number of Image queue families</returns>
	inline uint32_t getNumQueueFamilyIndices() const
	{
		return _numQueueFamilyIndices;
	}
	/// <summary>Set the number of queue family inidices specifying the number of queue families which can use the PVRVk Image</summary>
	/// <param name="numQueueFamilyIndices">The number of queue family inidices specifying the number of queue families which can use the PVRVk Image</param>
	inline void setNumQueueFamilyIndices(uint32_t numQueueFamilyIndices)
	{
		this->_numQueueFamilyIndices = numQueueFamilyIndices;
	}
	/// <summary>Get a pointer to a list of queue family inidices for the image</summary>
	/// <returns>A pointer to the list of Image queue families</returns>
	inline const uint32_t* getQueueFamilyIndices() const
	{
		return _queueFamilyIndices;
	}

private:
	/// <summary>Flags to use for creating the image</summary>
	ImageCreateFlags _flags;
	/// <summary>The type of the image (1D, 2D, 3D)</summary>
	ImageType _imageType;
	/// <summary>The extent of the image</summary>
	Extent3D _extent;
	/// <summary>The number of mipmap levels</summary>
	uint32_t _numMipLevels;
	/// <summary>The number of array layers</summary>
	uint32_t _numArrayLayers;
	/// <summary>The number of samples to use</summary>
	SampleCountFlags _numSamples;
	/// <summary>The image format</summary>
	Format _format;
	/// <summary>Specifies the image sharing mode specifying how the image can be used by multiple queue families</summary>
	SharingMode _sharingMode;
	/// <summary>describes the images' intended usage</summary>
	ImageUsageFlags _usageFlags;
	/// <summary>Specifies the initial layout of all image subresources of the image</summary>
	ImageLayout _initialLayout;
	/// <summary>Specifies the tiling arrangement of the data elements in memory</summary>
	ImageTiling _tiling;
	/// <summary>The number of queue families in the _queueFamilyIndices array</summary>
	uint32_t _numQueueFamilyIndices;
	/// <summary>The list of queue families that will access this image</summary>
	const uint32_t* _queueFamilyIndices;
};

namespace impl {
/// <summary>ImageVk implementation that wraps the Vulkan Texture object</summary>
class Image_ : public DeviceObjectHandle<VkImage>, public DeviceObjectDebugMarker<Image_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(Image_)

	/// <summary>Return a reference to the creation descriptor of the image</summary>
	/// <returns>The reference to the ImageCreateInfo</returns>
	pvrvk::ImageCreateInfo getCreateInfo() const
	{
		return _createInfo;
	}

	/// <summary>Get Image creation Flags</summary>
	/// <returns>Image creation flags (ImageCreateFlags)</returns>
	inline ImageCreateFlags getFlags() const
	{
		return _createInfo.getFlags();
	}

	/// <summary>Get Image creation type</summary>
	/// <returns>Image creation type (ImageType)</returns>
	inline ImageType getImageType() const
	{
		return _createInfo.getImageType();
	}

	/// <summary>Get Extent</summary>
	/// <returns>Extent</returns>
	inline Extent3D getExtent() const
	{
		return _createInfo.getExtent();
	}

	/// <summary>Get the number of Image mip map levels</summary>
	/// <returns>Image mip map levels</returns>
	inline uint32_t getNumMipLevels() const
	{
		return _createInfo.getNumMipLevels();
	}

	/// <summary>Get the number of Image array layers</summary>
	/// <returns>Image array layers</returns>
	inline uint32_t getNumArrayLayers() const
	{
		return _createInfo.getNumArrayLayers();
	}

	/// <summary>Get the number of samples taken for the Image</summary>
	/// <returns>Image number of samples (SampleCountFlags)</returns>
	inline SampleCountFlags getNumSamples() const
	{
		return _createInfo.getNumSamples();
	}

	/// <summary>Get Image format</summary>
	/// <returns>Image format (Format)</returns>
	inline Format getFormat() const
	{
		return _createInfo.getFormat();
	}

	/// <summary>Get Image sharing mode</summary>
	/// <returns>Image sharing mode (SharingMode)</returns>
	inline SharingMode getSharingMode() const
	{
		return _createInfo.getSharingMode();
	}

	/// <summary>Get Image usage flags</summary>
	/// <returns>Image usage flags (ImageUsageFlags)</returns>
	inline ImageUsageFlags getUsageFlags() const
	{
		return _createInfo.getUsageFlags();
	}

	/// <summary>Get Image initial layout</summary>
	/// <returns>Image initial layout (ImageLayout)</returns>
	inline ImageLayout getInitialLayout() const
	{
		return _createInfo.getInitialLayout();
	}

	/// <summary>Get Image tiling mode</summary>
	/// <returns>Image initial tiling mode (ImageTiling)</returns>
	inline ImageTiling getTiling() const
	{
		return _createInfo.getTiling();
	}

	/// <summary>Get the number of queue family inidices for the image</summary>
	/// <returns>The number of Image queue families</returns>
	inline uint32_t getNumQueueFamilyIndices() const
	{
		return _createInfo.getNumQueueFamilyIndices();
	}

	/// <summary>Get a pointer to a list of queue family inidices for the image</summary>
	/// <returns>A pointer to the list of Image queue families</returns>
	inline const uint32_t* const getQueueFamilyIndices() const
	{
		return _createInfo.getQueueFamilyIndices();
	}

	/// <summary>Check if this texture is a cubemap</summary>
	/// <returns>true if the texture is a cubemap, otherwise false</returns>
	bool isCubeMap() const
	{
		return (_createInfo.getFlags() & pvrvk::ImageCreateFlags::e_CUBE_COMPATIBLE_BIT) == pvrvk::ImageCreateFlags::e_CUBE_COMPATIBLE_BIT;
	}

	/// <summary>Check if this texture is allocated.</summary>
	/// <returns>true if the texture is allocated. Otherwise, the texture is empty and must be constructed.</returns>
	bool isAllocated() const
	{
		return (getVkHandle() != VK_NULL_HANDLE);
	}

	/// <summary>Get the width of this texture (number of columns of texels in the lowest mipmap)</summary>
	/// <returns>Texture width</returns>
	uint16_t getWidth() const
	{
		return static_cast<uint16_t>(_createInfo.getExtent().getWidth());
	}

	/// <summary>Get the height of this texture (number of rows of texels in the lowest mipmap)</summary>
	/// <returns>Texture height</returns>
	uint16_t getHeight() const
	{
		return static_cast<uint16_t>(_createInfo.getExtent().getHeight());
	}

	/// <summary>Get the depth of this texture (number of (non-array) layers of texels in the lowest mipmap)
	/// </summary>
	/// <returns>Texture depth</returns>
	uint16_t getDepth() const
	{
		return static_cast<uint16_t>(_createInfo.getExtent().getDepth());
	}

	/// <summary>Bind memory to this non sparse image.</summary>
	/// <param name="memory">The memory to attach to the given image object</param>
	/// <param name="offset">The offset into the given memory to attach to the given image object</param>
	void bindMemoryNonSparse(DeviceMemory memory, VkDeviceSize offset)
	{
		if ((_createInfo.getFlags() &
				(pvrvk::ImageCreateFlags::e_SPARSE_ALIASED_BIT | pvrvk::ImageCreateFlags::e_SPARSE_BINDING_BIT | pvrvk::ImageCreateFlags::e_SPARSE_RESIDENCY_BIT)) != 0)
		{
			throw ErrorValidationFailedEXT("Cannot bind memory: Image is Sparce so cannot have bound memory.");
		}
		if (_memory.isValid())
		{
			throw ErrorValidationFailedEXT("Cannot bind memory: A memory block is already bound");
		}

		vkThrowIfFailed(
			_device->getVkBindings().vkBindImageMemory(_device->getVkHandle(), getVkHandle(), memory->getVkHandle(), offset), "Failed to bind a memory block to this image");
		_memory = memory;
	}

	/// <summary>Get the memory requirements of the image</summary>
	/// <returns>The memory requirements of the image (MemoryRequirements)</returns>
	const pvrvk::MemoryRequirements& getMemoryRequirement() const
	{
		return _memReqs;
	}

protected:
	/// <summary>Destructor. Will properly release all resources held by this object.</summary>
	virtual ~Image_();

	/// <summary>Constructor.</summary>
	/// <param name="device">The Devices from which to create the image from</param>
	/// <param name="createInfo">The ImageCreateInfo descriptor specifying creation parameters</param>
	Image_(const DeviceWeakPtr& device, const ImageCreateInfo& createInfo);

	/// <summary>Constructor.</summary>
	/// <param name="device">The Devices from which to create the image from</param>
	explicit Image_(const DeviceWeakPtr& device) : DeviceObjectHandle(device), DeviceObjectDebugMarker(pvrvk::DebugReportObjectTypeEXT::e_IMAGE_EXT) {}

	/// <summary>Image specific memory requirements</summary>
	pvrvk::MemoryRequirements _memReqs;
	/// <summary>Device memory bound to this image (Only for non sparse image).</summary>
	DeviceMemory _memory;
	/// <summary>Creation information used when creating the image.</summary>
	pvrvk::ImageCreateInfo _createInfo;

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
};

/// <summary>The Specialized image for swapchain</summary>
class SwapchainImage_ : public Image_
{
public:
	DECLARE_NO_COPY_SEMANTICS(SwapchainImage_)
private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
	friend class ::pvrvk::impl::Swapchain_;
	virtual ~SwapchainImage_();

	SwapchainImage_(const DeviceWeakPtr& device, const VkImage& swapchainImage, const Format& format, const Extent3D& extent, uint32_t numArrayLevels, uint32_t numMipLevels,
		const ImageUsageFlags& usage);
};

/// <summary>ImageView implementation that wraps the Vulkan Texture View object</summary>
class ImageView_ : public DeviceObjectHandle<VkImageView>, public DeviceObjectDebugMarker<ImageView_>
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
	bool isAllocated() const
	{
		return ((_resource.isValid() && _resource->isAllocated()) != 0);
	}

	/// <summary>Get vulkan object</summary>
	/// <returns>pvrvk::ImageView&</returns>
	pvrvk::ImageViewType getViewType() const
	{
		return _viewType;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	ImageView_(const Image& image, pvrvk::ImageViewType viewType, pvrvk::Format format, const ImageSubresourceRange& range, ComponentMapping swizzleChannels);

	~ImageView_();

	pvrvk::ImageViewType _viewType;
	Image _resource; //!< Texture view implementations access the underlying texture through this
};
} // namespace impl
} // namespace pvrvk
