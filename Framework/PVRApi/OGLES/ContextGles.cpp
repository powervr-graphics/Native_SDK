/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ContextGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of the ContextGles class. See ContextGles.h, IGraphicsContext.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/BufferGles.h"
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRApi/OGLES/DescriptorSetGles.h"
#include "PVRApi/OGLES/SamplerGles.h"
#include "PVRNativeApi/ShaderUtils.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/FboGles.h"
#include "PVRApi/OGLES/RenderPassGles.h"
#include "PVRApi/OGLES/DescriptorSetGles.h"
#include "PVRApi/OGLES/GraphicsPipelineGles.h"
#include "PVRApi/OGLES/ComputePipelineGles.h"
#include "PVRApi/OGLES/PipelineLayoutGles.h"
#include "PVRApi/OGLES/CommandPoolGles.h"
#include "PVRApi/ApiObjects/Sync.h"
#include "PVRApi/EffectApi.h"
#include <algorithm>

using std::string;
namespace pvr {
//Creates an instance of a graphics context.
GraphicsContextStrongReference createGraphicsContext()
{
	GraphicsContextStrongReference ctx = platform::ContextGles::createNew();
	//DEFAULT CONTEXT PER PLATFORM. CAN(WILL) BE OVERRIDEN BY PVRShell
	return ctx;
}

inline GraphicsContext IGraphicsContext::getWeakRef()
{
	return static_cast<platform::ContextGles&>(*this).getWeakReference();
}

namespace platform {
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
	const char* extensionString; //<! If minExtensionLevel is not "unspecified", this is the name of the extension that will be queried.
	Api minExtensionLevel; //<! Minimum API level that an extension may support this capability. If not "unspecified", implies there is an extension for it
	Api minCoreLevel;//<! Minimum API level that this capability is core. If "unspecified", no version supports it as core
};

/*!**********************************************************************************************************
\description This table describes what capabilities each OpenGL ES Api has.
************************************************************************************************************/
static ExtensionEntry extensionMap[] =
{
	//Common to all OpenGL ES versions - but other APIS might not support them...

	//Always extensions, OpenGL ES 2+
	{ ApiCapabilities::DebugCallback, "GL_KHR_debug", Api::OpenGLES2, Api::Unspecified },
	{ ApiCapabilities::AnisotropicFiltering, "GL_EXT_texture_filter_anisotropic", Api::OpenGLES2, Api::Unspecified },
	{ ApiCapabilities::BicubicFiltering, "GL_IMG_texture_filter_cubic", Api::OpenGLES2, Api::OpenGLESMaxVersion },

	//Always extensions, for OpenGL ES3+
	{ ApiCapabilities::ShaderPixelLocalStorage, "GL_EXT_shader_pixel_local_storage", Api::OpenGLES3, Api::Unspecified },
	{ ApiCapabilities::Tessellation, "GL_EXT_tessellation_shader", Api::OpenGLES31, Api::Unspecified },
	{ ApiCapabilities::ClearTexImage, "GL_IMG_clear_texture", Api::OpenGLES31, Api::Unspecified },
	{ ApiCapabilities::ShaderPixelLocalStorage2, "GL_EXT_shader_pixel_local_storage2", Api::OpenGLES3, Api::Unspecified },
	{ ApiCapabilities::GeometryShader, "GL_EXT_geometry_shader", Api::OpenGLES31, Api::Unspecified },

	//Extensions for any OpenGL ES2+, core at later versions
	{ ApiCapabilities::Texture3D, "GL_OES_texture_3D", Api::OpenGLES2, Api::OpenGLES3 },
	{ ApiCapabilities::ShadowSamplers, "GL_EXT_shadow_samplers", Api::OpenGLES2, Api::OpenGLES3 },
	{ ApiCapabilities::MapBuffer, "GL_OES_mapbuffer", Api::OpenGLES2, Api::OpenGLES3 },
	{ ApiCapabilities::MapBufferRange, "GL_EXT_map_buffer_range", Api::OpenGLES2, Api::OpenGLES3 },
	{ ApiCapabilities::TexureStorage, "GL_EXT_texture_storage_DISABLED", Api::OpenGLES2, Api::OpenGLES3 },
	{ ApiCapabilities::Instancing, "GL_EXT_draw_instanced", Api::OpenGLES2, Api::OpenGLES3 },
	{ ApiCapabilities::InvalidateFrameBuffer, "GL_EXT_discard_framebuffer", Api::OpenGLES2, Api::OpenGLES3 },

	//Core Only (all ES versions support them - but Vulkan does not!)
	{ ApiCapabilities::Uniforms, NULL, Api::Unspecified, Api::OpenGLES2 },
	{ ApiCapabilities::ShaderAttributeReflection, NULL, Api::Unspecified, Api::OpenGLES2 },
	//Core Only (ES 3)
	{ ApiCapabilities::Sampler, NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::TextureSwizzling, NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::Texture2DArray, NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::Ubo, NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::UintUniforms, NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::ShaderAttributeExplicitBind, NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::ClearBuffer, NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::FramebufferTextureLayer, NULL, Api::Unspecified, Api::OpenGLES3},
	//Core Only (ES 3.1)
	{ ApiCapabilities::ComputeShader, NULL, Api::Unspecified, Api::OpenGLES31 },
	{ ApiCapabilities::ImageStore, NULL, Api::Unspecified, Api::OpenGLES31 },
	{ ApiCapabilities::Ssbo, NULL, Api::Unspecified, Api::OpenGLES31 },
	{ ApiCapabilities::AtomicBuffer, NULL, Api::Unspecified, Api::OpenGLES31 },
	{ ApiCapabilities::Texture2DMS, NULL, Api::Unspecified, Api::OpenGLES31},


	{ ApiCapabilities::Texture2DArrayMS, NULL, Api::Unspecified, Api::Unspecified}

};
}


ContextGles::ContextGles(size_t implementationID)   : IGraphicsContext(Api::OpenGLESMaxVersion),
	m_ContextImplementationID(implementationID)
{ }

void ContextGles::waitIdle()
{
	gl::Finish();
}

Result ContextGles::init(OSManager& osManager)
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

	if (m_apiType < Api::OpenGLES31 && (uint32(osManager.getDeviceQueueTypesRequired() & DeviceQueueType::Compute) != 0))
	{
		Log(Log.Error, "Compute queues are not supported in OpenGL ES versions less than 3.1 -- Requested api was %s",
		    apiName(m_apiType));
		return Result::UnsupportedRequest;
	}

	//These cannot fail...
	gl::initGl();
	glext::initGlext();
	m_platformContext = &osManager.getPlatformContext();
	m_osManager = &osManager;

	m_platformContext->makeCurrent();

	int32 maxTexUnit = api::gpuCapabilities::get(*this, api::gpuCapabilities::TextureAndSamplers::MaxTextureImageUnit);
	m_renderStatesTracker.texSamplerBindings.clear();
	m_renderStatesTracker.texSamplerBindings.resize(maxTexUnit > 0 ? maxTexUnit : 100);
	setUpCapabilities();

#ifdef DEBUG
	if (m_apiCapabilities.supports(ApiCapabilities::DebugCallback))
	{
		glext::DebugMessageCallbackKHR(&debugCallback, NULL);
	}
#endif

	// create the default commandpool
	m_defaultCmdPool = createCommandPool();

	assets::SamplerCreateParam defaultSamplerInfo;
	m_defaultSampler = createSampler(defaultSamplerInfo);

	m_renderStatesTracker.viewport =
	  Rectanglei(0, 0, getDisplayAttributes().width, getDisplayAttributes().height);
	m_renderStatesTracker.scissor = m_renderStatesTracker.viewport;
	return Result::Success;
}

void ContextGles::setUpCapabilities()
{
	ApiCapabilitiesPrivate& caps = (ApiCapabilitiesPrivate&)m_apiCapabilities;
	caps.maxglslesversion = (m_apiType >= Api::OpenGLES31 ? 310 : m_apiType >= Api::OpenGLES3 ? 300 : 200);
	if (m_apiType >= Api::OpenGLES3)
	{
		GLint tmp;
#ifdef GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
		gl::GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &tmp);
		caps.uboOffsetAlignment = tmp;
#ifdef GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT
		gl::GetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &tmp);
		caps.ssboOffsetAlignment = tmp;
#endif
	}
#endif


	// EXTENSIONS -- SEE TOP OF THIS FILE.
	// For each extension, make sure that the we properly determine native or extension support.
	for (uint32 i = 0; i < sizeof(extensionMap) / sizeof(extensionMap[0]); ++i)
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

ContextGles::~ContextGles()
{
	release();
}

void ContextGles::popPipeline()
{
	if (m_pushedPipelines.size())
	{
		if (m_pushedPipelines.back().second)
		{
			m_pushedPipelines.back().first(m_pushedPipelines.back().second, *this);
		}
		m_pushedPipelines.pop_back();// pop the pipeline (including the null pipeline)
	}
	else
	{
		Log(Log.Error, "Tried to Pop pipeline, but a pipeline was not pushed(pipeline stack was empty)");
		//assertion(0);
	}
}

void ContextGles::pushPipeline(fnBindPipeline bindPipePtr, void* pipe)
{
	m_pushedPipelines.push_back(std::make_pair(bindPipePtr, pipe));
}

api::GraphicsPipeline ContextGles::createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc)
{
	return createGraphicsPipeline(desc, api::ParentableGraphicsPipeline());
}

api::GraphicsPipeline ContextGles::createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc,
    api::ParentableGraphicsPipeline parent)
{
	std::auto_ptr<api::impl::GraphicsPipelineImplBase>impl(new api::gles::GraphicsPipelineImplGles(getWeakRef()));
	api::GraphicsPipeline gp; gp.construct(impl);
	if (!static_cast<api::gles::GraphicsPipelineImplGles&>(gp->getImpl()).init(desc, parent.get(), gp.get()))
	{
		gp.reset();
		Log(Log.Error, "Failed to create graphics pipeline");
	}
	return gp;
}

api::ParentableGraphicsPipeline ContextGles::createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& desc)
{
	std::auto_ptr<api::impl::GraphicsPipelineImplBase> impl(new api::gles::GraphicsPipelineImplGles(getWeakRef()));
	api::ParentableGraphicsPipeline gp; gp.construct(impl);
	if (!static_cast<api::gles::GraphicsPipelineImplGles&>(gp->getImpl()).init(desc, NULL, gp.get()))
	{
		Log(Log.Error, "Failed to create parentable graphics pipeline");
		gp.reset();
	}
	return gp;
}

api::ComputePipeline ContextGles::createComputePipeline(const api::ComputePipelineCreateParam& createParam)
{
	std::auto_ptr<api::impl::ComputePipelineImplBase> impl(new api::gles::ComputePipelineImplGles(getWeakRef()));
	api::ComputePipeline cp; cp.construct(impl);

	if (!static_cast<api::gles::ComputePipelineImplGles&>(cp->getImpl()).init(createParam, cp.get()))
	{
		Log(Log.Error, "Failed to create compute pipeline");
		cp.reset();
	}
	return cp;
}

bool ContextGles::screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* pBuffer,
                                      ImageFormat requestedImageFormat)
{
	if (!pBuffer)
	{
		Log(Log.Error, "Screenshot not taken - provided buffer was null");
		return false;
	}

	gl::ReadPixels(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLint>(w),
	               static_cast<GLint>(h), GL_RGBA, GL_UNSIGNED_BYTE, pBuffer);

	GLenum err = gl::GetError();

	switch (err)
	{
	case GL_NO_ERROR:
	{
		switch (requestedImageFormat)
		{
		case IGraphicsContext::ImageFormatBGRA:
		{
			uint32 size = (w - x) * (h - y) * 4;

			// Switch the red and blue channels to convert to BGRA
			for (uint32 i = 0; i < size; i += 4)
			{
				byte tmp = pBuffer[i];
				pBuffer[i] = pBuffer[i + 2];
				pBuffer[i + 2] = tmp;
			}
		}
		break;
		default:
			// Do nothing
			break;
		}

		return true;
	}
	case GL_INVALID_VALUE:
	default:
		return false;
	}
}


string ContextGles::getInfo()const
{
	string out, tmp;
	out.reserve(2048);
	tmp.reserve(1024);
	out.append("\nGL:\n");

	tmp = strings::createFormatted("\tVendor:   %hs\n", (const char*)gl::GetString(GL_VENDOR));
	out.append(tmp);

	tmp = strings::createFormatted("\tRenderer: %hs\n", (const char*)gl::GetString(GL_RENDERER));
	out.append(tmp);

	tmp = strings::createFormatted("\tVersion:  %hs\n", (const char*)gl::GetString(GL_VERSION));
	out.append(tmp);

	tmp = strings::createFormatted("\tExtensions:  %hs\n",
	                               (const char*)gl::GetString(GL_EXTENSIONS));
	out.append(tmp);

	return out;
}

bool ContextGles::isExtensionSupported(const char8* extension) const
{
	if (m_extensions.empty())
	{
		const char* extensions = (const char*)gl::GetString(GL_EXTENSIONS);
		if (extensions)
		{
			m_extensions.assign(extensions);
		}
		else
		{
			m_extensions.assign("");
		}
	}
	return m_extensions.find(extension) != m_extensions.npos;
}


api::TextureStore ContextGles::createTexture()
{
	api::gles::TextureStoreGles tex;
	tex.construct(getWeakRef());
	return tex;
}

api::TextureView ContextGles::createTextureView(const api::TextureStore& texture, types::SwizzleChannels swizzle /* = types::SwizzleChannels() */)
{
	api::gles::TextureViewGles tex;
	tex.construct(static_cast<const api::gles::TextureStoreGles>(texture), types::ImageSubresourceRange(), swizzle);
	return tex;
}

api::TextureView ContextGles::createTextureView(const api::TextureStore& texture, types::ImageSubresourceRange range, types::SwizzleChannels swizzle /* = types::SwizzleChannels() */)
{
	api::gles::TextureViewGles tex;
	tex.construct(static_cast<const api::gles::TextureStoreGles>(texture), range, swizzle);
	return tex;
}

api::DescriptorSet ContextGles::createDescriptorSetOnDefaultPool(const api::DescriptorSetLayout& layout)
{
	//For OpenGL ES, DescriptorPool is dummy, so this is the only implementation and all sets
	//will appear to have the same pool, regardless of which one was used to create them.
	api::gles::DescriptorSetGles set;
	set.construct(layout, getDefaultDescriptorPool());
	if (!set->init())
	{
		set.reset();
	}
	return set;
}

api::CommandBuffer ContextGles::createCommandBufferOnDefaultPool()
{
	return getDefaultCommandPool()->allocateCommandBuffer();
}

api::SecondaryCommandBuffer ContextGles::createSecondaryCommandBufferOnDefaultPool()
{
	return getDefaultCommandPool()->allocateSecondaryCommandBuffer();
}

api::EffectApi ContextGles::createEffectApi(assets::Effect& effectDesc, api::GraphicsPipelineCreateParam& pipeDesc,
    api::AssetLoadingDelegate& effectDelegate)
{
	api::EffectApi effect;
	effect.construct(getWeakRef(), effectDelegate);
	if (effect->init(effectDesc, pipeDesc) != Result::Success)
	{
		effect.reset();
	}
	return effect;
}

api::Buffer ContextGles::createBuffer(uint32 size, types::BufferBindingUse bufferUsage,
                                      bool isMappable)
{
	api::gles::BufferGles buffer;
	buffer.construct(getWeakRef());
	if (!buffer->allocate(size, bufferUsage, isMappable)) { buffer.reset(); }
	return buffer;
}

api::RenderPass ContextGles::createRenderPass(const api::RenderPassCreateParam& renderPass)
{
	api::RenderPassGles rp;
	rp.construct(getWeakRef());
	if (!rp->init(renderPass))
	{
		rp.reset();
	}
	return rp;
}

api::Sampler ContextGles::createSampler(const assets::SamplerCreateParam& desc)
{
	api::gles::SamplerGles sampler;
	sampler.construct(getWeakRef());
	if (!sampler->init(desc))
	{
		sampler.reset();
	}
	return sampler;
}

api::Shader ContextGles::createShader(const Stream& shaderSrc, types::ShaderType type, const char* const* defines, uint32 numDefines)
{
	api::gles::ShaderGles vs;
	vs.construct(getWeakRef(), 0);
	if (!utils::loadShader(native::HContext_(), shaderSrc, type, defines, numDefines, *vs, &m_apiCapabilities))
	{
		Log(Log.Error, "Failed to create Shader.");
		vs.reset();
	}
	return vs;
}

api::Shader ContextGles::createShader(Stream& shaderData, types::ShaderType type, types::ShaderBinaryFormat binaryFormat)
{
	api::gles::ShaderGles vs;
	vs.construct(getWeakRef(), 0);
	if (!utils::loadShader(native::HContext_(), shaderData, type, binaryFormat, *vs, &m_apiCapabilities))
	{
		Log(Log.Error, "Failed to create Shader.");
		vs.reset();
	}
	return vs;
}

api::Fbo ContextGles::createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass,
    const api::OnScreenFboCreateParam& onScreenFboCreateParam)
{
	if (!renderPass.isValid())
	{
		assertion(renderPass.isValid(), "Invalid Renderpass object");
		Log("Invalid Renderpass object");
		return api::Fbo();
	}
	pvr::api::FboCreateParam fboInfo;
	fboInfo.width = getDisplayAttributes().width;
	fboInfo.height = getDisplayAttributes().height;
	fboInfo.setRenderPass(renderPass);
	fboInfo.setRenderPass(renderPass);
	pvr::api::gles::DefaultFboGles fbo;
	fbo.construct(getWeakRef());
	if (fbo->init(fboInfo))
	{
		fbo->bind(*this, types::FboBindingTarget::ReadWrite);
	}
	else
	{
		fbo.reset();
	}
	return fbo;
}

api::BufferView ContextGles::createBufferView(const pvr::api::Buffer& buffer, pvr::uint32 offset, pvr::uint32 range)
{
	api::gles::BufferViewGles ubo;
	if (hasApiCapability(ApiCapabilities::Ubo)) // If it has SSBOS/atomics, it will definitely have Ubos...
	{
		assertion(static_cast<pvr::uint32>(buffer->getBufferUsage() & types::BufferBindingUse::UniformBuffer) != 0
		          || static_cast<pvr::uint32>(buffer->getBufferUsage() & types::BufferBindingUse::StorageBuffer) != 0
		          || static_cast<pvr::uint32>(buffer->getBufferUsage()) != 0);
		ubo.construct(buffer, offset, std::min(range, buffer->getSize() - offset));
		assertion(range == 0xFFFFFFFFu || (range <= buffer->getSize() - offset));
	}
	else
	{
		pvr::Log("Indexed buffers (Ubo, Ssbo) not supported by this api");
	}
	return ubo;
}

api::BufferView ContextGles::createBufferAndView(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable)
{
	api::gles::BufferViewGles ubo;
	if (hasApiCapability(ApiCapabilities::Ubo)) // If it has SSBOS/atomics, it will definitely have Ubos...
	{
		assertion(static_cast<pvr::uint32>(bufferUsage & types::BufferBindingUse::UniformBuffer) != 0 ||
		          static_cast<pvr::uint32>(bufferUsage & types::BufferBindingUse::StorageBuffer) != 0);
		ubo.construct(createBuffer(size, bufferUsage, isMappable), 0, size);
	}
	else
	{
		pvr::Log("Indexed buffers (Ubo, Ssbo) not supported by this api");
	}
	return ubo;
}

api::PipelineLayout ContextGles::createPipelineLayout(const api::PipelineLayoutCreateParam& desc)
{
	pvr::api::gles::PipelineLayoutGles pipelayout;
	pipelayout.construct(getWeakRef());
	if (!pipelayout->init(desc))
	{
		pipelayout.reset();
	}
	return pipelayout;
}

api::Fbo ContextGles::createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass)
{
	pvr::api::FboCreateParam fboInfo;
	fboInfo.setRenderPass(renderPass);
	pvr::api::gles::DefaultFboGles fbo;
	fbo.construct(getWeakRef());
	fboInfo.width = getDisplayAttributes().width;
	fboInfo.height = getDisplayAttributes().height;

	if (fbo->init(fboInfo))
	{
		fbo->bind(*this, types::FboBindingTarget::ReadWrite);
	}
	else
	{
		fbo.reset();
	}
	return fbo;
}

api::Fbo ContextGles::createOnScreenFbo(uint32 swapIndex, types::LoadOp colorLoadOp, types::StoreOp colorStoreOp,
                                        types::LoadOp depthLoadOp, types::StoreOp depthStoreOp, types::LoadOp stencilLoadOp,
                                        types::StoreOp stencilStoreOp)
{
	// create the default fbo
	api::RenderPassColorInfo colorInfo;
	api::RenderPassDepthStencilInfo dsInfo;
	colorInfo.format = getPresentationImageFormat();
	dsInfo.format = getDepthStencilImageFormat();
	colorInfo.loadOpColor = colorLoadOp;
	colorInfo.storeOpColor = colorStoreOp;

	dsInfo.loadOpDepth = depthLoadOp;
	dsInfo.storeOpDepth = depthStoreOp;
	dsInfo.loadOpStencil = stencilLoadOp;
	dsInfo.storeOpStencil = stencilStoreOp;

	pvr::api::RenderPassCreateParam renderPassDesc;
	renderPassDesc.setColorInfo(0, colorInfo);
	renderPassDesc.setDepthStencilInfo(dsInfo);

	// Require at least one sub pass
	pvr::api::SubPass subPass;
	subPass.setColorAttachment(0, 0); // use color attachment 0
	renderPassDesc.setSubPass(0, subPass);

	return createOnScreenFboWithRenderPass(0, createRenderPass(renderPassDesc));
}

api::DescriptorPool ContextGles::createDescriptorPool(const api::DescriptorPoolCreateParam& createParam)
{
	api::gles::DescriptorPoolGles descPool;
	descPool.construct(getWeakRef());
	if (!descPool->init(createParam))
	{
		descPool.reset();
	}
	return descPool;
}

api::CommandPool ContextGles::createCommandPool()
{
	api::gles::CommandPoolGles cmdpool = api::gles::CommandPoolGles_::createNew(getWeakRef());
	if (!cmdpool->init()) { cmdpool.reset(); }
	return cmdpool;
}

api::Fbo ContextGles::createFbo(const api::FboCreateParam& desc)
{
	api::gles::FboGles fbo;
	// create fbo
	fbo.construct(getWeakRef());
	if (!fbo->init(desc))
	{
		fbo.reset();
	}
	return fbo;
}

api::FboSet ContextGles::createFboSet(const Multi<api::FboCreateParam>& fboInfo)
{
	api::FboSet fbo;
	for (uint32 i = 0; i < fboInfo.size(); ++i) {  fbo[i] = createFbo(fboInfo[i]);   }
	return fbo;
}


api::DescriptorSetLayout ContextGles::createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& desc)
{
	api::gles::DescriptorSetLayoutGles layout;
	layout.construct(getWeakRef(), desc);
	return layout;
}


api::FboSet ContextGles::createOnScreenFboSet(types::LoadOp colorLoadOp, types::StoreOp colorStoreOp, types::LoadOp depthLoadOp, types::StoreOp depthStoreOp,
    types::LoadOp stencilLoadOp, types::StoreOp stencilStoreOp)
{
	api::FboSet fbos;
	fbos.add(createOnScreenFbo(0, colorLoadOp, colorStoreOp, depthLoadOp, depthStoreOp, stencilLoadOp, stencilStoreOp));
	return fbos;
}

api::FboSet ContextGles::createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass)
{
	api::FboSet fbos;
	fbos.add(createOnScreenFboWithRenderPass(0, renderPass));
	return fbos;
}

api::FboSet ContextGles::createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass,
    pvr::Multi<api::OnScreenFboCreateParam>& onScreenFboCreateParams)
{
	api::FboSet fbos;
	for (uint32 i = 0; i < (uint32)FrameworkCaps::MaxSwapChains; ++i)
	{
		fbos[i] = createOnScreenFboWithRenderPass(i, renderPass, onScreenFboCreateParams[i]);
	}
	return fbos;
}
}
}
//!\endcond
