#include "PVRAssets/Texture/PixelFormat.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
namespace pvr {
namespace apiutils {
namespace vulkan {

void beginCommandBuffer(VkCommandBuffer& cmdBuffer)
{
    VkCommandBufferBeginInfo beginInfo={};
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
	VkMemoryAllocateInfo memAllocInfo;
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
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.allocationSize = memReqPtr->size;
	image.memory = allocateMemory(device, deviceMemProperty, *memReqPtr, memReqPtr->memoryTypeBits, allocMemProperty);
	if (image.memory == VK_NULL_HANDLE)
	{
		pvr::Log("Failed to allocate Image memory");
		return false;
	}
	vk::BindImageMemory(device, image.image, image.memory, 0);
	return true;
}

}
}
}
