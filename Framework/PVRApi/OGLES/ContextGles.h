/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ContextGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Definition of the OpenGL ES implementation of the GraphicsContext (pvr::platform::ContextGles)
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRCore/IPlatformContext.h"
#include "PVRApi/OGLES/FboGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/ApiObjects/DescriptorTable.h"
#include "PVRApi/GpuCapabilities.h"
#include "PVRApi/ApiObjectTypes.h"
#include <map>
#include <set>
#include <stdlib.h>
#include <bitset>

namespace pvr {
/*!**********************************************************************************************************
\brief Contains functions and methods related to the wiring of the PVRApi library to the underlying platform,
including extensions and the Context classes.
************************************************************************************************************/
namespace platform {

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
	{ ApiCapabilities::DebugCallback,				"GL_KHR_debug",							Api::OpenGLES2,		Api::Unspecified },
	{ ApiCapabilities::AnisotropicFiltering,		"GL_EXT_texture_filter_anisotropic",	Api::OpenGLES2,		Api::Unspecified },
	//Extensions for any OpenGL ES2+, core later
	{ ApiCapabilities::Texture3D,					"GL_OES_texture_3D",					Api::OpenGLES2,		Api::OpenGLES3 },
	{ ApiCapabilities::ShadowSamplers,				"GL_EXT_shadow_samplers",				Api::OpenGLES2,		Api::OpenGLES3 },
	{ ApiCapabilities::MapBuffer,					"GL_OES_mapbuffer",						Api::OpenGLES2,		Api::OpenGLES3 },
	{ ApiCapabilities::TexureStorage,				"GL_EXT_texture_storage_DISABLED",				Api::OpenGLES2,		Api::OpenGLES3 },
    { ApiCapabilities::Instancing,               "GL_EXT_draw_instanced",                Api::OpenGLES2,     Api::OpenGLES3 },
    { ApiCapabilities::InvalidateFrameBuffer,       "GL_EXT_discard_framebuffer",           Api::OpenGLES2,     Api::OpenGLES3 },
	//Extensions for OpenGL ES3+
	{ ApiCapabilities::ShaderPixelLocalStorage,		"GL_EXT_shader_pixel_local_storage",	Api::OpenGLES3,		Api::Unspecified },

	//Core Only
	{ ApiCapabilities::Uniforms,					NULL, Api::Unspecified, Api::OpenGLES2 },
	{ ApiCapabilities::ShaderAttributeReflection,	NULL, Api::Unspecified, Api::OpenGLES2 },

	{ ApiCapabilities::Sampler,						NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::TextureSwizzling,			NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::Texture2DArray,				NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::Ubo,							NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::UintUniforms,				NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::ShaderAttributeExplicitBind, NULL, Api::Unspecified, Api::OpenGLES3 },
	{ ApiCapabilities::ClearBuffer,					NULL, Api::Unspecified, Api::OpenGLES3 },

	{ ApiCapabilities::ComputeShader,				NULL, Api::Unspecified, Api::OpenGLES31 },
	{ ApiCapabilities::ImageStore,					NULL, Api::Unspecified,	Api::OpenGLES31 },
	{ ApiCapabilities::Ssbo,						NULL, Api::Unspecified,	Api::OpenGLES31 },
	{ ApiCapabilities::AtomicBuffer,				NULL, Api::Unspecified,	Api::OpenGLES31 },

};

/*!*********************************************************************************************************************
\brief This class is added as the Debug Callback. Redirects the debug output to the Log object.
***********************************************************************************************************************/
inline void GL_APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                      const GLchar* message, const void* userParam)
{
	Log(Log.Debug, "%s", message);
}

/*!*********************************************************************************************************************
\brief IGraphicsContext implementation that supports all OpenGL ES 2 and above versions.
***********************************************************************************************************************/
class ContextGles : public IGraphicsContext
{
public:
	struct BufferRange
	{
		api::Buffer buffer;
		uint32 offset;
		uint32 range;
		BufferRange() : offset(0), range(0) {}
		BufferRange(const api::Buffer& buffer, uint32 offset, uint32 range) : buffer(buffer), offset(offset), range(range) {}
	};

private:
	struct BufferBindingComp
	{
		BufferBindingComp(uint16 index) : index(index) { }
		bool operator()(const std::pair<uint16, BufferRange>& p)
		{
			return p.first == index;
		}
		uint16 index;
	};

public:
	/*!*********************************************************************************************************************
	\brief Create a new, empty, uninitialized context.
	***********************************************************************************************************************/
	ContextGles() {}

	/*!*********************************************************************************************************************
	\brief Virtual destructor.
	***********************************************************************************************************************/
	virtual ~ContextGles()
	{
		release();
	}

	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Initializes this class using an OS manager.
	\description This function must be called before using the Context. Will use the OS manager to make this Context
	ready to use.
	***********************************************************************************************************************/
	Result::Enum init(OSManager& osManager, GraphicsContext& my_wrapper)
	{
		if (m_osManager)
		{
			return Result::AlreadyInitialised;
		}
		if (!osManager.getPlatformContext().isInitialised())
		{
			return Result::NotInitialised;
		}
		m_this_shared = my_wrapper;
		m_apiType = osManager.getApiTypeRequired(); //PlatformContext should have already made sure that this is actually possible.

		if (m_apiType < Api::OpenGLES31 && ((osManager.getDeviceQueueTypesRequired() & DeviceQueueType::Compute) != 0))
		{
			Log(Log.Error, "Compute queues are not supported in OpenGL ES versions less than 3.1 -- Requested api was %s",
			    Api::getApiName(m_apiType));
			return Result::UnsupportedRequest;
		}

#if BUILD_API_MAX&&BUILD_API_MAX<30
		if (m_apiType == Api::OpenGLES30)
		{
			Log(Log.Critical, "PVRApi library built without OpenGL ES 3.0 support, (BUILD_API_MAX was defined and less than 30)"
			    " but an ES 3.0 context was requested.");
			return Result::UnsupportedRequest;
		}
#endif
#if BUILD_API_MAX&&BUILD_API_MAX<31
		if (m_apiType >= Api::OpenGLES31)
		{
			Log(Log.Critical,
			    "PVRApi library built without OpenGL ES 3.1 support, (BUILD_API_MAX was defined and greater than 30)"
			    " but an ES 3.1 context was requested.");
			return Result::UnsupportedRequest;
		}
#endif

		//These cannot fail...
		initialiseNativeContext();
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

		assets::SamplerCreateParam defaultSamplerInfo;
		m_defaultSampler = createSampler(defaultSamplerInfo);

		m_renderStatesTracker.viewport =
		  Rectanglei(0, 0, getDisplayAttributes().width, getDisplayAttributes().height);
		m_renderStatesTracker.scissor = m_renderStatesTracker.viewport;
		return Result::Success;
	}

	void setUpCapabilities()
	{
		ApiCapabilitiesPrivate& caps = (ApiCapabilitiesPrivate&)m_apiCapabilities;
		caps.maxglslesversion = (m_apiType >= Api::OpenGLES31 ? 310 : m_apiType >= Api::OpenGLES3 ? 300 : 200);

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

	/*!****************************************************************************************************************
	\brief	Implementation of IGraphicsContext. Release the resources held by this context.
	*******************************************************************************************************************/
	void release()
	{
		if (m_osManager) //Is already initialised?
		{
			m_osManager = 0;
			memset(&(ApiCapabilitiesPrivate&)m_apiCapabilities, 0, sizeof(ApiCapabilitiesPrivate));
			m_defaultPool.release();
			m_defaultRenderPass.release();
			m_extensions.clear();
			m_ContextImplementationID = (size_t)(-1);
			m_platformContext = 0;
			m_apiType = Api::Unspecified;
			m_renderStatesTracker.releaseAll();
			releaseNativeContext();
		}
		m_this_shared.release();
	}		//Error if no display set

	/*!****************************************************************************************************************
	\brief	Implementation of IGraphicsContext. Τake a screenshot in the specified buffer of the specified screen area.
	*******************************************************************************************************************/
	Result::Enum screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* buffer,
	                                 ImageFormat requestedImageFormat);
	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Query if a specific extension is supported.
	\param extension A c-style string representing the extension
	\return True if the extension is supported
	***********************************************************************************************************************/
	bool isExtensionSupported(const char8* extension) const;

	/*!*********************************************************************************************************************
	\brief  Implementation of IGraphicsContext. Print information about this IGraphicsContext.
	***********************************************************************************************************************/
	std::string getInfo()const;

	/*!*********************************************************************************************************************
	\brief	return true if last bound pipeline was graphics
	\return	bool
	***********************************************************************************************************************/
	bool isLastBoundPipelineGraphics()const	{ return m_renderStatesTracker.lastBoundPipe == RenderStatesTracker::PipelineGraphics; }

	/*!*********************************************************************************************************************
	\brief	return true if last bound pipeline was compute
	\return	bool
	***********************************************************************************************************************/
	bool isLastBoundPipelineCompute()const{ return m_renderStatesTracker.lastBoundPipe == RenderStatesTracker::PipelineCompute; }

	void onBind(pvr::api::impl::GraphicsPipelineImpl* pipeline)
	{
		m_renderStatesTracker.lastBoundPipe = RenderStatesTracker::PipelineGraphics;
		setBoundGraphicsPipeline(pipeline);
	}

	void onBind(pvr::api::impl::ComputePipelineImpl* pipeline)
	{
		m_renderStatesTracker.lastBoundPipe = RenderStatesTracker::PipelineCompute;
		setBoundComputePipeline(pipeline);
	}

	/*!*********************************************************************************************************************
	\brief Internal use. State tracking. Outside code calls this to notify the context that a new texture has been bound to a texture unit.
	***********************************************************************************************************************/
	void onBind(const api::impl::TextureViewImpl& texture, uint16 bindIndex)
	{
		if (m_renderStatesTracker.texSamplerBindings.size() <= bindIndex)
		{
			PVR_ASSERT(false && "UnSupported Texture Unit binding");
			Log("UnSupported Texture Unit binding %d", bindIndex);
		}
		m_renderStatesTracker.lastBoundTexBindIndex = bindIndex;
		m_renderStatesTracker.texSamplerBindings[bindIndex].toBindTex = &texture;
	}

	void onBind(const api::impl::SamplerImpl& sampler, uint16 bindIndex)
	{
		if (m_renderStatesTracker.texSamplerBindings.size() <= bindIndex)
		{
			PVR_ASSERT(false && "UnSupported Sampler Unit binding");
			Log("UnSupported Sampler Unit binding %d", bindIndex);
		}
		m_renderStatesTracker.texSamplerBindings[bindIndex].lastBoundSampler = &sampler;
	}

	void onBindUbo(uint16 bindIndex, const api::Buffer& buffer, uint32 offset, uint32 range)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(m_renderStatesTracker.uboBufferBindings.begin(), m_renderStatesTracker.uboBufferBindings.end(), BufferBindingComp(bindIndex));
		if (it != m_renderStatesTracker.uboBufferBindings.end())
		{
			it->second.offset = offset;
			it->second.range = range;
			it->second.buffer = buffer;
		}
		else
		{
			m_renderStatesTracker.uboBufferBindings.push_back(std::make_pair(bindIndex, BufferRange(buffer, offset, range)));
		}
	}
	void onBindSsbo(uint16 bindIndex, const api::Buffer& buffer, uint32 offset, uint32 range)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(m_renderStatesTracker.ssboBufferBindings.begin(), m_renderStatesTracker.ssboBufferBindings.end(), BufferBindingComp(bindIndex));
		if (it != m_renderStatesTracker.ssboBufferBindings.end())
		{
			it->second.offset = offset;
			it->second.range = range;
			it->second.buffer = buffer;
		}
		else
		{
			m_renderStatesTracker.ssboBufferBindings.push_back(std::make_pair(bindIndex, BufferRange(buffer, offset, range)));
		}
	}
	void onBindAtomicBuffer(uint16 bindIndex, const api::Buffer& buffer, uint32 offset, uint32 range)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(m_renderStatesTracker.atomicBufferBindings.begin(), m_renderStatesTracker.atomicBufferBindings.end(), BufferBindingComp(bindIndex));
		if (it != m_renderStatesTracker.atomicBufferBindings.end())
		{
			it->second.offset = offset;
			it->second.range = range;
			it->second.buffer = buffer;
		}
		else
		{
			m_renderStatesTracker.atomicBufferBindings.push_back(std::make_pair(bindIndex, BufferRange(buffer, offset, range)));
		}
	}

	BufferRange getBoundProgramBufferUbo(uint16 bindIndex)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(m_renderStatesTracker.uboBufferBindings.begin(), m_renderStatesTracker.uboBufferBindings.end(), BufferBindingComp(bindIndex));
		return (it != m_renderStatesTracker.uboBufferBindings.end() ? it->second : BufferRange());
	}
	BufferRange getBoundProgramBufferSsbo(uint16 bindIndex)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it = 
			std::find_if(m_renderStatesTracker.ssboBufferBindings.begin(), m_renderStatesTracker.ssboBufferBindings.end(), BufferBindingComp(bindIndex));
		return (it != m_renderStatesTracker.ssboBufferBindings.end() ? it->second : BufferRange());
	}
	BufferRange getBoundProgramBufferAtomicBuffer(uint16 bindIndex)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it = 
			std::find_if(m_renderStatesTracker.atomicBufferBindings.begin(), m_renderStatesTracker.atomicBufferBindings.end(), BufferBindingComp(bindIndex));
		return (it != m_renderStatesTracker.atomicBufferBindings.end() ? it->second : BufferRange());
	}

	const api::Fbo& getBoundFbo()const
	{
		return m_renderStatesTracker.boundFbo;
	}

	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for
	rendering). This version uses a RenderPass that was provided by the user. This renderpass must be compatible with the BackBuffer
	format and options - this is the user's responsibility.
	\param[in] renderPass The renderpass that this FBO will use
	\return A new FBO who can be used to write to the Screen.
	***********************************************************************************************************************/
	api::Fbo createOnScreenFboWithRenderPass(const api::RenderPass& renderPass);

	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Create an FBO that represents the actual backbuffer (i.e. an fbo to be used for
	rendering). This implementation uses the most typical parameters used for the backbuffer, and also creates the RenderPass for the
	FBO.
	WARNING: Defaults discard Depth and Stencil at the end of the renderpass, for performance. If you wish to preserve depth
	and/or stencil, please specify StoreOp:Store for depth and stencil.
	\param[in] colorLoadOp The Load Operation for the color attachment (Default is LoadOp::Clear, clearing the Color of the screen at every frame start)
	\param[in] colorStoreOp The Store Operation for the color attachment (Default is StoreOp::Store, storing the Color before buffer swapping)
	\param[in] depthLoadOp The Load Operation for the depth (Default is LoadOp::Clear, clearing the Depth at every frame start)
	\param[in] depthStoreOp The Store Operation for the depth buffer (Default is StoreOp::Ignore, discarding the Depth before buffer swapping)
	\param[in] stencilLoadOp The Load Operation for the stencil buffer (Default is LoadOp::Clear, clearing the Stencil at every frame start)
	\param[in] stencilStoreOp The Store Operation for the stencil buffer (Default is StoreOp::Ignore, discarding the Stencil before buffer swapping)
	\param[in] numColorSamples The number of Samples for an MSAA Color attachment
	\param[in] numDepthStencilSamples The number of Samples for an MSAA Depth/Stencil attachment
	\return A new FBO who can be used to write to the Screen.
	***********************************************************************************************************************/
	api::Fbo createOnScreenFboWithParams(LoadOp::Enum colorLoadOp = LoadOp::Clear, StoreOp::Enum colorStoreOp = StoreOp::Store,
	                                     LoadOp::Enum depthLoadOp = LoadOp::Clear, StoreOp::Enum depthStoreOp = StoreOp::Ignore,
	                                     LoadOp::Enum stencilLoadOp = LoadOp::Clear, StoreOp::Enum stencilStoreOp = StoreOp::Ignore,
	                                     uint32 numColorSamples = 1, uint32 numDepthStencilSamples = 1);

	/*!*********************************************************************************************************************
	\brief  Implementation of IGraphicsContext. Return the  default renderpass.
	\return The default renderpass
	***********************************************************************************************************************/
	const api::RenderPass& getDefaultRenderPass()const { return m_defaultRenderPass; }

	/*!*********************************************************************************************************************
	\brief  Implementation of IGraphicsContext. Return the  platform context that powers this graphics context.
	***********************************************************************************************************************/
	IPlatformContext& getPlatformContext()const { return *m_platformContext; }

	/*!*********************************************************************************************************************
	\brief Internal use. State tracking. enable veretx atribute.
	***********************************************************************************************************************/
	void enableAttribute(uint16 attributeIdx)
	{
		m_renderStatesTracker.attributesToEnableBitfield |= (1 << attributeIdx);
		if ((m_renderStatesTracker.attributesEnabledBitfield & (1 << attributeIdx)) == 0)
		{
			gl::EnableVertexAttribArray(attributeIdx);
			m_renderStatesTracker.attributesEnabledBitfield |= (1 << attributeIdx);
		}
		m_renderStatesTracker.attributesMaxToEnable = (std::max)(attributeIdx, m_renderStatesTracker.attributesMaxToEnable) + 1;
	}

	/*!*********************************************************************************************************************
	\brief Internal use. State tracking.disable unneeded vertex attributes.
	***********************************************************************************************************************/
	void disableUnneededAttributes()
	{
		uint32 deactivate = m_renderStatesTracker.attributesEnabledBitfield ^ m_renderStatesTracker.attributesToEnableBitfield;

		for (uint8 i = 0; i < m_renderStatesTracker.attributesMaxEnabled; ++i)
		{
			if (Bitfield<uint32>::isSet(deactivate, i))
			{
				gl::DisableVertexAttribArray(i);
			}
		}
		m_renderStatesTracker.attributesEnabledBitfield = m_renderStatesTracker.attributesToEnableBitfield;
		m_renderStatesTracker.attributesMaxEnabled = m_renderStatesTracker.attributesMaxToEnable;
		m_renderStatesTracker.attributesMaxToEnable = 0;
		m_renderStatesTracker.attributesToEnableBitfield = 0;
	}

	api::DescriptorSetLayout createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& desc);

	api::DescriptorSet allocateDescriptorSet(const api::DescriptorSetLayout& layout,
	    const api::DescriptorPool& pool);

	/*!****************************************************************************************************************
	\brief	A map of VBO bindings.
	*******************************************************************************************************************/
	typedef std::map<uint16, api::Buffer> VboBindingMap;//< bind index, buffer

	struct TextureBinding
	{
		const api::impl::TextureViewImpl* toBindTex;
		const api::impl::TextureViewImpl* lastBoundTex;
		const api::impl::SamplerImpl* lastBoundSampler;


		TextureBinding() : toBindTex(0), lastBoundTex(0), lastBoundSampler(0) {}
	};

	/*!****************************************************************************************************************
	\brief	A map of texture bindings.
	*******************************************************************************************************************/
	typedef std::vector<TextureBinding> TextureBindingList;

	typedef std::vector<std::pair<uint16, BufferRange>/**/> ProgBufferBingingList;

	/*!****************************************************************************************************************
	\brief internal state tracker for OpenGLES
	*******************************************************************************************************************/
	struct RenderStatesTracker
	{
		enum LastBoundPipe{ PipelineGraphics, PipelineCompute, PipelineNone };
		friend class ::pvr::platform::ContextGles;
		// Stencil
		struct DepthStencilState
		{
			bool depthTest;
			bool depthWrite;
			uint32 stencilWriteMask;
			bool enableStencilTest;
			int32 clearStencilValue;
			ComparisonMode::Enum depthOp;
			// Front
			api::StencilOp::Enum stencilFailOpFront;
			api::StencilOp::Enum depthStencilPassOpFront;
			api::StencilOp::Enum depthFailOpFront;
			ComparisonMode::Enum stencilOpFront;

			// Back
			api::StencilOp::Enum stencilFailOpBack;
			api::StencilOp::Enum depthStencilPassOpBack;
			api::StencilOp::Enum depthFailOpBack;
			ComparisonMode::Enum stencilOpBack;

			int32 refFront, refBack;

			uint32 readMaskFront, readMaskBack, writeMaskFront, writeMaskBack;

			glm::bvec4 m_colorWriteMask;

			DepthStencilState() : depthTest(false), depthWrite(true),
				stencilWriteMask(0xFFFFFFFF), enableStencilTest(false), clearStencilValue(0),
				depthOp(ComparisonMode::Less),
				stencilFailOpFront(api::StencilOp::DefaultStencilFailFront),
				depthStencilPassOpFront(api::StencilOp::DefaultDepthStencilPassFront),
				depthFailOpFront(api::StencilOp::DefaultDepthFailFront),
				stencilOpFront(ComparisonMode::DefaultStencilOpFront),

				stencilFailOpBack(api::StencilOp::DefaultStencilFailBack),
				depthStencilPassOpBack(api::StencilOp::DefaultDepthStencilPassBack),
				depthFailOpBack(api::StencilOp::DefaultDepthFailBack),
				stencilOpBack(ComparisonMode::DefaultStencilOpBack),

				refFront(0), refBack(0), readMaskFront(0xff), readMaskBack(0xff), writeMaskFront(0xff), writeMaskBack(0xff)
			{}
		};
		struct IndexBufferState
		{
			api::Buffer buffer;
			uint32 offset;
			IndexType::Enum indexArrayFormat;
			IndexBufferState() : offset(0) {}
		};

		IndexBufferState iboState;
		VboBindingMap vboBindings;
		api::Buffer lastBoundVbo;
		TextureBindingList texSamplerBindings;
		uint32 lastBoundTexBindIndex;
		DepthStencilState depthStencil;
		api::Fbo boundFbo;
		PrimitiveTopology::Enum primitiveTopology;
		glm::bvec4 colorWriteMask;
		api::Face::Enum cullFace;
		api::PolygonWindingOrder::Enum polyWindingOrder;
		ProgBufferBingingList uboBufferBindings;
		ProgBufferBingingList ssboBufferBindings;
		ProgBufferBingingList atomicBufferBindings;
		api::BlendOp::Enum rgbBlendOp;
		api::BlendOp::Enum alphaBlendOp;
		api::BlendFactor::Enum srcRgbFactor, srcAlphaFactor, destRgbFactor, destAlphaFactor;
		bool enabledScissorTest;
		bool enabledBlend;

		Rectanglei viewport;
		Rectanglei scissor;

	private:
		void releaseAll() {	*this = RenderStatesTracker();	}
		uint32 attributesToEnableBitfield;
		uint32 attributesEnabledBitfield;
		uint16 attributesMaxToEnable;
		uint16 attributesMaxEnabled;
		LastBoundPipe lastBoundPipe;
	public:
		RenderStatesTracker() : lastBoundTexBindIndex(0), colorWriteMask(true),
			cullFace(api::Face::Back),
			polyWindingOrder(api::PolygonWindingOrder::FrontFaceCCW),
			rgbBlendOp(api::BlendOp::Add), alphaBlendOp(api::BlendOp::Add),
			srcRgbFactor(api::BlendFactor::One), srcAlphaFactor(api::BlendFactor::One),
			destRgbFactor(api::BlendFactor::Zero), destAlphaFactor(api::BlendFactor::Zero),
			enabledScissorTest(false), enabledBlend(false), viewport(0, 0, 0, 0), scissor(0, 0, 0, 0),
			attributesToEnableBitfield(0), attributesEnabledBitfield(0), attributesMaxToEnable(0),
			attributesMaxEnabled(0), lastBoundPipe(PipelineNone){}
		~RenderStatesTracker() {}
	};
	RenderStatesTracker& getCurrentRenderStates() { return m_renderStatesTracker; }
	RenderStatesTracker const& getCurrentRenderStates()const { return m_renderStatesTracker; }

	api::Sampler getDefaultSampler()const { return m_defaultSampler; }

protected:
	RenderStatesTracker m_renderStatesTracker;
	/*!*********************************************************************************************************************
	\brief Implements IGraphicsContext. Get the source of a shader as a stream.
	\param stream A Stream object containing the source of the shader.
	\param outSourceData Reference to an std::string where the source data will be read to.
	\return Result::Success on success
	***********************************************************************************************************************/
	Result::Enum createShader(const Stream& stream, string& outSourceData);

	/*!*********************************************************************************************************************
	\brief Internal use. State tracking. Notify fbo unbind.
	***********************************************************************************************************************/
	ContextGles(size_t implementationId);

private:
	api::RenderPass m_defaultRenderPass;
	size_t m_ContextImplementationID;
	IPlatformContext* m_platformContext;
	mutable std::string m_extensions;
	api::Sampler m_defaultSampler;
	
};
}
}
