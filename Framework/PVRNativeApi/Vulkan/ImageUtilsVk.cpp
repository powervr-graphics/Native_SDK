/*!
\brief Implementations of functions for creating Vulkan Image objects.
\file PVRNativeApi/Vulkan/ImageUtilsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRNativeApi/Vulkan/ImageUtilsVk.h"
#include "PVRCore/PixelFormat.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/PlatformContext.h"
#include "PVRNativeApi/Vulkan/PlatformHandlesVulkanGlue.h"
#include "PVRNativeApi/Vulkan/BufferUtilsVk.h"
#include "PVRNativeApi/Vulkan/VkErrors.h"
#include "PVRCore/Texture/PVRTDecompress.h"

namespace pvr {
namespace utils {

// HELPER FUNCTIONS - Shortcuts: Create/destroy/begin/end/submit command buffers etc
namespace {
// helper functions

inline void destroyBufferAndMemory(VkDevice device, pvr::native::HBuffer_& buffer)
{
	vk::FreeMemory(device, buffer.memory, NULL);
	vk::DestroyBuffer(device, buffer.buffer, NULL);
	buffer.buffer = VK_NULL_HANDLE;
	buffer.memory = VK_NULL_HANDLE;
}

inline void destroyImageAndMemory(VkDevice device, pvr::native::HTexture_& texture)
{
	vk::FreeMemory(device, texture.memory, NULL);
	vk::DestroyImage(device, texture.image, NULL);
	texture.image = VK_NULL_HANDLE;
	texture.memory = VK_NULL_HANDLE;
}

inline bool isCompressedFormat(VkFormat fmt)
{
	return (fmt == VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK_IMG_BETA || fmt == VK_FORMAT_RGBA_PVRTC1_4BPP_BLOCK_IMG_BETA ||
	        (fmt >= VK_FORMAT_BC1_RGB_UNORM_BLOCK && fmt <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK) ||
	        (fmt >= VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG && fmt <= VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG));
}


//VkImage createImage(VkDevice device,
//                    const types::Extent3D& dimension, uint32 arrayLayer,
//                    VkSampleCountFlagBits sampleCount, uint32 numMipLevels, bool isCubeMap,
//                    VkImageType imageType, VkFormat format, VkImageUsageFlags imageUsageFlags)
//{
//	uint32 totalArrayLayers = arrayLayer * (isCubeMap ? 6 : 1);
//
//	VkImageCreateInfo nfo = {};
//	nfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//	nfo.pNext = NULL;
//	nfo.flags = pvr::uint32(isCubeMap) * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
//	nfo.imageType = imageType;
//	nfo.extent.width = dimension.width;
//	nfo.extent.height = dimension.height;
//	nfo.extent.depth = dimension.depth;
//	nfo.mipLevels = numMipLevels;
//	nfo.arrayLayers = totalArrayLayers;
//	nfo.samples = sampleCount;
//	nfo.format = format;
//	nfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//	nfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//	nfo.usage = imageUsageFlags;
//	nfo.queueFamilyIndexCount = 1;
//	uint32_t queueFamilyZero = 0;
//	nfo.pQueueFamilyIndices = &queueFamilyZero;
//	nfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
//
//	//Create the final image
//	VkImage image;
//	if (vk::CreateImage(device, &nfo, NULL, &image) != VK_SUCCESS)
//	{
//		image = VK_NULL_HANDLE;
//	}
//	return image;
//}
//
//template<typename ImageIterator, typename OutOffsetIterator>
//void createMemoryBlockForImagesAndBind(VkDevice device, ImageIterator begin, ImageIterator end,
//                                       OutOffsetIterator beginOffset, OutOffsetIterator endOffset, VkMemoryPropertyFlagBits allocMemProperty,
//                                       VkDeviceMemory& outMemoryBlock, VkDeviceSize& outSize)
//{
//	outMemoryBlock = VK_NULL_HANDLE;
//	outSize = 0;
//	VkMemoryRequirements memReq = {};
//	VkMemoryRequirements memReqTmp = {};
//
//	if (begin == end || endOffset - beginSize < end - begin)
//	{
//		assertion(0); return;
//	}
//	for (; begin != end; ++begin, ++beginOffset)
//	{
//		vk::GetImageMemoryRequirements(device, begin, &memReqTmp);
//		if (memReq.memoryTypeBits == 0) // find the first allowed type
//		{
//			pvr::Log("Failed to get buffer memory requirements: memory requirements are 0");
//			return false;
//		}
//		memReq.memoryTypeBits != memReqTmp.memoryTypeBits;
//
//		*beginOffset = math::roundAwayFromZero(memReq.size, memReqTmp.alignment); // Get the offset of the current image
//
//		memReq.alignment = math::lcm_with_max(memReq.alignment, memReqTmp.alignment); // Get the alignment (so far)
//
//		memReq.size = *beginOffset + memReqTmp.size; // We make sure we take into consideration any alignment added here
//	}
//	outMemoryBlock = allocateMemory(device, allocMemProperty, memReq, allocMemProperty);
//	if (outMemory == VK_NULL_HANDLE)
//	{
//		pvr::Log("Failed to allocate Image memory");
//		return false;
//	}
//	vk::BindImageMemory(device, image, outMemory, 0);
//	return true;
//}

inline void beginCommandBuffer(VkCommandBuffer& cmdBuffer)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vk::BeginCommandBuffer(cmdBuffer, &beginInfo);
}
inline void endCommandBuffer(VkCommandBuffer& cmdBuffer)
{
	vk::EndCommandBuffer(cmdBuffer);
}

inline VkFence submitCommandBuffer(VkDevice device, VkQueue queue, VkCommandBuffer cbuff, VkSemaphore waitSema, VkSemaphore signalSema)
{
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pCommandBuffers = &cbuff;
	submit.commandBufferCount = 1;
	submit.pWaitSemaphores = &waitSema;
	submit.waitSemaphoreCount = (waitSema != VK_NULL_HANDLE);
	submit.pSignalSemaphores = &signalSema;
	submit.signalSemaphoreCount = (signalSema != VK_NULL_HANDLE);

	VkFence fence;
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	nativeVk::vkThrowIfFailed(vk::CreateFence(device, &fenceInfo, NULL, &fence), "Failed to create fence");

	vk::QueueSubmit(queue, 1, &submit, fence);
	return fence;
}

inline static VkCommandBuffer allocateCommandBuffer(VkDevice device, VkCommandPool pool)
{
	VkCommandBuffer cbuff;
	VkCommandBufferAllocateInfo cmdAlloc = {};
	cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdAlloc.commandBufferCount = 1;
	cmdAlloc.commandPool = pool;
	VkResult res = vk::AllocateCommandBuffers(device, &cmdAlloc, &cbuff);
	return res == VK_SUCCESS ? cbuff : VK_NULL_HANDLE;
}

inline static void freeCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer cbuff)
{
	vk::FreeCommandBuffers(device, pool, 1, &cbuff);
}

inline static void waitAndDestroyFence(VkDevice dev, VkFence fence)
{
	vk::WaitForFences(dev, 1, &fence, true, std::numeric_limits<uint64>::max());
	vk::DestroyFence(dev, fence, NULL);
}

inline void submitWaitAndDestroy(VkDevice device, VkQueue transferQueue, VkQueue otherQueue,
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
		static const VkSemaphoreCreateInfo nfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
		vk::CreateSemaphore(device, &nfo, NULL, &sema1);
		vk::CreateSemaphore(device, &nfo, NULL, &sema2);

		VkPipelineStageFlags stage_mask_all_graphics = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		VkPipelineStageFlags stage_mask_transfer = VK_PIPELINE_STAGE_TRANSFER_BIT;

		VkSubmitInfo submit_take_ownership = {};
		submit_take_ownership.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_take_ownership.pCommandBuffers = &cbuffTakeOwnership;
		submit_take_ownership.commandBufferCount = 1;
		submit_take_ownership.pWaitSemaphores = NULL;
		submit_take_ownership.waitSemaphoreCount = 0;
		submit_take_ownership.pSignalSemaphores = &sema1;
		submit_take_ownership.signalSemaphoreCount = 1;
		submit_take_ownership.pWaitDstStageMask = NULL;



		VkSubmitInfo submit_xfer = {};

		submit_xfer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_xfer.pCommandBuffers = &cbuffTransfer;
		submit_xfer.commandBufferCount = 1;
		submit_xfer.pWaitSemaphores = &sema1;
		submit_xfer.waitSemaphoreCount = 1;
		submit_xfer.pSignalSemaphores = &sema2;
		submit_xfer.signalSemaphoreCount = 1;
		submit_xfer.pWaitDstStageMask = &stage_mask_all_graphics;


		VkSubmitInfo submit_release_ownership = {};
		submit_release_ownership.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_release_ownership.pCommandBuffers = &cbuffReturnOwnership;
		submit_release_ownership.commandBufferCount = 1;
		submit_release_ownership.pWaitSemaphores = &sema2;
		submit_release_ownership.waitSemaphoreCount = 1;
		submit_release_ownership.pSignalSemaphores = NULL;
		submit_release_ownership.signalSemaphoreCount = 0;
		submit_release_ownership.pWaitDstStageMask = &stage_mask_transfer;


		VkFence fence;
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		nativeVk::vkThrowIfFailed(vk::CreateFence(device, &fenceInfo, NULL, &fence), "Failed to create fence");

		vk::QueueSubmit(otherQueue, 1, &submit_take_ownership, VK_NULL_HANDLE);
		vk::QueueSubmit(transferQueue, 1, &submit_xfer, VK_NULL_HANDLE);
		vk::QueueSubmit(otherQueue, 1, &submit_release_ownership, fence);
		waitAndDestroyFence(device, fence);
		freeCommandBuffer(device, transferOpPool, cbuffTransfer);
		freeCommandBuffer(device, ownershipPool, cbuffTakeOwnership);
		freeCommandBuffer(device, ownershipPool, cbuffReturnOwnership);
	}

}

inline void submitAndGetFence(VkDevice device, VkQueue transferQueue, VkQueue ownershipQueue,
                              VkCommandPool transferOpPool, VkCommandPool ownershipPool,
                              VkCommandBuffer cbuffTransfer, VkCommandBuffer cbuffTakeOwnership,
                              VkCommandBuffer cbuffReturnOwnership, utils::vulkan::TextureUploadAsyncResultsData_& results)
{
	if (cbuffTakeOwnership == VK_NULL_HANDLE)
	{
		results.fence = submitCommandBuffer(device, transferQueue, cbuffTransfer, VK_NULL_HANDLE, VK_NULL_HANDLE);
		results.xferCmd = cbuffTransfer;
		results.xferPool = transferOpPool;
	}
	else
	{
		VkSemaphore sema1 = VK_NULL_HANDLE;
		VkSemaphore sema2 = VK_NULL_HANDLE;
		static const VkSemaphoreCreateInfo nfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
		vk::CreateSemaphore(device, &nfo, NULL, &sema1);
		vk::CreateSemaphore(device, &nfo, NULL, &sema2);

		VkPipelineStageFlags stage_mask_all_graphics = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		VkPipelineStageFlags stage_mask_transfer = VK_PIPELINE_STAGE_TRANSFER_BIT;

		VkSubmitInfo submit_take_ownership = {};
		submit_take_ownership.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_take_ownership.pCommandBuffers = &cbuffTakeOwnership;
		submit_take_ownership.commandBufferCount = 1;
		submit_take_ownership.pWaitSemaphores = NULL;
		submit_take_ownership.waitSemaphoreCount = 0;
		submit_take_ownership.pSignalSemaphores = &sema1;
		submit_take_ownership.signalSemaphoreCount = 1;
		submit_take_ownership.pWaitDstStageMask = NULL;



		VkSubmitInfo submit_xfer = {};

		submit_xfer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_xfer.pCommandBuffers = &cbuffTransfer;
		submit_xfer.commandBufferCount = 1;
		submit_xfer.pWaitSemaphores = &sema1;
		submit_xfer.waitSemaphoreCount = 1;
		submit_xfer.pSignalSemaphores = &sema2;
		submit_xfer.signalSemaphoreCount = 1;
		submit_xfer.pWaitDstStageMask = &stage_mask_all_graphics;


		VkSubmitInfo submit_release_ownership = {};
		submit_release_ownership.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_release_ownership.pCommandBuffers = &cbuffReturnOwnership;
		submit_release_ownership.commandBufferCount = 1;
		submit_release_ownership.pWaitSemaphores = &sema2;
		submit_release_ownership.waitSemaphoreCount = 1;
		submit_release_ownership.pSignalSemaphores = NULL;
		submit_release_ownership.signalSemaphoreCount = 0;
		submit_release_ownership.pWaitDstStageMask = &stage_mask_transfer;


		VkFence fence;
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		nativeVk::vkThrowIfFailed(vk::CreateFence(device, &fenceInfo, NULL, &fence), "Failed to create fence");

		vk::QueueSubmit(ownershipQueue, 1, &submit_take_ownership, VK_NULL_HANDLE);
		vk::QueueSubmit(transferQueue, 1, &submit_xfer, VK_NULL_HANDLE);
		vk::QueueSubmit(ownershipQueue, 1, &submit_release_ownership, fence);

		results.fence = fence;
		results.xferCmd = cbuffTransfer;
		results.xferPool = transferOpPool;
		results.ownCmd[0] = cbuffTakeOwnership;
		results.ownCmd[1] = cbuffReturnOwnership;
		results.ownPool = ownershipPool;
	}

}


VkImageAspectFlags inferAspectFromUsageAndFormat(VkFormat format, VkImageUsageFlags imageUsageFlags)
{
	VkImageAspectFlags imageAspect = (imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? 0 : VK_IMAGE_ASPECT_COLOR_BIT);

	if (!imageAspect && format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		const VkImageAspectFlags aspects[] =
		{
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,  //  VK_FORMAT_D32_SFLOAT_S8_UINT
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,  //  VK_FORMAT_D24_UNORM_S8_UINT
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,  //  VK_FORMAT_D16_UNORM_S8_UINT
			VK_IMAGE_ASPECT_STENCIL_BIT,          //  VK_FORMAT_S8_UINT
			VK_IMAGE_ASPECT_DEPTH_BIT,          //  VK_FORMAT_D32_SFLOAT
			VK_IMAGE_ASPECT_DEPTH_BIT,          //  VK_FORMAT_X8_D24_UNORM_PACK32
			VK_IMAGE_ASPECT_DEPTH_BIT,          //  VK_FORMAT_D16_UNORM
		};
		// (Depthstenil format end) - format
		imageAspect = aspects[VK_FORMAT_D32_SFLOAT_S8_UINT - format];
	}
	return imageAspect;
}


bool getMemoryTypeIndex(VkPhysicalDeviceMemoryProperties& deviceMemProps,
                        uint32 typeBits, VkMemoryPropertyFlagBits properties,
                        uint32& outTypeIndex)
{
	for (;;)
	{
		uint32 typeBitsTmp = typeBits;
		for (uint32 i = 0; i < 32; ++i)
		{
			if ((typeBitsTmp & 1) == 1)
			{
				if (VkMemoryPropertyFlagBits(deviceMemProps.memoryTypes[i].propertyFlags & properties) == properties)
				{
					outTypeIndex = i;
					return true;
				}
			}
			typeBitsTmp >>= 1;
		}
		if (properties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
		{
			properties = VkMemoryPropertyFlagBits(properties & ~VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
			continue;
		}
		if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			properties = VkMemoryPropertyFlagBits(properties & ~VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			continue;
		}
		if (properties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
		{
			properties = VkMemoryPropertyFlagBits(properties & ~VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
			continue;
		}
		else
		{
			break;
		}
	}
	return false;
}

VkDeviceMemory allocateMemory(VkDevice device, VkPhysicalDeviceMemoryProperties& deviceMemProps,
                              const VkMemoryRequirements& memoryRequirements,
                              VkMemoryPropertyFlagBits allocMemProperty)
{
	VkDeviceMemory memory;
	VkMemoryAllocateInfo sMemoryAllocInfo;
	sMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	sMemoryAllocInfo.pNext = NULL;
	sMemoryAllocInfo.allocationSize = memoryRequirements.size;
	pvr::assertion(getMemoryTypeIndex(deviceMemProps, memoryRequirements.memoryTypeBits,
	                                  allocMemProperty, sMemoryAllocInfo.memoryTypeIndex));
	vk::AllocateMemory(device, &sMemoryAllocInfo, NULL, &memory);
	return memory;
}

bool createImage(VkDevice device, VkCommandBuffer cbuff,
                 const types::Extent3D& dimension, uint32 arrayLayer,
                 VkSampleCountFlagBits sampleCount, uint32 numMipLevels, bool isCubeMap,
                 VkImageType imageType, VkFormat format, VkImageUsageFlags imageUsageFlags,
                 VkImage& outimage)
{
	uint32 totalArrayLayers = arrayLayer * (isCubeMap ? 6 : 1);

	VkImageCreateInfo nfo = {};
	nfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	nfo.pNext = NULL;
	nfo.flags = pvr::uint32(isCubeMap) * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	nfo.imageType = imageType;
	nfo.extent.width = dimension.width;
	nfo.extent.height = dimension.height;
	nfo.extent.depth = dimension.depth;
	nfo.mipLevels = numMipLevels;
	nfo.arrayLayers = totalArrayLayers;
	nfo.samples = sampleCount;
	nfo.format = format;
	nfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	nfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	nfo.usage = imageUsageFlags;
	nfo.queueFamilyIndexCount = 1;
	uint32_t queueFamilyZero = 0;
	nfo.pQueueFamilyIndices = &queueFamilyZero;
	nfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	//Create the final image
	nativeVk::vkThrowIfFailed(vk::CreateImage(device, &nfo, NULL, &outimage), "TextureUtils:TextureUpload createImage");
	return true;
}



}

void decompressPvrtc(const Texture& texture, Texture& cDecompressedTexture)
{
	//Set up the new texture and header.
	TextureHeader cDecompressedHeader(texture);
	// robin: not sure what should happen here. The PVRTGENPIXELID4 macro is used in the old SDK.
	cDecompressedHeader.setPixelFormat(GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);

	cDecompressedHeader.setChannelType(VariableType::UnsignedByteNorm);
	cDecompressedTexture = Texture(cDecompressedHeader);

	//Do decompression, one surface at a time.
	for (uint32 uiMIPLevel = 0; uiMIPLevel < texture.getNumberOfMIPLevels(); ++uiMIPLevel)
	{
		for (uint32 uiArray = 0; uiArray < texture.getNumberOfArrayMembers(); ++uiArray)
		{
			for (uint32 uiFace = 0; uiFace < texture.getNumberOfFaces(); ++uiFace)
			{
				PVRTDecompressPVRTC(texture.getDataPointer(uiMIPLevel, uiArray, uiFace),
				                    (texture.getBitsPerPixel() == 2 ? 1 : 0),
				                    texture.getWidth(uiMIPLevel), texture.getHeight(uiMIPLevel),
				                    cDecompressedTexture.getDataPointer(uiMIPLevel, uiArray, uiFace));
			}
		}
	}
}


namespace vulkan {

VkImageAspectFlags inferAspectFromFormat(VkFormat format)
{
	VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;

	if (format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		const VkImageAspectFlags aspects[] =
		{
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,  //  VK_FORMAT_D32_SFLOAT_S8_UINT
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,  //  VK_FORMAT_D24_UNORM_S8_UINT
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,  //  VK_FORMAT_D16_UNORM_S8_UINT
			VK_IMAGE_ASPECT_STENCIL_BIT,          //  VK_FORMAT_S8_UINT
			VK_IMAGE_ASPECT_DEPTH_BIT,          //  VK_FORMAT_D32_SFLOAT
			VK_IMAGE_ASPECT_DEPTH_BIT,          //  VK_FORMAT_X8_D24_UNORM_PACK32
			VK_IMAGE_ASPECT_DEPTH_BIT,          //  VK_FORMAT_D16_UNORM
		};
		// (Depthstenil format end) - format
		imageAspect = aspects[VK_FORMAT_D32_SFLOAT_S8_UINT - format];
	}
	return imageAspect;
}

VkImageAspectFlags inferAspectFromFormat(PixelFormat format)
{
	return inferAspectFromFormat(nativeVk::ConvertToVk::pixelFormat(format));
}


bool allocateImageDeviceMemory(VkDevice device, VkPhysicalDeviceMemoryProperties& memprops,
                               VkMemoryPropertyFlagBits allocMemProperty,
                               VkImage& image, VkDeviceMemory& outMemory,
                               VkMemoryRequirements* outMemRequirements)
{
	VkMemoryRequirements memReq;
	VkMemoryRequirements* memReqPtr = &memReq;
	if (outMemRequirements)
	{
		memReqPtr = outMemRequirements;
	}
	vk::GetImageMemoryRequirements(device, image, memReqPtr);
	if (memReqPtr->memoryTypeBits == 0) // find the first allowed type
	{
		pvr::Log("Failed to get buffer memory requirements: memory requirements are 0");
		return false;
	}
	outMemory = allocateMemory(device, memprops, *memReqPtr, allocMemProperty);
	if (outMemory == VK_NULL_HANDLE)
	{
		pvr::Log("Failed to allocate Image memory");
		return false;
	}
	vk::BindImageMemory(device, image, outMemory, 0);
	return true;
}

inline VkAccessFlags getAccesFlagsFromLayout(VkImageLayout layout)
{
	switch (layout)
	{
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return VK_ACCESS_TRANSFER_WRITE_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_ACCESS_TRANSFER_READ_BIT;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_SHADER_READ_BIT;
	case VK_IMAGE_LAYOUT_PREINITIALIZED: return VK_ACCESS_HOST_WRITE_BIT;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return VK_ACCESS_MEMORY_READ_BIT;
	default: return 0;
	}

}

inline static bool areQueueFamiliesSameOrInvalid(uint32 lhs, uint32 rhs)
{
	debug_assertion((lhs != -1 && rhs != -1) || (lhs == rhs), "ImageUtilsVK(areQueueFamiliesSameOrInvalid): Only one queue family was valid. "
	                "Either both must be valid, or both must be ignored (-1)"); // Don't pass one non-null only...
	return lhs == rhs || lhs == -1 || rhs == -1;
}
inline static bool isMultiQueue(uint32 queueFamilySrc, uint32 queueFamilyDst)
{
	return !areQueueFamiliesSameOrInvalid(queueFamilySrc, queueFamilyDst);
}

// TESTS IF THE OPERATIONS ARE ACTUALLY REQUIRED - NOPS IF BOTH LAYOUTS AND QUEUES ARE SAME
// If any of the command buffers is not null, it use that. If both command buffers are not null,
// but the queue families are the same, it is an error.
void setImageLayoutAndQueueOwnership(VkCommandBuffer srccmd, VkCommandBuffer dstcmd,
                                     uint32 srcQueueFamily, uint32 dstQueueFamily,
                                     VkImageLayout oldLayout, VkImageLayout newLayout,
                                     VkImage image, uint32 baseMipLevel, uint32 numMipLevels,
                                     uint32 baseArrayLayer, uint32 numArrayLayers,
                                     VkImageAspectFlags aspect)
{
	bool multiQueue = isMultiQueue(srcQueueFamily, dstQueueFamily);

	// No operation required: We don't have a layout transition, and we don't have a queue family change.
	if (newLayout == oldLayout && !multiQueue)
	{
		return;
	} // No transition required

	if (multiQueue)
	{
		assertion(srccmd != NULL && dstcmd != NULL,
		          "Vulkan Utils setImageLayoutAndQueueOwnership: An ownership change was required, "
		          "but at least one null command buffers was passed as parameters");
	}
	else
	{
		assertion(
		  srccmd == VK_NULL_HANDLE || dstcmd == VK_NULL_HANDLE,
		  "Vulkan Utils setImageLayoutAndQueueOwnership: An ownership change was not required, "
		  "but two non-null command buffers were passed as parameters");
	}

	VkImageMemoryBarrier imageMemBarrier = {};
	imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.pNext = NULL;
	imageMemBarrier.srcAccessMask = 0;
	imageMemBarrier.dstAccessMask = 0;
	imageMemBarrier.oldLayout = oldLayout;
	imageMemBarrier.newLayout = newLayout;
	imageMemBarrier.image = image;
	imageMemBarrier.subresourceRange =
	  VkImageSubresourceRange{ aspect, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers };
	imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;


	imageMemBarrier.srcAccessMask = getAccesFlagsFromLayout(oldLayout);
	imageMemBarrier.dstAccessMask = getAccesFlagsFromLayout(newLayout);

	// WORKAROUND - MAKE THE LAYERS SHUT UP (due to spec bug)
#if 1
	{
		if (multiQueue && newLayout != oldLayout)
		{
			vk::CmdPipelineBarrier(srccmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
			                       NULL, 1, &imageMemBarrier);
			imageMemBarrier.oldLayout = imageMemBarrier.newLayout;
			imageMemBarrier.srcAccessMask = getAccesFlagsFromLayout(imageMemBarrier.oldLayout);
		}
	}
#endif

	if (multiQueue)
	{
		imageMemBarrier.srcQueueFamilyIndex = srcQueueFamily;
		imageMemBarrier.dstQueueFamilyIndex = dstQueueFamily;
	}

	//Support any one of the command buffers being NOT null - either first or second is fine.
	if (srccmd)
	{
		vk::CmdPipelineBarrier(srccmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
		                       NULL, 1, &imageMemBarrier);
	}
	if (dstcmd)
	{
		vk::CmdPipelineBarrier(dstcmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
		                       NULL, 1, &imageMemBarrier);
	}
}

CleanupObject updateImageDeferred(VkDevice device,
                                  VkCommandBuffer cbuffTransfer,
                                  VkCommandBuffer cbuffTakeOwn,
                                  VkCommandBuffer cbuffRelinquishOwn,
                                  uint32 srcQueueFamily, uint32 dstQueueFamily,
                                  const VkPhysicalDeviceMemoryProperties& memprops,
                                  ImageUpdateParam* updateParams, uint32 numUpdateParams,
                                  VkFormat format, VkImageLayout layout,
                                  bool isCubeMap, VkImage image)
{
	TextureUpdateCleanupObject_& res = *new TextureUpdateCleanupObject_(device);
	CleanupObject retval(&res);
	uint32 numFace = (isCubeMap ? 6 : 1);

	uint32 hwSlice;

	std::vector<native::HBuffer_> stagingBuffers;
	struct BufferIteratorAdapter
	{
		std::vector<native::HBuffer_>::iterator myit;
	public:
		BufferIteratorAdapter(std::vector<native::HBuffer_>::iterator it): myit(it) {}
		VkBuffer& operator*() { return myit->buffer; }
		VkBuffer* operator->() { return &myit->buffer; }
		BufferIteratorAdapter& operator++() { ++myit; return *this; }
		BufferIteratorAdapter operator++(int)
		{
			BufferIteratorAdapter ret(*this); ++(*this); return  ret;
		}
		bool operator!=(const BufferIteratorAdapter& rhs)
		{
			return myit != rhs.myit;
		}
		bool operator==(const BufferIteratorAdapter& rhs)
		{
			return myit == rhs.myit;
		}
	};

	struct MemoryIteratorAdapter
	{
		std::vector<native::HBuffer_>::iterator myit;
	public:
		MemoryIteratorAdapter(std::vector<native::HBuffer_>::iterator it): myit(it) {}
		VkDeviceMemory& operator*() { return myit->memory; }
		VkDeviceMemory* operator->() { return &myit->memory; }
		MemoryIteratorAdapter& operator++() { ++myit; return *this; }
		MemoryIteratorAdapter operator++(int)
		{
			MemoryIteratorAdapter ret(*this); ++(*this); return  ret;
		}
		bool operator!=(const MemoryIteratorAdapter& rhs)
		{
			return myit != rhs.myit;
		}
		bool operator==(const MemoryIteratorAdapter& rhs)
		{
			return myit == rhs.myit;
		}
	};

	{
		stagingBuffers.resize(numUpdateParams);
		VkBufferImageCopy imgcp = {};
		VkResult res;

		for (uint32 i = 0; i < numUpdateParams; ++i)
		{

			const ImageUpdateParam& mipLevelUpdate = updateParams[i];
			assertion(mipLevelUpdate.data && mipLevelUpdate.dataSize, "Data and Data size must be valid");

			hwSlice = mipLevelUpdate.arrayIndex * numFace + mipLevelUpdate.cubeFace;

			// Will write the switch layout commands from the universal queue to the transfer queue to both the
			// transfer command buffer and the universal command buffer
			setImageLayoutAndQueueOwnership(
			  cbuffTakeOwn, cbuffTransfer, srcQueueFamily, dstQueueFamily,
			  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			  image, mipLevelUpdate.mipLevel, 1, hwSlice, 1, inferAspectFromFormat(format));

			native::HBuffer_& buffer = stagingBuffers[i];
			VkMemoryRequirements memReqSrc = {};


			// create the staging buffer
			if (!createBufferAndMemory(device, memprops, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			                           types::BufferBindingUse::TransferSrc, mipLevelUpdate.dataSize, buffer, &memReqSrc))
			{
				Log("Failed to create staging buffer for mip level %d slice %d", mipLevelUpdate.mipLevel, hwSlice);
				return retval;
			}

			imgcp.imageOffset = VkOffset3D{ mipLevelUpdate.offsetX, mipLevelUpdate.offsetY, mipLevelUpdate.offsetZ };
			imgcp.imageExtent = VkExtent3D{ mipLevelUpdate.width, mipLevelUpdate.height, 1 };
			imgcp.imageSubresource.aspectMask = inferAspectFromFormat(format);
			imgcp.imageSubresource.baseArrayLayer = hwSlice;
			imgcp.imageSubresource.layerCount = 1;
			imgcp.imageSubresource.mipLevel = updateParams[i].mipLevel;
			imgcp.bufferRowLength = mipLevelUpdate.width;
			imgcp.bufferImageHeight = mipLevelUpdate.height;


			const uint8* srcData;
			uint32 srcDataSize;
			srcData = (uint8*)mipLevelUpdate.data;
			srcDataSize = mipLevelUpdate.dataSize;
			uint8* mappedData = NULL;
			res = vk::MapMemory(device, buffer.memory, 0, memReqSrc.size, 0, (void**)&mappedData);
			if (res != VK_SUCCESS)
			{
				nativeVk::vkThrowIfFailed(res, "ImageUtils:updateImage Linear staging buffer Map Memory Failed");
				Log("ImageUtils:updateImage Linear staging buffer Map Memory Failed");
				vk::DestroyBuffer(device, buffer.buffer, NULL);
				vk::FreeMemory(device, buffer.memory, NULL);
				return retval;
			}
			for (uint32_t slice3d = 0; !slice3d || (slice3d < mipLevelUpdate.depth); ++slice3d)
			{
				memcpy(mappedData, srcData, srcDataSize);
				mappedData += srcDataSize;
				srcData += srcDataSize;
			}
			vk::UnmapMemory(device, buffer.memory);
			vk::CmdCopyBufferToImage(cbuffTransfer, buffer.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgcp);

			// CAUTION: We swapped src and dst queue families as, if there was no ownership transfer, no problem - queue families
			// will be ignored.
			// Will write the switch layout commands from the transfer queue to the universal queue to both the
			// transfer command buffer and the universal command buffer
			setImageLayoutAndQueueOwnership(
			  cbuffTransfer, cbuffRelinquishOwn, dstQueueFamily, srcQueueFamily,
			  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layout, image, mipLevelUpdate.mipLevel, 1,
			  hwSlice, 1, inferAspectFromFormat(format));
		}
	}

	res.addBuffers(BufferIteratorAdapter(stagingBuffers.begin()), BufferIteratorAdapter(stagingBuffers.end()));
	res.addMemories(MemoryIteratorAdapter(stagingBuffers.begin()), MemoryIteratorAdapter(stagingBuffers.end()));
	return retval;
}

void updateImage(IPlatformContext& ctx, ImageUpdateParam* updateParams, uint32 numUpdateParams,
                 uint32 numArraySlice, VkFormat format, bool isCubeMap, VkImage image,
                 VkImageLayout currentLayout)
{
	auto& handles = ctx.getNativePlatformHandles();
	const VkDevice& device = handles.context.device;
	const VkPhysicalDeviceMemoryProperties& memprops = handles.deviceMemProperties;

	VkCommandBuffer cbuff_give_ownership = VK_NULL_HANDLE;
	VkCommandBuffer cbuff_transfer_operation = VK_NULL_HANDLE;
	VkCommandBuffer cbuff_return_ownership = VK_NULL_HANDLE;

	beginCommandBuffer(cbuff_transfer_operation);
	cbuff_transfer_operation = allocateCommandBuffer(device, handles.universalCommandPool);
	beginCommandBuffer(cbuff_transfer_operation);

	auto res = updateImageDeferred(handles.context.device, cbuff_transfer_operation, cbuff_give_ownership,
	                               cbuff_return_ownership, handles.universalQueueFamily, -1,
	                               memprops, updateParams, numUpdateParams,
	                               format, currentLayout,
	                               isCubeMap, image);

	endCommandBuffer(cbuff_transfer_operation);

	submitWaitAndDestroy(device, handles.mainQueue(), handles.mainQueue(),
	                     handles.universalCommandPool, handles.universalCommandPool,
	                     cbuff_transfer_operation, cbuff_give_ownership, cbuff_return_ownership);
}

bool createImageAndMemory(platform::NativePlatformHandles_& handles,
  const types::Extent3D &dimension, uint32 arrayLayer,
  VkSampleCountFlagBits sampleCount, uint32 numMipLevels, bool isCubeMap,
  VkImageType imageType, VkFormat format, VkImageUsageFlags imageUsageFlags,
  VkMemoryPropertyFlagBits memPropertyFlags, native::HTexture_& outTexture)
{
	return createImageAndMemory(
	         handles.context.device,
	         handles.deviceMemProperties,
	         dimension, arrayLayer,
	         sampleCount, numMipLevels, isCubeMap,
	         imageType, format, imageUsageFlags,
	         memPropertyFlags,
	         outTexture.image, outTexture.memory);
}

bool createImageAndMemory(VkDevice device,  VkPhysicalDeviceMemoryProperties memprops,
                          const types::Extent3D &dimension, uint32 arrayLayer,
                          VkSampleCountFlagBits sampleCount, uint32 numMipLevels, bool isCubeMap,
                          VkImageType imageType, VkFormat format, VkImageUsageFlags imageUsageFlags,
                          VkMemoryPropertyFlagBits memPropertyFlags, VkImage& outimage,
                          VkDeviceMemory& outmemory)
{
	uint32 totalArrayLayers = arrayLayer * (isCubeMap ? 6 : 1);

	VkImageCreateInfo nfo = {};
	nfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	nfo.pNext = NULL;
	nfo.flags = pvr::uint32(isCubeMap) * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	nfo.imageType = imageType;
	nfo.extent.width = dimension.width;
	nfo.extent.height = dimension.height;
	nfo.extent.depth = dimension.depth;
	nfo.mipLevels = numMipLevels;
	nfo.arrayLayers = totalArrayLayers;
	nfo.samples = sampleCount;
	nfo.format = format;
	nfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	nfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	nfo.usage = imageUsageFlags;
	nfo.queueFamilyIndexCount = 0;
	nfo.pQueueFamilyIndices = 0;
	nfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (memPropertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT &&
	    memPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		pvr::Log("Invalid memory property flags. Cannot allocate memory with VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT and "
		         "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT");
		return false;
	}

	if (memPropertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT && !(imageUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT))
	{
		pvr::Log("Invalid combination memory property flag and usage. A memory type with VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT"
		         " flag set is only allowed to be bound to a image whose usage flags include VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT.");
		return false;
	}

	//Create the final image
	nativeVk::vkThrowIfFailed(vk::CreateImage(device, &nfo, NULL, &outimage),
	                          "TextureUtils:TextureUpload createImage");

	VkMemoryRequirements memReqDst;
	vulkan::allocateImageDeviceMemory(device, memprops, memPropertyFlags, outimage, outmemory, &memReqDst);
	return true;
}


const Texture* decompressIfRequired(const Texture& texture, Texture& decompressedTexture,
                                    bool allowDecompress, bool supportPvrtc, bool supportPvrtc2,
                                    TextureUploadResultsData_& results)
{
	const Texture* textureToUse = &texture;
	// Setup code to get various state
	// Generic error strings for textures being unsupported.
	const char8* cszUnsupportedFormat =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation.\n";
	const char8* cszUnsupportedFormatDecompressionAvailable =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation."
	  " Allowing software decompression (allowDecompress=true) will enable you to use this format.\n";

	// Check that extension support exists for formats supported in this way.

	// Check format not supportedfor formats only supported by extensions.
	switch (texture.getPixelFormat().getPixelTypeId())
	{
	case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGB:
	case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGBA:
	case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGB:
	case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGBA:
	{
		bool decompress = !supportPvrtc;
		if (decompress)
		{
			if (allowDecompress)
			{
				Log(Log.Information, "PVRTC texture format support not detected. Decompressing PVRTC to"
				    " corresponding format (RGBA32 or RGB24)");
				decompressPvrtc(texture, decompressedTexture);
				textureToUse = &decompressedTexture;
				results.decompressed = true;
			}
			else
			{
				Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "PVRTC1");
				results.result = Result::UnsupportedRequest;
				return NULL;
			}
		}
		break;
	}
	case (uint64)CompressedPixelFormat::PVRTCII_2bpp:
	case (uint64)CompressedPixelFormat::PVRTCII_4bpp:
	{
		//useTexStorage = false;
		if (!supportPvrtc2)
		{
			Log(Log.Error, cszUnsupportedFormat, "PVRTC2");
			results.result = Result::UnsupportedRequest;
			return NULL;
		}
		break;
	}
	case (uint64)CompressedPixelFormat::ETC1:
	{
		//useTexStorage = false;
		if (!supportPvrtc2)
		{
			Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "ETC1");
			results.result = Result::UnsupportedRequest;
			return NULL;
		}
		break;
	}
	case (uint64)CompressedPixelFormat::DXT1: Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "DXT1");
		results.result = Result::UnsupportedRequest;
		return NULL;
	case (uint64)CompressedPixelFormat::DXT3: Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "DXT1");
		results.result = Result::UnsupportedRequest;
		return NULL;
	case (uint64)CompressedPixelFormat::DXT5: Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "DXT3");
		results.result = Result::UnsupportedRequest;
		return NULL;
	default:
	{}
	}
	return textureToUse;
}



TextureUploadResults textureUpload(IPlatformContext& ctx, const Texture& texture, bool allowDecompress)
{
	auto& handles = ctx.getNativePlatformHandles();
	const VkDevice& device = handles.context.device;
	const VkPhysicalDeviceMemoryProperties& memprops = handles.deviceMemProperties;
	const VkCommandPool& pool = handles.universalCommandPool;
	bool supportPvrtc = handles.platformInfo.supportPvrtcImage;
	bool supportPvrtc2 = false;

	VkCommandBuffer cbuff = allocateCommandBuffer(device, pool);
	TextureUploadResultsData_ results;

	// Check that the texture is valid.
	if (!texture.getDataSize())
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs.\n");
		results.result = Result::UnsupportedRequest;
		return results;
	}


	VkFormat format = VK_FORMAT_UNDEFINED;

	//Texture to use if we decompress in software.
	Texture decompressedTexture;

	// Texture pointer which points at the texture we should use for the function.
	// Allows switching to, for example, a decompressed version of the texture.
	const Texture* textureToUse = decompressIfRequired(texture, decompressedTexture, allowDecompress, supportPvrtc,
	                              supportPvrtc2, results);


	// Check that the format is a valid format for this API - Doesn't check specifically between OpenGL/ES,
	// it simply gets the values that would be set for a KTX file.
	if ((format = nativeVk::ConvertToVk::pixelFormat(textureToUse->getPixelFormat(), textureToUse->getColorSpace(),
	              textureToUse->getChannelType())) == VK_FORMAT_UNDEFINED)
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Texture's pixel type is not supported by this API.\n");
		results.result = Result::UnsupportedRequest;
		return results;
	}
	results.format = textureToUse->getPixelFormat();
	results.textureSize = textureToUse->getTotalDimensions();

	uint32 texWidth = textureToUse->getWidth(), texHeight = textureToUse->getHeight(), texDepth = textureToUse->getDepth(),
	       texMipLevels = textureToUse->getNumberOfMIPLevels(), texArraySlices = textureToUse->getNumberOfArrayMembers(),
	       texFaces = textureToUse->getNumberOfFaces();

	// create the out image and prepare for trasfer operation
	if (!vulkan::createImageAndMemory(device, memprops,
	                                  types::Extent3D(texWidth, texHeight, texDepth), texArraySlices,
	                                  VK_SAMPLE_COUNT_1_BIT, texMipLevels, textureToUse->getNumberOfFaces() > 1,
	                                  texDepth > 1 ? VK_IMAGE_TYPE_3D : texHeight > 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D,
	                                  format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                                  results.image.image, results.image.memory))
	{
		Log("Failed to create the Image");
		results.result = Result::UnknownError;
		return results;
	}

	beginCommandBuffer(cbuff);
	//Create a bunch of buffers that will be used as copy destinations - each will be one mip level, one array slice / one face
	//Faces are considered array elements, so each Framework array slice in a cube array will be 6 vulkan array slices.

	//Edit the info to be the small, linear images that we are using.
	using utils::vulkan::ImageUpdateParam;
	std::vector<ImageUpdateParam> imageUpdates(texMipLevels * texArraySlices * texFaces);
	uint32 imageUpdateIndex = 0;
	for (uint32 mipLevel = 0; mipLevel < texMipLevels; ++mipLevel)
	{
		texWidth = textureToUse->getWidth(mipLevel);
		texHeight = textureToUse->getHeight(mipLevel);
		texDepth = textureToUse->getDepth(mipLevel);
		for (uint32 arraySlice = 0; arraySlice < texArraySlices; ++arraySlice)
		{
			for (uint32 face = 0; face < texFaces; ++face)
			{
				ImageUpdateParam& update = imageUpdates[imageUpdateIndex];
				update.width = texWidth;
				update.height = texHeight;
				update.depth = texDepth;
				update.arrayIndex = arraySlice;
				update.cubeFace = face;
				update.mipLevel = mipLevel;
				update.data = textureToUse->getDataPointer(mipLevel, arraySlice, face);
				update.dataSize = textureToUse->getDataSize(mipLevel, false, false);
				++imageUpdateIndex;
			}// next face
		}// next arrayslice
	}// next miplevel

	auto result = vulkan::updateImageDeferred(
	                device, cbuff, VK_NULL_HANDLE, VK_NULL_HANDLE, -1, -1, memprops,
	                imageUpdates.data(), (uint32)imageUpdates.size(),
	                format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texFaces > 1, results.image.image);

	nativeVk::vkThrowIfFailed(vk::EndCommandBuffer(cbuff), "TextureUtils:TextureUpload End");

	/// --- SUBMIT AND CHECK THE FENCES ---
	submitWaitAndDestroy(device, handles.mainQueue(), VK_NULL_HANDLE, pool, VK_NULL_HANDLE, cbuff, VK_NULL_HANDLE, VK_NULL_HANDLE);

	results.result = Result::Success;
	return results;
}


TextureUploadAsyncResults textureUploadDeferred(ISharedPlatformContext& ctx, const Texture& texture, bool allowDecompress)
{
	auto& handle = ctx.getParentContext().getNativePlatformHandles();
	auto& sharedhandle = ctx.getSharedHandles();
	const VkDevice& device = handle.context.device;
	const VkPhysicalDeviceMemoryProperties& memprops = handle.deviceMemProperties;
	bool supportPvrtc = handle.platformInfo.supportPvrtcImage;
	bool supportPvrtc2 = false;

	TextureUploadAsyncResultsData_ results;
	results.result = Result::Success;

	// Check that the texture is valid.
	if (!texture.getDataSize())
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs.\n");
		results.result = Result::UnsupportedRequest;
		return TextureUploadAsyncResults(new TextureUploadAsyncResults_(std::move(results)));
	}


	VkFormat format = VK_FORMAT_UNDEFINED;

	//Texture to use if we decompress in software.
	Texture decompressedTexture;

	// Texture pointer which points at the texture we should use for the function.
	// Allows switching to, for example, a decompressed version of the texture.
	const Texture* textureToUse = decompressIfRequired(texture, decompressedTexture, allowDecompress, supportPvrtc,
	                              supportPvrtc2, results);


	// Check that the format is a valid format for this API - Doesn't check specifically between OpenGL/ES,
	// it simply gets the values that would be set for a KTX file.
	if ((format = nativeVk::ConvertToVk::pixelFormat(textureToUse->getPixelFormat(), textureToUse->getColorSpace(),
	              textureToUse->getChannelType())) == VK_FORMAT_UNDEFINED)
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Texture's pixel type is not supported by this API.\n");
		results.result = Result::UnsupportedRequest;
		return TextureUploadAsyncResults(new TextureUploadAsyncResults_(std::move(results)));
	}
	results.format = textureToUse->getPixelFormat();
	results.textureSize = textureToUse->getTotalDimensions();

	uint32 texWidth = textureToUse->getWidth(), texHeight = textureToUse->getHeight(), texDepth = textureToUse->getDepth(),
	       texMipLevels = textureToUse->getNumberOfMIPLevels(), texArraySlices = textureToUse->getNumberOfArrayMembers(),
	       texFaces = textureToUse->getNumberOfFaces();

	// create the out image and prepare for trasfer operation
	if (!vulkan::createImageAndMemory(
	      device, memprops, types::Extent3D(texWidth, texHeight, texDepth), texArraySlices,
	      VK_SAMPLE_COUNT_1_BIT, texMipLevels, textureToUse->getNumberOfFaces() > 1,
	      texDepth > 1 ? VK_IMAGE_TYPE_3D : texHeight > 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D,
	      format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	      results.image.image, results.image.memory))
	{
		Log("Failed to create the Image");
		results.result = Result::UnknownError;
		return TextureUploadAsyncResults(new TextureUploadAsyncResults_(std::move(results)));
	}

	// POPULATE, TRANSITION ETC
	{
		bool multiQFamily = isMultiQueue(sharedhandle.queueFamily, handle.universalQueueFamily);

		VkCommandBuffer cbuffMain = allocateCommandBuffer(device, sharedhandle.pool);
		beginCommandBuffer(cbuffMain);
		VkCommandBuffer cbuffTakeOwn = VK_NULL_HANDLE;
		VkCommandBuffer cbuffRelinquishOwn = VK_NULL_HANDLE;


		if (multiQFamily)
		{
			cbuffTakeOwn = allocateCommandBuffer(device, handle.universalCommandPool);
			cbuffRelinquishOwn = allocateCommandBuffer(device, handle.universalCommandPool);
			beginCommandBuffer(cbuffTakeOwn);
			beginCommandBuffer(cbuffRelinquishOwn);
		}

		//Create a bunch of buffers that will be used as copy destinations - each will be one mip level, one array slice / one face
		//Faces are considered array elements, so each Framework array slice in a cube array will be 6 vulkan array slices.

		//Edit the info to be the small, linear images that we are using.
		using utils::vulkan::ImageUpdateParam;
		std::vector<ImageUpdateParam> imageUpdates(texMipLevels * texArraySlices * texFaces);
		uint32 imageUpdateIndex = 0;
		for (uint32 mipLevel = 0; mipLevel < texMipLevels; ++mipLevel)
		{
			texWidth = textureToUse->getWidth(mipLevel);
			texHeight = textureToUse->getHeight(mipLevel);
			texDepth = textureToUse->getDepth(mipLevel);
			for (uint32 arraySlice = 0; arraySlice < texArraySlices; ++arraySlice)
			{
				for (uint32 face = 0; face < texFaces; ++face)
				{
					ImageUpdateParam& update = imageUpdates[imageUpdateIndex];
					update.width = texWidth;
					update.height = texHeight;
					update.depth = texDepth;
					update.arrayIndex = arraySlice;
					update.cubeFace = face;
					update.mipLevel = mipLevel;
					update.data = textureToUse->getDataPointer(mipLevel, arraySlice, face);
					update.dataSize = textureToUse->getDataSize(mipLevel, false, false);
					++imageUpdateIndex;
				}// next face
			}// next arrayslice
		}// next miplevel

		results.updateCleanupData = updateImageDeferred(
		                              device, cbuffMain, cbuffTakeOwn, cbuffRelinquishOwn, handle.universalQueueFamily,
		                              sharedhandle.queueFamily, memprops, imageUpdates.data(), (uint32)imageUpdates.size(),
		                              format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texFaces > 1, results.image.image);

		nativeVk::vkThrowIfFailed(vk::EndCommandBuffer(cbuffMain), "TextureUtils:TextureUpload End");
		if (multiQFamily)
		{
			nativeVk::vkThrowIfFailed(vk::EndCommandBuffer(cbuffTakeOwn), "TextureUtils:TextureUpload End");
			nativeVk::vkThrowIfFailed(vk::EndCommandBuffer(cbuffRelinquishOwn), "TextureUtils:TextureUpload End");
		}
		results.device = device;
		/// --- SUBMIT AND CHECK THE FENCES ---
		submitAndGetFence(device, sharedhandle.queue, handle.mainQueue(),
		                  sharedhandle.pool, handle.universalCommandPool,
		                  cbuffMain, cbuffTakeOwn, cbuffRelinquishOwn, results);
	}
	results.result = Result::Success;
	return TextureUploadAsyncResults(new TextureUploadAsyncResults_(std::move(results)));
}




}// namespace vulkan
}// namespace utils
}// namespace pvr

//!\endcond
