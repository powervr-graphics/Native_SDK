/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/ContextVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of the ContextVulkan class. See ContextVulkan.h, IGraphicsContext.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/ShaderUtils.h"
#include "PVRApi/Vulkan/BufferVk.h"
#include "PVRApi/Vulkan/CommandPoolVk.h"
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/ApiObjects/CommandBuffer.h"
#include "PVRApi/Vulkan/ComputePipelineVk.h"
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/Vulkan/FboVk.h"
#include "PVRApi/Vulkan/PipelineLayoutVk.h"
#include "PVRApi/Vulkan/SamplerVk.h"
#include "PVRApi/Vulkan/ShaderVk.h"
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRApi/Vulkan/RenderPassVk.h"
#include "PVRApi/Vulkan/SyncVk.h"
#include "PVRApi/EffectApi.h"
#include "PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h"
#include "PVRApi/Vulkan/GraphicsPipelineVk.h"
//#include "PVRPlatformGlue/Vulkan/NativeLibraryVulkanGlue.h"
using std::string;
namespace pvr {
namespace {
/*!**********************************************************************************************************
\brief This struct is used to describe an extension entry for the purpose of OpenGL ES Capability definition.
Capabilities can the be queried with the IGraphicsContext::hasApiCapability() functions
\description A table of those describes which capabilities are present in which API version, core or with extensions.
If this struct is used to populated properly a table, the context will automatically query all
defined capabilities, and the presense or absense of a specific capability will be able to be queried, as
well as if it is supported natively or through an extension.
************************************************************************************************************/
struct ExtensionEntry
{
	ApiCapabilities::Enum capability;
	const char* extensionString;
	Api minExtensionLevel;
	Api minCoreLevel;
};

/*!**********************************************************************************************************
\description This table describes what capabilities each OpenGL ES Api has.
************************************************************************************************************/
static ExtensionEntry extensionMap[] =
{
//Common to all OpenGL ES versions - but other APIS might not support them...
//Extensions for OpenGL ES 2+
	{ ApiCapabilities::DebugCallback,				NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::AnisotropicFiltering,		NULL, Api::Unspecified,	Api::Vulkan },
//Extensions for any OpenGL ES2+, core later
	{ ApiCapabilities::Texture3D,					NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::ShadowSamplers,				NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::MapBuffer,					NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::TexureStorage,				NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::Instancing,					NULL, Api::Unspecified, Api::Vulkan },
	{ ApiCapabilities::InvalidateFrameBuffer,       NULL, Api::Unspecified, Api::Vulkan },
//Extensions for OpenGL ES3+
	{ ApiCapabilities::ShaderPixelLocalStorage,		NULL, Api::Unspecified,	Api::Vulkan },

//Core Only
	{ ApiCapabilities::Uniforms,					NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::ShaderAttributeReflection,	NULL, Api::Unspecified,	Api::Vulkan },

	{ ApiCapabilities::Sampler,						NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::TextureSwizzling,			NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::Texture2DArray,				NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::Ubo,							NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::UintUniforms,				NULL, Api::Unspecified, Api::Vulkan },
	{ ApiCapabilities::ShaderAttributeExplicitBind, NULL, Api::Unspecified, Api::Vulkan },
	{ ApiCapabilities::ClearBuffer,					NULL, Api::Unspecified, Api::Vulkan },

	{ ApiCapabilities::ComputeShader,				NULL, Api::Unspecified, Api::Vulkan },
	{ ApiCapabilities::ImageStore,					NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::Ssbo,						NULL, Api::Unspecified,	Api::Vulkan },
	{ ApiCapabilities::AtomicBuffer,				NULL, Api::Unspecified,	Api::Vulkan },

};

}

GraphicsContext IGraphicsContext::getWeakRef() { return static_cast<platform::ContextVk&>(*this).getWeakReference(); }


namespace platform {

api::GraphicsPipeline ContextVk::createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc)
{
	return createGraphicsPipeline(desc, api::ParentableGraphicsPipeline());
}

api::GraphicsPipeline ContextVk::createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc,
    api::ParentableGraphicsPipeline parent)
{
	std::auto_ptr<api::impl::GraphicsPipelineImplBase> pimpleVk(new api::vulkan::GraphicsPipelineImplVk(getWeakRef()));
	if (!static_cast<api::vulkan::GraphicsPipelineImplVk&>(*pimpleVk).init(desc, parent.get()))
	{
		Log(Log.Error, "Failed to create graphics pipeline.");
		return api::GraphicsPipeline();
	}
	api::GraphicsPipeline gp; gp.construct(pimpleVk);
	return gp;
}

api::ParentableGraphicsPipeline ContextVk::createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& desc)
{
	std::auto_ptr<api::impl::GraphicsPipelineImplBase> pimpleVk(new api::vulkan::ParentableGraphicsPipelineImplVk(getWeakRef()));
	if (!static_cast<api::vulkan::ParentableGraphicsPipelineImplVk&>(*pimpleVk).init(desc))
	{
		Log(Log.Error, "Failed to create parentable graphics pipeline.");
		return api::ParentableGraphicsPipeline();
	}
	api::ParentableGraphicsPipeline gp;
	gp.construct(pimpleVk);
	return gp;
}

api::ComputePipeline ContextVk::createComputePipeline(const api::ComputePipelineCreateParam& desc)
{
	api::ComputePipeline cp;
	cp.construct(std::auto_ptr<api::impl::ComputePipelineImplBase>(new api::vulkan::ComputePipelineImplVk(getWeakRef())));
	if (!static_cast<api::vulkan::ComputePipelineImplVk&>(cp->getImpl()).init(desc))
	{
		Log(Log.Error, "Failed to create compute pipeline");
		cp.reset();
	}
	return cp;
}

api::TextureStore ContextVk::createTexture()
{
	api::vulkan::TextureStoreVk tex;
	tex.construct(getWeakRef());
	return tex;
}

api::TextureView ContextVk::createTextureView(const api::TextureStore& texture,
    types::ImageSubresourceRange range, types::SwizzleChannels swizzleChannels)
{
	api::vulkan::TextureViewVk texview;
	texview.construct(static_cast<const api::vulkan::TextureStoreVk>(texture), range, swizzleChannels);
	return texview;
}

api::TextureView ContextVk::createTextureView(const api::TextureStore& texture, types::SwizzleChannels swizzleChannels)
{
	const api::vulkan::TextureStoreVk_& texvk = static_cast<const api::vulkan::TextureStoreVk_&>(*texture);

	types::ImageSubresourceRange range;
	if (texture->getFormat().format == PixelFormat::Depth8  ||
	    texture->getFormat().format == PixelFormat::Depth16  ||
	    texture->getFormat().format == PixelFormat::Depth24 ||
	    texture->getFormat().format == PixelFormat::Depth32)
	{
		range.aspect = types::ImageAspect::Depth;
	}
	else if (texture->getFormat().format == PixelFormat::Depth24Stencil8 ||
	         texture->getFormat().format == PixelFormat::Depth32Stencil8)
	{
		range.aspect = types::ImageAspect::DepthAndStencil;
	}
	else if (texture->getFormat().format == PixelFormat::Stencil8)
	{
		range.aspect = types::ImageAspect::Stencil;
	}
	else
	{
		range.aspect = types::ImageAspect::Color;
	}
	range.arrayLayerOffset = 0;
	range.mipLevelOffset = 0;
	types::ImageLayersSize& layers = range;
	layers = texvk.getLayers();
	return createTextureView(texture, range, swizzleChannels);
}

api::DescriptorSet ContextVk::createDescriptorSetOnDefaultPool(const api::DescriptorSetLayout& layout)
{
	return getDefaultDescriptorPool()->allocateDescriptorSet(layout);
}

api::Fbo ContextVk::createFbo(const api::FboCreateParam& desc)
{
	api::vulkan::FboVk fbo;
	// create fbo
	fbo.construct(getWeakRef());
	if (!fbo->init(desc)) {	fbo.reset(); }
	return fbo;
}

api::FboSet ContextVk::createFboSet(const Multi<api::FboCreateParam>& createParams)
{
	api::FboSet fbos;
	for (uint32 i = 0; i < createParams.size(); ++i) { fbos[i] = createFbo(createParams[i]); }
	return fbos;
}

api::Fence ContextVk::createFence(bool createSignaled)
{
	api::vulkan::FenceVk fence;
	fence.construct(getWeakRef());
	if (!fence->init(createSignaled)) {	fence.reset();	}
	return fence;
}

api::Semaphore ContextVk::createSemaphore()
{
	api::vulkan::SemaphoreVk semaphore;
	semaphore.construct(getWeakRef());
	if (!semaphore->init())	{	semaphore.reset();	}
	return semaphore;
}

api::CommandBuffer ContextVk::createCommandBufferOnDefaultPool()
{
	return getDefaultCommandPool()->allocateCommandBuffer();
}

api::SecondaryCommandBuffer ContextVk::createSecondaryCommandBufferOnDefaultPool()
{
	return getDefaultCommandPool()->allocateSecondaryCommandBuffer();
}

api::Buffer ContextVk::createBuffer(uint32 size, types::BufferBindingUse bufferUsage,
                                    bool isMappable)
{
	api::vulkan::BufferVk buffer;
	buffer.construct(getWeakRef());
	if (!buffer->allocate(size, types::BufferBindingUse(bufferUsage), isMappable))
	{
		buffer.reset();
	}
	return buffer;
}

api::Shader ContextVk::createShader(const Stream& shaderSrc, types::ShaderType type,
                                    const char* const* defines, uint32 numDefines)
{
	api::vulkan::ShaderVk vs; vs.construct(getWeakRef());
	if (!utils::loadShader(static_cast<platform::ContextVk&>(*this).getContextHandle(), shaderSrc, type,
	                       defines, numDefines, *vs, &m_apiCapabilities))
	{
		Log(Log.Error, "Failed to create VertexShader.");
		vs.reset();
	}
	return vs;
}

api::Shader ContextVk::createShader(Stream& shaderData, types::ShaderType type,
                                    types::ShaderBinaryFormat binaryFormat)
{
	api::vulkan::ShaderVk vs; vs.construct(getWeakRef());
	if (!utils::loadShader(static_cast<platform::ContextVk&>(*this).getContextHandle(), shaderData, type,
	                       binaryFormat, *vs, &m_apiCapabilities))
	{
		Log(Log.Error, "Failed to create VertexShader.");
		vs.reset();
	}
	return vs;
}

api::Sampler ContextVk::createSampler(const api::SamplerCreateParam& desc)
{
	api::vulkan::SamplerVk sampler;
	sampler.construct(getWeakRef());
	if (!sampler->init(desc))
	{
		sampler.reset();
	}
	return sampler;
}

api::RenderPass ContextVk::createRenderPass(const api::RenderPassCreateParam& renderPass)
{
	api::vulkan::RenderPassVk rp;
	rp.construct(getWeakRef());
	if (!rp->init(renderPass))
	{
		rp.reset();
	}
	return rp;
}

api::BufferView ContextVk::createBufferView(const pvr::api::Buffer& buffer, pvr::uint32 offset, pvr::uint32 range)
{
	api::vulkan::BufferViewVk bufferview;
	bufferview.construct(buffer, offset, std::min(range, buffer->getSize() - offset));
	assertion(range == 0xFFFFFFFFu || (range <= buffer->getSize() - offset));
	return bufferview;
}

api::BufferView ContextVk::createBufferAndView(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable)
{
	api::vulkan::BufferViewVk bufferview;
	bufferview.construct(createBuffer(size, bufferUsage, isMappable), 0, size);
	return bufferview;
}

api::Fbo ContextVk::createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass)
{
	const api::OnScreenFboCreateParam onScreenFboCreateParam;
	return createOnScreenFboWithRenderPass(swapIndex, renderPass, onScreenFboCreateParam);
}

Multi<api::Fbo> ContextVk::createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass,
    pvr::Multi<api::OnScreenFboCreateParam>& onScreenFboCreateParams)
{
	Multi<api::Fbo> fbos;
	uint32 swapLength = api::native_cast(*this).m_platformContext->getNativeDisplayHandle().swapChainLength;

	assertion(onScreenFboCreateParams.size() == swapLength, " The number of OnScreenFboCreateParams must match the length of the swap chain");

	for (uint32 i = 0; i < swapLength; ++i)
	{
		fbos.add(createOnScreenFboWithRenderPass(i, renderPass, onScreenFboCreateParams[i]));
		if (fbos.back().isNull())
		{
			assertion(false, "Failed to create FBO multibuffering member");
			return Multi<api::Fbo>();
		}
	}
	return fbos;
}

Multi<api::Fbo> ContextVk::createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass)
{
	pvr::Multi<api::OnScreenFboCreateParam> onScreenFboCreateParams;

	// for each swap chain add a default onScreenFboCreateParam
	uint32 swapLength = api::native_cast(*this).m_platformContext->getNativeDisplayHandle().swapChainLength;
	for (uint32 i = 0; i < swapLength; ++i)
	{
		api::OnScreenFboCreateParam onScreenFboCreateParam;
		onScreenFboCreateParams.add(onScreenFboCreateParam);
	}

	return createOnScreenFboSetWithRenderPass(renderPass, onScreenFboCreateParams);
}

api::Fbo ContextVk::createOnScreenFbo(uint32 swapIndex, types::LoadOp colorLoadOp,
                                      types::StoreOp colorStoreOp, types::LoadOp depthLoadOp, types::StoreOp depthStoreOp,
                                      types::LoadOp stencilLoadOp, types::StoreOp stencilStoreOp)
{
	// create the default fbo
	api::RenderPassColorInfo colorInfo;
	api::RenderPassDepthStencilInfo dsInfo;
	colorInfo.format = getPresentationImageFormat();
	dsInfo.format = getDepthStencilImageFormat();
	colorInfo.loadOpColor = colorLoadOp;
	colorInfo.storeOpColor = colorStoreOp;
	colorInfo.numSamples = 1;

	dsInfo.loadOpDepth = depthLoadOp;
	dsInfo.storeOpDepth = depthStoreOp;
	dsInfo.loadOpStencil = stencilLoadOp;
	dsInfo.storeOpStencil = stencilStoreOp;
	dsInfo.numSamples = 1;

	pvr::api::RenderPassCreateParam renderPassDesc;
	renderPassDesc.setColorInfo(0, colorInfo);
	renderPassDesc.setDepthStencilInfo(dsInfo);

	// Require at least one sub pass
	pvr::api::SubPass subPass;
	subPass.setColorAttachment(0, 0); // use color attachment 0

	// disable depth stencil for the sub pass if depth and stencil bpp are 0
	if (getDisplayAttributes().depthBPP == 0 && getDisplayAttributes().stencilBPP == 0)
	{
		subPass.setDepthStencilAttachment(false);
	}

	renderPassDesc.setSubPass(0, subPass);

	return createOnScreenFboWithRenderPass(swapIndex, createRenderPass(renderPassDesc));
}

Multi<api::Fbo> ContextVk::createOnScreenFboSet(types::LoadOp colorLoadOp,
    types::StoreOp colorStoreOp, types::LoadOp depthLoadOp, types::StoreOp depthStoreOp,
    types::LoadOp stencilLoadOp, types::StoreOp stencilStoreOp)
{
	// create the default fbo
	api::RenderPassColorInfo colorInfo;
	api::RenderPassDepthStencilInfo dsInfo;
	colorInfo.format = getPresentationImageFormat();
	dsInfo.format = getDepthStencilImageFormat();
	colorInfo.loadOpColor = colorLoadOp;
	colorInfo.storeOpColor = colorStoreOp;
	colorInfo.numSamples = 1;

	dsInfo.loadOpDepth = depthLoadOp;
	dsInfo.storeOpDepth = depthStoreOp;
	dsInfo.loadOpStencil = stencilLoadOp;
	dsInfo.storeOpStencil = stencilStoreOp;
	dsInfo.numSamples = 1;

	pvr::api::RenderPassCreateParam renderPassDesc;
	renderPassDesc.setColorInfo(0, colorInfo);
	renderPassDesc.setDepthStencilInfo(dsInfo);

	// Require at least one sub pass
	pvr::api::SubPass subPass;
	subPass.setColorAttachment(0, 0); // use color attachment 0
	subPass.setDepthStencilAttachment(true);
	renderPassDesc.setSubPass(0, subPass);

	return createOnScreenFboSetWithRenderPass(createRenderPass(renderPassDesc));
}

api::DescriptorPool ContextVk::createDescriptorPool(const api::DescriptorPoolCreateParam& createParam)
{
	api::vulkan::DescriptorPoolVk descPool = api::vulkan::DescriptorPoolVk_::createNew(getWeakRef());
	if (!descPool->init(createParam)) {	descPool.reset();	}
	return descPool;
}

api::CommandPool ContextVk::createCommandPool()
{
	api::vulkan::CommandPoolVk cmdpool = api::vulkan::CommandPoolVk_::createNew(getWeakRef());
	if (!cmdpool->init()) {	cmdpool.reset(); }
	return cmdpool;
}

api::PipelineLayout ContextVk::createPipelineLayout(const api::PipelineLayoutCreateParam& desc)
{
	pvr::api::vulkan::PipelineLayoutVk pipelayout;
	pipelayout.construct(getWeakRef());
	if (!pipelayout->init(desc))
	{
		pipelayout.reset();
	}
	return pipelayout;
}

api::DescriptorSetLayout ContextVk::createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& desc)
{
	api::vulkan::DescriptorSetLayoutVk layout;
	layout.construct(getWeakRef(), desc);
	if (!layout->init())
	{
		layout.reset();
	}
	return layout;
}

void ContextVk::release()
{
	if (m_osManager) //Is already initialized?
	{
		m_osManager = 0;
		memset(&(ApiCapabilitiesPrivate&)m_apiCapabilities, 0, sizeof(ApiCapabilitiesPrivate));
		m_defaultSampler.reset();
		m_descriptorPool.reset();
		m_cmdTextureUpload.reset();
		m_commandPool.reset();
		m_extensions.clear();
		m_ContextImplementationID = (size_t)(-1);
		m_platformContext = 0;
		m_apiType = Api::Unspecified;
		vk::releaseVk();
	}
}

void ContextVk::setUpCapabilities()
{
	vk::GetPhysicalDeviceMemoryProperties(getPhysicalDevice(), &m_memoryProperties);
	VkPhysicalDeviceProperties props = {};
	vk::GetPhysicalDeviceProperties(getPhysicalDevice(), &props);

	ApiCapabilitiesPrivate& caps = (ApiCapabilitiesPrivate&)m_apiCapabilities;
	caps.maxglslesversion = 0;
	caps.uboOffsetAlignment = (uint32)props.limits.minUniformBufferOffsetAlignment;
	caps.ssboOffsetAlignment = (uint32)props.limits.minStorageBufferOffsetAlignment;

	// EXTENSIONS -- SEE TOP OF THIS FILE.
	// For each extension, make sure that the we properly determine native or extension support.
	for (int i = 0; i < sizeof(extensionMap) / sizeof(extensionMap[0]); ++i)
	{
		if (extensionMap[i].minCoreLevel != Api::Unspecified && m_apiType >= extensionMap[i].minCoreLevel)
		{
			caps.nativeSupport[extensionMap[i].capability] = true;
		}
		else if (extensionMap[i].minExtensionLevel != Api::Unspecified && m_apiType >= extensionMap[i].minExtensionLevel)
		{
			caps.extensionSupport[extensionMap[i].capability] = isExtensionSupported(extensionMap[i].extensionString);
		}
	}
}

api::Fbo ContextVk::createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass,
    const api::OnScreenFboCreateParam& onScreenFboCreateParam)
{
	if (!renderPass.isValid())
	{
		assertion(renderPass.isValid(), "Invalid Renderpass object");
		Log("Invalid Renderpass object");
		return api::Fbo();
	}
	api::FboCreateParam fboInfo;
	fboInfo.width = getDisplayAttributes().width;
	fboInfo.height = getDisplayAttributes().height;
	fboInfo.setRenderPass(renderPass);
	api::vulkan::DefaultFboVk fbo;
	{
		api::vulkan::TextureViewVk texViewColor, texViewDs;
		platform::NativeDisplayHandle_::FrameBuffer& fb = api::native_cast(*this).m_platformContext->getNativeDisplayHandle().onscreenFbo;
		int i = swapIndex;
		native::HTexture_ hColorTex;
		hColorTex.undeletable = true;
		hColorTex.image = fb.colorImages[i];
		api::vulkan::TextureStoreVk texColor;
		texColor.construct(getWeakRef(), hColorTex, types::ImageBaseType::Image2D);
		native::HImageView_ hTexViewColor(fb.colorImageViews[i], true);
		texViewColor.construct(texColor, hTexViewColor);
		api::ImageStorageFormat fmt; fmt.numSamples = 1; fmt.mipmapLevels = 1;
		static_cast<api::ImageDataFormat&>(fmt) = api::ConvertFromVulkan::imageDataFormat(fb.colorFormat);
		texColor->getFormat() = fmt;
		texColor->setDimensions(types::Extent3D((uint16)fboInfo.width, (uint16)fboInfo.height));
		//texColor->setLayers()... Default is 1 array level, one mip level

		fboInfo.setColor(0, texViewColor);

		if (fb.hasDepthStencil)
		{
			native::HTexture_ hDepthTex;
			hDepthTex.undeletable = true;
			hDepthTex.image = fb.depthStencilImage[i].first;
			api::vulkan::TextureStoreVk texDs;
			texDs.construct(getWeakRef(), hDepthTex, types::ImageBaseType::Image2D);

			native::HImageView_ hTexViewDs(fb.depthStencilImageView[i], true);
			texViewDs.construct(texDs, hTexViewDs);

			static_cast<api::ImageDataFormat&>(fmt) = api::ConvertFromVulkan::imageDataFormat(fb.depthStencilFormat);
			texDs->setDimensions(types::Extent3D((uint16)fboInfo.width, (uint16)fboInfo.height));
			texDs->getFormat() = fmt;
			texDs->setDimensions(types::Extent3D((uint16)fboInfo.width, (uint16)fboInfo.height));
			fboInfo.setDepthStencil(texViewDs);
		}
		fboInfo.setColor(0, texViewColor);

		// add any additional color view attachments provided to the on screen fbo
		for (auto i = 0u; i < onScreenFboCreateParam.getNumOffScreenColor(); ++i)
		{
			auto colorAttachment = onScreenFboCreateParam.getOffScreenColor(i + 1);
			assertion(colorAttachment.isValid(), "On-screen fbo color attachments indexes are not consecutive");
			fboInfo.setColor(i + 1, colorAttachment);
		}
	}
	fbo.construct(getWeakRef());
	if (!fbo->init(fboInfo)) {	fbo.reset();	}
	return fbo;
}


bool ContextVk::screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* pBuffer,
                                    ImageFormat requestedImageFormat)
{
	api::CommandBuffer cmdBuffer =  createCommandBufferOnDefaultPool();
	native::HTexture_ vkTexHandle; vkTexHandle.image = getPlatformContext().getNativeDisplayHandle().onscreenFbo.colorImages[getLastSwapChainIndex()];
	api::vulkan::TextureStoreVk srcVkTex; srcVkTex.construct(getWeakRef(),
	    vkTexHandle, types::ImageBaseType::Image2D);

	api::TextureStore srcTex(srcVkTex);

	const uint32 width = w - x;
	const uint32 height = h - y;
	const uint32 dataSize = 4 * width * height;
	// create the destination texture which does the format conversion
	api::ImageStorageFormat fmt[] =
	{
		api::ImageStorageFormat(PixelFormat::RGBA_8888, 1, types::ColorSpace::lRGB, VariableType::UnsignedByteNorm, 1),

		api::ImageStorageFormat(api::ImageStorageFormat(PixelFormat::BGRA_8888, 1, types::ColorSpace::lRGB,
		VariableType::UnsignedByteNorm, 1))
	};
	VkFormat vkFormat = api::ConvertToVk::pixelFormat(fmt[requestedImageFormat].format, fmt[requestedImageFormat].colorSpace, fmt[requestedImageFormat].dataType);
	VkFormatProperties prop;
	vk::GetPhysicalDeviceFormatProperties(getPlatformContext().getNativePlatformHandles().context.physicalDevice,
	                                      vkFormat, &prop);
	if (!(prop.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
	{
		Log("Screen Capture requested Image format is not supported");
		return false;
	}

	api::TextureStore dstTex = createTexture();
	dstTex->allocate2D(fmt[requestedImageFormat], width, height, types::ImageUsageFlags::TransferDest |
	                   types::ImageUsageFlags::TransferSrc, types::ImageLayout::TransferDstOptimal);

	types::Offset3D srcOffsets[2] =
	{
		types::Offset3D((uint16)x, (uint16)y, 0),
		types::Offset3D((uint16)w, (uint16)h, 1)
	};

	types::Offset3D dstOffsets[2] =
	{
		types::Offset3D((uint16)x, (uint16)h, 0),
		types::Offset3D((uint16)w, (uint16)y, 1)
	};


	//create the final destination buffer for reading
	api::Buffer buffer = createBuffer(dataSize, types::BufferBindingUse::TransferDest, true);

	cmdBuffer->beginRecording();
	types::ImageBlitRange copyRange(srcOffsets, dstOffsets);

	// transform the layout from the color attachment to transfer src
	cmdBuffer->pipelineBarrier(types::PipelineStageFlags::AllGraphics, types::PipelineStageFlags::AllGraphics,
	                           api::MemoryBarrierSet().addBarrier(api::ImageAreaBarrier(types::AccessFlags::ColorAttachmentRead,
	                               types::AccessFlags::TransferRead, srcTex, types::ImageSubresourceRange(),
	                               types::ImageLayout::PresentSrc, types::ImageLayout::TransferSrcOptimal)), true);

	cmdBuffer->blitImage(srcTex, dstTex, types::ImageLayout::TransferSrcOptimal,
	                     types::ImageLayout::TransferDstOptimal, &copyRange, 1,  types::SamplerFilter::Linear);

	types::BufferImageCopy region(0, 0, 0, glm::uvec3(x, y, 0), glm::uvec3(w, h, 1));

	cmdBuffer->pipelineBarrier(types::PipelineStageFlags::AllGraphics, types::PipelineStageFlags::AllGraphics,
	                           api::MemoryBarrierSet()
	                           // transform back the color attachment optimal
	                           .addBarrier(api::ImageAreaBarrier(types::AccessFlags::TransferRead,
	                                       types::AccessFlags::ColorAttachmentRead, srcTex, types::ImageSubresourceRange(),
	                                       types::ImageLayout::TransferSrcOptimal, types::ImageLayout::PresentSrc))
	                           // transform the format conversion texture to trasfer src
	                           .addBarrier(api::ImageAreaBarrier(types::AccessFlags::TransferWrite,
	                                       types::AccessFlags::TransferRead, dstTex, types::ImageSubresourceRange(),
	                                       types::ImageLayout::TransferDstOptimal, types::ImageLayout::TransferSrcOptimal)),
	                           true);

	cmdBuffer->copyImageToBuffer(dstTex, types::ImageLayout::TransferSrcOptimal, buffer, &region, 1);
	cmdBuffer->endRecording();
	// create a fence for wait.
	api::Fence fenceWait = createFence(false);

	cmdBuffer->submit(api::Semaphore(), api::Semaphore(), fenceWait);
	fenceWait->wait();// wait for the submit to finish so that the command buffer get destroyed properly.
	// map the buffer and copy the data
	byte* data = (byte*)buffer->map(types::MapBufferFlags::Read, 0, dataSize);
	memcpy(pBuffer, data, dataSize);
	buffer->unmap();
	return true;
}

void ContextVk::waitIdle()
{
	vkIsSuccessful(vk::QueueWaitIdle(getPlatformContext().getNativePlatformHandles().graphicsQueue),
	               "ConstextVK::waitIdle - error in preceeding command.");
}

string ContextVk::getInfo()const
{
	return "";
}

Result ContextVk::init(OSManager& osManager)
{
	if (m_osManager)
	{
		return Result::AlreadyInitialized;
	}
	if (!osManager.getPlatformContext().isInitialized())
	{
		return Result::NotInitialized;
	}
	m_apiType = osManager.getApiTypeRequired(); //PlatformContext should have already made sure that this is actually possible.

	if (m_apiType != Api::Vulkan)
	{
		Log(Log.Error, "Non-vulkan api was requested %s", apiName(m_apiType));
		return Result::UnsupportedRequest;
	}

	//These cannot fail...
	m_platformContext = &osManager.getPlatformContext();
	vk::initVk(m_platformContext->getNativePlatformHandles().context.instance,
	           m_platformContext->getNativePlatformHandles().context.device);
	m_osManager = &osManager;
	setUpCapabilities();

	// create the default command pool
	m_commandPool = createCommandPool();

	//--- create the descriptor pool
	pvr::api::DescriptorPoolCreateParam parm;
	parm.addDescriptorInfo(types::DescriptorType::CombinedImageSampler, 500);
	parm.addDescriptorInfo(types::DescriptorType::InputAttachment, 50);
	parm.addDescriptorInfo(types::DescriptorType::UniformBuffer, 500);
	parm.addDescriptorInfo(types::DescriptorType::UniformBufferDynamic, 500);
	parm.addDescriptorInfo(types::DescriptorType::StorageBuffer, 500);
	parm.addDescriptorInfo(types::DescriptorType::StorageBufferDynamic, 500);
	parm.setMaxDescriptorSets(100);
	m_descriptorPool = createDescriptorPool(parm);

	// create the default sampler
	types::SamplerCreateParam defaultSamplerInfo;
	m_defaultSampler = createSampler(defaultSamplerInfo);

	// create the texture upload commandbuffer
	m_cmdTextureUpload = createCommandBufferOnDefaultPool();

	return Result::Success;
}

}// namespace platform

//Creates an instance of a graphics context.
GraphicsContextStrongReference createGraphicsContext()
{
	GraphicsContextStrongReference ctx = platform::ContextVk::createNew();
	//DEFAULT CONTEXT PER PLATFORM. CAN(WILL) BE OVERRIDEN BY PVRShell
	return ctx;
}

}// namespace pvr
//!\endcond
