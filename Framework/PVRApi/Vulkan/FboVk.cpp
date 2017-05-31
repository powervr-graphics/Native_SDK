/*!
\brief Vulkan Implementation of the FBO supporting classes (Fbo, Color attachment view etc). See FboVulkan.h.
\file PVRApi/Vulkan/FboVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/Vulkan/FboVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRApi/Vulkan/RenderPassVk.h"

namespace pvr {
namespace api {
namespace impl {

Fbo_::Fbo_(const GraphicsContext& context) : _context(context) {}

Fbo_::Fbo_(const FboCreateParam& desc, GraphicsContext& context) : _context(context)
{
}

}//namespace impl


namespace vulkan {

FboVk_::~FboVk_()
{
	if (_context.isValid())
	{
		destroy();
	}
	else
	{
		Log(Log.Warning, "Attempted to free FBO after corresponding Context was destroyed.");
	}

}
void FboVk_::destroy()
{
<<<<<<< HEAD
	if (m_context.isValid() && native_cast(*this) != VK_NULL_HANDLE)
	{
		vk::DestroyFramebuffer(native_cast(*m_context).getDevice(), native_cast(*this), NULL);
		debugLogApiError("Fbo_::destroy exit");
		handle = VK_NULL_HANDLE;
	}
	m_desc.clear();
=======
	if (_context.isValid() && native_cast(*this) != VK_NULL_HANDLE)
	{
		vk::DestroyFramebuffer(native_cast(*_context).getDevice(), native_cast(*this), NULL);
		handle = VK_NULL_HANDLE;
	}
	_desc.clear();
>>>>>>> 1776432f... 4.3
}

DefaultFboVk_::DefaultFboVk_(const GraphicsContext& context) : FboVk_(context) {}

bool FboVk_::init(const FboCreateParam& desc)
{
	// validate
	assertion(desc.renderPass.isValid(),  "Invalid RenderPass");
	// validate the dimension.
	if (desc.width == 0 || desc.height == 0)
	{
		assertion(false, "Framebuffer with and height must be valid size");
		Log("Invalid Framebuffer Dimension width:%d height:%d", desc.width, desc.height);
		return false;
	}
	platform::ContextVk& contextVk = native_cast(*_context);

	_desc = desc;
	VkFramebufferCreateInfo fboCreateInfo = {};

	fboCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fboCreateInfo.pNext = NULL;
	fboCreateInfo.height = desc.height;
	fboCreateInfo.width = desc.width;
	fboCreateInfo.flags = 0;
	fboCreateInfo.layers = desc.layers;
	fboCreateInfo.renderPass = native_cast(*desc.renderPass);
	fboCreateInfo.attachmentCount = desc.getNumColorAttachements() + desc.getNumDepthStencilAttachments();

	std::vector<VkImageView> imageViews;
	imageViews.resize(fboCreateInfo.attachmentCount);
	fboCreateInfo.pAttachments = imageViews.data();
	uint32 imageViewIndex = 0;
	// do all the color attachments
	for (uint32 i = 0; i < desc.getNumColorAttachements(); ++i)
	{
		imageViews[imageViewIndex++] = static_cast<const TextureViewVk_&>(*desc.getColorAttachment(i));
	}

	// do all the depth stencil attachments
	for (uint32 i = 0; i < desc.getNumDepthStencilAttachments(); ++i)
	{
		imageViews[imageViewIndex++] = static_cast<const TextureViewVk_&>(*desc.getDepthStencilAttachment(i));
	}
	return (vk::CreateFramebuffer(contextVk.getDevice(), &fboCreateInfo, NULL, &handle) == VK_SUCCESS);
}

<<<<<<< HEAD
FboVk_::FboVk_(GraphicsContext& context) : Fbo_(context) {handle = VK_NULL_HANDLE;}
=======
FboVk_::FboVk_(const GraphicsContext& context) : Fbo_(context) {handle = VK_NULL_HANDLE;}
>>>>>>> 1776432f... 4.3

}// namespace vulkan
}// namespace api
}// namespace pvr
