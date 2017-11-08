/*!
\brief Function implementations for the Frambuffer Object class
\file PVRVk/FramebufferVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/FramebufferVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/RenderPassVk.h"

namespace pvrvk {
namespace impl {
void Framebuffer_::destroy()
{
	if (_vkFramebuffer != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			vk::DestroyFramebuffer(_device->getNativeObject(), _vkFramebuffer, nullptr);
			_vkFramebuffer = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterContext("Framebuffer");
		}
	}
	_createInfo.clear();
}

bool Framebuffer_::init(const FramebufferCreateInfo& createInfo)
{
	// validate
	assertion(createInfo.renderPass.isValid(),  "Invalid RenderPass");
	// validate the dimension.
	if (createInfo.width == 0 || createInfo.height == 0)
	{
		assertion(false, "Framebuffer with and height must be valid size");
		Log("Invalid Framebuffer Dimension width:%d height:%d", createInfo.width, createInfo.height);
		return false;
	}

	_createInfo = createInfo;
	VkFramebufferCreateInfo framebufferCreateInfo = {};

	framebufferCreateInfo.sType = VkStructureType::e_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.height = createInfo.height;
	framebufferCreateInfo.width = createInfo.width;
	framebufferCreateInfo.layers = createInfo.layers;
	framebufferCreateInfo.renderPass = createInfo.renderPass->getNativeObject();
	framebufferCreateInfo.attachmentCount = createInfo.getNumAttachments();

	std::vector<VkImageView> imageViews;
	imageViews.resize(framebufferCreateInfo.attachmentCount);
	framebufferCreateInfo.pAttachments = imageViews.data();
	// do all the color attachments
	for (uint32_t i = 0; i < createInfo.getNumAttachments(); ++i)
	{
		imageViews[i] = createInfo.getAttachment(i)->getNativeObject();
	}
	return (vk::CreateFramebuffer(_device->getNativeObject(), &framebufferCreateInfo, NULL, &_vkFramebuffer) == VkResult::e_SUCCESS);
}
}// namespace vulkan
}// namespace pvrvk
