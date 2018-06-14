/*!
\brief Function definitions for PVRVk image class.
\file PVRVk/ImageVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "PVRVk/ImageVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/QueueVk.h"
#include "PVRVk/CommandPoolVk.h"
namespace pvrvk {
namespace impl {
Image_::~Image_()
{
	if (isAllocated())
	{
		if (_device.isValid())
		{
			if (getVkHandle() != VK_NULL_HANDLE)
			{
				_device->getVkBindings().vkDestroyImage(_device->getVkHandle(), getVkHandle(), nullptr);
				_vkHandle = VK_NULL_HANDLE;
			}
			// DUE TO SHARED POINTERS, NO VIEWS EXIST IF THIS IS CALLED.
		}
		else
		{
			Log(LogLevel::Warning, "Texture object was not released up before context destruction");
		}
	}
}

Image_::Image_(const DeviceWeakPtr& device, const ImageCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_IMAGE_EXT), _createInfo(createInfo)
{
	VkImageCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_IMAGE_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkImageCreateFlags>(_createInfo.getFlags());
	vkCreateInfo.imageType = static_cast<VkImageType>(_createInfo.getImageType());
	vkCreateInfo.extent.width = _createInfo.getExtent().getWidth();
	vkCreateInfo.extent.height = _createInfo.getExtent().getHeight();
	vkCreateInfo.extent.depth = _createInfo.getExtent().getDepth();
	vkCreateInfo.mipLevels = _createInfo.getNumMipLevels();
	vkCreateInfo.arrayLayers = _createInfo.getNumArrayLayers();
	vkCreateInfo.samples = static_cast<VkSampleCountFlagBits>(_createInfo.getNumSamples());
	vkCreateInfo.format = static_cast<VkFormat>(_createInfo.getFormat());
	vkCreateInfo.sharingMode = static_cast<VkSharingMode>(_createInfo.getSharingMode());
	vkCreateInfo.tiling = static_cast<VkImageTiling>(_createInfo.getTiling());
	vkCreateInfo.usage = static_cast<VkImageUsageFlags>(_createInfo.getUsageFlags());
	vkCreateInfo.queueFamilyIndexCount = _createInfo.getNumQueueFamilyIndices();
	vkCreateInfo.pQueueFamilyIndices = _createInfo.getQueueFamilyIndices();
	vkCreateInfo.initialLayout = static_cast<VkImageLayout>(_createInfo.getInitialLayout());

#ifdef DEBUG
	_device->getPhysicalDevice()->getImageFormatProperties(static_cast<pvrvk::Format>(_createInfo.getFormat()), static_cast<pvrvk::ImageType>(_createInfo.getImageType()),
		static_cast<pvrvk::ImageTiling>(_createInfo.getTiling()), static_cast<pvrvk::ImageUsageFlags>(_createInfo.getUsageFlags()),
		static_cast<pvrvk::ImageCreateFlags>(_createInfo.getFlags()));
#endif

	impl::vkThrowIfFailed(device->getVkBindings().vkCreateImage(device->getVkHandle(), &vkCreateInfo, NULL, &_vkHandle), "ImageVk createImage");

	device->getVkBindings().vkGetImageMemoryRequirements(device->getVkHandle(), _vkHandle, reinterpret_cast<VkMemoryRequirements*>(&_memReqs));
}
} // namespace impl

namespace impl {
SwapchainImage_::~SwapchainImage_()
{
	if (isAllocated())
	{
		if (!getDevice().isValid())
		{
			Log(LogLevel::Warning, "Texture object was not released up before context destruction");
		}
	}
	_vkHandle = VK_NULL_HANDLE;
}

SwapchainImage_::SwapchainImage_(const DeviceWeakPtr& device, const VkImage& swapchainImage, const Format& format, const Extent3D& extent, uint32_t numArrayLevels,
	uint32_t numMipLevels, const ImageUsageFlags& usage)
	: Image_(device)
{
	_vkHandle = swapchainImage;

	_createInfo = pvrvk::ImageCreateInfo();
	_createInfo.setImageType(ImageType::e_2D);
	_createInfo.setFormat(format);
	_createInfo.setExtent(extent);
	_createInfo.setNumArrayLayers(numArrayLevels);
	_createInfo.setNumMipLevels(numMipLevels);
	_createInfo.setUsageFlags(usage);

	_memReqs = {};
}
} // namespace impl

namespace impl {
ImageView_::ImageView_(const Image& image, ImageViewType viewType, Format format, const ImageSubresourceRange& range, ComponentMapping swizzleChannels)
	: DeviceObjectHandle(), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_IMAGE_VIEW_EXT), _viewType(ImageViewType::e_MAX_ENUM)
{
	_device = image->getDevice();
	_resource = image;
	_viewType = viewType;
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_IMAGE_VIEW_CREATE_INFO);
	viewCreateInfo.image = image->getVkHandle();
	viewCreateInfo.viewType = static_cast<VkImageViewType>(viewType);

	viewCreateInfo.format = static_cast<VkFormat>(format);
	viewCreateInfo.components = *reinterpret_cast<VkComponentMapping*>(&swizzleChannels);
	viewCreateInfo.subresourceRange.aspectMask = static_cast<VkImageAspectFlags>(range.getAspectMask());
	viewCreateInfo.subresourceRange.baseMipLevel = range.getBaseMipLevel();
	viewCreateInfo.subresourceRange.levelCount = range.getLevelCount();
	viewCreateInfo.subresourceRange.baseArrayLayer = range.getBaseArrayLayer();
	viewCreateInfo.subresourceRange.layerCount = range.getLayerCount();
	vkThrowIfFailed(_device->getVkBindings().vkCreateImageView(image->getDevice()->getVkHandle(), &viewCreateInfo, nullptr, &_vkHandle), "Failed to create ImageView");
}
ImageView_::~ImageView_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroyImageView(_device->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterDevice("ImageView");
		}
	}
}

} // namespace impl
} // namespace pvrvk

//!\endcond
