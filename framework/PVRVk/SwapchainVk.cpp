/*!
\brief Function implementations for the Swapchain class.
\file PVRVk/SwapchainVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRVk/SwapchainVk.h"
#include "PVRVk/SurfaceVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/FenceVk.h"
#include "PVRVk/SemaphoreVk.h"

namespace pvrvk {
namespace impl {
Swapchain_::~Swapchain_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			for (uint32_t i = 0; i < _swapChainLength; ++i)
			{
				_colorImageViews[i].reset();
			}

			_device->getVkBindings().vkDestroySwapchainKHR(_device->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
			_surface.reset();
		}
		else
		{
			reportDestroyedAfterDevice("Swapchain");
		}
	}
}

bool Swapchain_::acquireNextImage(uint64_t timeOut, Semaphore signalSemaphore, Fence signalFence)
{
	Result res;
	vkThrowIfError(
		res = static_cast<pvrvk::Result>(_device->getVkBindings().vkAcquireNextImageKHR(_device->getVkHandle(), getVkHandle(), timeOut,
			signalSemaphore.isValid() ? signalSemaphore->getVkHandle() : VK_NULL_HANDLE, signalFence.isValid() ? signalFence->getVkHandle() : VK_NULL_HANDLE, &_swapchainId)),
		"PlatformContext:PresentBackbuffer AcquireNextImage error");
	return res == Result::e_SUCCESS;
}

bool Swapchain_::acquireNextImage(uint64_t timeOut, Semaphore signalSemaphore)
{
	Result res;
	vkThrowIfError(res = static_cast<pvrvk::Result>(_device->getVkBindings().vkAcquireNextImageKHR(
					   _device->getVkHandle(), getVkHandle(), timeOut, signalSemaphore.isValid() ? signalSemaphore->getVkHandle() : VK_NULL_HANDLE, VK_NULL_HANDLE, &_swapchainId)),
		"PlatformContext:PresentBackbuffer AcquireNextImage error");
	return res == Result::e_SUCCESS;
}

Swapchain_::Swapchain_(DeviceWeakPtr device, Surface surface, const SwapchainCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_SWAPCHAIN_KHR_EXT), _swapchainId(static_cast<uint32_t>(-1)),
	  _swapChainLength(static_cast<uint32_t>(-1))
{
	_createInfo = createInfo;
	_surface = surface;
	//--- create the swap chain
	VkSwapchainCreateInfoKHR swapchainCreate = {};
	swapchainCreate.sType = static_cast<VkStructureType>(StructureType::e_SWAPCHAIN_CREATE_INFO_KHR);
	swapchainCreate.clipped = _createInfo.clipped;
	swapchainCreate.compositeAlpha = static_cast<VkCompositeAlphaFlagBitsKHR>(_createInfo.compositeAlpha);
	swapchainCreate.surface = surface->getVkHandle();
	swapchainCreate.minImageCount = _createInfo.minImageCount;
	swapchainCreate.imageFormat = static_cast<VkFormat>(_createInfo.imageFormat);
	swapchainCreate.imageArrayLayers = _createInfo.imageArrayLayers;
	swapchainCreate.imageColorSpace = static_cast<VkColorSpaceKHR>(_createInfo.imageColorSpace);
	swapchainCreate.imageExtent.width = _createInfo.imageExtent.getWidth();
	swapchainCreate.imageExtent.height = _createInfo.imageExtent.getHeight();
	swapchainCreate.imageUsage = static_cast<VkImageUsageFlags>(_createInfo.imageUsage);
	swapchainCreate.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(_createInfo.preTransform);
	swapchainCreate.imageSharingMode = static_cast<VkSharingMode>(_createInfo.imageSharingMode);
	swapchainCreate.presentMode = static_cast<VkPresentModeKHR>(_createInfo.presentMode);
	swapchainCreate.queueFamilyIndexCount = _createInfo.numQueueFamilyIndex;

	ArrayOrVector<uint32_t, 10> queueFamilyIndices(swapchainCreate.queueFamilyIndexCount);
	for (uint32_t i = 0; i < swapchainCreate.queueFamilyIndexCount; i++)
	{
		queueFamilyIndices[i] = _createInfo.queueFamilyIndices[i];
	}

	swapchainCreate.pQueueFamilyIndices = queueFamilyIndices.get();

	assertion(swapchainCreate.minImageCount <= static_cast<uint32_t>(FrameworkCaps::MaxSwapChains), "Minimum number of swapchain images is larger than Max set");

	vkThrowIfFailed(_device->getVkBindings().vkCreateSwapchainKHR(_device->getVkHandle(), &swapchainCreate, NULL, &_vkHandle), "Could not create the swap chain");
	vkThrowIfFailed(_device->getVkBindings().vkGetSwapchainImagesKHR(_device->getVkHandle(), getVkHandle(), &_swapChainLength, NULL), "Could not get swapchain length");

	Log(LogLevel::Information, "Swapchain image count: %u ", _swapChainLength);

	assertion(_swapChainLength <= static_cast<uint32_t>(FrameworkCaps::MaxSwapChains), "Number of swapchain images is larger than Max set");

	VkImage swapchainImages[static_cast<uint32_t>(FrameworkCaps::MaxSwapChains)];
	vkThrowIfFailed(_device->getVkBindings().vkGetSwapchainImagesKHR(_device->getVkHandle(), getVkHandle(), &_swapChainLength, &swapchainImages[0]), "Could not get swapchain images");

	for (uint32_t i = 0; i < _swapChainLength; ++i)
	{
		// create the image wrapper
		SwapchainImage image;
		image.construct(_device->getWeakReference(), swapchainImages[i], _createInfo.imageFormat,
			pvrvk::Extent3D(_createInfo.imageExtent.getWidth(), _createInfo.imageExtent.getHeight(), 1), _createInfo.imageArrayLayers, 1, _createInfo.imageUsage);
		_colorImageViews[i] = _device->createImageView(ImageViewCreateInfo(image));
	}
}
} // namespace impl
} // namespace pvrvk
