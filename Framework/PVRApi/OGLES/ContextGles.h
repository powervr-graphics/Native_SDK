/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ContextGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Definition of the OpenGL ES implementation of the GraphicsContext (platform::ContextGles)
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRCore/IPlatformContext.h"
#include "PVRApi/OGLES/FboGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRApi/GpuCapabilities.h"
#include <map>
#include <set>
#include <stdlib.h>
#include <bitset>

namespace pvr {
inline void reportDestroyedAfterContext(const char* objectName)
{
	Log(Log.Warning, "Attempted to destroy object of type [%s] after its corresponding context", objectName);
#ifdef DEBUG
#endif
}

/*!**********************************************************************************************************
\brief Contains functions and methods related to the wiring of the PVRApi library to the underlying platform,
including extensions and the Context classes.
************************************************************************************************************/
namespace platform {

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
class ContextGles : public IGraphicsContext, public EmbeddedRefCount<ContextGles>
{
public:
	typedef void(*fnBindPipeline)(void*, IGraphicsContext& context);
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
	ContextGles() : IGraphicsContext(Api::OpenGLESMaxVersion) {}

	/*!*********************************************************************************************************************
	\brief Virtual destructor.
	***********************************************************************************************************************/
	virtual ~ContextGles();

	void waitIdle();

	void popPipeline();

	void pushPipeline(fnBindPipeline bindPipePtr, void* pipe);

	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Initializes this class using an OS manager.
	\description This function must be called before using the Context. Will use the OS manager to make this Context
	ready to use.
	***********************************************************************************************************************/
	Result init(OSManager& osManager);

	void setUpCapabilities();

	/*!****************************************************************************************************************
	\brief	Implementation of IGraphicsContext. Release the resources held by this context.
	*******************************************************************************************************************/
	void release()
	{
		if (m_osManager) //Is already initialized?
		{
			m_osManager = 0;
			memset(&(ApiCapabilitiesPrivate&)m_apiCapabilities, 0, sizeof(ApiCapabilitiesPrivate));
			m_extensions.clear();
			m_defaultCmdPool.reset();
			m_defaultDescPool.reset();
			m_defaultSampler.reset();
			m_ContextImplementationID = (size_t)(-1);
			m_platformContext = 0;
			m_apiType = Api::Unspecified;
			m_renderStatesTracker.releaseAll();
			gl::releaseGl();
		}
	}		//Error if no display set

	/*!****************************************************************************************************************
	\brief	Implementation of IGraphicsContext. Τake a screenshot in the specified buffer of the specified screen area.
	*******************************************************************************************************************/
	bool screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* buffer,
	                         ImageFormat requestedImageFormat);

	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Query if a specific extension is supported.
	\param extension A c-style string representing the extension
	\return True if the extension is supported
	***********************************************************************************************************************/
	bool isExtensionSupported(const char8* extension) const;

	api::Fbo createFbo(const api::FboCreateParam& desc);

	api::FboSet createFboSet(const Multi<api::FboCreateParam>& fboInfo);


	/*!****************************************************************************************************************
	\brief	Get the ImageDataFormat associated with the presentation image for this GraphicsContext.
	*******************************************************************************************************************/
	const api::ImageDataFormat getPresentationImageFormat()const
	{
		api::ImageDataFormat presentationFormat;
		presentationFormat.format = PixelFormat(getDisplayAttributes().redBits ? 'r' : 0, getDisplayAttributes().greenBits ? 'g' : 0,
		                                        getDisplayAttributes().blueBits ? 'b' : 0, getDisplayAttributes().alphaBits ? 'a' : 0,
		                                        (uint8)getDisplayAttributes().redBits, (uint8)getDisplayAttributes().greenBits,
		                                        (uint8)getDisplayAttributes().blueBits, (uint8)getDisplayAttributes().alphaBits);
		presentationFormat.colorSpace = getDisplayAttributes().frameBufferSrgb ? types::ColorSpace::sRGB : types::ColorSpace::lRGB;;
		presentationFormat.dataType = VariableType::UnsignedByteNorm;
		return presentationFormat;
	}

	/*!****************************************************************************************************************
	\brief	Get the ImageDataFormat associated with the depth stencil image for this GraphicsContext.
	*******************************************************************************************************************/
	const api::ImageDataFormat getDepthStencilImageFormat()const
	{
		api::ImageDataFormat depthStencilFormat;
		depthStencilFormat.format = PixelFormat('d', (getDisplayAttributes().stencilBPP ? 's' : 0), (uint8)0, (uint8)0,
		                                        (uint8)getDisplayAttributes().depthBPP, (uint8)getDisplayAttributes().stencilBPP, (uint8)0, (uint8)0);
		depthStencilFormat.colorSpace = types::ColorSpace::lRGB;
		depthStencilFormat.dataType = (uint8)getDisplayAttributes().depthBPP == 16 ?
		                              VariableType::UnsignedShortNorm : (uint8)getDisplayAttributes().depthBPP == 24 ?
		                              VariableType::UnsignedInteger : VariableType::Float;
		return depthStencilFormat;
	}

	std::string getInfo()const;

	api::ParentableGraphicsPipeline  createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam);

	api::ComputePipeline createComputePipeline(const api::ComputePipelineCreateParam& createParam);

	api::GraphicsPipeline createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc);

	api::GraphicsPipeline createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc,
	    api::ParentableGraphicsPipeline parent);


	api::Sampler createSampler(const api::SamplerCreateParam& createParam);

    api::EffectApi createEffectApi(assets::Effect& effectDesc, api::GraphicsPipelineCreateParam& pipeDesc,
	                               api::AssetLoadingDelegate& effectDelegate);

	api::TextureStore createTexture();

	api::TextureView createTextureView(const api::TextureStore& texture, types::ImageSubresourceRange range, types::SwizzleChannels);

	api::TextureView createTextureView(const api::TextureStore& texture, types::SwizzleChannels);

	api::BufferView createBufferView(const api::Buffer& buffer, uint32 offset, uint32 range);

	api::BufferView createBufferAndView(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);

	api::Buffer createBuffer(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);

	api::CommandBuffer createCommandBufferOnDefaultPool();

	api::SecondaryCommandBuffer createSecondaryCommandBufferOnDefaultPool();

	api::Shader createShader(const Stream& shaderSrc, types::ShaderType shaderType,
	                         const char* const* defines, uint32 numDefines);

	api::Shader createShader(Stream& shaderData, types::ShaderType shaderType,
	                         types::ShaderBinaryFormat binaryFormat);

	api::Fbo createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass,
	    const api::OnScreenFboCreateParam& onScreenFboCreateParam);

	api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass);


	api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass,
        Multi<api::OnScreenFboCreateParam>& onScreenFboCreateParams);

	api::Fbo createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass);


    api::FboSet createOnScreenFboSet(types::LoadOp colorLoadOp, types::StoreOp colorStoreOp,
                                     types::LoadOp depthLoadOp, types::StoreOp depthStoreOp,
                                     types::LoadOp stencilLoadOp, types::StoreOp stencilStoreOp);


    api::Fbo createOnScreenFbo(uint32 swapIndex, types::LoadOp colorLoadOp,
                               types::StoreOp colorStoreOp, types::LoadOp depthLoadOp,
                               types::StoreOp depthStoreOp, types::LoadOp stencilLoadOp,
                               types::StoreOp stencilStoreOp);

	api::RenderPass createRenderPass(const api::RenderPassCreateParam& renderPassDesc);

	api::DescriptorPool createDescriptorPool(const api::DescriptorPoolCreateParam& createParam);

	api::DescriptorSet createDescriptorSetOnDefaultPool(const api::DescriptorSetLayout& layout);

	api::DescriptorSetLayout createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& createParam);

	api::PipelineLayout createPipelineLayout(const api::PipelineLayoutCreateParam& createParam);

	api::CommandPool createCommandPool();

	api::Fence createFence(bool createSignaled) { return api::Fence(); }

	api::Semaphore createSemaphore() { return api::Semaphore(); }

	uint32 getSwapChainLength()const { return 1;}

	uint32 getSwapChainIndex()const { return 0; }

	/*!*********************************************************************************************************************
	\brief	return true if last bound pipeline was graphics
	\return	bool
	***********************************************************************************************************************/
	bool isLastBoundPipelineGraphics()const
	{
        return m_renderStatesTracker.lastBoundPipe == RenderStatesTracker::LastBoundPipeline::PipelineGraphics;
	}

	/*!*********************************************************************************************************************
	\brief	return true if last bound pipeline was compute
	\return	bool
	***********************************************************************************************************************/
	bool isLastBoundPipelineCompute()const
	{
        return m_renderStatesTracker.lastBoundPipe == RenderStatesTracker::LastBoundPipeline::PipelineCompute;
	}

    void onBind(api::impl::GraphicsPipeline_* pipeline)
	{
        m_renderStatesTracker.lastBoundPipe = RenderStatesTracker::LastBoundPipeline::PipelineGraphics;
		setBoundGraphicsPipeline(pipeline);
	}

    void onBind(api::impl::ComputePipeline_* pipeline)
	{
        m_renderStatesTracker.lastBoundPipe = RenderStatesTracker::LastBoundPipeline::PipelineCompute;
		setBoundComputePipeline(pipeline);
	}

	/*!*********************************************************************************************************************
	\brief Internal use. State tracking. Outside code calls this to notify the context that a new texture has been bound to a texture unit.
	***********************************************************************************************************************/
	void onBind(const api::impl::TextureView_& texture, uint16 bindIndex)
	{
		if (m_renderStatesTracker.texSamplerBindings.size() <= bindIndex)
		{
			assertion(false , "UnSupported Texture Unit binding");
			Log("UnSupported Texture Unit binding %d", bindIndex);
		}
		m_renderStatesTracker.lastBoundTexBindIndex = bindIndex;
		m_renderStatesTracker.texSamplerBindings[bindIndex].toBindTex = &texture;
		//m_renderStatesTracker.texUnits.push_back(std::pair<string, uint32>(shaderVaribleName, bindIndex));
	}

	void onBind(const api::impl::Sampler_& sampler, uint16 bindIndex)
	{
		if (m_renderStatesTracker.texSamplerBindings.size() <= bindIndex)
		{
			assertion(false , "UnSupported Sampler Unit binding");
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

	/*!****************************************************************************************************************
	\brief	A map of VBO bindings.
	*******************************************************************************************************************/
	typedef std::map<uint16, api::Buffer> VboBindingMap;//< bind index, buffer

	struct TextureBinding
	{
		const api::impl::TextureView_* toBindTex;
		const api::impl::TextureView_* lastBoundTex;
		const api::impl::Sampler_* lastBoundSampler;


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
        friend class pvr::platform::ContextGles;
		// Stencil
		struct DepthStencilState
		{
			bool depthTest;
			bool depthWrite;
			uint32 stencilWriteMask;
			bool enableStencilTest;
			int32 clearStencilValue;
			types::ComparisonMode depthOp;
			// Front
			types::StencilOp stencilFailOpFront;
			types::StencilOp depthStencilPassOpFront;
			types::StencilOp depthFailOpFront;
			types::ComparisonMode stencilOpFront;

			// Back
			types::StencilOp stencilFailOpBack;
			types::StencilOp depthStencilPassOpBack;
			types::StencilOp depthFailOpBack;
			types::ComparisonMode stencilOpBack;

			int32 refFront, refBack;

			uint32 readMaskFront, readMaskBack, writeMaskFront, writeMaskBack;

			glm::bvec4 m_colorWriteMask;

            DepthStencilState() : depthTest(types::PipelineDefaults::DepthStencilStates::DepthTestEnabled),
                depthWrite(types::PipelineDefaults::DepthStencilStates::DepthWriteEnabled),
                stencilWriteMask(types::PipelineDefaults::DepthStencilStates::StencilWriteMask),
                enableStencilTest(types::PipelineDefaults::DepthStencilStates::StencilTestEnabled),
                clearStencilValue(types::PipelineDefaults::DepthStencilStates::StencilClearValue),
				depthOp(types::ComparisonMode::DefaultDepthFunc),
				stencilFailOpFront(types::StencilOp::DefaultStencilFailFront),
				depthStencilPassOpFront(types::StencilOp::DefaultDepthStencilPassFront),
				depthFailOpFront(types::StencilOp::DefaultDepthFailFront),
				stencilOpFront(types::ComparisonMode::DefaultStencilOpFront),

				stencilFailOpBack(types::StencilOp::DefaultStencilFailBack),
				depthStencilPassOpBack(types::StencilOp::DefaultDepthStencilPassBack),
				depthFailOpBack(types::StencilOp::DefaultDepthFailBack),
				stencilOpBack(types::ComparisonMode::DefaultStencilOpBack),

                refFront(types::PipelineDefaults::DepthStencilStates::StencilReference),
                refBack(types::PipelineDefaults::DepthStencilStates::StencilReference),
                readMaskFront(types::PipelineDefaults::DepthStencilStates::StencilReadMask),
                readMaskBack(types::PipelineDefaults::DepthStencilStates::StencilReadMask),
                writeMaskFront(types::PipelineDefaults::DepthStencilStates::StencilWriteMask),
                writeMaskBack(types::PipelineDefaults::DepthStencilStates::StencilWriteMask)
			{}
		};
		struct IndexBufferState
		{
			api::Buffer buffer;
			uint32 offset;
			types::IndexType indexArrayFormat;
			IndexBufferState() : offset(0) {}
		};

		IndexBufferState iboState;
		VboBindingMap vboBindings;
		api::Buffer lastBoundVbo;
		TextureBindingList texSamplerBindings;
		uint32 lastBoundTexBindIndex;
		DepthStencilState depthStencil;
		api::Fbo boundFbo;
		types::PrimitiveTopology primitiveTopology;
		glm::bvec4 colorWriteMask;
		types::Face cullFace;
		types::PolygonWindingOrder polyWindingOrder;
		ProgBufferBingingList uboBufferBindings;
		ProgBufferBingingList ssboBufferBindings;
		ProgBufferBingingList atomicBufferBindings;
		types::BlendOp rgbBlendOp;
		types::BlendOp alphaBlendOp;
		types::BlendFactor srcRgbFactor, srcAlphaFactor, destRgbFactor, destAlphaFactor;
		bool enabledScissorTest;
		bool enabledBlend;

		Rectanglei viewport;
		Rectanglei scissor;

		std::vector<std::pair<string, uint32>/**/> texUnits;
	private:
		void releaseAll() {	*this = RenderStatesTracker();	}
		uint32 attributesToEnableBitfield;
		uint32 attributesEnabledBitfield;
		uint16 attributesMaxToEnable;
		uint16 attributesMaxEnabled;

        enum class LastBoundPipeline
        {
            PipelineGraphics,
            PipelineCompute,
            PipelineNone,
            Default = PipelineNone
        };

        LastBoundPipeline lastBoundPipe;
	public:
		RenderStatesTracker() :
            lastBoundTexBindIndex(0),
            colorWriteMask(types::PipelineDefaults::ColorWrite::ColorMaskR, types::PipelineDefaults::ColorWrite::ColorMaskG,
                           types::PipelineDefaults::ColorWrite::ColorMaskB, types::PipelineDefaults::ColorWrite::ColorMaskA),
			cullFace(types::Face::DefaultCullFace),
			polyWindingOrder(types::PolygonWindingOrder::Default),
            rgbBlendOp(types::BlendOp::Default),
            alphaBlendOp(types::BlendOp::Default),
            srcRgbFactor(types::BlendFactor::DefaultSrcRgba),
            srcAlphaFactor(types::BlendFactor::DefaultSrcRgba),
            destRgbFactor(types::BlendFactor::DefaultDestRgba),
            destAlphaFactor(types::BlendFactor::DefaultDestRgba),
            enabledScissorTest(types::PipelineDefaults::ViewportScissor::ScissorTestEnabled),
            enabledBlend(types::PipelineDefaults::ColorBlend::BlendEnabled),
            viewport(types::PipelineDefaults::ViewportScissor::OffsetX,
                     types::PipelineDefaults::ViewportScissor::OffsetY,
                     types::PipelineDefaults::ViewportScissor::Width,
                     types::PipelineDefaults::ViewportScissor::Height),
            scissor(types::PipelineDefaults::ViewportScissor::OffsetX,
                    types::PipelineDefaults::ViewportScissor::OffsetY,
                    types::PipelineDefaults::ViewportScissor::Width,
                    types::PipelineDefaults::ViewportScissor::Height),
            attributesToEnableBitfield(0),
            attributesEnabledBitfield(0),
            attributesMaxToEnable(0),
            attributesMaxEnabled(0),
            lastBoundPipe(LastBoundPipeline::Default) {}

        ~RenderStatesTracker() {}
	};
	RenderStatesTracker& getCurrentRenderStates() { return m_renderStatesTracker; }
	RenderStatesTracker const& getCurrentRenderStates()const { return m_renderStatesTracker; }

	api::Sampler getDefaultSampler()const { return m_defaultSampler; }
	api::CommandPool& getDefaultCommandPool() { return m_defaultCmdPool; }
	const api::CommandPool& getDefaultCommandPool()const { return m_defaultCmdPool; }
	api::DescriptorPool& getDefaultDescriptorPool()
	{
		if (!m_defaultDescPool.isValid())
		{
			api::DescriptorPoolCreateParam poolInfo;
			m_defaultDescPool = createDescriptorPool(poolInfo);
		}
		return m_defaultDescPool;
	}

	const api::DescriptorPool& getDefaultDescriptorPool()const { return m_defaultDescPool; }
	static StrongReferenceType createNew()
	{
		return EmbeddedRefCount<ContextGles>::createNew();
	}
protected:
	RenderStatesTracker m_renderStatesTracker;

	/*!*********************************************************************************************************************
	\brief Implements IGraphicsContext. Get the source of a shader as a stream.
	\param stream A Stream object containing the source of the shader.
	\param outSourceData Reference to an std::string where the source data will be read to.
	\return Result::Success on success
	***********************************************************************************************************************/
	Result createShader(const Stream& stream, string& outSourceData);

	/*!*********************************************************************************************************************
	\brief Internal use. State tracking. Notify fbo unbind.
	***********************************************************************************************************************/
	ContextGles(size_t implementationId);

	api::CommandPool m_defaultCmdPool;
	api::DescriptorPool m_defaultDescPool;
	size_t m_ContextImplementationID;
	IPlatformContext* m_platformContext;
	mutable std::string m_extensions;
	api::Sampler m_defaultSampler;
	std::vector<std::pair<fnBindPipeline, void*>/**/> m_pushedPipelines;

private:
	void destroyObject() { release(); }
};
}
}
//!\endcond