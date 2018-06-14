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
Semaphore_::Semaphore_(const DeviceWeakPtr& device) : DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_SEMAPHORE_EXT)
{
	VkSemaphoreCreateInfo nfo;
	nfo.sType = static_cast<VkStructureType>(StructureType::e_SEMAPHORE_CREATE_INFO);
	nfo.pNext = 0;
	nfo.flags = static_cast<VkSemaphoreCreateFlags>(SemaphoreCreateFlags::e_NONE);
	vkThrowIfFailed(_device->getVkBindings().vkCreateSemaphore(_device->getVkHandle(), &nfo, NULL, &_vkHandle), "Semaphore::failed to create Semaphore object");
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