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
#include "PVRVk/SyncVk.h"

namespace pvrvk {
namespace impl {
Swapchain_::~Swapchain_()
{
	release();
}

void Swapchain_::release()
{
	VkDevice device = _device->getNativeObject();
	for (uint32_t i = 0; i < _swapChainLength; ++i)
	{
		_colorImageViews[i].reset();
	}

	_createInfo.~SwapchainCreateInfo();

	_device.reset();
	vk::DestroySwapchainKHR(device, _vkSwapchain, nullptr);
	_vkSwapchain = VK_NULL_HANDLE;
}

bool Swapchain_::acquireNextImage(uint64_t timeOut, Semaphore signalSemaphore, Fence signalFence)
{
	if (!vkIsSuccessful(vk::AcquireNextImageKHR(_device->getNativeObject(),
	                    _vkSwapchain, timeOut, signalSemaphore.isValid() ?  signalSemaphore->getNativeObject() : VK_NULL_HANDLE,
	                    signalFence.isValid() ? signalFence->getNativeObject() : VK_NULL_HANDLE, &_swapchainId),
	                    "PlatformContext:PresentBackbuffer AcquireNextImage error"))
	{
		return false;
	}
	return true;
}

bool Swapchain_::init(Surface surface, const SwapchainCreateInfo& createInfo)
{
	_createInfo = createInfo;
	//--- create the swap chain
	VkSwapchainCreateInfoKHR swapchainCreate = {};
	swapchainCreate.sType = VkStructureType::e_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreate.clipped = _createInfo.clipped;
	swapchainCreate.compositeAlpha = _createInfo.compositeAlpha;
	swapchainCreate.surface = surface->getNativeObject();
	swapchainCreate.minImageCount = _createInfo.minImageCount;
	swapchainCreate.imageFormat = _createInfo.imageFormat;
	swapchainCreate.imageArrayLayers = _createInfo.imageArrayLayers;
	swapchainCreate.imageColorSpace = _createInfo.imageColorSpace;
	swapchainCreate.imageExtent.width = _createInfo.imageExtent.width;
	swapchainCreate.imageExtent.height = _createInfo.imageExtent.height;
	swapchainCreate.imageUsage = _createInfo.imageUsage;
	swapchainCreate.preTransform = _createInfo.preTransform;
	swapchainCreate.imageSharingMode = _createInfo.imageSharingMode;
	swapchainCreate.presentMode = _createInfo.presentMode;
	swapchainCreate.queueFamilyIndexCount = _createInfo.numQueueFamilyIndex;

	std::vector<uint32_t> queueFamilyIndices;
	for (uint32_t i = 0; i < swapchainCreate.queueFamilyIndexCount; i++)
	{
		queueFamilyIndices.push_back(_createInfo.queueFamilyIndices[i]);
	}

	swapchainCreate.pQueueFamilyIndices = queueFamilyIndices.data();

	assertion(queueFamilyIndices.size() == _createInfo.numQueueFamilyIndex,
	          "Queue Family index count does not match the number of queue family indices.");

	assertion(swapchainCreate.minImageCount <= static_cast<uint32_t>(FrameworkCaps::MaxSwapChains), "Minimum number of swapchain images is larger than Max set");

	if (!vkIsSuccessful(vk::CreateSwapchainKHR(_device->getNativeObject(), &swapchainCreate,
	                    NULL, &_vkSwapchain), "Could not create the swap chain"))
	{
		return false;
	}

	// get the number of swapchains
	if (!vkIsSuccessful(vk::GetSwapchainImagesKHR(_device->getNativeObject(), _vkSwapchain,
	                    &_swapChainLength, NULL), "Could not get swapchain length"))
	{
		return false;
	}

	Log(LogLevel::Information, "Swapchain image count: %u ", _swapChainLength);

	assertion(_swapChainLength <= static_cast<uint32_t>(FrameworkCaps::MaxSwapChains), "Number of swapchain images is larger than Max set");


	VkImage swapchainImages[static_cast<uint32_t>(FrameworkCaps::MaxSwapChains)];
	if (!vkIsSuccessful(vk::GetSwapchainImagesKHR(_device->getNativeObject(), _vkSwapchain,
	                    &_swapChainLength, &swapchainImages[0]), "Could not get swapchain images"))
	{
		return false;
	}

	for (uint32_t i = 0; i < _swapChainLength; ++i)
	{
		// create the image wrapper
		SwapchainImage image;

		image.construct(_device->getWeakReference());
		image->init(swapchainImages[i], _createInfo.imageFormat,
		            ImageAreaSize(ImageLayersSize(static_cast<uint16_t>(_createInfo.imageArrayLayers), 1),
		                          _createInfo.imageExtent), _createInfo.imageUsage);

		_colorImageViews[i] = _device->createImageView(image);

		if (_colorImageViews[i].isNull())
		{
			Log("Failed to create display Swapchain Image view");
			return false;
		}
	}

	return _initialized = true;
}
}// namespace impl
}// namespace pvrvk