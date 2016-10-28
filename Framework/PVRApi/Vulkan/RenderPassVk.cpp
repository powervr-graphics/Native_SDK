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
	platform::NativePlatformHandles_& platformHandle = getContext()->getPlatformContext().getNativePlatformHandles();

	VkRenderPassCreateInfo renderPassInfoVk;
	memset(&renderPassInfoVk, 0, sizeof(renderPassInfoVk));
	renderPassInfoVk.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	std::vector<VkAttachmentDescription> attachmentDescVk;
	std::vector<VkSubpassDescription> subPassesVk;
	std::vector<VkSubpassDependency> subPassDependenciesVk;

	//--- Color Attachments
	renderPassInfoVk.attachmentCount = createParam.getNumColorInfo() +
	                                   (createParam.getDepthStencilInfo().format.format != PixelFormat::Unknown ? 1 : 0);

	attachmentDescVk.resize(renderPassInfoVk.attachmentCount);
	pvr::uint32 i = 0;
	for (; i < createParam.getNumColorInfo(); ++i)
	{
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
		if (!((prop.bufferFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) ||
		      (prop.bufferFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)))
		{
			//Log("Device not support this Format for color attachment");
			//return false;
		}

		attachmentDescVk[i].loadOp = ConvertToVk::loadOp(colorInfo.loadOpColor);
		attachmentDescVk[i].storeOp = ConvertToVk::storeOp(colorInfo.storeOpColor);
		attachmentDescVk[i].initialLayout = ConvertToVk::imageLayout(colorInfo.initialLayout);
		attachmentDescVk[i].finalLayout = ConvertToVk::imageLayout(colorInfo.finalLayout);
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
	}
	renderPassInfoVk.pAttachments = attachmentDescVk.data();

	//--- Subpass
	renderPassInfoVk.subpassCount = createParam.getNumSubPass();

	std::vector<VkAttachmentReference> colorAttachmentRefs;
	std::vector<VkAttachmentReference> inputAttachmentsRefs;
	std::vector<VkAttachmentReference> resolveAttachmentsRefs;
	std::vector<pvr::uint32> preserveAttachments;
	{
		//calc the attachmentRefs total sizes
		int32 colorAttachmentRefSize = 0;
		int32 inputAttachmentRefSize = 0;
		int32 resolveAttachmentRefSize = 0;
		int32 preserveAttachmentRefSize = 0;

		for (uint32 i = 0; i < createParam.getNumSubPass(); ++i)
		{
			const SubPass& subPass = createParam.getSubPass(i);
			colorAttachmentRefSize += subPass.getNumColorAttachment();

			if (subPass.usesDepthStencilAttachment())
			{
				colorAttachmentRefSize += 1;// one for depth stencil attachment
			}

			inputAttachmentRefSize += subPass.getNumInputAttachment();
			resolveAttachmentRefSize += subPass.getNumResolveAttachment();
			preserveAttachmentRefSize += subPass.getNumPreserveAttachment();
		}
		colorAttachmentRefs.resize(colorAttachmentRefSize);
		inputAttachmentsRefs.resize(inputAttachmentRefSize);
		resolveAttachmentsRefs.resize(resolveAttachmentRefSize);
		preserveAttachments.resize(preserveAttachmentRefSize);
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
			assertion(subPass.getNumResolveAttachment() == subPass.getNumColorAttachment(),
			          " If the number of resolve attachments is not 0 then it must have colorAttachmentCount entries");

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
			subPassVk.pPreserveAttachments = &preserveAttachments[preserveAttachmentRefOffset];

			for (pvr::uint8 j = 0; j < subPass.getNumPreserveAttachment(); ++j)
			{
				preserveAttachments[preserveAttachmentRefOffset] = subPass.getPreserveAttachmentId(j);
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
		if (subPass.usesDepthStencilAttachment() && createParam.getDepthStencilInfo().format.format != PixelFormat::Unknown)
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
		subPassDependencyVk.srcStageMask = (uint32)subPassDependency.srcStageMask;
		subPassDependencyVk.dstStageMask = (uint32)subPassDependency.dstStageMask;
		subPassDependencyVk.srcAccessMask = (uint32)subPassDependency.srcAccessMask;
		subPassDependencyVk.dstAccessMask = (uint32)subPassDependency.dstAccessMask;
		subPassDependencyVk.dependencyFlags = subPassDependency.dependencyByRegion;

		subPassDependenciesVk[i] = subPassDependencyVk;
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
#ifdef DEBUG
	if (m_context.isValid())
	{
		if (handle != VK_NULL_HANDLE)
		{
			vk::DestroyRenderPass(native_cast(getContext())->getDevice(), getNativeObject(), NULL);
			handle = VK_NULL_HANDLE;
		}
	}
#else
	if (handle != VK_NULL_HANDLE)
	{
		vk::DestroyRenderPass(native_cast(getContext())->getDevice(), getNativeObject(), NULL);
		handle = VK_NULL_HANDLE;
	}
#endif
	m_context.reset();
}

RenderPassVk_::~RenderPassVk_()
{
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
}
}//	namespace vulkan

namespace impl {
const native::HRenderPass_& RenderPass_::getNativeObject() const { return native_cast(*this); }
native::HRenderPass_& RenderPass_::getNativeObject() { return native_cast(*this); }
}// namespace impl
}// namespace api
}// namespace pvr
//!\endcond
