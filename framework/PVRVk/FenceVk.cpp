/*!
\brief Function definitions for Fence class
\file PVRVk/FenceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/FenceVk.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/ImageVk.h"

namespace pvrvk {
namespace impl {
Fence_::Fence_(const DeviceWeakPtr& device, const FenceCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_FENCE_EXT), _createInfo(createInfo)
{
	VkFenceCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_FENCE_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkFenceCreateFlags>(createInfo.getFlags());
	vkThrowIfFailed(_device->getVkBindings().vkCreateFence(_device->getVkHandle(), &vkCreateInfo, NULL, &_vkHandle), "Failed to create Fence");
}

Fence_::~Fence_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroyFence(_device->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterDevice("Fence");
		}
	}
}

bool Fence_::wait(uint64_t timeoutNanos)
{
	Result res;
	vkThrowIfError(
		res = static_cast<pvrvk::Result>(_device->getVkBindings().vkWaitForFences(_device->getVkHandle(), 1, &getVkHandle(), true, timeoutNanos)), "Fence::wait returned an error");
	debug_assertion(res == Result::e_SUCCESS || res == Result::e_TIMEOUT || res == Result::e_NOT_READY, "Fence returned invalid non-error VkResult!");
	return (res == Result::e_SUCCESS);
}

void Fence_::reset()
{
	vkThrowIfFailed(_device->getVkBindings().vkResetFences(_device->getVkHandle(), 1, &getVkHandle()), "Fence::reset returned an error");
}

bool Fence_::isSignalled()
{
	Result res;
	vkThrowIfError(res = static_cast<pvrvk::Result>(_device->getVkBindings().vkGetFenceStatus(_device->getVkHandle(), getVkHandle())), "Fence::getStatus returned an error");
	debug_assertion(res == Result::e_SUCCESS || res == Result::e_TIMEOUT || res == Result::e_NOT_READY, "Fence returned invalid non-error VkResult!");
	return (res == Result::e_SUCCESS);
}
} // namespace impl
} // namespace pvrvk
