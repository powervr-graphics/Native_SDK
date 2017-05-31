/*!
\brief Definitions of the Vulkan implementation of the RenderPass.
\file PVRApi/Vulkan/RenderPassVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/Vulkan/RenderPassVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include <vector>
namespace pvr {
namespace api {
namespace vulkan {
using namespace ::pvr::nativeVk;
bool RenderPassVk_::init(const RenderPassCreateParam& createParam)
{
	_createParam = createParam;
	platform::ContextVk& contextVk = native_cast(*getContext());

	VkRenderPassCreateInfo renderPassInfoVk;
	memset(&renderPassInfoVk, 0, sizeof(renderPassInfoVk));
	renderPassInfoVk.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	std::vector<VkAttachmentDescription> attachmentDescVk;
	std::vector<VkSubpassDescription> subPassesVk;
	std::vector<VkSubpassDependency> subPassDependenciesVk;

	//--- Attachments
	renderPassInfoVk.attachmentCount = createParam.getNumColorInfo() + createParam.getNumDepthStencilInfo();

	attachmentDescVk.resize(renderPassInfoVk.attachmentCount);
	pvr::uint32 attachmentIndex = 0;
	for (; attachmentIndex < createParam.getNumColorInfo(); ++attachmentIndex)
	{
		const RenderPassColorInfo& colorInfo = createParam.getColorInfo(attachmentIndex);
		attachmentDescVk[attachmentIndex].flags = 0;
		attachmentDescVk[attachmentIndex].samples = ConvertToVk::aaSamples((uint8)colorInfo.numSamples);

		if ((attachmentDescVk[attachmentIndex].format = ConvertToVk::pixelFormat(colorInfo.format.format,
		     colorInfo.format.colorSpace, colorInfo.format.dataType)) == VK_FORMAT_UNDEFINED)
		{
			Log("RenderPassVk: Unsupported Color Format");
			return false;
		}
		VkFormatProperties prop;

		vk::GetPhysicalDeviceFormatProperties(contextVk.getPhysicalDevice(), attachmentDescVk[attachmentIndex].format, &prop);
		if (!((prop.bufferFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) ||
		      (prop.bufferFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)))
		{
			//Log("Device not support this Format for color attachment");
			//return false;
		}

		attachmentDescVk[attachmentIndex].loadOp = ConvertToVk::loadOp(colorInfo.loadOpColor);
		attachmentDescVk[attachmentIndex].storeOp = ConvertToVk::storeOp(colorInfo.storeOpColor);
		attachmentDescVk[attachmentIndex].initialLayout = ConvertToVk::imageLayout(colorInfo.initialLayout);
		attachmentDescVk[attachmentIndex].finalLayout = ConvertToVk::imageLayout(colorInfo.finalLayout);
		// Not relevent for color attachment
		attachmentDescVk[attachmentIndex].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescVk[attachmentIndex].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}// next color attachment

	//--- Depth Stencil Attachments
	uint32 depthStencilAttachmentBeginIndex = createParam.getNumColorInfo();
	for (uint32 j = 0; j < createParam.getNumDepthStencilInfo(); ++j, ++attachmentIndex)
	{
		const RenderPassDepthStencilInfo& depthStencilInfo = createParam.getDepthStencilInfo(j);
		attachmentDescVk[attachmentIndex].flags = 0;
		attachmentDescVk[attachmentIndex].samples = ConvertToVk::aaSamples((uint8)depthStencilInfo.numSamples);

		if ((attachmentDescVk[attachmentIndex].format = ConvertToVk::pixelFormat(depthStencilInfo.format.format,
		     depthStencilInfo.format.colorSpace, depthStencilInfo.format.dataType)) == VK_FORMAT_UNDEFINED)
		{
			Log("Unsupported depth-stencil Format");
			return false;
		}

		// validate the format feature
		VkFormatProperties prop;
		vk::GetPhysicalDeviceFormatProperties(contextVk.getPhysicalDevice(), attachmentDescVk[attachmentIndex].format, &prop);

		if (!(prop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
		{
			Log("Device not support this Format for depth-stencil attachment");
			return false;
		}

		attachmentDescVk[attachmentIndex].loadOp = ConvertToVk::loadOp(depthStencilInfo.loadOpDepth);
		attachmentDescVk[attachmentIndex].storeOp = ConvertToVk::storeOp(depthStencilInfo.storeOpDepth);
		attachmentDescVk[attachmentIndex].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescVk[attachmentIndex].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescVk[attachmentIndex].stencilLoadOp =  ConvertToVk::loadOp(depthStencilInfo.loadOpStencil);
		attachmentDescVk[attachmentIndex].stencilStoreOp =  ConvertToVk::storeOp(depthStencilInfo.storeOpStencil);
	}
	renderPassInfoVk.pAttachments = attachmentDescVk.data();

	//--- Subpass
	renderPassInfoVk.subpassCount = createParam.getNumSubPass();

	std::vector<VkAttachmentReference> colorAttachmentRefs;
	std::vector<VkAttachmentReference> inputAttachmentsRefs;
	std::vector<VkAttachmentReference> resolveAttachmentsRefs;
	std::vector<VkAttachmentReference> depthStencilAttachmentRefs;
	std::vector<pvr::uint32> preserveAttachments;
	{
		//calc the attachmentRefs total sizes
		uint32 colorAttachmentRefSize = 0;
		uint32 inputAttachmentRefSize = 0;
		uint32 resolveAttachmentRefSize = 0;
		uint32 preserveAttachmentRefSize = 0;
		uint32 depthStencilAttachmentRefSize = 0;

		for (uint32 i = 0; i < createParam.getNumSubPass(); ++i)
		{
			const SubPass& subPass = createParam.getSubPass(i);
			colorAttachmentRefSize += subPass.getNumColorAttachment();
			inputAttachmentRefSize += subPass.getNumInputAttachment();
			depthStencilAttachmentRefSize +=  subPass.usesDepthStencilAttachment();
			resolveAttachmentRefSize += subPass.getNumResolveColorAttachment() + subPass.getNumResolveDepthStencilAttachment();
			preserveAttachmentRefSize += subPass.getNumPreserveAttachment();
		}
		colorAttachmentRefs.resize(colorAttachmentRefSize);
		inputAttachmentsRefs.resize(inputAttachmentRefSize);
		resolveAttachmentsRefs.resize(resolveAttachmentRefSize);
		preserveAttachments.resize(preserveAttachmentRefSize);
		depthStencilAttachmentRefs.resize(depthStencilAttachmentRefSize);
	}
	subPassesVk.resize(renderPassInfoVk.subpassCount);
	uint32 colorAttachmentRefOffset = 0;
	uint32 inputAttachmentRefOffset = 0;
	uint32 resolveAttachmentRefOffset = 0;
	uint32 preserveAttachmentRefOffset = 0;
	uint32 dsAttachmentRefOffset = 0;
	for (uint32 subpassId = 0; subpassId < createParam.getNumSubPass(); ++subpassId)
	{
		const SubPass& subPass = createParam.getSubPass(subpassId);
		VkSubpassDescription subPassVk = {};
		subPassVk.pipelineBindPoint = ConvertToVk::pipelineBindPoint(subPass.getPipelineBindPoint());

		// input attachments
		if (subPass.getNumInputAttachment() > 0)
		{
			subPassVk.pInputAttachments = &inputAttachmentsRefs[inputAttachmentRefOffset];

			for (pvr::uint8 j = 0; j < subPass.getNumInputAttachment(); ++j)
			{
				inputAttachmentsRefs[inputAttachmentRefOffset].attachment = subPass.getInputAttachmentId(j);
				inputAttachmentsRefs[inputAttachmentRefOffset].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				++inputAttachmentRefOffset;
			}
			subPassVk.inputAttachmentCount = subPass.getNumInputAttachment();
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


		if (subPass.getNumResolveColorAttachment() > 0 ||
		    subPass.getNumResolveDepthStencilAttachment() > 0)
		{
			subPassVk.pResolveAttachments = &resolveAttachmentsRefs[resolveAttachmentRefOffset];
		}
		// resolve color attachments
		if (subPass.getNumResolveColorAttachment() > 0)
		{
			assertion(subPass.getNumResolveColorAttachment() == subPass.getNumColorAttachment(),
			          "If the number of resolve attachments is not 0 then it must be same as number of color attachment entries");

			for (pvr::uint8 j = 0; j < subPass.getNumResolveColorAttachment(); ++j)
			{
				resolveAttachmentsRefs[resolveAttachmentRefOffset].attachment = subPass.getResolveColorAttachmentId(j);
				resolveAttachmentsRefs[resolveAttachmentRefOffset].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				++resolveAttachmentRefOffset;
			}
		}

		// resolve depth stencincil attachments
		if (subPass.getNumResolveDepthStencilAttachment() > 0)
		{
			for (pvr::uint8 j = 0; j < subPass.getNumResolveDepthStencilAttachment(); ++j)
			{
				resolveAttachmentsRefs[resolveAttachmentRefOffset].attachment =
				  subPass.getResolveDepthStencilAttachmentId(j) + depthStencilAttachmentBeginIndex;

				resolveAttachmentsRefs[resolveAttachmentRefOffset].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				++resolveAttachmentRefOffset;
			}
		}


		// preserve attachments
		if (subPass.getNumPreserveAttachment() > 0)
		{
			subPassVk.pPreserveAttachments = &preserveAttachments[preserveAttachmentRefOffset];

			for (pvr::uint8 j = 0; j < subPass.getNumPreserveAttachment(); ++j)
			{
				preserveAttachments[preserveAttachmentRefOffset] = subPass.getPreserveAttachmentId(j);
				++preserveAttachmentRefOffset;
			}
			subPassVk.preserveAttachmentCount = subPass.getNumPreserveAttachment();
		}

		// depth-stencil attachment
		if (subPass.usesDepthStencilAttachment())
		{
			subPassVk.pDepthStencilAttachment = &depthStencilAttachmentRefs[dsAttachmentRefOffset];
			depthStencilAttachmentRefs[dsAttachmentRefOffset].attachment =
			  depthStencilAttachmentBeginIndex + subPass.getDepthStencilAttachmentId();
			depthStencilAttachmentRefs[dsAttachmentRefOffset].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			++dsAttachmentRefOffset;
		}
		subPassesVk[subpassId] = subPassVk;
	}// next subpass
	renderPassInfoVk.pSubpasses = subPassesVk.data();

//--- Sub pass dependencies
	renderPassInfoVk.dependencyCount = createParam.getNumSubPassDependencies();
	subPassDependenciesVk.resize(renderPassInfoVk.dependencyCount);

	for (attachmentIndex = 0; attachmentIndex < createParam.getNumSubPassDependencies(); ++attachmentIndex)
	{
		const SubPassDependency& subPassDependency = createParam.getSubPassDependency(attachmentIndex);
		VkSubpassDependency subPassDependencyVk;
		memset(&subPassDependencyVk, 0, sizeof(subPassDependencyVk));

		subPassDependencyVk.srcSubpass = subPassDependency.srcSubPass;
		subPassDependencyVk.dstSubpass = subPassDependency.dstSubPass;
		subPassDependencyVk.srcStageMask = (uint32)subPassDependency.srcStageMask;
		subPassDependencyVk.dstStageMask = (uint32)subPassDependency.dstStageMask;
		subPassDependencyVk.srcAccessMask = (uint32)subPassDependency.srcAccessMask;
		subPassDependencyVk.dstAccessMask = (uint32)subPassDependency.dstAccessMask;
		subPassDependencyVk.dependencyFlags = subPassDependency.dependencyByRegion;

		subPassDependenciesVk[attachmentIndex] = subPassDependencyVk;
	}// next subpass dependency
	renderPassInfoVk.pDependencies = subPassDependenciesVk.data();

	VkResult res = vk::CreateRenderPass(native_cast(getContext())->getDevice(), &renderPassInfoVk, NULL, &handle);
	if (res != VK_SUCCESS)
	{
		vkThrowIfFailed(res, "Create RenderPass");
	}
	return (res == VK_SUCCESS ? true : false);
}

void RenderPassVk_::destroy()
{
	if (_context.isValid())
	{
		if (handle != VK_NULL_HANDLE)
		{
			vk::DestroyRenderPass(native_cast(getContext())->getDevice(), handle, NULL);
			handle = VK_NULL_HANDLE;
		}
	}
	else
	{
		reportDestroyedAfterContext("RenderPass");
	}
	_context.reset();
}

RenderPassVk_::~RenderPassVk_()
{
	if (_context.isValid())
	{
		destroy();
	}
}
}// namespace vulkan

}// namespace api
}// namespace pvr
