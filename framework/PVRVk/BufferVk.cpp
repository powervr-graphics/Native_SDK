/*!
\brief Function definitions for the Buffer class.
\file PVRVk/BufferVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN

#include "PVRVk/BufferVk.h"
namespace pvrvk {
namespace impl {
Buffer_::~Buffer_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroyBuffer(_device->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterDevice("Buffer");
		}
	}
}

Buffer_::Buffer_(DeviceWeakPtr device, const BufferCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_BUFFER_EXT), _createInfo(createInfo)
{
	VkBufferCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_BUFFER_CREATE_INFO);
	vkCreateInfo.size = _createInfo.getSize();
	vkCreateInfo.usage = static_cast<VkBufferUsageFlags>(_createInfo.getUsageFlags());
	vkCreateInfo.flags = static_cast<VkBufferCreateFlags>(_createInfo.getFlags());
	vkCreateInfo.sharingMode = static_cast<VkSharingMode>(_createInfo.getSharingMode());
	vkCreateInfo.pQueueFamilyIndices = _createInfo.getQueueFamilyIndices();
	vkCreateInfo.queueFamilyIndexCount = _createInfo.getNumQueueFamilyIndices();
	vkThrowIfFailed(static_cast<Result>(device->getVkBindings().vkCreateBuffer(device->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle)), "Failed to create Buffer");

	device->getVkBindings().vkGetBufferMemoryRequirements(device->getVkHandle(), _vkHandle, reinterpret_cast<VkMemoryRequirements*>(&_memRequirements));
}

BufferView_::BufferView_(const DeviceWeakPtr& device, const Buffer& buffer, Format format, VkDeviceSize offset, VkDeviceSize size)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_BUFFER_VIEW_EXT), _offset(offset), _size(size), _format(format), _buffer(buffer)
{
	VkBufferViewCreateInfo createInfo = {};
	createInfo.sType = static_cast<VkStructureType>(StructureType::e_BUFFER_CREATE_INFO);
	createInfo.buffer = _buffer->getVkHandle();
	createInfo.format = static_cast<VkFormat>(_format);
	createInfo.offset = _offset;
	createInfo.range = _size;
	vkThrowIfFailed(static_cast<Result>(_device->getVkBindings().vkCreateBufferView(_device->getVkHandle(), &createInfo, nullptr, &_vkHandle)), "Failed to create BufferView");
}

void BufferView_::destroy()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroyBufferView(_device->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice("BufferView");
		}
	}
}
} // namespace impl
} // namespace pvrvk

//!\endcond
