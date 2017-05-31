<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ContextGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Definition of the OpenGL ES implementation of the GraphicsContext (platform::ContextGles)
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Definition of the OpenGL ES implementation of the GraphicsContext (platform::ContextGles)
\file PVRApi/OGLES/ContextGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRCore/Interfaces/IGraphicsContext.h"
#include "PVRCore/Interfaces/IPlatformContext.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/FboGles.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include <map>
#include <set>
#include <stdlib.h>
#include <bitset>
#include "PVRNativeApi/PlatformContext.h"

namespace pvr {
inline void reportDestroyedAfterContext(const char* objectName)
{
	Log(Log.Warning, "Attempted to destroy object of type [%s] after its corresponding context", objectName);
#ifdef DEBUG
#endif
}

namespace api { namespace gles { class TextureStoreGles_; class TextureViewGles_; class SamplerGles_; } }

/// <summary>Contains functions and methods related to the wiring of the PVRApi library to the underlying platform,
/// including extensions and the Context classes.</summary>
namespace platform {

/// <summary>This class is added as the Debug Callback. Redirects the debug output to the Log object.</summary>
inline void GL_APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                      const GLchar* message, const void* userParam)
{
	Log(Log.Debug, "%s", message);
}

/// <summary>IGraphicsContext implementation that supports all OpenGL ES 2 and above versions.</summary>
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
<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief Create a new, empty, uninitialized context.
	***********************************************************************************************************************/
	ContextGles() : IGraphicsContext(Api::OpenGLESMaxVersion) {}

	/*!*********************************************************************************************************************
	\brief Virtual destructor.
	***********************************************************************************************************************/
=======
	/// <summary>Create a new, empty, uninitialized context.</summary>
	ContextGles() : IGraphicsContext(Api::OpenGLESMaxVersion) {}

	api::TextureView uploadTexture(const Texture& texture, bool allowDecompress);

	SharedContext createSharedContext(uint32 contextId);

	/// <summary>Virtual destructor.</summary>
>>>>>>> 1776432f... 4.3
	virtual ~ContextGles();

	void waitIdle();

	void popPipeline();

	void pushPipeline(fnBindPipeline bindPipePtr, void* pipe);

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Initializes this class using an OS manager.
	\description This function must be called before using the Context. Will use the OS manager to make this Context
	ready to use.
	***********************************************************************************************************************/
=======
	/// <summary>Implementation of IGraphicsContext. Initializes this class using an OS manager.</summary>
	/// <remarks>This function must be called before using the Context. Will use the OS manager to make this Context
	/// ready to use.</remarks>
>>>>>>> 1776432f... 4.3
	Result init(OSManager& osManager);

	void setUpCapabilities();

	/// <summary>Implementation of IGraphicsContext. Release the resources held by this context.</summary>
	void release()
	{
		if (_osManager) //Is already initialized?
		{
<<<<<<< HEAD
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
=======
			_osManager = 0;
			memset(&(ApiCapabilitiesPrivate&)_apiCapabilities, 0, sizeof(ApiCapabilitiesPrivate));
			_extensions.clear();
			_defaultCmdPool.reset();
			_defaultDescPool.reset();
			_defaultSampler.reset();
			_platformContext = 0;
			_apiType = Api::Unspecified;
			_renderStatesTracker.releaseAll();
>>>>>>> 1776432f... 4.3
			gl::releaseGl();
		}
	}   //Error if no display set

	/// <summary>Implementation of IGraphicsContext. Τake a screenshot in the specified buffer of the specified screen area.
	/// </summary>
	bool screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* buffer,
	                         ImageFormat requestedImageFormat);

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Query if a specific extension is supported.
	\param extension A c-style string representing the extension
	\return True if the extension is supported
	***********************************************************************************************************************/
=======
	/// <summary>Implementation of IGraphicsContext. Query if a specific extension is supported.</summary>
	/// <param name="extension">A c-style string representing the extension</param>
	/// <returns>True if the extension is supported</returns>
>>>>>>> 1776432f... 4.3
	bool isExtensionSupported(const char8* extension) const;

	api::Fbo createFbo(const api::FboCreateParam& desc);

	api::FboSet createFboSet(const Multi<api::FboCreateParam>& fboInfo);

<<<<<<< HEAD

	/*!****************************************************************************************************************
	\brief	Get the ImageDataFormat associated with the presentation image for this GraphicsContext.
	*******************************************************************************************************************/
	const api::ImageDataFormat getPresentationImageFormat()const
	{
		api::ImageDataFormat presentationFormat;
=======
	/// <summary>Get the ImageDataFormat associated with the presentation image for this GraphicsContext.</summary>
	const ImageDataFormat getPresentationImageFormat()const
	{
		ImageDataFormat presentationFormat;
>>>>>>> 1776432f... 4.3
		presentationFormat.format = PixelFormat(getDisplayAttributes().redBits ? 'r' : 0, getDisplayAttributes().greenBits ? 'g' : 0,
		                                        getDisplayAttributes().blueBits ? 'b' : 0, getDisplayAttributes().alphaBits ? 'a' : 0,
		                                        (uint8)getDisplayAttributes().redBits, (uint8)getDisplayAttributes().greenBits,
		                                        (uint8)getDisplayAttributes().blueBits, (uint8)getDisplayAttributes().alphaBits);
		presentationFormat.colorSpace = getDisplayAttributes().frameBufferSrgb ? types::ColorSpace::sRGB : types::ColorSpace::lRGB;;
		presentationFormat.dataType = VariableType::UnsignedByteNorm;
		return presentationFormat;
	}

<<<<<<< HEAD
	/*!****************************************************************************************************************
	\brief	Get the ImageDataFormat associated with the depth stencil image for this GraphicsContext.
	*******************************************************************************************************************/
	const api::ImageDataFormat getDepthStencilImageFormat()const
	{
		api::ImageDataFormat depthStencilFormat;
=======
	/// <summary>Get the ImageDataFormat associated with the depth stencil image for this GraphicsContext.</summary>
	const ImageDataFormat getDepthStencilImageFormat()const
	{
		ImageDataFormat depthStencilFormat;
>>>>>>> 1776432f... 4.3
		depthStencilFormat.format = PixelFormat('d', (getDisplayAttributes().stencilBPP ? 's' : 0), (uint8)0, (uint8)0,
		                                        (uint8)getDisplayAttributes().depthBPP, (uint8)getDisplayAttributes().stencilBPP, (uint8)0, (uint8)0);
		depthStencilFormat.colorSpace = types::ColorSpace::lRGB;
		depthStencilFormat.dataType = (uint8)getDisplayAttributes().depthBPP == 16 ?
		                              VariableType::UnsignedShortNorm : (uint8)getDisplayAttributes().depthBPP == 24 ?
		                              VariableType::UnsignedInteger : VariableType::Float;
		return depthStencilFormat;
	}

	std::string getInfo()const;

<<<<<<< HEAD
	api::ParentableGraphicsPipeline  createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam);

	api::ComputePipeline createComputePipeline(const api::ComputePipelineCreateParam& createParam);

	api::GraphicsPipeline createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc);

	api::GraphicsPipeline createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc,
	    api::ParentableGraphicsPipeline parent);


	api::Sampler createSampler(const api::SamplerCreateParam& createParam);

    api::EffectApi createEffectApi(assets::Effect& effectDesc, api::GraphicsPipelineCreateParam& pipeDesc,
	                               api::AssetLoadingDelegate& effectDelegate);

=======
	api::GraphicsPipeline createGraphicsPipeline(const api::GraphicsPipelineCreateParam& desc);

	api::GraphicsPipeline createGraphicsPipeline(const api::GraphicsPipelineCreateParam& desc,
	    api::ParentableGraphicsPipeline parent);

	api::ParentableGraphicsPipeline createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam);

	api::ParentableGraphicsPipeline createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& desc,
	    const api::ParentableGraphicsPipeline& parent);

	api::ComputePipeline createComputePipeline(const api::ComputePipelineCreateParam& createParam);

	api::Sampler createSampler(const api::SamplerCreateParam& createParam);

>>>>>>> 1776432f... 4.3
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
<<<<<<< HEAD
        Multi<api::OnScreenFboCreateParam>& onScreenFboCreateParams);
=======
	    const Multi<api::OnScreenFboCreateParam>& onScreenFboCreateParams);
>>>>>>> 1776432f... 4.3

	api::Fbo createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass);


<<<<<<< HEAD
    api::FboSet createOnScreenFboSet(types::LoadOp colorLoadOp, types::StoreOp colorStoreOp,
                                     types::LoadOp depthLoadOp, types::StoreOp depthStoreOp,
                                     types::LoadOp stencilLoadOp, types::StoreOp stencilStoreOp);


    api::Fbo createOnScreenFbo(uint32 swapIndex, types::LoadOp colorLoadOp,
                               types::StoreOp colorStoreOp, types::LoadOp depthLoadOp,
                               types::StoreOp depthStoreOp, types::LoadOp stencilLoadOp,
                               types::StoreOp stencilStoreOp);
=======
	api::FboSet createOnScreenFboSet(types::LoadOp colorLoadOp, types::StoreOp colorStoreOp,
	                                 types::LoadOp depthLoadOp, types::StoreOp depthStoreOp,
	                                 types::LoadOp stencilLoadOp, types::StoreOp stencilStoreOp);


	api::Fbo createOnScreenFbo(uint32 swapIndex, types::LoadOp colorLoadOp,
	                           types::StoreOp colorStoreOp, types::LoadOp depthLoadOp,
	                           types::StoreOp depthStoreOp, types::LoadOp stencilLoadOp,
	                           types::StoreOp stencilStoreOp);

	api::RenderPass createOnScreenRenderpass(
	  types::LoadOp colorLoadOp = types::LoadOp::Clear,
	  types::StoreOp colorStoreOp = types::StoreOp::Store,
	  types::LoadOp depthLoadOp = types::LoadOp::Clear,
	  types::StoreOp depthStoreOp = types::StoreOp::Ignore,
	  types::LoadOp stencilLoadOp = types::LoadOp::Clear,
	  types::StoreOp stencilStoreOp = types::StoreOp::Ignore);

>>>>>>> 1776432f... 4.3

	api::RenderPass createRenderPass(const api::RenderPassCreateParam& renderPassDesc);

	api::DescriptorPool createDescriptorPool(const api::DescriptorPoolCreateParam& createParam);

	api::DescriptorSet createDescriptorSetOnDefaultPool(const api::DescriptorSetLayout& layout);

	api::DescriptorSetLayout createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& createParam);

	api::PipelineLayout createPipelineLayout(const api::PipelineLayoutCreateParam& createParam);

	api::CommandPool createCommandPool();

<<<<<<< HEAD
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
=======
	api::Fence createFence(bool createSignaled);

	api::Semaphore createSemaphore() { return api::Semaphore(); }

	api::SceneHierarchy createSceneHierarchy(const api::SceneHierarchyCreateParam& createParam);

	api::VertexRayPipeline createVertexRayPipeline(const api::VertexRayPipelineCreateParam& createParam);

	api::SceneTraversalPipeline createSceneTraversalPipeline(const api::SceneTraversalPipelineCreateParam& createParam);

	api::IndirectRayPipeline createIndirectRayPipeline(const api::IndirectRayPipelineCreateParam& createParam);

	/// <summary>return true if last bound pipeline was graphics</summary>
	/// <returns>bool</returns>
	bool isLastBoundPipelineGraphics()const
	{
		return _renderStatesTracker.lastBoundPipe == RenderStatesTracker::LastBoundPipeline::PipelineGraphics;
	}

	/// <summary>return true if last bound pipeline was compute</summary>
	/// <returns>bool</returns>
	bool isLastBoundPipelineCompute()const
	{
		return _renderStatesTracker.lastBoundPipe == RenderStatesTracker::LastBoundPipeline::PipelineCompute;
	}

	void onBind(api::impl::GraphicsPipeline_* pipeline);

	void onBind(api::impl::ComputePipeline_* pipeline);

	void onBind(api::impl::VertexRayPipeline_* pipeline)
	{
		assertion(false, "Binding a vertex ray pipeline is unsupported.");
	}

	void onBind(api::impl::SceneTraversalPipeline_* pipeline)
	{
		assertion(false, "Binding a scene traversal pipeline is unsupported.");
	}

	/// <summary>Internal use. State tracking. Outside code calls this to notify the context that a new texture has been
	/// bound to a texture unit.</summary>
	void onBind(const api::gles::TextureStoreGles_& texture, uint16 bindIndex)
>>>>>>> 1776432f... 4.3
	{
		if (_renderStatesTracker.texSamplerBindings.size() <= bindIndex)
		{
			assertion(false, "UnSupported Texture Unit binding");
			Log("UnSupported Texture Unit binding %d", bindIndex);
		}
<<<<<<< HEAD
		m_renderStatesTracker.lastBoundTexBindIndex = bindIndex;
		m_renderStatesTracker.texSamplerBindings[bindIndex].toBindTex = &texture;
		//m_renderStatesTracker.texUnits.push_back(std::pair<string, uint32>(shaderVaribleName, bindIndex));
=======
		_renderStatesTracker.lastBoundTexBindIndex = bindIndex;
		_renderStatesTracker.texSamplerBindings[bindIndex].lastBoundTex = &texture;
		//_renderStatesTracker.texUnits.push_back(std::pair<string, uint32>(shaderVaribleName, bindIndex));
	}

	/// <summary>Internal use. State tracking. Outside code calls this to notify the context that a new texture has been
	/// bound to a texture unit.</summary>
	void onBindImage(const api::gles::TextureStoreGles_& texture, uint16 imageUnit)
	{
		assertion(_renderStatesTracker.imageBindings.size() > imageUnit, "UnSupported Image Unit binding");
		_renderStatesTracker.imageBindings[imageUnit] = &texture;
>>>>>>> 1776432f... 4.3
	}

	void onBind(const api::gles::SamplerGles_& sampler, uint16 bindIndex)
	{
		if (_renderStatesTracker.texSamplerBindings.size() <= bindIndex)
		{
			assertion(false, "UnSupported Sampler Unit binding");
			Log("UnSupported Sampler Unit binding %d", bindIndex);
		}
		_renderStatesTracker.texSamplerBindings[bindIndex].lastBoundSampler = &sampler;
	}

	void onBindUbo(uint16 bindIndex, const api::Buffer& buffer, uint32 offset, uint32 range)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(_renderStatesTracker.uboBufferBindings.begin(), _renderStatesTracker.uboBufferBindings.end(), BufferBindingComp(bindIndex));
		if (it != _renderStatesTracker.uboBufferBindings.end())
		{
			it->second.offset = offset;
			it->second.range = range;
			it->second.buffer = buffer;
		}
		else
		{
			_renderStatesTracker.uboBufferBindings.push_back(std::make_pair(bindIndex, BufferRange(buffer, offset, range)));
		}
	}

	void onBindSsbo(uint16 bindIndex, const api::Buffer& buffer, uint32 offset, uint32 range)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(_renderStatesTracker.ssboBufferBindings.begin(), _renderStatesTracker.ssboBufferBindings.end(), BufferBindingComp(bindIndex));
		if (it != _renderStatesTracker.ssboBufferBindings.end())
		{
			it->second.offset = offset;
			it->second.range = range;
			it->second.buffer = buffer;
		}
		else
		{
			_renderStatesTracker.ssboBufferBindings.push_back(std::make_pair(bindIndex, BufferRange(buffer, offset, range)));
		}
	}

	void onBindAtomicBuffer(uint16 bindIndex, const api::Buffer& buffer, uint32 offset, uint32 range)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(_renderStatesTracker.atomicBufferBindings.begin(), _renderStatesTracker.atomicBufferBindings.end(), BufferBindingComp(bindIndex));
		if (it != _renderStatesTracker.atomicBufferBindings.end())
		{
			it->second.offset = offset;
			it->second.range = range;
			it->second.buffer = buffer;
		}
		else
		{
			_renderStatesTracker.atomicBufferBindings.push_back(std::make_pair(bindIndex, BufferRange(buffer, offset, range)));
		}
	}

	BufferRange getBoundProgramBufferUbo(uint16 bindIndex)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(_renderStatesTracker.uboBufferBindings.begin(), _renderStatesTracker.uboBufferBindings.end(), BufferBindingComp(bindIndex));
		return (it != _renderStatesTracker.uboBufferBindings.end() ? it->second : BufferRange());
	}

	BufferRange getBoundProgramBufferSsbo(uint16 bindIndex)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(_renderStatesTracker.ssboBufferBindings.begin(), _renderStatesTracker.ssboBufferBindings.end(), BufferBindingComp(bindIndex));
		return (it != _renderStatesTracker.ssboBufferBindings.end() ? it->second : BufferRange());
	}

	BufferRange getBoundProgramBufferAtomicBuffer(uint16 bindIndex)
	{
		std::vector<std::pair<uint16, BufferRange>/**/>::iterator it =
		  std::find_if(_renderStatesTracker.atomicBufferBindings.begin(), _renderStatesTracker.atomicBufferBindings.end(), BufferBindingComp(bindIndex));
		return (it != _renderStatesTracker.atomicBufferBindings.end() ? it->second : BufferRange());
	}

	const api::Fbo& getBoundFbo()const
	{
		return _renderStatesTracker.boundFbo;
	}

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief  Implementation of IGraphicsContext. Return the  platform context that powers this graphics context.
	***********************************************************************************************************************/
	IPlatformContext& getPlatformContext()const { return *m_platformContext; }

	/*!*********************************************************************************************************************
	\brief Internal use. State tracking. enable veretx atribute.
	***********************************************************************************************************************/
=======
	/// <summary>Implementation of IGraphicsContext. Return the platform context that powers this graphics context.</summary>
	IPlatformContext& getPlatformContext()const { return *_platformContext; }

	/// <summary>Internal use. State tracking. enable veretx atribute.</summary>
>>>>>>> 1776432f... 4.3
	void enableAttribute(uint16 attributeIdx)
	{
		_renderStatesTracker.attributesToEnableBitfield |= (1 << attributeIdx);
		if ((_renderStatesTracker.attributesEnabledBitfield & (1 << attributeIdx)) == 0)
		{
			gl::EnableVertexAttribArray(attributeIdx);
			_renderStatesTracker.attributesEnabledBitfield |= (1 << attributeIdx);
		}
		_renderStatesTracker.attributesMaxToEnable = (std::max)(attributeIdx, _renderStatesTracker.attributesMaxToEnable) + 1;
	}

	/// <summary>Internal use. State tracking.disable unneeded vertex attributes.</summary>
	void disableUnneededAttributes()
	{
		uint32 deactivate = _renderStatesTracker.attributesEnabledBitfield ^ _renderStatesTracker.attributesToEnableBitfield;

		for (uint8 i = 0; i < _renderStatesTracker.attributesMaxEnabled; ++i)
		{
			if (Bitfield<uint32>::isSet(deactivate, i))
			{
				gl::DisableVertexAttribArray(i);
			}
		}
		_renderStatesTracker.attributesEnabledBitfield = _renderStatesTracker.attributesToEnableBitfield;
		_renderStatesTracker.attributesMaxEnabled = _renderStatesTracker.attributesMaxToEnable;
		_renderStatesTracker.attributesMaxToEnable = 0;
		_renderStatesTracker.attributesToEnableBitfield = 0;
	}

	/// <summary>A map of VBO bindings.</summary>
	typedef std::map<uint16, api::Buffer> VboBindingMap;//< bind index, buffer

	struct TextureBinding
	{
		const api::gles::TextureStoreGles_* lastBoundTex;
		const api::gles::SamplerGles_* lastBoundSampler;
		TextureBinding() : /*toBindTex(0),*/ lastBoundTex(0), lastBoundSampler(0) {}
	};

	typedef std::vector<TextureBinding> TextureBindingList;
	typedef std::vector < const api::gles::TextureStoreGles_* > ImageBindingList;

	typedef std::vector<std::pair<uint16, BufferRange>/**/> ProgBufferBingingList;

	/// <summary>internal state tracker for OpenGLES</summary>
	struct RenderStatesTracker
	{
<<<<<<< HEAD
        friend class pvr::platform::ContextGles;
=======
		friend class platform::ContextGles;
>>>>>>> 1776432f... 4.3
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
<<<<<<< HEAD
=======

			bool depthBias;
			float32 depthBiasMax;
			float32 depthBiasConstantFactor;
			float32 depthBiasSlopeFactor;
			float32 depthBiasClamp;
>>>>>>> 1776432f... 4.3

			int32 refFront, refBack;

			uint32 readMaskFront, readMaskBack, writeMaskFront, writeMaskBack;

<<<<<<< HEAD
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
=======
			glm::bvec4 _colorWriteMask;

			DepthStencilState() : depthTest(false),
				depthWrite(true),
				stencilWriteMask(0xff),
				enableStencilTest(false),
				clearStencilValue(0),
				depthOp(types::ComparisonMode::DefaultDepthFunc),
				stencilFailOpFront(types::StencilOp::Default),
				depthStencilPassOpFront(types::StencilOp::Default),
				depthFailOpFront(types::StencilOp::Default),
				stencilOpFront(types::ComparisonMode::DefaultStencilFunc),

				stencilFailOpBack(types::StencilOp::Default),
				depthStencilPassOpBack(types::StencilOp::Default),
				depthFailOpBack(types::StencilOp::Default),
				stencilOpBack(types::ComparisonMode::DefaultStencilFunc),

				depthBias(false),
				depthBiasMax(0.f),
				depthBiasConstantFactor(0.f),
				depthBiasSlopeFactor(0.f),
				depthBiasClamp(0.f),

				refFront(0),
				refBack(0),
				readMaskFront(0xff),
				readMaskBack(0xff),
				writeMaskFront(0xff),
				writeMaskBack(0xff)
>>>>>>> 1776432f... 4.3
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
		ImageBindingList imageBindings;
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
<<<<<<< HEAD
=======
		native::HPipeline_ lastBoundProgram;
>>>>>>> 1776432f... 4.3
		bool enabledScissorTest;
		bool enabledBlend;

		Rectanglei viewport;
		Rectanglei scissor;

		std::vector<std::pair<string, uint32>/**/> texUnits;
	private:
		void releaseAll() { *this = RenderStatesTracker();  }
		uint32 attributesToEnableBitfield;
		uint32 attributesEnabledBitfield;
		uint16 attributesMaxToEnable;
		uint16 attributesMaxEnabled;

<<<<<<< HEAD
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
=======
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
			cullFace(types::Face::Default),
			polyWindingOrder(types::PolygonWindingOrder::Default),
			rgbBlendOp(types::BlendOp::Default),
			alphaBlendOp(types::BlendOp::Default),
			srcRgbFactor(types::BlendFactor::DefaultSrcRgba),
			srcAlphaFactor(types::BlendFactor::DefaultSrcRgba),
			destRgbFactor(types::BlendFactor::DefaultDestRgba),
			destAlphaFactor(types::BlendFactor::DefaultDestRgba),
			enabledScissorTest(types::PipelineDefaults::ViewportScissor::ScissorTestEnabled),
			enabledBlend(types::PipelineDefaults::ColorBlend::BlendEnabled),
			viewport(0, 0, 0, 0),
			scissor(0, 0, 0, 0),
			attributesToEnableBitfield(0),
			attributesEnabledBitfield(0),
			attributesMaxToEnable(0),
			attributesMaxEnabled(0),
			lastBoundPipe(LastBoundPipeline::Default),
			lastBoundProgram(0) {}

		~RenderStatesTracker() {}
>>>>>>> 1776432f... 4.3
	};
	RenderStatesTracker& getCurrentRenderStates() { return _renderStatesTracker; }
	RenderStatesTracker const& getCurrentRenderStates()const { return _renderStatesTracker; }

	api::Sampler getDefaultSampler()const { return _defaultSampler; }
	api::CommandPool& getDefaultCommandPool() { return _defaultCmdPool; }
	const api::CommandPool& getDefaultCommandPool()const { return _defaultCmdPool; }
	api::DescriptorPool& getDefaultDescriptorPool()
	{
		if (!_defaultDescPool.isValid())
		{
			api::DescriptorPoolCreateParam poolInfo;
			_defaultDescPool = createDescriptorPool(poolInfo);
		}
		return _defaultDescPool;
	}

	const api::DescriptorPool& getDefaultDescriptorPool()const { return _defaultDescPool; }
	static StrongReferenceType createNew()
	{
		return EmbeddedRefCount<ContextGles>::createNew();
	}
protected:
<<<<<<< HEAD
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
=======
	RenderStatesTracker _renderStatesTracker;

	api::CommandPool _defaultCmdPool;
	api::DescriptorPool _defaultDescPool;
	mutable std::string _extensions;
	api::Sampler _defaultSampler;
	std::vector<std::pair<fnBindPipeline, void*>/**/> _pushedPipelines;

private:
	void destroyObject() { release(); }
};

class SharedContextGles : public ISharedContext, public EmbeddedRefCount<SharedContextGles>
{
	friend class ContextGles;
	template<typename> friend class ::pvr::EmbeddedRefCount;
	static EmbeddedRefCountedResource<SharedContextGles> createNew(const GraphicsContext& ctx)
	{
		return EmbeddedRefCount<SharedContextGles>::createNew(ctx);
	}
	SharedContextGles(const GraphicsContext& ctx) : ISharedContext(ctx, ctx->getPlatformContext().createSharedPlatformContext((uint32) - 1))
	{ }
public:
	api::TextureAndFence uploadTextureDeferred(const Texture& texture, bool allowDecompress = true);
	//api::TextureAndFence createGraphicsPipelineDeferred();
	void destroyObject()
	{
		_context.reset();
		_platformContext.reset();
	}

	ISharedPlatformContext& getSharedPlatformContext() { return *_platformContext; }

>>>>>>> 1776432f... 4.3
};

}
}
namespace pvr {
namespace api {
inline const platform::ContextGles& native_cast(const IGraphicsContext& object) { return static_cast<const platform::ContextGles&>(object); } \
inline const platform::ContextGles* native_cast(const GraphicsContext& object) { return &native_cast(*object); } \
inline const platform::ContextGles* native_cast(const IGraphicsContext* object) { return &native_cast(*object); } \
inline platform::ContextGles& native_cast(IGraphicsContext& object) { return static_cast<platform::ContextGles&>(object); } \
inline platform::ContextGles* native_cast(GraphicsContext& object) { return &native_cast(*object); } \
inline platform::ContextGles* native_cast(IGraphicsContext* object) { return &native_cast(*object); } \
}
}
<<<<<<< HEAD
//!\endcond
=======

>>>>>>> 1776432f... 4.3
