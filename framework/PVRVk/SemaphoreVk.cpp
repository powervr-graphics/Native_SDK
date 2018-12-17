/*!
\brief Function definitions for Semaphore class
\file PVRVk/SemaphoreVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/SemaphoreVk.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/ImageVk.h"

namespace pvrvk {
namespace impl {
Semaphore_::Semaphore_(const DeviceWeakPtr& device, const SemaphoreCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_SEMAPHORE_EXT), _createInfo(createInfo)
{
	VkSemaphoreCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_SEMAPHORE_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkSemaphoreCreateFlags>(_createInfo.getFlags());
	vkThrowIfFailed(_device->getVkBindings().vkCreateSemaphore(getDevice()->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle), "Failed to create Semaphore");
}

Semaphore_::~Semaphore_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroySemaphore(_device->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterDevice("Semaphore");
		}
	}
}
} // namespace impl
} // namespace pvrvk
