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
//!\cond NO_DOXYGEN
Semaphore_::Semaphore_(make_shared_enabler, const DeviceWeakPtr& device, const SemaphoreCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	VkSemaphoreCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_SEMAPHORE_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkSemaphoreCreateFlags>(_createInfo.getFlags());

	if (createInfo.getExternalSemaphoreHandleTypeFlagBitslags() != ExternalSemaphoreHandleTypeFlagBits::e_FLAG_BITS_MAX_ENUM)
	{
		VkExportSemaphoreCreateInfoKHR exportSemaphoreCreateInfoKHR = {};
		exportSemaphoreCreateInfoKHR.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR;
		exportSemaphoreCreateInfoKHR.handleTypes = static_cast<VkExternalSemaphoreHandleTypeFlags>(createInfo.getExternalSemaphoreHandleTypeFlagBitslags());

		vkCreateInfo.pNext = &exportSemaphoreCreateInfoKHR;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		WindowsExportMemorySecurity windowsExportMemorySecurity;
		windowsExportMemorySecurity.init();

		VkExportSemaphoreWin32HandleInfoKHR exportSemaphoreWin32HandleInfoKHR = {};
		exportSemaphoreWin32HandleInfoKHR.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR;
		exportSemaphoreWin32HandleInfoKHR.pAttributes = windowsExportMemorySecurity.getSecurityAttributes();
		exportSemaphoreWin32HandleInfoKHR.dwAccess = DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE;
		exportSemaphoreCreateInfoKHR.pNext = &exportSemaphoreWin32HandleInfoKHR;	
#endif

		vkThrowIfFailed(getDevice()->getVkBindings().vkCreateSemaphore(getDevice()->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle), "Failed to create Semaphore");
	}
	else
	{
		vkThrowIfFailed(getDevice()->getVkBindings().vkCreateSemaphore(getDevice()->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle), "Failed to create Semaphore");
	}
}

Semaphore_::~Semaphore_()
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
//!\endcond
} // namespace impl
} // namespace pvrvk
