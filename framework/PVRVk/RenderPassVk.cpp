/*!
\brief Function definitions for the RenderPass.
\file PVRVk/RenderPassVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/RenderPassVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
RenderPass_::RenderPass_(make_shared_enabler, const DeviceWeakPtr& device, const RenderPassCreateInfo& createInfo) : PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils()
{
	_createInfo = createInfo;
	VkRenderPassCreateInfo renderPassInfoVk;
	memset(&renderPassInfoVk, 0, sizeof(renderPassInfoVk));
	renderPassInfoVk.sType = static_cast<VkStructureType>(StructureType::e_RENDER_PASS_CREATE_INFO);
	std::vector<VkAttachmentDescription> attachmentDescVk;
	std::vector<VkSubpassDescription> subPassesVk;
	std::vector<VkSubpassDependency> subPassDependenciesVk;

	//--- Attachments
	renderPassInfoVk.attachmentCount = createInfo.getNumAttachmentDescription();

	attachmentDescVk.resize(renderPassInfoVk.attachmentCount);
	for (uint32_t attachmentIndex = 0; attachmentIndex < createInfo.getNumAttachmentDescription(); ++attachmentIndex)
	{
		const AttachmentDescription& attachmentDesc = createInfo.getAttachmentDescription(attachmentIndex);
		attachmentDescVk[attachmentIndex].flags = static_cast<VkAttachmentDescriptionFlags>(AttachmentDescriptionFlags::e_NONE);
		attachmentDescVk[attachmentIndex].samples = static_cast<VkSampleCountFlagBits>(attachmentDesc.getSamples());

		if ((attachmentDescVk[attachmentIndex].format = static_cast<VkFormat>(attachmentDesc.getFormat())) == Format::e_UNDEFINED)
		{
			throw ErrorValidationFailedEXT("RenderPassVk: Unsupported Color VkFormat");
		}
		attachmentDescVk[attachmentIndex].loadOp = static_cast<VkAttachmentLoadOp>(attachmentDesc.getLoadOp());
		attachmentDescVk[attachmentIndex].storeOp = static_cast<VkAttachmentStoreOp>(attachmentDesc.getStoreOp());
		attachmentDescVk[attachmentIndex].initialLayout = static_cast<VkImageLayout>(attachmentDesc.getInitialLayout());
		attachmentDescVk[attachmentIndex].finalLayout = static_cast<VkImageLayout>(attachmentDesc.getFinalLayout());
		attachmentDescVk[attachmentIndex].stencilLoadOp = static_cast<VkAttachmentLoadOp>(attachmentDesc.getStencilLoadOp());
		attachmentDescVk[attachmentIndex].stencilStoreOp = static_cast<VkAttachmentStoreOp>(attachmentDesc.getStencilStoreOp());
	} // next attachment

	renderPassInfoVk.pAttachments = attachmentDescVk.data();

	//--------------------
	//--- Subpass
	renderPassInfoVk.subpassCount = createInfo.getNumSubpasses();
	std::vector<VkAttachmentReference> attachmentReferenceVk;
	// calc the attachmentRefs total sizes
	{
		uint32_t totalNumAttachmentRef = 0;

		for (uint32_t i = 0; i < createInfo.getNumSubpasses(); ++i)
		{
			const SubpassDescription& subpass = createInfo.getSubpass(i);
			totalNumAttachmentRef += subpass.getNumColorAttachmentReference();
			totalNumAttachmentRef += subpass.getNumInputAttachmentReference();
			totalNumAttachmentRef += static_cast<uint32_t>(subpass.getDepthStencilAttachmentReference().getLayout() != ImageLayout::e_UNDEFINED);
			totalNumAttachmentRef += subpass.getNumResolveAttachmentReference();
			totalNumAttachmentRef += subpass.getNumPreserveAttachmentReference();
		}
		attachmentReferenceVk.resize(totalNumAttachmentRef);
	}
	subPassesVk.resize(renderPassInfoVk.subpassCount);
	uint32_t attachmentOffset = 0;
	for (uint32_t subpassId = 0; subpassId < createInfo.getNumSubpasses(); ++subpassId)
	{
		const SubpassDescription& subpass = createInfo.getSubpass(subpassId);
		VkSubpassDescription subPassVk = {};
		subPassVk.pipelineBindPoint = static_cast<VkPipelineBindPoint>(subpass.getPipelineBindPoint());

		// input attachments
		if (subpass.getNumInputAttachmentReference() > 0)
		{
			subPassVk.pInputAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumInputAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getInputAttachmentReference(j).getAttachment();
				attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getInputAttachmentReference(j).getLayout());
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
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getColorAttachmentReference(j).getAttachment();
				attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getColorAttachmentReference(j).getLayout());
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
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getResolveAttachmentReference(j).getAttachment();
				attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getResolveAttachmentReference(j).getLayout());
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
		if (subpass.getDepthStencilAttachmentReference().getLayout() != ImageLayout::e_UNDEFINED)
		{
			subPassVk.pDepthStencilAttachment = &attachmentReferenceVk[attachmentOffset];
			attachmentReferenceVk[attachmentOffset].attachment = subpass.getDepthStencilAttachmentReference().getAttachment();
			attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getDepthStencilAttachmentReference().getLayout());
			++attachmentOffset;
		}
		subPassesVk[subpassId] = subPassVk;
	} // next subpass
	renderPassInfoVk.pSubpasses = subPassesVk.data();

	//--- Sub pass dependencies
	renderPassInfoVk.dependencyCount = createInfo.getNumSubpassDependencies();
	subPassDependenciesVk.resize(renderPassInfoVk.dependencyCount);

	for (uint32_t attachmentIndex = 0; attachmentIndex < createInfo.getNumSubpassDependencies(); ++attachmentIndex)
	{
		const SubpassDependency& subPassDependency = createInfo.getSubpassDependency(attachmentIndex);
		VkSubpassDependency subPassDependencyVk;
		memset(&subPassDependencyVk, 0, sizeof(subPassDependencyVk));

		subPassDependencyVk.srcSubpass = subPassDependency.getSrcSubpass();
		subPassDependencyVk.dstSubpass = subPassDependency.getDstSubpass();
		subPassDependencyVk.srcStageMask = static_cast<VkPipelineStageFlags>(subPassDependency.getSrcStageMask());
		subPassDependencyVk.dstStageMask = static_cast<VkPipelineStageFlags>(subPassDependency.getDstStageMask());
		subPassDependencyVk.srcAccessMask = static_cast<VkAccessFlags>(subPassDependency.getSrcAccessMask());
		subPassDependencyVk.dstAccessMask = static_cast<VkAccessFlags>(subPassDependency.getDstAccessMask());
		subPassDependencyVk.dependencyFlags = static_cast<VkDependencyFlags>(subPassDependency.getDependencyFlags());

		subPassDependenciesVk[attachmentIndex] = subPassDependencyVk;
	} // next subpass dependency
	renderPassInfoVk.pDependencies = subPassDependenciesVk.data();

	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateRenderPass(getDevice()->getVkHandle(), &renderPassInfoVk, NULL, &_vkHandle), "Create RenderPass Failed");
}

RenderPass_::~RenderPass_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyRenderPass(getDevice()->getVkHandle(), getVkHandle(), NULL);
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
