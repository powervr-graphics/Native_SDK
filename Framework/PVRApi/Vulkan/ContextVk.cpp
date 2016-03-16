/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/ContextVulkan.cpp
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
#include "PVRApi/ApiObjects/ComputePipeline.h"
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
	Api::Enum minExtensionLevel;
	Api::Enum minCoreLevel;
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

api::EffectApi IGraphicsContext::createEffectApi(assets::Effect& effectDesc, api::GraphicsPipelineCreateParam& pipeDesc,
    api::AssetLoadingDelegate& effectDelegate)
{
	api::EffectApi effect;
	effect.construct(m_this_shared, effectDelegate);
	if (effect->init(effectDesc, pipeDesc) != Result::Success)
	{
		effect.release();
	}
	return effect;
}
api::TextureStore IGraphicsContext::createTexture()
{
	api::vulkan::TextureStoreVk tex;
	tex.construct(m_this_shared);
	return tex;
}

api::TextureView IGraphicsContext::createTextureView(const api::TextureStore& texture, types::ImageSubresourceRange range, types::SwizzleChannels swizzleChannels)
{
	api::vulkan::TextureViewVk texview;
	texview.construct(static_cast<const api::vulkan::TextureStoreVk>(texture), range, swizzleChannels);
	return texview;
}

api::TextureView IGraphicsContext::createTextureView(const api::TextureStore& texture, types::SwizzleChannels swizzleChannels)
{
	const api::vulkan::TextureStoreVk_& texvk = static_cast<const api::vulkan::TextureStoreVk_&>(*texture);

	types::ImageSubresourceRange range;
	range.aspect = types::ImageAspect::Color;
	range.arrayLayerOffset = 0;
	range.mipLevelOffset = 0;
	types::ImageLayersSize& layers = range;
	layers = texvk.layersSize;
	return createTextureView(texture, range);
}

api::DescriptorSet IGraphicsContext::createDescriptorSetOnDefaultPool(const api::DescriptorSetLayout& layout)
{
	return getDefaultDescriptorPool()->allocateDescriptorSet(layout);
}


//Creates an instance of a graphics context.
GraphicsContextStrongReference createGraphicsContext()
{
	RefCountedResource<platform::ContextVk> ctx;
	ctx.construct();
	//DEFAULT CONTEXT PER PLATFORM. CAN(WILL) BE OVERRIDEN BY PVRShell
	ctx->m_apiType = Api::Vulkan;
	return ctx;
}

api::Fbo IGraphicsContext::createFbo(const api::FboCreateParam& desc)
{
	api::vulkan::FboVk fbo;
	// create fbo
	fbo.construct(m_this_shared);
	if (!fbo->init(desc))
	{
		fbo.release();
	}
	return fbo;
}


api::Fence IGraphicsContext::createFence(bool createSignaled)
{
	api::vulkan::FenceVk fence;
	fence.construct(m_this_shared);
	if (!fence->init(createSignaled))
	{
		fence.reset();
	}
	return fence;
}

const api::CommandPool& IGraphicsContext::getDefaultCommandPool() const
{
	return pvr::api::native_cast(*this).getDefaultCommandPool();
}

api::CommandPool& IGraphicsContext::getDefaultCommandPool()
{
	return pvr::api::native_cast(*this).getDefaultCommandPool();
}

const api::DescriptorPool& IGraphicsContext::getDefaultDescriptorPool()const
{
	return pvr::api::native_cast(*this).getDefaultDescriptorPool();
}

api::DescriptorPool& IGraphicsContext::getDefaultDescriptorPool()
{
	return pvr::api::native_cast(*this).getDefaultDescriptorPool();
}

api::CommandBuffer IGraphicsContext::createCommandBufferOnDefaultPool()
{
	return getDefaultCommandPool()->allocateCommandBuffer();
}

api::SecondaryCommandBuffer IGraphicsContext::createSecondaryCommandBufferOnDefaultPool()
{
	return getDefaultCommandPool()->allocateSecondaryCommandBuffer();
}

api::Buffer IGraphicsContext::createBuffer(uint32 size, types::BufferBindingUse::Bits bufferUsage, types::BufferUse::Flags hint)
{
	api::vulkan::BufferVk buffer;
	buffer.construct(m_this_shared);
	if (!buffer->allocate(size, types::BufferBindingUse::Enum(bufferUsage), hint))
	{
		buffer.reset();
	}
	return buffer;
}

api::Shader IGraphicsContext::createShader(const Stream& shaderSrc, types::ShaderType::Enum type, const char* const* defines, uint32 numDefines)
{
	api::vulkan::ShaderVk vs; vs.construct(m_this_shared);
	if (!utils::loadShader(static_cast<platform::ContextVk&>(*this).getContextHandle(), shaderSrc, type, defines, numDefines, *vs, &m_apiCapabilities))
	{
		Log(Log.Error, "Failed to create VertexShader.");
		vs.reset();
	}
	return vs;
}

api::Shader IGraphicsContext::createShader(Stream& shaderData, types::ShaderType::Enum type, types::ShaderBinaryFormat::Enum binaryFormat)
{
	api::vulkan::ShaderVk vs; vs.construct(m_this_shared);
	if (!utils::loadShader(static_cast<platform::ContextVk&>(*this).getContextHandle(), shaderData, type, binaryFormat, *vs, &m_apiCapabilities))
	{
		Log(Log.Error, "Failed to create VertexShader.");
		vs.release();
	}
	return vs;
}

api::Sampler IGraphicsContext::createSampler(const api::SamplerCreateParam& desc)
{
	api::vulkan::SamplerVk sampler;
	sampler.construct(m_this_shared);
	if (!sampler->init(desc))
	{
		sampler.release();
	}
	return sampler;
}

api::GraphicsPipeline IGraphicsContext::createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc)
{
	api::GraphicsPipeline gp;
	gp.construct(m_this_shared);
	Result::Enum result = gp->init(desc);
	if (result != Result::Success)
	{
		gp.reset();
		Log(Log.Error, "Failed to create graphics pipeline. Error value was: %s", Log.getResultCodeString(result));
	}
	return gp;
}

api::GraphicsPipeline IGraphicsContext::createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc, api::ParentableGraphicsPipeline parent)
{
	api::GraphicsPipeline gp;
	gp.construct(m_this_shared);
	Result::Enum result = gp->init(desc, parent.get());
	if (result != Result::Success)
	{
		gp.reset();
		Log(Log.Error, "Failed to create graphics pipeline. Error value was: %s", Log.getResultCodeString(result));
	}
	return gp;
}

api::ComputePipeline IGraphicsContext::createComputePipeline(const api::ComputePipelineCreateParam& desc)
{
	api::ComputePipeline cp;
	cp.construct(m_this_shared);
	Result::Enum result = cp->init(desc);
	if (result != Result::Success)
	{
		Log(Log.Error, "Failed to create graphics pipeline. Error value was: %s",
		    Log.getResultCodeString(result));
		cp.reset();
	}
	return cp;
}

api::ParentableGraphicsPipeline IGraphicsContext::createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& desc)
{
	api::ParentableGraphicsPipeline gp;
	gp.construct(m_this_shared);
	Result::Enum result = gp->init(desc);
	if (result != Result::Success)
	{
		Log(Log.Error, "Failed to create parentable graphics pipeline. Error value was: %s", Log.getResultCodeString(result));
		gp.reset();
	}
	return gp;
}

api::RenderPass IGraphicsContext::createRenderPass(const api::RenderPassCreateParam& renderPass)
{
	api::vulkan::RenderPassVk rp;
	rp.construct(m_this_shared);
	if (!rp->init(renderPass))
	{
		rp.release();
	}
	return rp;
}

api::BufferView IGraphicsContext::createBufferView(const pvr::api::Buffer& buffer, pvr::uint32 offset, pvr::uint32 range)
{
	api::vulkan::BufferViewVk bufferview;
	bufferview.construct(buffer, offset, std::min(range, buffer->getSize() - offset));
	assertion(range == 0xFFFFFFFFu || (range <= buffer->getSize() - offset));
	return bufferview;
}

api::BufferView IGraphicsContext::createBufferAndView(uint32 size, types::BufferBindingUse::Bits bufferUsage, types::BufferUse::Flags hint)
{
	api::vulkan::BufferViewVk bufferview;
	bufferview.construct(createBuffer(size, bufferUsage, hint), 0, size);
	return bufferview;
}

api::Fbo IGraphicsContext::createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass,
    const api::OnScreenFboCreateParam& onScreenFboCreateParam)
{
	api::FboCreateParam fboInfo;
	fboInfo.width = getDisplayAttributes().width;
	fboInfo.height = getDisplayAttributes().height;
	fboInfo.setRenderPass(renderPass);
	api::native_cast(*this).m_defaultRenderPass = renderPass;
	api::vulkan::DefaultFboVk fbo;
	{
		api::vulkan::TextureViewVk texViewColor, texViewDs;
		system::NativeDisplayHandle_::FrameBuffer& fb = api::native_cast(*this).m_platformContext->getNativeDisplayHandle().fb;
		int i = swapIndex;
		native::HTexture_ hColorTex, hDepthTex;
		hColorTex.undeletable = hDepthTex.undeletable = true;
		hColorTex.image = fb.colorImages[i];
		hDepthTex.image = fb.depthStencilImage[i].first;
		api::vulkan::TextureStoreVk texColor;
		api::vulkan::TextureStoreVk texDs;
		texColor.construct(m_this_shared, hColorTex, types::TextureDimension::Texture2D);
		texDs.construct(m_this_shared, hDepthTex, types::TextureDimension::Texture2D);
		native::HImageView_ hTexViewColor(fb.colorImageViews[i], true);
		native::HImageView_ hTexViewDs(fb.depthStencilImageView[i], true);

		texViewColor.construct(texColor,hTexViewColor);
		texViewDs.construct(texDs, hTexViewDs);
		api::ImageStorageFormat fmt; fmt.numSamples = 1; fmt.mipmapLevels = 1;
		static_cast<api::ImageDataFormat&>(fmt) = api::ConvertFromVulkan::imageDataFormat(fb.colorFormat);
		texColor->getFormat() = fmt;
		texColor->setDimensions(types::ImageExtents((uint16)fboInfo.width, (uint16)fboInfo.height));
		//texColor->setLayers()... Default is 1 array level, one mip level

		static_cast<api::ImageDataFormat&>(fmt) = api::ConvertFromVulkan::imageDataFormat(fb.depthStencilFormat);
		texDs->getFormat() = fmt;
		texDs->setDimensions(types::ImageExtents((uint16)fboInfo.width, (uint16)fboInfo.height));
		fboInfo.addColor(0, texViewColor).setDepthStencil(texViewDs);

		// add any additional color view attachments provided to the on screen fbo
		for (auto it = onScreenFboCreateParam.getColorAttachments().begin(); it != onScreenFboCreateParam.getColorAttachments().end(); ++it)
		{
			fboInfo.addColor(it->first, it->second);
		}

	}
	fbo.construct(m_this_shared);
	if (!fbo->init(fboInfo))
	{
		fbo.reset();
	}
	return fbo;
}

api::Fbo IGraphicsContext::createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass)
{
	const api::OnScreenFboCreateParam onScreenFboCreateParam;
	return createOnScreenFboWithRenderPass(swapIndex, renderPass, onScreenFboCreateParam);
}

uint32 IGraphicsContext::getSwapChainLength()const { return getPlatformContext().getSwapChainLength();  }

uint32 IGraphicsContext::getCurrentSwapChain()const { return getPlatformContext().getSwapChainIndex(); }

Multi<api::Fbo> IGraphicsContext::createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass,
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
		}
	}
	return fbos;
}

Multi<api::Fbo> IGraphicsContext::createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass)
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

api::Fbo IGraphicsContext::createOnScreenFbo(uint32 swapIndex, types::LoadOp::Enum colorLoadOp, types::StoreOp::Enum colorStoreOp,
    types::LoadOp::Enum depthLoadOp, types::StoreOp::Enum depthStoreOp,
    types::LoadOp::Enum stencilLoadOp, types::StoreOp::Enum stencilStoreOp,
    uint32 numColorSamples, uint32 numDepthStencilSamples)
{
	// create the default fbo
	api::RenderPassColorInfo colorInfo;
	api::RenderPassDepthStencilInfo dsInfo;
	api::getDisplayFormat(getDisplayAttributes(), &colorInfo.format, &dsInfo.format);
	colorInfo.loadOpColor = colorLoadOp;
	colorInfo.storeOpColor = colorStoreOp;
	colorInfo.numSamples = numColorSamples;

	dsInfo.loadOpDepth = depthLoadOp;
	dsInfo.storeOpDepth = depthStoreOp;
	dsInfo.loadOpStencil = stencilLoadOp;
	dsInfo.storeOpStencil = stencilStoreOp;
	dsInfo.numSamples = numDepthStencilSamples;

	pvr::api::RenderPassCreateParam renderPassDesc;
	renderPassDesc.addColorInfo(0, colorInfo);
	renderPassDesc.setDepthStencilInfo(dsInfo);

	// Require at least one sub pass
	pvr::api::SubPass subPass;
	subPass.setColorAttachment(0);// use color attachment 0
	renderPassDesc.addSubPass(0, subPass);

	return createOnScreenFboWithRenderPass(swapIndex, createRenderPass(renderPassDesc));
}

Multi<api::Fbo> IGraphicsContext::createOnScreenFboSet(types::LoadOp::Enum colorLoadOp, types::StoreOp::Enum colorStoreOp,
    types::LoadOp::Enum depthLoadOp, types::StoreOp::Enum depthStoreOp,
    types::LoadOp::Enum stencilLoadOp, types::StoreOp::Enum stencilStoreOp,
    uint32 numColorSamples, uint32 numDepthStencilSamples)
{
	// create the default fbo
	api::RenderPassColorInfo colorInfo;
	api::RenderPassDepthStencilInfo dsInfo;
	api::getDisplayFormat(getDisplayAttributes(), &colorInfo.format, &dsInfo.format);
	colorInfo.loadOpColor = colorLoadOp;
	colorInfo.storeOpColor = colorStoreOp;
	colorInfo.numSamples = numColorSamples;

	dsInfo.loadOpDepth = depthLoadOp;
	dsInfo.storeOpDepth = depthStoreOp;
	dsInfo.loadOpStencil = stencilLoadOp;
	dsInfo.storeOpStencil = stencilStoreOp;
	dsInfo.numSamples = numDepthStencilSamples;

	pvr::api::RenderPassCreateParam renderPassDesc;
	renderPassDesc.addColorInfo(0, colorInfo);
	renderPassDesc.setDepthStencilInfo(dsInfo);

	// Require at least one sub pass
	pvr::api::SubPass subPass;
	subPass.setColorAttachment(0);// use color attachment 0
	renderPassDesc.addSubPass(0, subPass);

	return createOnScreenFboSetWithRenderPass(createRenderPass(renderPassDesc));
}

api::DescriptorPool IGraphicsContext::createDescriptorPool(const api::DescriptorPoolCreateParam& createParam)
{
	api::vulkan::DescriptorPoolVk descPool = api::vulkan::DescriptorPoolVk_::createNew(m_this_shared);
	if (!descPool->init(createParam))
	{
		descPool.release();
	}
	return descPool;
}

api::CommandPool IGraphicsContext::createCommandPool()
{
	api::vulkan::CommandPoolVk cmdpool = api::vulkan::CommandPoolVk_::createNew(m_this_shared);
	if (!cmdpool->init())
	{
		cmdpool.release();
	}
	return cmdpool;
}

api::PipelineLayout IGraphicsContext::createPipelineLayout(const api::PipelineLayoutCreateParam& desc)
{
	pvr::api::vulkan::PipelineLayoutVk pipelayout;
	pipelayout.construct(m_this_shared);
	if (!pipelayout->init(desc))
	{
		pipelayout.release();
	}
	return pipelayout;
}

api::DescriptorSetLayout IGraphicsContext::createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& desc)
{
	api::vulkan::DescriptorSetLayoutVk layout;
	layout.construct(m_this_shared, desc);
	if (!layout->init()) { layout.release(); }
	return layout;
}


namespace platform {
void ContextVk::release()
{
	if (m_osManager) //Is already initialized?
	{
		m_osManager = 0;
		memset(&(ApiCapabilitiesPrivate&)m_apiCapabilities, 0, sizeof(ApiCapabilitiesPrivate));
		m_defaultSampler.reset();
		m_descriptorPool.reset();
		m_commandPool.reset();
		m_cmdTextureUpload.reset();
		m_defaultRenderPass.reset();
		m_extensions.clear();
		m_ContextImplementationID = (size_t)(-1);
		m_platformContext = 0;
		m_apiType = Api::Unspecified;
		m_renderStatesTracker.releaseAll();
		vk::releaseVk();
	}
	m_this_shared.release();
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

ContextVk::ContextVk(size_t implementationID) : m_ContextImplementationID(implementationID)
{ }


bool ContextVk::screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* pBuffer,
                                    ImageFormat requestedImageFormat)
{
	return false;
}

void ContextVk::waitIdle()
{
	vkIsSuccessful(vk::QueueWaitIdle(getPlatformContext().getNativePlatformHandles().graphicsQueue), "ConstextVK::waitIdle - error in preceeding command.");
}

string ContextVk::getInfo()const
{
	return "";
}

Result::Enum ContextVk::init(OSManager& osManager, GraphicsContext& my_wrapper)
{
	if (m_osManager)
	{
		return Result::AlreadyInitialized;
	}
	if (!osManager.getPlatformContext().isInitialized())
	{
		return Result::NotInitialized;
	}
	m_this_shared = my_wrapper;
	m_apiType = osManager.getApiTypeRequired(); //PlatformContext should have already made sure that this is actually possible.

	if (m_apiType != Api::Vulkan)
	{
		Log(Log.Error, "Non-vulkan api was requested %s", Api::getApiName(m_apiType));
		return Result::UnsupportedRequest;
	}

	//These cannot fail...
	m_platformContext = &osManager.getPlatformContext();
	vk::initVk(m_platformContext->getNativePlatformHandles().context.instance, m_platformContext->getNativePlatformHandles().context.device);
	m_osManager = &osManager;
	setUpCapabilities();

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

	//m_renderStatesTracker.viewport =
	//  Rectanglei(0, 0, getDisplayAttributes().width, getDisplayAttributes().height);
	//m_renderStatesTracker.scissor = m_renderStatesTracker.viewport;
	return Result::Success;
}

}// namespace platform
}// namespace pvr
//!\endcond
