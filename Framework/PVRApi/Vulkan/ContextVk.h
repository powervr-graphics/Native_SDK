<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/ContextVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Definition of the Vulkan implementation of the GraphicsContext (pvr::platform::ContextVulkan)
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Definition of the Vulkan implementation of the GraphicsContext (pvr::platform::ContextVulkan)
\file PVRApi/Vulkan/ContextVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3

#pragma once
#include "PVRCore/Interfaces/IGraphicsContext.h"
#include "PVRCore/Interfaces/IPlatformContext.h"
#include "PVRApi/Vulkan/FboVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/Vulkan/VkErrors.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRNativeApi/Vulkan/PlatformHandlesVulkanGlue.h"
#include "PVRApi/ApiObjects/CommandBuffer.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/ComputePipeline.h"
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

/// <summary>Contains functions and methods related to the wiring of the PVRApi library to the underlying platform,
/// including extensions and the Context classes.</summary>
namespace platform {

/// <summary>IGraphicsContext implementation that supports Vulkan</summary>
class ContextVk : public IGraphicsContext, public EmbeddedRefCount<ContextVk>
{
public:
<<<<<<< HEAD

	Result init(OSManager& osManager);

	void release();

	void waitIdle();

	bool screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* buffer, ImageFormat imageFormat);

	std::string getInfo()const ;

	const api::ImageDataFormat getDepthStencilImageFormat()const
	{
		return pvr::api::ConvertFromVulkan::imageDataFormat(getPlatformContext().getNativeDisplayHandle().onscreenFbo.depthStencilFormat);
	}

	/*!*********************************************************************************************************************
	\brief  Implementation of IGraphicsContext. Return the  platform context that powers this graphics context.
	***********************************************************************************************************************/
	IPlatformContext& getPlatformContext()const { return *m_platformContext; }

	bool isExtensionSupported(const char8* /*extension*/) const { return false;/*NOT SUPPORTED YET*/ }

	api::ComputePipeline createComputePipeline(const api::ComputePipelineCreateParam& createParam);

	api::GraphicsPipeline createGraphicsPipeline(api::GraphicsPipelineCreateParam& createParam);

	api::GraphicsPipeline createGraphicsPipeline(api::GraphicsPipelineCreateParam& createParam,
	    api::ParentableGraphicsPipeline parent);

	api::ParentableGraphicsPipeline  createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam);

	api::Sampler createSampler(const api::SamplerCreateParam& createParam);

	api::EffectApi createEffectApi(
	  ::pvr::assets::Effect& /*effectDesc*/, api::GraphicsPipelineCreateParam& /*pipeDesc*/,
	  api::AssetLoadingDelegate& /*effectDelegate*/)
	{
		return api::EffectApi();
	}

	api::TextureStore createTexture();

	api::TextureView createTextureView(const api::TextureStore& texture, types::ImageSubresourceRange range,
	                                   types::SwizzleChannels);

	api::TextureView createTextureView(const api::TextureStore& texture, types::SwizzleChannels);

	api::BufferView createBufferView(const api::Buffer& buffer, uint32 offset, uint32 range);

	api::BufferView createBufferAndView(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);

	api::Buffer createBuffer(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);

	api::CommandBuffer createCommandBufferOnDefaultPool();

	api::SecondaryCommandBuffer createSecondaryCommandBufferOnDefaultPool();

	api::Shader createShader(const Stream& shaderSrc, types::ShaderType shaderType, const char* const* defines, uint32 numDefines);

	api::Shader createShader(Stream& shaderData, types::ShaderType shaderType, types::ShaderBinaryFormat binaryFormat);

	api::Fbo createFbo(const api::FboCreateParam& createParam);

	api::FboSet createFboSet(const Multi<api::FboCreateParam>& createParams);

	api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass);

	api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass,
	    pvr::Multi<api::OnScreenFboCreateParam>& onScreenFboCreateParams);

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

	api::Fence createFence(bool createSignaled);

	api::Semaphore createSemaphore();

	ContextVk() : IGraphicsContext(Api::Vulkan) {}

	/*!*********************************************************************************************************************
	\brief Virtual destructor.
	***********************************************************************************************************************/
	~ContextVk() {	release();	}

	void setUpCapabilities();
=======

	Result init(OSManager& osManager);

	void release();

	void waitIdle();
>>>>>>> 1776432f... 4.3

	bool screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* buffer, ImageFormat imageFormat);

	std::string getInfo()const ;

	const ImageDataFormat getDepthStencilImageFormat()const
	{
		return nativeVk::ConvertFromVulkan::imageDataFormat(getPlatformContext().getNativeDisplayHandle().onscreenFbo.depthStencilFormat);
	}

	IPlatformContext& getPlatformContext()const { return *_platformContext; }

	api::TextureView uploadTexture(const Texture& texture, bool allowDecompress);

	bool isExtensionSupported(const char8* /*extension*/) const { return false;/*NOT SUPPORTED YET*/ }

	api::ComputePipeline createComputePipeline(const api::ComputePipelineCreateParam& createParam);

	api::GraphicsPipeline createGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam);

	api::GraphicsPipeline createGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam,
	    api::ParentableGraphicsPipeline parent);

	api::ParentableGraphicsPipeline createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& desc);

	api::ParentableGraphicsPipeline  createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& createParam,
	    const api::ParentableGraphicsPipeline& parent);

	api::Sampler createSampler(const api::SamplerCreateParam& createParam);

	api::TextureStore createTexture();

	api::TextureView createTextureView(const api::TextureStore& texture, types::ImageSubresourceRange range,
	                                   types::SwizzleChannels);

	api::TextureView createTextureView(const api::TextureStore& texture, types::SwizzleChannels);

	api::BufferView createBufferView(const api::Buffer& buffer, uint32 offset, uint32 range);

	api::BufferView createBufferAndView(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);

	api::Buffer createBuffer(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);

	api::CommandBuffer createCommandBufferOnDefaultPool();

	api::SecondaryCommandBuffer createSecondaryCommandBufferOnDefaultPool();

	api::Shader createShader(const Stream& shaderSrc, types::ShaderType shaderType, const char* const* defines, uint32 numDefines);

	api::Shader createShader(Stream& shaderData, types::ShaderType shaderType, types::ShaderBinaryFormat binaryFormat);

	api::Fbo createFbo(const api::FboCreateParam& createParam);

	api::FboSet createFboSet(const Multi<api::FboCreateParam>& createParams);

	api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass);

	api::FboSet createOnScreenFboSetWithRenderPass(const api::RenderPass& renderPass,
	    const pvr::Multi<api::OnScreenFboCreateParam>& onScreenFboCreateParams);

	api::Fbo createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass);

	api::FboSet createOnScreenFboSet(types::LoadOp colorLoadOp, types::StoreOp colorStoreOp,
	                                 types::LoadOp depthLoadOp, types::StoreOp depthStoreOp,
	                                 types::LoadOp stencilLoadOp, types::StoreOp stencilStoreOp);

	api::Fbo createOnScreenFbo(uint32 swapIndex, types::LoadOp colorLoadOp,
	                           types::StoreOp colorStoreOp, types::LoadOp depthLoadOp,
	                           types::StoreOp depthStoreOp, types::LoadOp stencilLoadOp,
	                           types::StoreOp stencilStoreOp);

	api::RenderPass createOnScreenRenderpass(types::LoadOp colorLoadOp, types::StoreOp colorStoreOp,
	    types::LoadOp depthLoadOp, types::StoreOp depthStoreOp, types::LoadOp stencilLoadOp,
	    types::StoreOp stencilStoreOp);

	api::RenderPass createRenderPass(const api::RenderPassCreateParam& renderPassDesc);

	api::DescriptorPool createDescriptorPool(const api::DescriptorPoolCreateParam& createParam);

	api::DescriptorSet createDescriptorSetOnDefaultPool(const api::DescriptorSetLayout& layout);

	api::DescriptorSetLayout createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& createParam);

	api::PipelineLayout createPipelineLayout(const api::PipelineLayoutCreateParam& createParam);

	api::CommandPool createCommandPool();

	api::Fence createFence(bool createSignaled);

	api::Semaphore createSemaphore();

	api::SceneHierarchy createSceneHierarchy(const api::SceneHierarchyCreateParam& createParam);

	api::VertexRayPipeline createVertexRayPipeline(const api::VertexRayPipelineCreateParam& createParam);

	api::VertexRayPipeline createVertexRayPipeline(const api::VertexRayPipelineCreateParam& createParam, api::ParentableVertexRayPipeline parent);

	api::SceneTraversalPipeline createSceneTraversalPipeline(const api::SceneTraversalPipelineCreateParam& createParam);

	api::SceneTraversalPipeline createSceneTraversalPipeline(const api::SceneTraversalPipelineCreateParam& createParam, api::ParentableSceneTraversalPipeline parent);

	api::IndirectRayPipeline createIndirectRayPipeline(const api::IndirectRayPipelineCreateParam& createParam);

	api::IndirectRayPipeline createIndirectRayPipeline(const api::IndirectRayPipelineCreateParam& createParam, api::ParentableIndirectRayPipeline parent);

	ContextVk() : IGraphicsContext(Api::Vulkan) {}

	/// <summary>Virtual destructor.</summary>
	~ContextVk() {  release();  }

	void setUpCapabilities();

	/// <summary>return true if last bound pipeline was graphics</summary>
	/// <returns>bool</returns>
	bool isLastBoundPipelineGraphics() const { return false; }

	/// <summary>return true if last bound pipeline was compute</summary>
	/// <returns>bool</returns>
	bool isLastBoundPipelineCompute() const { return false; }

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief Get the default sampler object
	\return Return the default sampler
	***********************************************************************************************************************/
	api::Sampler getDefaultSampler()const { return m_defaultSampler; }

	/*!*********************************************************************************************************************
	\brief Get the native handle to the context
	\return Return the context native handle
	***********************************************************************************************************************/
	native::HContext_& getContextHandle() { return m_platformContext->getNativePlatformHandles().context; }

	/*!*********************************************************************************************************************
	\brief Get the const native handle to the context
	\return Return the context const native handle
	***********************************************************************************************************************/
	const native::HContext_& getContextHandle() const { return m_platformContext->getNativePlatformHandles().context; }

	/*!*********************************************************************************************************************
	\brief Get the const vulkan device
	\return Return the const vulkan device
	***********************************************************************************************************************/
	VkDevice getDevice() const { return  m_platformContext->getNativePlatformHandles().context.device; }


	/*!*********************************************************************************************************************
	\brief Get the const vulkan physical device
	\return Return the const vulkan physical device
	***********************************************************************************************************************/
	VkPhysicalDevice getPhysicalDevice() const { return m_platformContext->getNativePlatformHandles().context.physicalDevice; }

	/*!*********************************************************************************************************************
	\brief Get the const reference to the device queue
	\return Return the const reference to the device queue
	***********************************************************************************************************************/
	VkQueue getQueue()const { return  m_platformContext->getNativePlatformHandles().graphicsQueue;}

	/*!*********************************************************************************************************************
	\brief Get the const Vulkan instance
	\return Return the const Vulkan instance
	***********************************************************************************************************************/
	VkInstance getVkInstance()const { return m_platformContext->getNativePlatformHandles().context.instance; }

	/*!*********************************************************************************************************************
	\brief Get the default DescriptorPool
	\return Return the default DescriptorPool
	***********************************************************************************************************************/
	const api::DescriptorPool& getDefaultDescriptorPool() const { return m_descriptorPool; }

	/*!*********************************************************************************************************************
	\brief Get the const default DescriptorPool
	\return Return the const default DescriptorPool
	***********************************************************************************************************************/
	api::DescriptorPool& getDefaultDescriptorPool() { return m_descriptorPool; }

	/*!*********************************************************************************************************************
	\brief Get the const default CommandPool
	\return Return the const default CommandPool
	***********************************************************************************************************************/
	const api::CommandPool& getDefaultCommandPool() const { return m_commandPool; }

	/*!*********************************************************************************************************************
	\brief Get the default CommandPool
	\return Return the default CommandPool
	***********************************************************************************************************************/
	api::CommandPool& getDefaultCommandPool() { return m_commandPool; }

	/*!*********************************************************************************************************************
	\brief Get the queue family id
	\return Return the queue famil id
	***********************************************************************************************************************/
	pvr::uint32 getQueueFamilyId()const { return getPlatformContext().getNativePlatformHandles().graphicsQueueIndex; }

	/*!*********************************************************************************************************************
	\brief Implements IGraphicsContext. Get the source of a shader as a stream.
	\param stream A Stream object containing the source of the shader.
	\param outSourceData Reference to an std::string where the source data will be read to.
	\return Result::Success on success
	***********************************************************************************************************************/
	Result createShader(const Stream& stream, string& outSourceData);

	/*!****************************************************************************************************************
	\brief	Get the ImageDataFormat associated with the presentation image for this GraphicsContext.
	*******************************************************************************************************************/
	const api::ImageDataFormat getPresentationImageFormat()const
	{
		return pvr::api::ConvertFromVulkan::imageDataFormat(getPlatformContext().getNativeDisplayHandle().onscreenFbo.colorFormat);
	}


	/*!*********************************************************************************************************************
	\brief Internal use. State tracking. Notify fbo unbind.
	***********************************************************************************************************************/
	ContextVk(size_t implementationId) : IGraphicsContext(Api::Vulkan), m_ContextImplementationID(implementationId) { }


	/*!*********************************************************************************************************************
	\brief Get the texture upload commandbuffer
	\return Return the Texture upload commandbuffer
	***********************************************************************************************************************/
	native::HCommandBuffer_& getTextureUploadCommandBuffer() {	return pvr::api::native_cast(*m_cmdTextureUpload);	}
=======
	/// <summary>Get the default sampler object</summary>
	/// <returns>Return the default sampler</returns>
	api::Sampler getDefaultSampler()const { return _defaultSampler; }

	/// <summary>Get the native handle to the context</summary>
	/// <returns>Return the context native handle</returns>
	native::HContext_& getContextHandle() { return _platformContext->getNativePlatformHandles().context; }

	/// <summary>Get the const native handle to the context</summary>
	/// <returns>Return the context const native handle</returns>
	const native::HContext_& getContextHandle() const { return _platformContext->getNativePlatformHandles().context; }

	/// <summary>Get the const vulkan device</summary>
	/// <returns>Return the const vulkan device</returns>
	VkDevice getDevice() const { return  _platformContext->getNativePlatformHandles().context.device; }


	/// <summary>Get the const vulkan physical device</summary>
	/// <returns>Return the const vulkan physical device</returns>
	VkPhysicalDevice getPhysicalDevice() const { return _platformContext->getNativePlatformHandles().context.physicalDevice; }


	/// <summary>Get the const reference to the device queue</summary>
	/// <returns>Return the const reference to the device queue</returns>
	VkQueue getQueue()const
	{
		return _platformContext->getNativePlatformHandles().mainQueue();
	}

	/// <summary>Get the const Vulkan instance</summary>
	/// <returns>Return the const Vulkan instance</returns>
	VkInstance getVkInstance()const { return _platformContext->getNativePlatformHandles().context.instance; }

	/// <summary>Get the default DescriptorPool</summary>
	/// <returns>Return the default DescriptorPool</returns>
	const api::DescriptorPool& getDefaultDescriptorPool() const { return _descriptorPool; }

	/// <summary>Get the const default DescriptorPool</summary>
	/// <returns>Return the const default DescriptorPool</returns>
	api::DescriptorPool& getDefaultDescriptorPool() { return _descriptorPool; }

	/// <summary>Get the const default CommandPool</summary>
	/// <returns>Return the const default CommandPool</returns>
	const api::CommandPool& getDefaultCommandPool() const { return _commandPool; }

	/// <summary>Get the default CommandPool</summary>
	/// <returns>Return the default CommandPool</returns>
	api::CommandPool& getDefaultCommandPool() { return _commandPool; }

	/// <summary>Get the queue family id</summary>
	/// <returns>Return the queue famil id</returns>
	pvr::uint32 getQueueFamilyId()const
	{
		return getPlatformContext().getNativePlatformHandles().universalQueueFamily;
	}

	/// <summary>Get the ImageDataFormat associated with the presentation image for this GraphicsContext.</summary>
	const ImageDataFormat getPresentationImageFormat()const
	{
		return nativeVk::ConvertFromVulkan::imageDataFormat(getPlatformContext().getNativeDisplayHandle().onscreenFbo.colorFormat);
	}


	/// <summary>Internal use. State tracking. Notify fbo unbind.</summary>
	ContextVk(size_t implementationId) : IGraphicsContext(Api::Vulkan), _contextImplementationID(implementationId) { }


	/// <summary>Get the texture upload commandbuffer</summary>
	/// <returns>Return the Texture upload commandbuffer</returns>
	native::HCommandBuffer_& getTextureUploadCommandBuffer() {  return pvr::api::native_cast(*_cmdTextureUpload); }
>>>>>>> 1776432f... 4.3

	api::Fbo createOnScreenFboWithRenderPass(uint32 swapIndex, const api::RenderPass& renderPass,
	    const api::OnScreenFboCreateParam& onScreenFboCreateParam);

<<<<<<< HEAD
=======
	SharedContext createSharedContext(uint32 contextId);

>>>>>>> 1776432f... 4.3
	static StrongReferenceType createNew() { return EmbeddedRefCount<ContextVk>::createNew(); }
	IPlatformContext* m_platformContext;
protected:
	void destroyObject() { release(); }
<<<<<<< HEAD
	size_t m_ContextImplementationID;
	mutable std::string m_extensions;
	api::Sampler m_defaultSampler;
	api::DescriptorPool m_descriptorPool;// default descriptor pool
	api::CommandPool m_commandPool;// default command pool
	api::CommandBuffer m_cmdTextureUpload;
	VkPhysicalDeviceMemoryProperties m_memoryProperties;
	pvr::uint32 queueFamilyIndex;
=======
	size_t _contextImplementationID;
	mutable std::string _extensions;
	api::Sampler _defaultSampler;
	api::DescriptorPool _descriptorPool;// default descriptor pool
	api::CommandPool _commandPool;// default command pool
	api::CommandBuffer _cmdTextureUpload;
	VkPhysicalDeviceMemoryProperties _memoryProperties;
	pvr::uint32 queueFamilyIndex;
};


class SharedContextVk : public ISharedContext, public EmbeddedRefCount<SharedContextVk>
{
	friend class ContextVk;
	template<typename> friend class ::pvr::EmbeddedRefCount;
	static EmbeddedRefCountedResource<SharedContextVk> createNew(const GraphicsContext& ctx, uint32 contextId)
	{
		return EmbeddedRefCount<SharedContextVk>::createNew(ctx, contextId);
	}
	SharedContextVk(const GraphicsContext& ctx, uint32 contextId) :
		ISharedContext(ctx, ctx->getPlatformContext().createSharedPlatformContext(contextId))
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
namespace api {
namespace vulkan {
typedef RefCountedResource<platform::ContextVk> ContextVkRef;
typedef RefCountedWeakReference<platform::ContextVk> ContextVkWeakRef;
}// namespace vulkan
}// namespace api
}


namespace pvr {
namespace api {
native::HCommandBuffer_& native_cast(pvr::api::impl::CommandBufferBase_& object);
const native::HCommandBuffer_& native_cast(const pvr::api::impl::CommandBufferBase_& object);
const native::HPipeline_& native_cast(const GraphicsPipeline& object);
native::HPipeline_& native_cast(GraphicsPipeline& object);
const native::HPipeline_& native_cast(const ParentableGraphicsPipeline& object);
native::HPipeline_& native_cast(ParentableGraphicsPipeline& object);
const native::HPipeline_& native_cast(const ComputePipeline& object);
native::HPipeline_& native_cast(ComputePipeline& object);

/// <summary>Cast from const reference to IGraphicsContext to const platform::ContextVk&</summary>
/// <returns>Return const platform::ContextVk&</returns>
inline const platform::ContextVk& native_cast(const IGraphicsContext& object) { return static_cast<const platform::ContextVk&>(object); }

/// <summary>Cast from reference to IGraphicsContext to platform::ContextVk&</summary>
/// <returns>Return platform::ContextVk&</returns>
inline platform::ContextVk& native_cast(IGraphicsContext& object) { return static_cast<platform::ContextVk&>(object); }

/// <summary>Cast from const reference to GraphicsContext to const platform::ContextVk*</summary>
/// <returns>Return const platform::ContextVk*</returns>
inline const platform::ContextVk* native_cast(const GraphicsContext& object) { return &native_cast(*object); }

/// <summary>Cast from const IGraphicsContext* to const platform::ContextVk*</summary>
/// <returns>Return const platform::ContextVk*</returns>
inline const platform::ContextVk* native_cast(const IGraphicsContext* object) { return &native_cast(*object); }

/// <summary> Cast from reference to GraphicsContext to platform::ContextVk* </summary>
/// <returns> Return platform ::ContextVk* </returns>
inline platform::ContextVk* native_cast(GraphicsContext& object) { return &native_cast(*object); }

/// <summary>Cast from IGraphicsContext* to platform::ContextVk*</summary>
/// <returns>Return platform::ContextVk*</returns>
inline platform::ContextVk* native_cast(IGraphicsContext* object) { return &native_cast(*object); }

<<<<<<< HEAD
/*!*********************************************************************************************************************
\brief Cast from const impl::GraphicsPipeline_& to const native::HPipeline_&
\return Return const native::HPipeline_&
***********************************************************************************************************************/
inline native::HPipeline_& native_cast(impl::GraphicsPipeline_& object) { return object.getNativeObject(); }

/*!*********************************************************************************************************************
\brief Cast from const impl::GraphicsPipeline_& to const native::HPipeline_&
\return Return const native::HPipeline_&
***********************************************************************************************************************/
const native::HPipeline_& native_cast(const impl::GraphicsPipeline_& object);

/*!*********************************************************************************************************************
\brief Cast from const GraphicsPipeline& to const native::HPipeline_*
\return Return const native::HPipeline_*
***********************************************************************************************************************/
inline const native::HPipeline_* native_cast(const GraphicsPipeline& object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const impl::GraphicsPipeline_* to const native::HPipeline_*
\return Return const native::HPipeline_*
***********************************************************************************************************************/
inline const native::HPipeline_* native_cast(const impl::GraphicsPipeline_* object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const GraphicsPipeline& to const native::HPipeline_*
\return Return const native::HPipeline_*
***********************************************************************************************************************/
inline native::HPipeline_* native_cast(GraphicsPipeline& object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const impl::GraphicsPipeline_& to const native::HPipeline_*
\return Return const native::HPipeline_*
***********************************************************************************************************************/
inline native::HPipeline_* native_cast(impl::GraphicsPipeline_* object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from impl::ComputePipeline_& to native::HPipeline_*
\return Return native::HPipeline_&
***********************************************************************************************************************/
native::HPipeline_& native_cast(impl::ComputePipeline_& object);

/*!*********************************************************************************************************************
\brief Cast from const impl::ComputePipeline_& to const native::HPipeline_*
\return Return const native::HPipeline_&
***********************************************************************************************************************/
const native::HPipeline_& native_cast(const impl::ComputePipeline_& object);

/*!*********************************************************************************************************************
\brief Cast from const ComputePipeline& to const native::HPipeline_*
\return Return const native::HPipeline_*
***********************************************************************************************************************/
inline const native::HPipeline_* native_cast(const ComputePipeline& object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const impl::ComputePipeline_& to const native::HPipeline_*
\return Return const native::HPipeline_*
***********************************************************************************************************************/
inline const native::HPipeline_* native_cast(const impl::ComputePipeline_* object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const ComputePipeline& to const native::HPipeline_*
\return Return const native::HPipeline_*
***********************************************************************************************************************/
inline native::HPipeline_* native_cast(ComputePipeline& object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const impl::ComputePipeline_& to const native::HPipeline_*
\return Return const native::HPipeline_*
***********************************************************************************************************************/
inline native::HPipeline_* native_cast(impl::ComputePipeline_* object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from impl::CommandBufferBase_& to native::HCommandBuffer_&
\return Return native::HCommandBuffer_&
***********************************************************************************************************************/
=======


/// <summary>Cast from impl::CommandBufferBase_& to native::HCommandBuffer_&</summary>
/// <returns>Return native::HCommandBuffer_&</returns>
>>>>>>> 1776432f... 4.3
native::HCommandBuffer_& native_cast(impl::CommandBufferBase_& object);

/// <summary>Cast from const impl::CommandBufferBase_& to const native::HCommandBuffer_&</summary>
/// <returns>Return const native::HCommandBuffer_&</returns>
const native::HCommandBuffer_& native_cast(const impl::CommandBufferBase_& object);

/// <summary>Cast from const CommandBufferBase& to const native::HCommandBuffer_*</summary>
/// <returns>Return const native::HCommandBuffer_*</returns>
inline const native::HCommandBuffer_* native_cast(const CommandBufferBase& object) { return &native_cast(*object); }

/// <summary>Cast from const impl::CommandBufferBase_* to const native::HCommandBuffer_*</summary>
/// <returns>Return const native::HCommandBuffer_*</returns>
inline const native::HCommandBuffer_* native_cast(const impl::CommandBufferBase_* object) { return &native_cast(*object); }

/// <summary>Cast from const impl::CommandBufferBase_& to const native::HCommandBuffer_*</summary>
/// <returns>Return const native::HCommandBuffer_*</returns>
inline native::HCommandBuffer_* native_cast(CommandBufferBase& object) { return &native_cast(*object); }

/// <summary>Cast from const impl::CommandBufferBase_& to const native::HCommandBuffer_*</summary>
/// <returns>Return const native::HCommandBuffer_*</returns>
inline native::HCommandBuffer_* native_cast(impl::CommandBufferBase_* object) { return &native_cast(*object); }

}//namespace api
}//namespace pvr

//!\endcond