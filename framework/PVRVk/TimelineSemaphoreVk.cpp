/*!
\brief Function definitions for Timelinesemaphore class
\file PVRVk/TimelineSemaphoreVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "TimelineSemaphoreVk.h"

#include <utility>

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
TimelineSemaphore_::TimelineSemaphore_(const TimelineSemaphore_::make_shared_enabler& enabler, const DeviceWeakPtr& device, const SemaphoreCreateInfo& createInfo)
	: Semaphore_(device, createInfo)
{
	VkSemaphoreCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_SEMAPHORE_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkSemaphoreCreateFlags>(_createInfo.getFlags());

	VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo;

	semaphoreTypeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	semaphoreTypeCreateInfo.pNext = nullptr;
	semaphoreTypeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
	semaphoreTypeCreateInfo.initialValue = 0;

	vkCreateInfo.pNext = &semaphoreTypeCreateInfo;

	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateSemaphore(getDevice()->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle), "Failed to create Semaphore");
}

TimelineSemaphore_::~TimelineSemaphore_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroySemaphore(getDevice()->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}

bool TimelineSemaphore_::wait(const uint64_t& waitValue, uint64_t timeoutNanos)
{
	VkSemaphoreWaitInfo waitInfo;
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	waitInfo.pNext = nullptr;
	waitInfo.flags = 0;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &this->getVkHandle();
	waitInfo.pValues = &waitValue;

	Result res = static_cast<pvrvk::Result>(getDevice()->getVkBindings().vkWaitSemaphoresKHR(getDevice()->getVkHandle(), &waitInfo, timeoutNanos));

	vkThrowIfError(res);
	return (res == Result::e_SUCCESS);
}
//!\endcond
} // namespace impl
} // namespace pvrvk
