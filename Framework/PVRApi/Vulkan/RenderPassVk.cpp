/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/RenderPassVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Definitions of the Vulkan implementation of the RenderPass.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/RenderPassVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include <vector>
namespace pvr {
namespace api {

namespace vulkan {
bool RenderPassVk_::init(const RenderPassCreateParam& createParam)
{
	VkRenderPassCreateInfo renderPassInfoVk;
	memset(&renderPassInfoVk, 0, sizeof(renderPassInfoVk));
	renderPassInfoVk.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	std::vector<VkAttachmentDescription> attachmentDescVk;
	std::vector<VkSubpassDescription> subPassesVk;
	std::vector<VkSubpassDependency> subPassDependenciesVk;

	//--- Color Attachments
	renderPassInfoVk.attachmentCount = createParam.getNumColorInfo() + (createParam.getDepthStencilInfo().format.format != PixelFormat::Unknown ? 1 : 0);
	attachmentDescVk.resize(renderPassInfoVk.attachmentCount);
	pvr::uint32 i = 0;
	for (; i < createParam.getNumColorInfo(); ++i)
	{
		const RenderPassColorInfo& colorInfo = createParam.getColorInfo(i);
		attachmentDescVk[i].flags = 0;
		attachmentDescVk[i].samples = ConvertToVk::aaSamples((uint8)colorInfo.numSamples);
		if ((attachmentDescVk[i].format = ConvertToVk::pixelFormat(colorInfo.format.format, colorInfo.format.colorSpace, colorInfo.format.dataType)) == VK_FORMAT_UNDEFINED)
		{
			Log("Unsupported Color Format");
			return false;
		}
		attachmentDescVk[i].loadOp = ConvertToVk::loadOp(colorInfo.loadOpColor);
		attachmentDescVk[i].storeOp = ConvertToVk::storeOp(colorInfo.storeOpColor);
		attachmentDescVk[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDescVk[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// Not relevent for color attachment
		attachmentDescVk[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescVk[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}// next color attachment

	//--- Depth Stencil Attachments
	if (createParam.getDepthStencilInfo().format.format != PixelFormat::Unknown)
	{
		const RenderPassDepthStencilInfo& depthStencilInfo = createParam.getDepthStencilInfo();
		attachmentDescVk[i].flags = 0 /*VkAttachmentDescriptionFlagBits::VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT*/;
		attachmentDescVk[i].samples = ConvertToVk::aaSamples((uint8)depthStencilInfo.numSamples);
		if ((attachmentDescVk[i].format = ConvertToVk::pixelFormat(depthStencilInfo.format.format, depthStencilInfo.format.colorSpace, depthStencilInfo.format.dataType)) == VK_FORMAT_UNDEFINED)
		{
			Log("Unsupported Color Format");
			return false;
		}
		attachmentDescVk[i].loadOp = ConvertToVk::loadOp(depthStencilInfo.loadOpDepth);
		attachmentDescVk[i].storeOp = ConvertToVk::storeOp(depthStencilInfo.storeOpDepth);
		attachmentDescVk[i].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescVk[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescVk[i].stencilLoadOp =  ConvertToVk::loadOp(depthStencilInfo.loadOpStencil);
		attachmentDescVk[i].stencilStoreOp =  ConvertToVk::storeOp(depthStencilInfo.storeOpStencil);
	}
	renderPassInfoVk.pAttachments = attachmentDescVk.data();

	//--- Subpass
	renderPassInfoVk.subpassCount = createParam.getNumSubPass();
	
	std::vector<VkAttachmentReference> colorAttachmentRefs;
	std::vector<VkAttachmentReference> inputAttachmentsRefs;
	std::vector<VkAttachmentReference> resolveAttachmentsRefs;
	std::vector<pvr::uint32> preserverAttachments;
	{
		//calc the attachmentRefs total sizes
		int colorAttachmentRefSize = 0;
		int inputAttachmentRefSize = 0;
		int resolveAttachmentRefSize = 0;
		int preserveAttachmentRefSize = 0;

		for (auto it = createParam.subPass.begin(); it != createParam.subPass.end(); ++it)
		{
			colorAttachmentRefSize += it->getNumColorAttachment();
			colorAttachmentRefSize += 1;// one for depth stencil attachment

			inputAttachmentRefSize += it->getNumInputAttachment();
			resolveAttachmentRefSize += it->getNumResolveAttachment();
			preserveAttachmentRefSize += it->getNumPreserveAttachment();
		}

		colorAttachmentRefs.resize(colorAttachmentRefSize);
		inputAttachmentsRefs.resize(inputAttachmentRefSize);
		resolveAttachmentsRefs.resize(resolveAttachmentRefSize);
		preserverAttachments.resize(preserveAttachmentRefSize);
	}
	subPassesVk.resize(renderPassInfoVk.subpassCount);
	uint32 colorAttachmentRefOffset = 0;
	uint32 inputAttachmentRefOffset = 0;
	uint32 resolveAttachmentRefOffset = 0;
	uint32 preserveAttachmentRefOffset = 0;

	for (i = 0; i < createParam.getNumSubPass(); ++i)
	{
		const SubPass& subPass = createParam.getSubPass(i);
		VkSubpassDescription subPassVk;
		memset(&subPassVk, 0, sizeof(subPassVk));

		// input attachments
		if (subPass.getNumInputAttachment() > 0)
		{
			subPassVk.pInputAttachments = &inputAttachmentsRefs[inputAttachmentRefOffset];

			for (pvr::uint8 j = 0; j < subPass.getNumInputAttachment(); ++j)
			{
				inputAttachmentsRefs[inputAttachmentRefOffset].attachment = subPass.getInputAttachmentId(j);
				inputAttachmentsRefs[inputAttachmentRefOffset].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				++inputAttachmentRefOffset;
			}
			subPassVk.inputAttachmentCount = subPass.getNumInputAttachment();
		}
		else
		{
			subPassVk.pInputAttachments = NULL;
			subPassVk.inputAttachmentCount = 0;
		}

		// Color attachments
		subPassVk.pColorAttachments = &colorAttachmentRefs[colorAttachmentRefOffset];
		for (pvr::uint8 j = 0; j < subPass.getNumColorAttachment(); ++j)
		{
			colorAttachmentRefs[colorAttachmentRefOffset].attachment = subPass.getColorAttachmentId(j);
			colorAttachmentRefs[colorAttachmentRefOffset].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			++colorAttachmentRefOffset;
		}
		subPassVk.colorAttachmentCount = subPass.getNumColorAttachment();

		// resolve attachments
		if (subPass.getNumResolveAttachment() > 0)
		{
			assertion(subPass.getNumResolveAttachment() == subPass.getNumColorAttachment(), " If the number of resolve attachments is not 0 then it must have colorAttachmentCount entries");
			
			subPassVk.pResolveAttachments = &resolveAttachmentsRefs[resolveAttachmentRefOffset];

			for (pvr::uint8 j = 0; j < subPass.getNumResolveAttachment(); ++j)
			{
				resolveAttachmentsRefs[resolveAttachmentRefOffset].attachment = subPass.getResolveAttachmentId(j);
				resolveAttachmentsRefs[resolveAttachmentRefOffset].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				++resolveAttachmentRefOffset;
			}
		}
		else
		{
			subPassVk.pResolveAttachments = NULL;
		}

		// preserve attachments
		if (subPass.getNumPreserveAttachment() > 0)
		{
			subPassVk.pPreserveAttachments = &preserverAttachments[preserveAttachmentRefOffset];

			for (pvr::uint8 j = 0; j < subPass.getNumPreserveAttachment(); ++j)
			{
				preserverAttachments[preserveAttachmentRefOffset] = subPass.getPreserveAttachmentId(j);
				++preserveAttachmentRefOffset;
			}
			subPassVk.preserveAttachmentCount = subPass.getNumPreserveAttachment();
		}
		else
		{
			subPassVk.pPreserveAttachments = NULL;
			subPassVk.preserveAttachmentCount = 0;
		}

		// depth-stencil attachment
		if (createParam.getDepthStencilInfo().format.format != PixelFormat::Unknown)
		{
			subPassVk.pDepthStencilAttachment = &colorAttachmentRefs[colorAttachmentRefOffset];
			colorAttachmentRefs[colorAttachmentRefOffset].attachment = (uint32)(attachmentDescVk.size() - 1);
			colorAttachmentRefs[colorAttachmentRefOffset].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			++colorAttachmentRefOffset;
		}
		subPassesVk[i] = subPassVk;
	}// next subpass
	renderPassInfoVk.pSubpasses = subPassesVk.data();

	//--- Sub pass dependencies
	renderPassInfoVk.dependencyCount = createParam.getNumSubPassDependencies();
	subPassDependenciesVk.resize(renderPassInfoVk.dependencyCount);

	for (i = 0; i < createParam.getNumSubPassDependencies(); ++i)
	{
		const SubPassDependency& subPassDependency = createParam.getSubPassDependency(i);
		VkSubpassDependency subPassDependencyVk;
		memset(&subPassDependencyVk, 0, sizeof(subPassDependencyVk));

		subPassDependencyVk.srcSubpass = subPassDependency.srcSubPass;
		subPassDependencyVk.dstSubpass = subPassDependency.dstSubPass;
		subPassDependencyVk.srcStageMask = subPassDependency.srcStageMask;
		subPassDependencyVk.dstStageMask = subPassDependency.dstStageMask;
		subPassDependencyVk.srcAccessMask = subPassDependency.srcAccessMask;
		subPassDependencyVk.dstAccessMask = subPassDependency.dstAccessMask;
		subPassDependencyVk.dependencyFlags = subPassDependency.dependencyByRegion;

		subPassDependenciesVk[i] = subPassDependencyVk;
	}// next subpass dependency
	renderPassInfoVk.pDependencies = subPassDependenciesVk.data();
	renderPassInfoVk.dependencyCount = createParam.getNumSubPassDependencies();

	VkResult res = vk::CreateRenderPass(native_cast(getContext())->getDevice(), &renderPassInfoVk, NULL, &handle);
	vkThrowIfFailed(res, "Create RenderPass");
	return (res == VK_SUCCESS ? true : false);
}
}//	namespace vulkan

namespace impl {
const native::HRenderPass_& RenderPass_::getNativeObject() const { return native_cast(*this); }
native::HRenderPass_& RenderPass_::getNativeObject() { return native_cast(*this); }
void RenderPass_::destroy()
{
	vk::DestroyRenderPass(native_cast(getContext())->getDevice(), getNativeObject(), NULL);
	getNativeObject() = VK_NULL_HANDLE;
}
}// namespace impl
}// namespace api
}// namespace pvr
//!\endcond
