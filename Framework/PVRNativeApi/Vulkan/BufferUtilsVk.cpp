/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\BufferUtilsVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Implementations of functions for creating Vulkan Buffer object.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "BufferUtilsVk.h"
#include "PVRNativeApi/BufferUtils.h"
#include "NativeObjectsVk.h"
#include "PVRPlatformGlue/PlatformContext.h"
#include "PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h"
namespace {
bool getMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& deviceMemProps,
                        pvr::uint32 typeBits, VkMemoryPropertyFlagBits properties,
                        pvr::uint32& outTypeIndex)
{
	for (pvr::uint32 i = 0; i < 32; ++i)
	{
		if ((typeBits & 1) == 1)
		{
			if (VkMemoryPropertyFlagBits(deviceMemProps.memoryTypes[i].propertyFlags & properties) == properties)
			{
				outTypeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}
}
namespace pvr {
namespace utils {
bool createBuffer(IPlatformContext& context, types::BufferBindingUse usage,
                  pvr::uint32 size, bool memHostVisible, native::HBuffer_& outBuffer)
{
	auto& nativeHandle = static_cast<pvr::platform::PlatformContext&>(context).getNativePlatformHandles();
	return vulkan::createBufferAndMemory(nativeHandle.context.device, nativeHandle.deviceMemProperties,
	                                     (memHostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	                                     usage, size, outBuffer, NULL);
}

namespace vulkan {
inline pvr::uint32 getVkBufferUsage(types::BufferBindingUse usage)
{
	pvr::uint32 vkBits = 0;

	if (static_cast<pvr::uint32>(usage & types::BufferBindingUse::TransferSrc) != 0) { vkBits |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; }
	if (static_cast<pvr::uint32>(usage & types::BufferBindingUse::TransferDest) != 0) { vkBits |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; }
	if (static_cast<pvr::uint32>(usage & types::BufferBindingUse::UniformTexelBuffer) != 0) { vkBits |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT; }
	if (static_cast<pvr::uint32>(usage & types::BufferBindingUse::StorageTexelBuffer) != 0) { vkBits |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT; }
	if (static_cast<pvr::uint32>(usage & types::BufferBindingUse::UniformBuffer) != 0) { vkBits |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }
	if (static_cast<pvr::uint32>(usage & types::BufferBindingUse::StorageBuffer) != 0) { vkBits |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; }
	if (static_cast<pvr::uint32>(usage & types::BufferBindingUse::IndexBuffer) != 0) { vkBits |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; }
	if (static_cast<pvr::uint32>(usage & types::BufferBindingUse::VertexBuffer) != 0) { vkBits |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
	if (static_cast<pvr::uint32>(usage & types::BufferBindingUse::IndirectBuffer) != 0) { vkBits |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT; }
	assertion(vkBits != 0, "Unsupported buffer binding usage");
	return vkBits;
}

bool allocateBufferDeviceMemory(VkDevice device, const VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                                VkMemoryPropertyFlagBits allocMemProperty, native::HBuffer_& inOutBuffer,
                                VkMemoryRequirements* outMemRequirements)
{
	VkMemoryAllocateInfo memAllocInfo;
	VkMemoryRequirements memReq;
	VkMemoryRequirements* memReqPtr = &memReq;
	if (outMemRequirements) { memReqPtr = outMemRequirements; }

	vk::GetBufferMemoryRequirements(device, inOutBuffer.buffer, memReqPtr);
	if (memReqPtr->memoryTypeBits == 0) // find the first allowed type
	{
		pvr::Log("Failed to get buffer memory requirements: memory requirements are 0");
		return false;
	}
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.allocationSize = memReqPtr->size;
	if (!getMemoryTypeIndex(deviceMemProperty, memReqPtr->memoryTypeBits, allocMemProperty,
	                        memAllocInfo.memoryTypeIndex))
	{
		return false;
	}
	if (vk::AllocateMemory(device, &memAllocInfo, NULL, &inOutBuffer.memory) != VK_SUCCESS)
	{
		pvr::Log("Failed to allocate buffer's memory");
		return false;
	}
	if (vk::BindBufferMemory(device, inOutBuffer.buffer, inOutBuffer.memory, 0) != VK_SUCCESS)
	{
		return false;
	}
	return true;
}

bool createBufferAndMemory(VkDevice device, const VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                           VkMemoryPropertyFlagBits allocMemProperty, types::BufferBindingUse usage,
                           pvr::uint32 size, native::HBuffer_& outBuffer, VkMemoryRequirements* outMemRequirements)
{
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = getVkBufferUsage(usage);
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vk::CreateBuffer(device, &createInfo, NULL, &outBuffer.buffer) != VK_SUCCESS)
	{
		pvr::Log(pvr::Log.Error, "Failed to allocate Buffer");
		return false;
	}
	return allocateBufferDeviceMemory(device, deviceMemProperty, allocMemProperty, outBuffer, outMemRequirements);
}
}
}
}
//!\endcond