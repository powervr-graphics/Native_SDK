/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/FboVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Vulkan Implementation of the FBO supporting classes (Fbo, Color attachment view etc). See FboVulkan.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/FboVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRApi/Vulkan/RenderPassVk.h"

namespace pvr {
namespace api {
namespace impl {
const native::HFbo_& Fbo_::getNativeObject() const
{
	return native_cast(*this);
}

native::HFbo_& Fbo_::getNativeObject()
{
	return native_cast(*this);
}


Fbo_::Fbo_(GraphicsContext& context) : m_context(context) {}

Fbo_::Fbo_(const FboCreateParam& desc, GraphicsContext& context) : m_context(context)
{
}

void Fbo_::destroy()
{
	if (m_context.isValid())
	{
		vk::DestroyFramebuffer(native_cast(*m_context).getDevice(), native_cast(*this), NULL);
		debugLogApiError("Fbo_::destroy exit");
		getNativeObject() = VK_NULL_HANDLE;
	}
	else
	{
		Log(Log.Warning, "FBO object was not cleaned up before context destruction");
	}
}
}//namespace impl


namespace vulkan {

DefaultFboVk_::DefaultFboVk_(GraphicsContext& context) : FboVk_(context) {}

bool FboVk_::init(const FboCreateParam& desc)
{
	// validate
	assertion(desc.renderPass.isValid() ,  "Invalid RenderPass");
	// validate the dimension.
	if (desc.width == 0 || desc.height == 0)
	{
		Log("Invalid Framebuffer Dimension width:%d height:%d", desc.width, desc.height);
		return false;
	}
	platform::ContextVk& contextVk = native_cast(*m_context);

	m_desc = desc;
	VkFramebufferCreateInfo fboCreateInfo = {};

	fboCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fboCreateInfo.pNext = NULL;
	fboCreateInfo.height = desc.height;
	fboCreateInfo.width = desc.width;
	fboCreateInfo.flags = 0;
	fboCreateInfo.layers = desc.layers;
	fboCreateInfo.renderPass = native_cast(*desc.renderPass);
	fboCreateInfo.attachmentCount = ((uint32)desc.colorViews.size() + (desc.depthStencilView.isValid() != 0));

	std::vector<VkImageView> imageViews;
	imageViews.resize(fboCreateInfo.attachmentCount);
	fboCreateInfo.pAttachments = imageViews.data();

	for (uint32 i = 0; i < desc.colorViews.size(); ++i)
	{
		imageViews[i] = *desc.colorViews[i]->getNativeObject();
	}

	if (desc.depthStencilView.isValid())
	{
		imageViews.back() = desc.depthStencilView->getNativeObject();
	}
	return (vk::CreateFramebuffer(contextVk.getDevice(), &fboCreateInfo, NULL, &handle) == VK_SUCCESS);
}

FboVk_::FboVk_(GraphicsContext& context) : Fbo_(context) {}

}// namespace vulkan
}// namespace api
}// namespace pvr
//!\endcond
