#include "BufferUtilsVk.h"
#include "PVRNativeApi/BufferUtils.h"
#include "NativeObjectsVk.h"
#include "PVRPlatformGlue/PlatformContext.h"
#include "PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h"
namespace {
bool getMemoryTypeIndex(VkPhysicalDeviceMemoryProperties& deviceMemProps,
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
namespace apiUtils {
bool createBuffer(IPlatformContext& context, types::BufferBindingUse::Bits usage,
                  pvr::uint32 size, bool memHostVisible, native::HBuffer_& outBuffer)
{
	auto& nativeHandle = static_cast<pvr::system::PlatformContext&>(context).getNativePlatformHandles();
	return vulkan::createBuffer(nativeHandle.context.device, nativeHandle.deviceMemProperties,
	                            (memHostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	                            usage, size, outBuffer, NULL);
}

namespace vulkan {
inline pvr::uint32 getVkBufferUsage(types::BufferBindingUse::Bits usage)
{
	if (usage & types::BufferBindingUse::VertexBuffer) { return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
	if (usage & types::BufferBindingUse::IndexBuffer) { return VK_BUFFER_USAGE_INDEX_BUFFER_BIT; }
	if (usage & types::BufferBindingUse::UniformBuffer) { return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }
	assertion(false, "Unsupported buffer binding usage");
	return 0;
}

bool allocateBufferDeviceMemory(VkDevice device, VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                                VkMemoryPropertyFlagBits allocMemProperty, native::HBuffer_& outBuffer,
                                VkMemoryRequirements* outMemRequirements)
{
	VkMemoryAllocateInfo memAllocInfo;
	VkMemoryRequirements memReq;
	VkMemoryRequirements* memReqPtr = &memReq;
	if (outMemRequirements) { memReqPtr = outMemRequirements; }

	vk::GetBufferMemoryRequirements(device, outBuffer.buffer, memReqPtr);
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
	if (vk::AllocateMemory(device, &memAllocInfo, NULL, &outBuffer.memory) != VK_SUCCESS)
	{
		pvr::Log("Failed to allocate buffer's memory");
		return false;
	}
	if (vk::BindBufferMemory(device, outBuffer.buffer, outBuffer.memory, 0) != VK_SUCCESS)
	{
		return false;
	}
	return true;
}

bool createBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                  VkMemoryPropertyFlagBits allocMemProperty, types::BufferBindingUse::Bits usage,
                  pvr::uint32 size, native::HBuffer_& outBuffer,
                  VkMemoryRequirements* outMemRequirements)
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
