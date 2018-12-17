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
Framebuffer_::~Framebuffer_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroyFramebuffer(_device->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterDevice("Framebuffer");
		}
	}
	_createInfo.clear();
}

Framebuffer_::Framebuffer_(const DeviceWeakPtr& device, const FramebufferCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_FRAMEBUFFER_EXT)
{
	// validate
	assertion(createInfo.getRenderPass().isValid(), "Invalid RenderPass");
	// validate the dimension.
	if (createInfo.getDimensions().getWidth() == 0 || createInfo.getDimensions().getHeight() == 0)
	{
		throw ErrorValidationFailedEXT("Framebuffer with and height must be valid size");
	}

	_createInfo = createInfo;
	VkFramebufferCreateInfo framebufferCreateInfo = {};

	framebufferCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_FRAMEBUFFER_CREATE_INFO);
	framebufferCreateInfo.width = createInfo.getDimensions().getWidth();
	framebufferCreateInfo.height = createInfo.getDimensions().getHeight();
	framebufferCreateInfo.layers = createInfo.getLayers();
	framebufferCreateInfo.renderPass = createInfo.getRenderPass()->getVkHandle();
	framebufferCreateInfo.attachmentCount = createInfo.getNumAttachments();

	std::vector<VkImageView> imageViews;
	imageViews.resize(framebufferCreateInfo.attachmentCount);
	framebufferCreateInfo.pAttachments = imageViews.data();
	// do all the color attachments
	for (uint32_t i = 0; i < createInfo.getNumAttachments(); ++i)
	{
		imageViews[i] = createInfo.getAttachment(i)->getVkHandle();
	}
	vkThrowIfFailed(_device->getVkBindings().vkCreateFramebuffer(_device->getVkHandle(), &framebufferCreateInfo, NULL, &_vkHandle), "Create Framebuffer Failed");
}
} // namespace impl
} // namespace pvrvk
