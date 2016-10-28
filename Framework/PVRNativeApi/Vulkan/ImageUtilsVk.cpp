/*!*********************************************************************************************************************
\file         PVRNativeApi/Vulkan/ImageUtilsVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        contain functions for creating Vulkan Image object.
***********************************************************************************************************************/
#include "PVRAssets/PixelFormat.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRPlatformGlue/PlatformContext.h"
#include "PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h"
#include "ImageUtilsVk.h"
#include "BufferUtilsVk.h"
#include "VkErrors.h"
namespace {
// helper functions
inline VkCommandBuffer createCommandBuffer(pvr::platform::NativePlatformHandles_& platformHandle)
{
	VkCommandBufferAllocateInfo cmdAlloc = {};
	cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAlloc.commandPool = platformHandle.commandPool;
	cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdAlloc.commandBufferCount = 1;
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
	vk::AllocateCommandBuffers(platformHandle.context.device, &cmdAlloc, &cmdBuffer);
	return cmdBuffer;
}

inline void destroyCommandBuffer(pvr::platform::NativePlatformHandles_& platformHandle, VkCommandBuffer cmd)
{
	vk::FreeCommandBuffers(platformHandle.context.device, platformHandle.commandPool, 1, &cmd);
}

inline void destroyBufferAndMemory(pvr::platform::NativePlatformHandles_& platformHandle, pvr::native::HBuffer_& buffer)
{
	vk::FreeMemory(platformHandle.context.device, buffer.memory, NULL);
	vk::DestroyBuffer(platformHandle.context.device, buffer.buffer, NULL);
	buffer.buffer = VK_NULL_HANDLE;
	buffer.memory = VK_NULL_HANDLE;
}

inline void destroyImageAndMemory(pvr::platform::NativePlatformHandles_& platformHandle, pvr::native::HTexture_& texture)
{
	vk::FreeMemory(platformHandle.context.device, texture.memory, NULL);
	vk::DestroyImage(platformHandle.context.device, texture.image, NULL);
	texture.image = VK_NULL_HANDLE;
	texture.memory = VK_NULL_HANDLE;
}

inline bool isCompressedFormat(VkFormat fmt)
{
	return fmt == VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK_IMG_BETA || fmt == VK_FORMAT_RGBA_PVRTC1_4BPP_BLOCK_IMG_BETA ||
	       (fmt > VK_FORMAT_BC1_RGB_UNORM_BLOCK && fmt < VK_FORMAT_ASTC_12x12_SRGB_BLOCK) ||
	       (fmt >= VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG && fmt <= VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG);
}
}

namespace pvr {
namespace utils {
namespace vulkan {

void setImageLayout(VkCommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout,
                    VkImage image, VkImageAspectFlags aspectFlags, uint32 baseMipLevel, uint32 numMipLevels,
                    uint32 baseArrayLayer, uint32 numArrayLayers)
{
	VkImageMemoryBarrier imageMemBarrier = {};
	imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.pNext = NULL;
	imageMemBarrier.srcAccessMask = 0;
	imageMemBarrier.dstAccessMask = 0;
	imageMemBarrier.oldLayout = oldLayout;
	imageMemBarrier.newLayout = newLayout;
	imageMemBarrier.image = image;
	imageMemBarrier.subresourceRange =
	  VkImageSubresourceRange { aspectFlags, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers };
	imageMemBarrier.srcQueueFamilyIndex = imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask =
		  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	}

	if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		imageMemBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		imageMemBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	VkImageMemoryBarrier* memBarries = &imageMemBarrier;
	vk::CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0,
	                       NULL, 1, memBarries);
}

void beginCommandBuffer(VkCommandBuffer& cmdBuffer)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vk::BeginCommandBuffer(cmdBuffer, &beginInfo);
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
                              const VkMemoryRequirements& memoryRequirements, pvr::uint32 typeBits,
                              VkMemoryPropertyFlagBits allocMemProperty)
{
	VkDeviceMemory memory;
	VkMemoryAllocateInfo sMemoryAllocInfo;
	sMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	sMemoryAllocInfo.pNext = NULL;
	sMemoryAllocInfo.allocationSize = memoryRequirements.size;
	pvr::assertion(getMemoryTypeIndex(deviceMemProps, typeBits, allocMemProperty, sMemoryAllocInfo.memoryTypeIndex));
	vk::AllocateMemory(device, &sMemoryAllocInfo, NULL, &memory);
	return memory;
}


bool allocateImageDeviceMemory(VkDevice device, VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                               VkMemoryPropertyFlagBits allocMemProperty,
                               native::HTexture_& image, VkMemoryRequirements* outMemRequirements)
{
	VkMemoryRequirements memReq;
	VkMemoryRequirements* memReqPtr = &memReq;
	if (outMemRequirements)
	{
		memReqPtr = outMemRequirements;
	}
	vk::GetImageMemoryRequirements(device, image.image, memReqPtr);
	if (memReqPtr->memoryTypeBits == 0) // find the first allowed type
	{
		pvr::Log("Failed to get buffer memory requirements: memory requirements are 0");
		return false;
	}
	image.memory = allocateMemory(device, deviceMemProperty, *memReqPtr, memReqPtr->memoryTypeBits, allocMemProperty);
	if (image.memory == VK_NULL_HANDLE)
	{
		pvr::Log("Failed to allocate Image memory");
		return false;
	}
	vk::BindImageMemory(device, image.image, image.memory, 0);
	return true;
}

bool updateImage(IPlatformContext& context, ImageUpdateParam* updateParams, uint32 numUpdateParams,
                 uint32 numArraySlice, VkFormat srcFormat, bool isCubeMap, VkImage image)
{
	platform::PlatformContext& contextVkGlue = static_cast<platform::PlatformContext&>(context);
	VkDevice& device = contextVkGlue.getNativePlatformHandles().context.device;
	VkCommandBuffer cmdBuffer = createCommandBuffer(contextVkGlue.getNativePlatformHandles());
	if (cmdBuffer == VK_NULL_HANDLE) { return false; }
	beginCommandBuffer(cmdBuffer);
	uint32 numFace = (isCubeMap ? 6 : 1);

	uint32 hwSlice;

	bool mustBeTiled = !isCompressedFormat(srcFormat);
	std::vector<native::HBuffer_> stagingBuffers;
	std::vector<native::HTexture_> stagingImages;
	if (mustBeTiled)
	{
		stagingBuffers.resize(numUpdateParams);
		VkBufferImageCopy imgcp = {};
		VkResult res;

		for (uint32 i = 0; i < numUpdateParams; ++i)
		{
			const ImageUpdateParam& mipLevelUpdate = updateParams[i];
			assertion(mipLevelUpdate.data && mipLevelUpdate.dataSize, "Data and Data size must be valid");
			hwSlice = mipLevelUpdate.arrayIndex * numFace + mipLevelUpdate.cubeFace;
			native::HBuffer_& buffer = stagingBuffers[i];
			VkMemoryRequirements memReqSrc = {};


			// create the staging buffer
			if (!pvr::utils::vulkan::createBufferAndMemory(device, contextVkGlue.getNativePlatformHandles().deviceMemProperties,
			    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, types::BufferBindingUse::TransferSrc, mipLevelUpdate.dataSize, buffer, &memReqSrc))
			{
				Log("Failed to create staging buffer for mip level %d slice %d", mipLevelUpdate.mipLevel, hwSlice);
				return false;
			}

			imgcp.imageOffset = VkOffset3D{ mipLevelUpdate.offsetX, mipLevelUpdate.offsetY, mipLevelUpdate.offsetZ };
			imgcp.imageExtent = VkExtent3D{ mipLevelUpdate.width, mipLevelUpdate.height, 1 };
			imgcp.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
				vkThrowIfFailed(res, "ImageUtils:updateImage Linear staging buffer Map Memory Failed");
				Log("ImageUtils:updateImage Linear staging buffer Map Memory Failed");
				vk::DestroyBuffer(device, buffer.buffer, NULL);
				vk::FreeMemory(device, buffer.memory, NULL);
				return false;
			}
			for (uint32_t slice3d = 0; !slice3d || (slice3d < mipLevelUpdate.depth); ++slice3d)
			{
				memcpy(mappedData, srcData, srcDataSize);
				mappedData += srcDataSize;
				srcData += srcDataSize;
			}
			vk::UnmapMemory(device, buffer.memory);
			vk::CmdCopyBufferToImage(cmdBuffer, buffer.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgcp);
		}

	}
	else
	{
		stagingImages.resize(numUpdateParams);

		VkImageCopy imgcp = {};
		imgcp.srcSubresource.layerCount = 1;
		imgcp.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imgcp.dstSubresource.layerCount = 1;
		VkResult res;
		VkImageType imageType = updateParams->depth > 1 ? VK_IMAGE_TYPE_3D : updateParams->height > 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D;
		bool tiling = (srcFormat == VK_FORMAT_RGBA_PVRTC1_4BPP_BLOCK_IMG_BETA || srcFormat == VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK_IMG_BETA ||
		               (srcFormat >= VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG && srcFormat <= VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG)
		              );
		VkMemoryRequirements memReqSrc;
		for (uint32 i = 0; i < numUpdateParams; ++i)
		{
			const ImageUpdateParam& mipLevelUpdate = updateParams[i];
			assertion(mipLevelUpdate.data && mipLevelUpdate.dataSize, "Data and Data size must be valid");
			hwSlice = mipLevelUpdate.arrayIndex * numFace + mipLevelUpdate.cubeFace;
			native::HTexture_& srcTex = stagingImages[i];


			// create the staging image
			if (!pvr::utils::vulkan::createImageAndMemory(context, types::Extent3D(mipLevelUpdate.width, mipLevelUpdate.height, mipLevelUpdate.depth),
			    1, VK_SAMPLE_COUNT_1_BIT, 1, tiling, isCubeMap, imageType, srcFormat, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
			    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, srcTex))
			{
				Log("Failed to create staging image for mip level %d slice %d", mipLevelUpdate.mipLevel, hwSlice);
				return false;
			}

			imgcp.srcOffset = VkOffset3D{ 0, 0, 0 };
			imgcp.dstOffset = VkOffset3D{ mipLevelUpdate.offsetX, mipLevelUpdate.offsetY, mipLevelUpdate.offsetZ };
			imgcp.extent = VkExtent3D{ mipLevelUpdate.width, mipLevelUpdate.height, mipLevelUpdate.depth };

			imgcp.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imgcp.srcSubresource.baseArrayLayer = 0;
			imgcp.srcSubresource.layerCount = 1;
			imgcp.srcSubresource.mipLevel = 0;

			imgcp.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imgcp.dstSubresource.baseArrayLayer = hwSlice;
			imgcp.dstSubresource.layerCount = 1;
			imgcp.dstSubresource.mipLevel = mipLevelUpdate.mipLevel;

			VkSubresourceLayout layout;
			VkImageSubresource sres;
			sres.arrayLayer = hwSlice;
			sres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			sres.mipLevel = 0;
			vk::GetImageSubresourceLayout(device, srcTex.image, &sres, &layout);
			uint8* mappedData = NULL;
			const uint8* srcData = (uint8*)mipLevelUpdate.data;
			vk::GetImageMemoryRequirements(device, srcTex.image, &memReqSrc);
			if (!vkIsSuccessful(res = vk::MapMemory(device, srcTex.memory, 0, memReqSrc.size, 0, (void**)&mappedData),
			                    "TextureUtils:TextureUpload Linear Temp Image Map Memory"))
			{
				vkThrowIfFailed(res, "ImageUtils:updateImage Linear staging buffer Map Memory Failed");
				Log("ImageUtils:updateImage Linear staging buffer Map Memory Failed");
				break;
			}

			for (uint32_t slice3d = 0; !slice3d || (slice3d < mipLevelUpdate.depth); ++slice3d)
			{
				memcpy(mappedData, srcData, mipLevelUpdate.dataSize);
				mappedData += layout.depthPitch;
				srcData += mipLevelUpdate.dataSize;
			}
			vk::UnmapMemory(device, srcTex.memory);

			// transform the layout
			setImageLayout(cmdBuffer, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcTex.image,
			               VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);


			vk::CmdCopyImage(cmdBuffer, srcTex.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
			                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgcp);
		}
	}

	// destroy the commandbuffer
	vk::EndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	VkQueue q = contextVkGlue.getNativePlatformHandles().graphicsQueue;
	vk::DeviceWaitIdle(device);
	vk::QueueSubmit(q, 1, &submitInfo, VK_NULL_HANDLE);
	vk::DeviceWaitIdle(device);

	// free the command buffer and the staging buffers
	for (uint32 i = 0; i < stagingBuffers.size(); ++i)
	{
		destroyBufferAndMemory(contextVkGlue.getNativePlatformHandles(), stagingBuffers[i]);
	}
	for (uint32 i = 0; i < stagingImages.size(); ++i)
	{
		destroyImageAndMemory(contextVkGlue.getNativePlatformHandles(), stagingImages[i]);
	}

	destroyCommandBuffer(contextVkGlue.getNativePlatformHandles(), cmdBuffer);
	return true;
}


bool createImageAndMemory(IPlatformContext& context, const types::Extent3D& dimension, uint32 arrayLayer,
                          VkSampleCountFlagBits sampleCount, uint32 numMipLevels, bool tilingOptimal, bool isCubeMap,
                          VkImageType imageType, VkFormat format, VkImageUsageFlags imageUsageFlags,
                          VkImageLayout  newLayout, VkMemoryPropertyFlagBits memPropertyFlags,
                          native::HTexture_& outTexture)
{
	uint32 totalArrayLayers = arrayLayer * (isCubeMap ? 6 : 1);
	platform::NativePlatformHandles_& platformVk = context.getNativePlatformHandles();

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
	nfo.tiling = (tilingOptimal ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR);
	nfo.usage = imageUsageFlags;
	nfo.queueFamilyIndexCount = 1;
	uint32_t queueFamilyZero = 0;
	nfo.pQueueFamilyIndices = &queueFamilyZero;
	nfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

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
	pvr::vkThrowIfFailed(vk::CreateImage(platformVk.context.device, &nfo, NULL, &outTexture.image),
	                     "TextureUtils:TextureUpload createImage");

	VkMemoryRequirements memReqDst;
	pvr::utils::vulkan::allocateImageDeviceMemory(platformVk.context.device, platformVk.deviceMemProperties,
	    memPropertyFlags, outTexture, &memReqDst);

	if (newLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)
	{
		VkCommandBufferAllocateInfo cmdAlloc = {};
		cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAlloc.commandBufferCount = 1;
		cmdAlloc.commandPool = context.getNativePlatformHandles().commandPool;
		VkCommandBuffer cmd;
		vk::AllocateCommandBuffers(platformVk.context.device, &cmdAlloc, &cmd);
		VkCommandBufferBeginInfo cmdBegin = {};
		cmdBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vk::BeginCommandBuffer(cmd, &cmdBegin);

		VkImageAspectFlags imageAspect = (imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ?
		                                  0 : VK_IMAGE_ASPECT_COLOR_BIT);

		if (!imageAspect && format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
		{
			const VkImageAspectFlags aspects[] =
			{
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,  //  VK_FORMAT_D32_SFLOAT_S8_UINT
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,  //  VK_FORMAT_D24_UNORM_S8_UINT
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,  //  VK_FORMAT_D16_UNORM_S8_UINT
				VK_IMAGE_ASPECT_DEPTH_BIT,          //  VK_FORMAT_D32_SFLOAT
				VK_IMAGE_ASPECT_STENCIL_BIT,          //  VK_FORMAT_S8_UINT
				VK_IMAGE_ASPECT_DEPTH_BIT,          //  VK_FORMAT_X8_D24_UNORM_PACK32
				VK_IMAGE_ASPECT_DEPTH_BIT,          //  VK_FORMAT_D16_UNORM
			};

			imageAspect = aspects[VK_FORMAT_D32_SFLOAT_S8_UINT - format];
		}

		pvr::utils::vulkan::setImageLayout(cmd, nfo.initialLayout, newLayout, outTexture.image, imageAspect,
		                                   0, nfo.mipLevels, 0, nfo.arrayLayers);
		vk::EndCommandBuffer(cmd);

		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pCommandBuffers = &cmd;
		submit.commandBufferCount = 1;

		VkFence fence;
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		pvr::vkThrowIfFailed(vk::CreateFence(platformVk.context.device, &fenceInfo, NULL, &fence), "Failed to create fence");

		vk::QueueSubmit(platformVk.graphicsQueue, 1, &submit, fence);
		vk::WaitForFences(platformVk.context.device, 1, &fence, true, uint64(-1));
		vk::DestroyFence(platformVk.context.device, fence, NULL);
		vk::FreeCommandBuffers(platformVk.context.device, cmdAlloc.commandPool, 1, &cmd);
	}
	return true;
}


}// namespace vulkan
}// namespace utils
}// namespace pvr
