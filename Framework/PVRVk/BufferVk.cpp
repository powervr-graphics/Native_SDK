/*!
\brief Function definitions for the Buffer class.
\file PVRVk/BufferVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN

#include "PVRVk/BufferVk.h"
#include "PVRVk/ErrorsVk.h"
namespace pvrvk {
namespace impl {

inline bool createBuffer(VkDevice device, VkBufferUsageFlags usage, VkDeviceSize size,
                         VkBuffer& outBuffer, VkMemoryRequirements* outMemRequirements,
                         VkBufferCreateFlags bufferCreateFlags, bool sharingExclusive, const uint32_t* queueFamilyIndices,
                         uint32_t numQueueFamilyIndices)
{
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VkStructureType::e_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = usage;
	createInfo.flags = bufferCreateFlags;
	createInfo.sharingMode = (sharingExclusive ? VkSharingMode::e_EXCLUSIVE : VkSharingMode::e_CONCURRENT);
	createInfo.pQueueFamilyIndices = queueFamilyIndices;
	createInfo.queueFamilyIndexCount = numQueueFamilyIndices;
	createInfo.sharingMode = VkSharingMode::e_EXCLUSIVE;
	if (vk::CreateBuffer(device, &createInfo, nullptr, &outBuffer) != VkResult::e_SUCCESS)
	{
		Log(LogLevel::Error, "Failed to create Buffer");
		return false;
	}
	if (outMemRequirements)
	{
		vk::GetBufferMemoryRequirements(device, outBuffer, outMemRequirements);
	}
	return true;
}

void Buffer_::destroy()
{
	if (_vkBuffer != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{

			vk::DestroyBuffer(_device->getNativeObject(), _vkBuffer, NULL);
			_vkBuffer = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterContext("Buffer");
		}
	}
}

bool Buffer_::init(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags bufferCreateFlags,
                   bool sharingExclusive, const uint32_t* queueFamilyIndices, uint32_t numQueueFamilyIndices)
{
	if (size == 0)
	{
		assertion(size != 0, "Failed to allocate buffer. Allocation size should not be 0");
		return false;
	}

	if (_vkBuffer != VK_NULL_HANDLE)// re-allocate if neccessary
	{
		Log(LogLevel::Debug, "BufferVulkanImpl::allocate: Vulkan buffer %d was already allocated, deleting it. "
		    "This should normally NOT happen - allocate is private.", _vkBuffer);
		destroy();
	}
	_size = size;
	_usage = usage;

	return createBuffer(_device->getNativeObject(), usage, size, _vkBuffer, &_memRequirements,
	                    bufferCreateFlags, sharingExclusive, queueFamilyIndices, numQueueFamilyIndices);
}

bool BufferView_::init()
{
	VkBufferViewCreateInfo createInfo = {};
	createInfo.buffer = _buffer->getNativeObject();
	createInfo.format = (VkFormat)_format;
	createInfo.offset = _offset;
	createInfo.range = _size;
	createInfo.sType = VkStructureType::e_BUFFER_CREATE_INFO;
	if (vk::CreateBufferView(_device->getNativeObject(), &createInfo, nullptr, &_vkBufferView) == VkResult::e_SUCCESS)
	{
		return true;
	}

	return false;
}

void BufferView_::release()
{
	if (!_device.isValid())
	{
		return;
	}
	if (_vkBufferView != VK_NULL_HANDLE)
	{
		vk::DestroyBufferView(_device->getNativeObject(), _vkBufferView, nullptr);
		_vkBufferView = VK_NULL_HANDLE;
	}
}
}// impl
}// namespace pvrvk

//!\endcond