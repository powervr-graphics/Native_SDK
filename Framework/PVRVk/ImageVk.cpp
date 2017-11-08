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

// HELPER FUNCTIONS - Shortcuts: Create/destroy/begin/end/submit command buffers etc
namespace {
struct ImageCreateInfo
{
	VkImageType                 imageType;//< 1D, 2D, 3D
	VkExtent3D                  extent;//< 1D, 2D, 3D
	uint32_t                    numMipLevels;
	uint32_t                    numArrayLayers;
	VkSampleCountFlags          numSamples;
	VkFormat                    format;
	bool                        mutableFormat;
	bool                        sharingExclusive;
	VkImageUsageFlags           imageUsageFlags;//< Use sparse bits only for sparse image, Cube Compatiblitiy only for 2d Image
	VkImageCreateFlags          imageCreateFlags;
};

inline void destroyBufferAndMemory(VkDevice device, VkBuffer& buffer, VkDeviceMemory& memory)
{
	vk::FreeMemory(device, memory, NULL);
	vk::DestroyBuffer(device, buffer, NULL);
	buffer = VK_NULL_HANDLE;
	memory = VK_NULL_HANDLE;
}

inline void destroyImageAndMemory(VkDevice device, VkImage& image, VkDeviceMemory& memory)
{
	vk::FreeMemory(device, memory, NULL);
	vk::DestroyImage(device, image, NULL);
	image = VK_NULL_HANDLE;
	memory = VK_NULL_HANDLE;
}

inline bool isCompressedFormat(VkFormat fmt)
{
	return ((fmt >= VkFormat::e_BC1_RGB_UNORM_BLOCK && fmt <= VkFormat::e_ASTC_12x12_SRGB_BLOCK) ||
	        (fmt >= VkFormat::e_PVRTC1_2BPP_UNORM_BLOCK_IMG && fmt <= VkFormat::e_PVRTC2_4BPP_SRGB_BLOCK_IMG));
}

inline void beginCommandBuffer(VkCommandBuffer& cmdBuffer)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VkStructureType::e_COMMAND_BUFFER_BEGIN_INFO;
	vk::BeginCommandBuffer(cmdBuffer, &beginInfo);
}
inline void endCommandBuffer(VkCommandBuffer& cmdBuffer)
{
	vk::EndCommandBuffer(cmdBuffer);
}

inline VkFence submitCommandBuffer(
  VkDevice device, VkQueue queue, VkCommandBuffer cbuff,
  VkSemaphore waitSema, VkSemaphore signalSema)
{
	VkSubmitInfo submit = {};
	submit.sType = VkStructureType::e_SUBMIT_INFO;
	submit.pCommandBuffers = &cbuff;
	submit.commandBufferCount = 1;
	submit.pWaitSemaphores = &waitSema;
	submit.waitSemaphoreCount = (waitSema != VK_NULL_HANDLE);
	submit.pSignalSemaphores = &signalSema;
	submit.signalSemaphoreCount = (signalSema != VK_NULL_HANDLE);

	VkFence fence;
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VkStructureType::e_FENCE_CREATE_INFO;
	impl::vkThrowIfFailed(vk::CreateFence(device, &fenceInfo, NULL, &fence), "Failed to create fence");

	vk::QueueSubmit(queue, 1, &submit, fence);
	return fence;
}

inline static VkCommandBuffer allocateCommandBuffer(VkDevice device, VkCommandPool pool)
{
	VkCommandBuffer cbuff;
	VkCommandBufferAllocateInfo cmdAlloc = {};
	cmdAlloc.sType = VkStructureType::e_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAlloc.level = VkCommandBufferLevel::e_PRIMARY;
	cmdAlloc.commandBufferCount = 1;
	cmdAlloc.commandPool = pool;
	VkResult res = vk::AllocateCommandBuffers(device, &cmdAlloc, &cbuff);
	return res == VkResult::e_SUCCESS ? cbuff : VK_NULL_HANDLE;
}

inline static void freeCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer cbuff)
{
	vk::FreeCommandBuffers(device, pool, 1, &cbuff);
}

inline static void waitAndDestroyFence(VkDevice dev, VkFence fence)
{
	vk::WaitForFences(dev, 1, &fence, true, std::numeric_limits<uint64_t>::max());
	vk::DestroyFence(dev, fence, NULL);
}

inline void submitWaitAndDestroy(
  VkDevice device, VkQueue transferQueue, VkQueue otherQueue,
  VkCommandPool transferOpPool, VkCommandPool ownershipPool,
  VkCommandBuffer cbuffTransfer, VkCommandBuffer cbuffTakeOwnership,
  VkCommandBuffer cbuffReturnOwnership)
{
	if (cbuffTakeOwnership == VK_NULL_HANDLE)
	{
		waitAndDestroyFence(device, submitCommandBuffer(device, transferQueue, cbuffTransfer, VK_NULL_HANDLE, VK_NULL_HANDLE));
		freeCommandBuffer(device, transferOpPool, cbuffTransfer);
	}
	else
	{
		VkSemaphore sema1 = VK_NULL_HANDLE;
		VkSemaphore sema2 = VK_NULL_HANDLE;
		static const VkSemaphoreCreateInfo nfo = { VkStructureType::e_SEMAPHORE_CREATE_INFO, 0, 0 };
		vk::CreateSemaphore(device, &nfo, NULL, &sema1);
		vk::CreateSemaphore(device, &nfo, NULL, &sema2);

		VkPipelineStageFlags stage_mask_all_graphics = VkPipelineStageFlags::e_ALL_GRAPHICS_BIT;
		VkPipelineStageFlags stage_mask_transfer = VkPipelineStageFlags::e_TRANSFER_BIT;

		VkSubmitInfo submit_take_ownership = {};
		submit_take_ownership.sType = VkStructureType::e_SUBMIT_INFO;
		submit_take_ownership.pCommandBuffers = &cbuffTakeOwnership;
		submit_take_ownership.commandBufferCount = 1;
		submit_take_ownership.pWaitSemaphores = nullptr;
		submit_take_ownership.waitSemaphoreCount = 0;
		submit_take_ownership.pSignalSemaphores = &sema1;
		submit_take_ownership.signalSemaphoreCount = 1;
		submit_take_ownership.pWaitDstStageMask = nullptr;

		VkSubmitInfo submit_xfer = {};

		submit_xfer.sType = VkStructureType::e_SUBMIT_INFO;
		submit_xfer.pCommandBuffers = &cbuffTransfer;
		submit_xfer.commandBufferCount = 1;
		submit_xfer.pWaitSemaphores = &sema1;
		submit_xfer.waitSemaphoreCount = 1;
		submit_xfer.pSignalSemaphores = &sema2;
		submit_xfer.signalSemaphoreCount = 1;
		submit_xfer.pWaitDstStageMask = &stage_mask_all_graphics;

		VkSubmitInfo submit_release_ownership = {};
		submit_release_ownership.sType = VkStructureType::e_SUBMIT_INFO;
		submit_release_ownership.pCommandBuffers = &cbuffReturnOwnership;
		submit_release_ownership.commandBufferCount = 1;
		submit_release_ownership.pWaitSemaphores = &sema2;
		submit_release_ownership.waitSemaphoreCount = 1;
		submit_release_ownership.pSignalSemaphores = nullptr;
		submit_release_ownership.signalSemaphoreCount = 0;
		submit_release_ownership.pWaitDstStageMask = &stage_mask_transfer;

		VkFence fence;
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VkStructureType::e_FENCE_CREATE_INFO;
		impl::vkThrowIfFailed(vk::CreateFence(device, &fenceInfo, nullptr, &fence), "Failed to create fence");

		vk::QueueSubmit(otherQueue, 1, &submit_take_ownership, VK_NULL_HANDLE);
		vk::QueueSubmit(transferQueue, 1, &submit_xfer, VK_NULL_HANDLE);
		vk::QueueSubmit(otherQueue, 1, &submit_release_ownership, fence);
		waitAndDestroyFence(device, fence);
		freeCommandBuffer(device, transferOpPool, cbuffTransfer);
		freeCommandBuffer(device, ownershipPool, cbuffTakeOwnership);
		freeCommandBuffer(device, ownershipPool, cbuffReturnOwnership);
	}

}

inline VkImageAspectFlags inferAspectFromUsageAndFormat(VkFormat format, VkImageUsageFlags imageUsageFlags)
{
	VkImageAspectFlags imageAspect = ((imageUsageFlags & VkImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT) != 0 ?
	                                  (VkImageAspectFlags)0 : VkImageAspectFlags::e_COLOR_BIT);

	if ((imageAspect != 0) && format >= VkFormat::e_D16_UNORM && format <= VkFormat::e_D32_SFLOAT_S8_UINT)
	{
		const VkImageAspectFlags aspects[] =
		{
			VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT,  //  VkFormat::e_D32_SFLOAT_S8_UINT
			VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT,  //  VkFormat::e_D24_UNORM_S8_UINT
			VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT,  //  VkFormat::e_D16_UNORM_S8_UINT
			VkImageAspectFlags::e_STENCIL_BIT,          //  VkFormat::e_S8_UINT
			VkImageAspectFlags::e_DEPTH_BIT,          //  VkFormat::e_D32_SFLOAT
			VkImageAspectFlags::e_DEPTH_BIT,          //  VkFormat::e_X8_D24_UNORM_PACK32
			VkImageAspectFlags::e_DEPTH_BIT,          //  VkFormat::e_D16_UNORM
		};
		// (Depthstenil format end) - format
		imageAspect = aspects[(int)VkFormat::e_D32_SFLOAT_S8_UINT - (int)format];
	}
	return imageAspect;
}
}

inline bool createImage(VkDevice device, const ImageCreateInfo& createparam,
                        const uint32_t* queueFamilyIndices, uint32_t numQueueFamilyIndices,
                        VkImage& outImage, VkMemoryRequirements* outMemReq)
{
	VkImageCreateInfo nfo = {};
	nfo.sType = VkStructureType::e_IMAGE_CREATE_INFO;
	nfo.flags = createparam.imageCreateFlags;
	nfo.imageType = createparam.imageType;
	nfo.extent.width = createparam.extent.width;
	nfo.extent.height = createparam.extent.height;
	nfo.extent.depth = createparam.extent.depth;
	nfo.mipLevels = createparam.numMipLevels;
	nfo.arrayLayers = createparam.numArrayLayers;
	nfo.samples = createparam.numSamples;
	nfo.format = createparam.format;
	nfo.sharingMode = createparam.sharingExclusive ? VkSharingMode::e_EXCLUSIVE :
	                  VkSharingMode::e_CONCURRENT;
	nfo.tiling = VkImageTiling::e_OPTIMAL;
	nfo.usage = createparam.imageUsageFlags;
	nfo.queueFamilyIndexCount = numQueueFamilyIndices;
	nfo.pQueueFamilyIndices = queueFamilyIndices;
	nfo.initialLayout = VkImageLayout::e_UNDEFINED;

	impl::vkThrowIfFailed(vk::CreateImage(device, &nfo, NULL, &outImage),
	                      "TextureUtils:TextureUpload createImage");
	if (outMemReq)
	{
		vk::GetImageMemoryRequirements(device, outImage, outMemReq);
	}
	return true;
}

void ImageView_::destroy()
{
	if (_vkImageView != VK_NULL_HANDLE)
	{
		DeviceWeakPtr device = _resource->getDevice();
		if (device.isValid())
		{
			vk::DestroyImageView(device->getNativeObject(), _vkImageView, nullptr);
			_vkImageView = VK_NULL_HANDLE;
			device.reset();
		}
		else
		{
			reportDestroyedAfterContext("ImageView");
		}
	}
}

}//namespace impl}

namespace impl {
Image_::~Image_()
{
	if (isAllocated())
	{
		if (_device.isValid())
		{
			if (_vkImage != VK_NULL_HANDLE)
			{
				vk::DestroyImage(_device->getNativeObject(), _vkImage, nullptr);
				_vkImage = VK_NULL_HANDLE;
			}
			//DUE TO SHARED POINTERS, NO VIEWS EXIST IF THIS IS CALLED.
		}
		else
		{
			Log(LogLevel::Warning, "Texture object was not released up before context destruction");
		}
	}
}

bool Image_::init(VkImageType imageType,  const ImageAreaSize& size, VkFormat format,
                  VkImageUsageFlags usage, VkImageCreateFlags createFlags,
                  VkSampleCountFlags samples, bool sharingExclusive,
                  const uint32_t* queueFamilyIndices, uint32_t numQueueFamilyIndices)
{
	_imageType = imageType;
	_format = format;
	_extents = size;
	_numSamples = samples;
	_usage = usage;
	_createFlags = createFlags;

	VkFormat vkFormat = format;
	if (vkFormat == VkFormat::e_UNDEFINED)
	{
		Log("Undefined Image VkFormat");
		return false;
	}

	impl::ImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.imageType = imageType;
	imageCreateInfo.format = vkFormat;
	imageCreateInfo.extent = VkExtent3D{size.width, size.height, size.depth};
	imageCreateInfo.imageCreateFlags = createFlags;
	imageCreateInfo.imageUsageFlags = usage;
	imageCreateInfo.numArrayLayers = size.numArrayLevels * (isCubeMap() ? 6 : 1);
	imageCreateInfo.numMipLevels = size.numMipLevels;
	imageCreateInfo.numSamples = samples;
	imageCreateInfo.sharingExclusive = sharingExclusive;
	return impl::createImage(_device->getNativeObject(), imageCreateInfo, queueFamilyIndices,
	                         numQueueFamilyIndices, _vkImage, &_memReqs);
}
}

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
	_vkImage = VK_NULL_HANDLE;
}

bool SwapchainImage_::init(const VkImage& swapchainImage, const VkFormat& format,
                           const ImageAreaSize& size, const VkImageUsageFlags& usage)
{
	_vkImage = swapchainImage;

	_imageType = VkImageType::e_2D;
	_format = format;
	_extents = size;
	_usage = usage;
	_isTransient = false;
	_numSamples = VkSampleCountFlags::e_1_BIT;
	_memReqs = {};
	_memory = DeviceMemory();
	_createFlags = VkImageCreateFlags(0);

	VkFormat vkFormat = format;
	if (vkFormat == VkFormat::e_UNDEFINED)
	{
		Log("Undefined Image VkFormat");
		return false;
	}

	return true;
}
}// namespace impl

namespace impl {
bool ImageView_::init(const Image& image, VkImageViewType viewType, VkFormat format,
                      const ImageSubresourceRange& range, ComponentMapping swizzleChannels)
{
	_resource = image;
	_viewType = viewType;
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VkStructureType::e_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image->getNativeObject();
	viewCreateInfo.viewType = viewType;

	viewCreateInfo.format = format;
	viewCreateInfo.components = *((VkComponentMapping*)&swizzleChannels);
	viewCreateInfo.subresourceRange.aspectMask = range.aspectMask;
	viewCreateInfo.subresourceRange.baseMipLevel = range.baseMipLevel;
	viewCreateInfo.subresourceRange.levelCount = range.levelCount;
	viewCreateInfo.subresourceRange.baseArrayLayer = range.baseArrayLayer;
	viewCreateInfo.subresourceRange.layerCount = range.layerCount;
	if (vk::CreateImageView(image->getDevice()->getNativeObject(),
	                        &viewCreateInfo, nullptr, &_vkImageView) != VkResult::e_SUCCESS)
	{
		Log("Failed to create ImageView");
		return false;
	}
	return true;
}
}// namespace vulkan
}// namespace pvrvk

//!\endcond