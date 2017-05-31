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
<<<<<<< HEAD
	platform::NativePlatformHandles_& platformHandle = getContext()->getPlatformContext().getNativePlatformHandles();
=======
	_createParam = createParam;
	platform::ContextVk& contextVk = native_cast(*getContext());
>>>>>>> 1776432f... 4.3

	VkRenderPassCreateInfo renderPassInfoVk;
	memset(&renderPassInfoVk, 0, sizeof(renderPassInfoVk));
	renderPassInfoVk.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	std::vector<VkAttachmentDescription> attachmentDescVk;
	std::vector<VkSubpassDescription> subPassesVk;
	std::vector<VkSubpassDependency> subPassDependenciesVk;

<<<<<<< HEAD
	//--- Color Attachments
	renderPassInfoVk.attachmentCount = createParam.getNumColorInfo() +
	                                   (createParam.getDepthStencilInfo().format.format != PixelFormat::Unknown ? 1 : 0);
=======
	//--- Attachments
	renderPassInfoVk.attachmentCount = createParam.getNumColorInfo() + createParam.getNumDepthStencilInfo();
>>>>>>> 1776432f... 4.3

	attachmentDescVk.resize(renderPassInfoVk.attachmentCount);
	pvr::uint32 attachmentIndex = 0;
	for (; attachmentIndex < createParam.getNumColorInfo(); ++attachmentIndex)
	{
<<<<<<< HEAD
		const RenderPassColorInfo& colorInfo = createParam.getColorInfo(i);
		attachmentDescVk[i].flags = 0;
		attachmentDescVk[i].samples = ConvertToVk::aaSamples((uint8)colorInfo.numSamples);

		if ((attachmentDescVk[i].format = ConvertToVk::pixelFormat(colorInfo.format.format,
		                                  colorInfo.format.colorSpace, colorInfo.format.dataType)) == VK_FORMAT_UNDEFINED)
		{
            Log("RenderPassVk: Unsupported Color Format");
			return false;
		}
		VkFormatProperties prop;
		vk::GetPhysicalDeviceFormatProperties(platformHandle.context.physicalDevice, attachmentDescVk[i].format, &prop);
=======
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
>>>>>>> 1776432f... 4.3
		if (!((prop.bufferFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) ||
		      (prop.bufferFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)))
		{
			//Log("Device not support this Format for color attachment");
			//return false;
		}

<<<<<<< HEAD
		attachmentDescVk[i].loadOp = ConvertToVk::loadOp(colorInfo.loadOpColor);
		attachmentDescVk[i].storeOp = ConvertToVk::storeOp(colorInfo.storeOpColor);
		attachmentDescVk[i].initialLayout = ConvertToVk::imageLayout(colorInfo.initialLayout);
		attachmentDescVk[i].finalLayout = ConvertToVk::imageLayout(colorInfo.finalLayout);
=======
		attachmentDescVk[attachmentIndex].loadOp = ConvertToVk::loadOp(colorInfo.loadOpColor);
		attachmentDescVk[attachmentIndex].storeOp = ConvertToVk::storeOp(colorInfo.storeOpColor);
		attachmentDescVk[attachmentIndex].initialLayout = ConvertToVk::imageLayout(colorInfo.initialLayout);
		attachmentDescVk[attachmentIndex].finalLayout = ConvertToVk::imageLayout(colorInfo.finalLayout);
>>>>>>> 1776432f... 4.3
		// Not relevent for color attachment
		attachmentDescVk[attachmentIndex].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescVk[attachmentIndex].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}// next color attachment

	//--- Depth Stencil Attachments
	uint32 depthStencilAttachmentBeginIndex = createParam.getNumColorInfo();
	for (uint32 j = 0; j < createParam.getNumDepthStencilInfo(); ++j, ++attachmentIndex)
	{
<<<<<<< HEAD
		const RenderPassDepthStencilInfo& depthStencilInfo = createParam.getDepthStencilInfo();
		attachmentDescVk[i].flags = 0 /*VkAttachmentDescriptionFlagBits::VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT*/;
		attachmentDescVk[i].samples = ConvertToVk::aaSamples((uint8)depthStencilInfo.numSamples);

		if ((attachmentDescVk[i].format = ConvertToVk::pixelFormat(depthStencilInfo.format.format,
		                                  depthStencilInfo.format.colorSpace, depthStencilInfo.format.dataType)) == VK_FORMAT_UNDEFINED)
		{
			Log("Unsupported depth-stencil Format");
			return false;
		}

		// validate the format feature
		VkFormatProperties prop;
		vk::GetPhysicalDeviceFormatProperties(platformHandle.context.physicalDevice, attachmentDescVk[i].format, &prop);

		if (!(prop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
		{
			Log("Device not support this Format for depth-stencil attachment");
			return false;
		}

		attachmentDescVk[i].loadOp = ConvertToVk::loadOp(depthStencilInfo.loadOpDepth);
		attachmentDescVk[i].storeOp = ConvertToVk::storeOp(depthStencilInfo.storeOpDepth);
		attachmentDescVk[i].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescVk[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescVk[i].stencilLoadOp =  ConvertToVk::loadOp(depthStencilInfo.loadOpStencil);
		attachmentDescVk[i].stencilStoreOp =  ConvertToVk::storeOp(depthStencilInfo.storeOpStencil);
=======
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
>>>>>>> 1776432f... 4.3
	}
	renderPassInfoVk.pAttachments = attachmentDescVk.data();

	//--- Subpass
	renderPassInfoVk.subpassCount = createParam.getNumSubPass();

	std::vector<VkAttachmentReference> colorAttachmentRefs;
	std::vector<VkAttachmentReference> inputAttachmentsRefs;
	std::vector<VkAttachmentReference> resolveAttachmentsRefs;
<<<<<<< HEAD
	std::vector<pvr::uint32> preserveAttachments;
	{
		//calc the attachmentRefs total sizes
		int32 colorAttachmentRefSize = 0;
		int32 inputAttachmentRefSize = 0;
		int32 resolveAttachmentRefSize = 0;
		int32 preserveAttachmentRefSize = 0;
=======
	std::vector<VkAttachmentReference> depthStencilAttachmentRefs;
	std::vector<pvr::uint32> preserveAttachments;
	{
		//calc the attachmentRefs total sizes
		uint32 colorAttachmentRefSize = 0;
		uint32 inputAttachmentRefSize = 0;
		uint32 resolveAttachmentRefSize = 0;
		uint32 preserveAttachmentRefSize = 0;
		uint32 depthStencilAttachmentRefSize = 0;
>>>>>>> 1776432f... 4.3

		for (uint32 i = 0; i < createParam.getNumSubPass(); ++i)
		{
			const SubPass& subPass = createParam.getSubPass(i);
			colorAttachmentRefSize += subPass.getNumColorAttachment();
<<<<<<< HEAD

			if (subPass.usesDepthStencilAttachment())
			{
				colorAttachmentRefSize += 1;// one for depth stencil attachment
			}

			inputAttachmentRefSize += subPass.getNumInputAttachment();
			resolveAttachmentRefSize += subPass.getNumResolveAttachment();
=======
			inputAttachmentRefSize += subPass.getNumInputAttachment();
			depthStencilAttachmentRefSize +=  subPass.usesDepthStencilAttachment();
			resolveAttachmentRefSize += subPass.getNumResolveColorAttachment() + subPass.getNumResolveDepthStencilAttachment();
>>>>>>> 1776432f... 4.3
			preserveAttachmentRefSize += subPass.getNumPreserveAttachment();
		}
		colorAttachmentRefs.resize(colorAttachmentRefSize);
		inputAttachmentsRefs.resize(inputAttachmentRefSize);
		resolveAttachmentsRefs.resize(resolveAttachmentRefSize);
		preserveAttachments.resize(preserveAttachmentRefSize);
<<<<<<< HEAD
=======
		depthStencilAttachmentRefs.resize(depthStencilAttachmentRefSize);
>>>>>>> 1776432f... 4.3
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

		subPassVk.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subPassVk.flags = 0;

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

<<<<<<< HEAD
		// resolve attachments
		if (subPass.getNumResolveAttachment() > 0)
		{
			assertion(subPass.getNumResolveAttachment() == subPass.getNumColorAttachment(),
			          " If the number of resolve attachments is not 0 then it must have colorAttachmentCount entries");
=======
>>>>>>> 1776432f... 4.3

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
<<<<<<< HEAD
		if (subPass.usesDepthStencilAttachment() && createParam.getDepthStencilInfo().format.format != PixelFormat::Unknown)
=======
		if (subPass.usesDepthStencilAttachment())
>>>>>>> 1776432f... 4.3
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
<<<<<<< HEAD
#ifdef DEBUG
	if (m_context.isValid())
=======
	if (_context.isValid())
>>>>>>> 1776432f... 4.3
	{
		if (handle != VK_NULL_HANDLE)
		{
			vk::DestroyRenderPass(native_cast(getContext())->getDevice(), handle, NULL);
			handle = VK_NULL_HANDLE;
		}
	}
<<<<<<< HEAD
#else
	if (handle != VK_NULL_HANDLE)
	{
		vk::DestroyRenderPass(native_cast(getContext())->getDevice(), getNativeObject(), NULL);
		handle = VK_NULL_HANDLE;
	}
#endif
	m_context.reset();
=======
	else
	{
		reportDestroyedAfterContext("RenderPass");
	}
	_context.reset();
>>>>>>> 1776432f... 4.3
}

RenderPassVk_::~RenderPassVk_()
{
<<<<<<< HEAD
#ifdef DEBUG
	if (m_context.isValid())
	{
		destroy();
	}
	else
	{
		reportDestroyedAfterContext("RenderPass");
	}
#else
	destroy();
#endif
=======
	if (_context.isValid())
	{
		destroy();
	}
>>>>>>> 1776432f... 4.3
}
}// namespace vulkan

}// namespace api
}// namespace pvr
