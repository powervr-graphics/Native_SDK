/*!
\brief Function definitions for the RenderPass.
\file PVRVk/RenderPassVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/RenderPassVk.h"

namespace pvrvk {
namespace impl {
bool RenderPass_::init(const RenderPassCreateInfo& createInfo)
{
	_createInfo = createInfo;
	VkRenderPassCreateInfo renderPassInfoVk;
	memset(&renderPassInfoVk, 0, sizeof(renderPassInfoVk));
	renderPassInfoVk.sType = VkStructureType::e_RENDER_PASS_CREATE_INFO;
	std::vector<VkAttachmentDescription> attachmentDescVk;
	std::vector<VkSubpassDescription> subPassesVk;
	std::vector<VkSubpassDependency> subPassDependenciesVk;

	//--- Attachments
	renderPassInfoVk.attachmentCount = createInfo.getNumAttachmentDescription();

	attachmentDescVk.resize(renderPassInfoVk.attachmentCount);
	for (uint32_t attachmentIndex = 0; attachmentIndex < createInfo.getNumAttachmentDescription(); ++attachmentIndex)
	{
		const AttachmentDescription& attachmentDesc = createInfo.getAttachmentDescription(attachmentIndex);
		attachmentDescVk[attachmentIndex].flags = (VkAttachmentDescriptionFlags)0;
		attachmentDescVk[attachmentIndex].samples = attachmentDesc.samples;

		if ((attachmentDescVk[attachmentIndex].format = (VkFormat)attachmentDesc.format) == VkFormat::e_UNDEFINED)
		{
			Log("RenderPassVk: Unsupported Color VkFormat");
			return false;
		}
		attachmentDescVk[attachmentIndex].loadOp = attachmentDesc.loadOp;
		attachmentDescVk[attachmentIndex].storeOp = attachmentDesc.storeOp;
		attachmentDescVk[attachmentIndex].initialLayout = attachmentDesc.initialLayout;
		attachmentDescVk[attachmentIndex].finalLayout = attachmentDesc.finalLayout;
		attachmentDescVk[attachmentIndex].stencilLoadOp = attachmentDesc.stencilLoadOp;
		attachmentDescVk[attachmentIndex].stencilStoreOp = attachmentDesc.stencilStoreOp;
	}// next attachment

	renderPassInfoVk.pAttachments = attachmentDescVk.data();

	//--------------------
	//--- Subpass
	renderPassInfoVk.subpassCount = createInfo.getNumSubPasses();
	std::vector<VkAttachmentReference> attachmentReferenceVk;
	//calc the attachmentRefs total sizes
	{
		uint32_t totalNumAttachmentRef = 0;

		for (uint32_t i = 0; i < createInfo.getNumSubPasses(); ++i)
		{
			const SubPassDescription& subpass = createInfo.getSubPass(i);
			totalNumAttachmentRef += subpass.getNumColorAttachmentReference();
			totalNumAttachmentRef += subpass.getNumInputAttachmentReference();
			totalNumAttachmentRef += static_cast<uint32_t>(subpass.getDepthStencilAttachmentReference().layout != VkImageLayout::e_UNDEFINED);
			totalNumAttachmentRef += subpass.getNumResolveAttachmentReference();
			totalNumAttachmentRef += subpass.getNumPreserveAttachmentReference();
		}
		attachmentReferenceVk.resize(totalNumAttachmentRef);
	}
	subPassesVk.resize(renderPassInfoVk.subpassCount);
	uint32_t attachmentOffset = 0;
	for (uint32_t subpassId = 0; subpassId < createInfo.getNumSubPasses(); ++subpassId)
	{
		const SubPassDescription& subpass = createInfo.getSubPass(subpassId);
		VkSubpassDescription subPassVk = {};
		subPassVk.pipelineBindPoint = subpass.getPipelineBindPoint();

		// input attachments
		if (subpass.getNumInputAttachmentReference() > 0)
		{
			subPassVk.pInputAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumInputAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getInputAttachmentReference(j).attachment;
				attachmentReferenceVk[attachmentOffset].layout = (VkImageLayout)subpass.getInputAttachmentReference(j).layout;
				++attachmentOffset;
			}
			subPassVk.inputAttachmentCount = subpass.getNumInputAttachmentReference();
		}

		// Color attachments
		if (subpass.getNumColorAttachmentReference() > 0)
		{
			subPassVk.pColorAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumColorAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getColorAttachmentReference(j).attachment;
				attachmentReferenceVk[attachmentOffset].layout = (VkImageLayout)subpass.getColorAttachmentReference(j).layout;
				++attachmentOffset;
			}
			subPassVk.colorAttachmentCount = subpass.getNumColorAttachmentReference();
		}

		// resolve attachments
		if (subpass.getNumResolveAttachmentReference() > 0)
		{
			subPassVk.pResolveAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumResolveAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getResolveAttachmentReference(j).attachment;
				attachmentReferenceVk[attachmentOffset].layout = (VkImageLayout)subpass.getResolveAttachmentReference(j).layout;
				++attachmentOffset;
			}
		}


		// preserve attachments
		if (subpass.getNumPreserveAttachmentReference() > 0)
		{
			subPassVk.pPreserveAttachments = subpass.getAllPreserveAttachments();
			subPassVk.preserveAttachmentCount = subpass.getNumPreserveAttachmentReference();
		}

		// depth-stencil attachment
		if (subpass.getDepthStencilAttachmentReference().layout != VkImageLayout::e_UNDEFINED)
		{
			subPassVk.pDepthStencilAttachment = &attachmentReferenceVk[attachmentOffset];
			attachmentReferenceVk[attachmentOffset].attachment = subpass.getDepthStencilAttachmentReference().attachment;
			attachmentReferenceVk[attachmentOffset].layout = (VkImageLayout)subpass.getDepthStencilAttachmentReference().layout;
			++attachmentOffset;
		}
		subPassesVk[subpassId] = subPassVk;
	}// next subpass
	renderPassInfoVk.pSubpasses = subPassesVk.data();

	//--- Sub pass dependencies
	renderPassInfoVk.dependencyCount = createInfo.getNumSubPassDependencies();
	subPassDependenciesVk.resize(renderPassInfoVk.dependencyCount);

	for (uint32_t attachmentIndex = 0; attachmentIndex < createInfo.getNumSubPassDependencies(); ++attachmentIndex)
	{
		const SubPassDependency& subPassDependency = createInfo.getSubPassDependency(attachmentIndex);
		VkSubpassDependency subPassDependencyVk;
		memset(&subPassDependencyVk, 0, sizeof(subPassDependencyVk));

		subPassDependencyVk.srcSubpass = subPassDependency.srcSubPass;
		subPassDependencyVk.dstSubpass = subPassDependency.dstSubPass;
		subPassDependencyVk.srcStageMask = subPassDependency.srcStageMask;
		subPassDependencyVk.dstStageMask = subPassDependency.dstStageMask;
		subPassDependencyVk.srcAccessMask = subPassDependency.srcAccessMask;
		subPassDependencyVk.dstAccessMask = subPassDependency.dstAccessMask;
		subPassDependencyVk.dependencyFlags = subPassDependency.dependencyByRegion;

		subPassDependenciesVk[attachmentIndex] = subPassDependencyVk;
	}// next subpass dependency
	renderPassInfoVk.pDependencies = subPassDependenciesVk.data();

	VkResult res = vk::CreateRenderPass(_device->getNativeObject(), &renderPassInfoVk, NULL, &_vkRenderPass);
	if (res != VkResult::e_SUCCESS)
	{
		vkThrowIfFailed(res, "Create RenderPass");
	}
	return (res == VkResult::e_SUCCESS ? true : false);
}

void RenderPass_::destroy()
{
	if (_vkRenderPass != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			vk::DestroyRenderPass(_device->getNativeObject(), _vkRenderPass, NULL);
			_vkRenderPass = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterContext("RenderPass");
		}
	}
}
}

AttachmentDescription AttachmentDescription::createColorDescription(VkFormat format, VkImageLayout initialLayout,
    VkImageLayout finalLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkSampleCountFlags numSamples)
{
	return AttachmentDescription(format, initialLayout, finalLayout, loadOp, storeOp, VkAttachmentLoadOp::e_DONT_CARE,
	                             VkAttachmentStoreOp::e_DONT_CARE, numSamples);
}

AttachmentDescription AttachmentDescription::createDepthStencilDescription(VkFormat format,
    VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp,
    VkSampleCountFlags numSamples)
{
	return AttachmentDescription(format, initialLayout, finalLayout, loadOp, storeOp, stencilLoadOp,
	                             stencilStoreOp, numSamples);
}

}// namespace pvrvk
