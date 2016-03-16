/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/ContextVulkan.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Definition of the Vulkan implementation of the GraphicsContext (pvr::platform::ContextVulkan)
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRCore/IPlatformContext.h"
#include "PVRApi/Vulkan/FboVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/Vulkan/VkErrors.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRApi/GpuCapabilities.h"
#include "PVRApi/ApiObjects/CommandBuffer.h"
#include "PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h"
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

/*!*********************************************************************************************************************
\brief IGraphicsContext implementation that supports Vulkan
***********************************************************************************************************************/
class ContextVk : public IGraphicsContext
{
public:
	/*!*********************************************************************************************************************
	\brief Create a new, empty, uninitialized context.
	***********************************************************************************************************************/
	ContextVk() {}

	/*!*********************************************************************************************************************
	\brief Virtual destructor.
	***********************************************************************************************************************/
	virtual ~ContextVk(){	release();	}

	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Initializes this class using an OS manager.
	\description This function must be called before using the Context. Will use the OS manager to make this Context
	ready to use.
	***********************************************************************************************************************/
	Result::Enum init(OSManager& osManager, GraphicsContext& my_wrapper);

	void setUpCapabilities();

	/*!****************************************************************************************************************
	\brief	Implementation of IGraphicsContext. Release the resources held by this context.
	*******************************************************************************************************************/
	void release();

	/*!****************************************************************************************************************
	\brief	Implementation of IGraphicsContext. Take a screenshot in the specified buffer of the specified screen area.
	*******************************************************************************************************************/
	bool screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* buffer, ImageFormat requestedImageFormat);

	/*!*********************************************************************************************************************
	\brief Implementation of IGraphicsContext. Query if a specific extension is supported.
	\param extension A c-style string representing the extension
	\return True if the extension is supported
	***********************************************************************************************************************/
	bool isExtensionSupported(const char8* extension) const {	return false;	}

	/*!*********************************************************************************************************************
	\brief  Implementation of IGraphicsContext. Print information about this IGraphicsContext.
	***********************************************************************************************************************/
	std::string getInfo() const;

	/*!*********************************************************************************************************************
	\brief  Wait until all pending operations are completed
	***********************************************************************************************************************/
	void waitIdle();

	/*!*********************************************************************************************************************
	\brief	return true if last bound pipeline was graphics
	\return	bool
	***********************************************************************************************************************/
	bool isLastBoundPipelineGraphics() const { return false; }

	/*!*********************************************************************************************************************
	\brief	return true if last bound pipeline was compute
	\return	bool
	***********************************************************************************************************************/
	bool isLastBoundPipelineCompute() const { return false; }

	/*!*********************************************************************************************************************
	\brief Get the last bound fbo.
	\return Return the bound fbo.
	***********************************************************************************************************************/
	const api::Fbo& getBoundFbo()const {	return m_renderStatesTracker.boundFbo; }

	/*!*********************************************************************************************************************
	\brief  Implementation of IGraphicsContext. Return the  default renderpass.
	\return The default renderpass
	***********************************************************************************************************************/
	const api::RenderPass& getDefaultRenderPass()const { return m_defaultRenderPass; }

	/*!*********************************************************************************************************************
	\brief  Implementation of IGraphicsContext. Return the  platform context that powers this graphics context.
	***********************************************************************************************************************/
	IPlatformContext& getPlatformContext() const { return *m_platformContext; }

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

	struct BufferRange
	{
		api::Buffer buffer;
		uint32 offset;
		uint32 range;
		BufferRange() : offset(0), range(0) {}
		BufferRange(const api::Buffer& buffer, uint32 offset, uint32 range) : buffer(buffer), offset(offset), range(range) {}
	};
	typedef std::vector<std::pair<uint16, BufferRange>/**/> ProgBufferBingingList;

	/*!****************************************************************************************************************
	\brief internal state tracker for Vulkan
	*******************************************************************************************************************/
	struct RenderStatesTracker
	{
		friend class ::pvr::platform::ContextVk;
		api::GraphicsPipeline pushedGraphicsPipeline;
		api::GraphicsPipeline pushedComputePipeline;
		api::Buffer lastBoundVbo;
		api::Fbo boundFbo;
	private:
		void releaseAll() {	*this = RenderStatesTracker();	}
	public:
	};

	RenderStatesTracker& getCurrentRenderStates() { return m_renderStatesTracker; }
	RenderStatesTracker const& getCurrentRenderStates()const { return m_renderStatesTracker; }
	
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
	\brief Get the vulkan device
	\return Return the vulkan device
	***********************************************************************************************************************/
	VkDevice& getDevice(){	return m_platformContext->getNativePlatformHandles().context.device;	}

	/*!*********************************************************************************************************************
	\brief Get the const vulkan device
	\return Return the const vulkan device
	***********************************************************************************************************************/
	const VkDevice& getDevice()const { return  m_platformContext->getNativePlatformHandles().context.device; }

	/*!*********************************************************************************************************************
	\brief Get the vulkan physical device
	\return Return the vulkan physical device
	***********************************************************************************************************************/
	VkPhysicalDevice& getPhysicalDevice() { return m_platformContext->getNativePlatformHandles().context.physicalDevice; }

	/*!*********************************************************************************************************************
	\brief Get the const vulkan physical device
	\return Return the const vulkan physical device
	***********************************************************************************************************************/
	const VkPhysicalDevice& getPhysicalDevice()const { return m_platformContext->getNativePlatformHandles().context.physicalDevice; }

	/*!*********************************************************************************************************************
	\brief Get the reference to the device queue
	\return Return the reference to the device queue
	***********************************************************************************************************************/
	VkQueue& getQueue() { return m_platformContext->getNativePlatformHandles().graphicsQueue; }
	
	/*!*********************************************************************************************************************
	\brief Get the const reference to the device queue
	\return Return the const reference to the device queue
	***********************************************************************************************************************/
	const VkQueue& getQueue()const { return  m_platformContext->getNativePlatformHandles().graphicsQueue;}

	/*!*********************************************************************************************************************
	\brief Get the const Vulkan instance
	\return Return the const Vulkan instance
	***********************************************************************************************************************/
	const VkInstance getVkInstance()const { return m_platformContext->getNativePlatformHandles().context.instance; }

	/*!*********************************************************************************************************************
	\brief Get the const Vulkan instance
	\return Return the const Vulkan instance
	***********************************************************************************************************************/
	VkInstance getVkInstance() { return m_platformContext->getNativePlatformHandles().context.instance; }

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
	Result::Enum createShader(const Stream& stream, string& outSourceData);

	/*!*********************************************************************************************************************
	\brief Internal use. State tracking. Notify fbo unbind.
	***********************************************************************************************************************/
	ContextVk(size_t implementationId);

	/*!*********************************************************************************************************************
	\brief Get the texture upload commandbuffer
	\return Return the Texture upload commandbuffer
	***********************************************************************************************************************/
	native::HCommandBuffer_& getTextureUploadCommandBuffer(){	return pvr::api::native_cast(*m_cmdTextureUpload);	}

	api::RenderPass m_defaultRenderPass;
	IPlatformContext* m_platformContext;
	RenderStatesTracker m_renderStatesTracker;
protected:
	size_t m_ContextImplementationID;
	mutable std::string m_extensions;
	api::Sampler m_defaultSampler;
	api::DescriptorPool m_descriptorPool;// default descriptor pool
	api::CommandPool m_commandPool;// default command pool
	api::CommandBuffer m_cmdTextureUpload;
	VkPhysicalDeviceMemoryProperties m_memoryProperties;
	//native::HContext_ m_context;
	api::Fbo m_fboOnScreen;
	pvr::uint32 queueFamilyIndex;

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
/*!*********************************************************************************************************************
\brief Cast from const IGraphicsContext& to const platform::ContextVk&
\return Return const platform::ContextVk&
***********************************************************************************************************************/
inline const platform::ContextVk& native_cast(const IGraphicsContext& object) { return static_cast<const platform::ContextVk&>(object); }

/*!*********************************************************************************************************************
\brief Cast from IGraphicsContext& to platform::ContextVk&
\return Return platform::ContextVk&
***********************************************************************************************************************/
inline platform::ContextVk& native_cast(IGraphicsContext& object) { return static_cast<platform::ContextVk&>(object); }

/*!*********************************************************************************************************************
\brief Cast from const GraphicsContext& to const platform::ContextVk*
\return Return const platform::ContextVk*
***********************************************************************************************************************/
inline const platform::ContextVk* native_cast(const GraphicsContext& object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const IGraphicsContext* to const platform::ContextVk*
\return Return const platform::ContextVk*
***********************************************************************************************************************/
inline const platform::ContextVk* native_cast(const IGraphicsContext* object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from GraphicsContext& to platform::ContextVk*
\return Return platform::ContextVk*
***********************************************************************************************************************/
inline platform::ContextVk* native_cast(GraphicsContext& object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from IGraphicsContext* to platform::ContextVk*
\return Return platform::ContextVk*
***********************************************************************************************************************/
inline platform::ContextVk* native_cast(IGraphicsContext* object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const impl::GraphicsPipeline_& to const native::HPipeline_&
\return Return const native::HPipeline_&
***********************************************************************************************************************/
native::HPipeline_& native_cast(impl::GraphicsPipeline_& object);

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
native::HCommandBuffer_& native_cast(impl::CommandBufferBase_& object);

/*!*********************************************************************************************************************
\brief Cast from const impl::CommandBufferBase_& to const native::HCommandBuffer_&
\return Return const native::HCommandBuffer_&
***********************************************************************************************************************/
const native::HCommandBuffer_& native_cast(const impl::CommandBufferBase_& object);

/*!*********************************************************************************************************************
\brief Cast from const CommandBufferBase& to const native::HCommandBuffer_*
\return Return const native::HCommandBuffer_*
***********************************************************************************************************************/
inline const native::HCommandBuffer_* native_cast(const CommandBufferBase& object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const impl::CommandBufferBase_* to const native::HCommandBuffer_*
\return Return const native::HCommandBuffer_*
***********************************************************************************************************************/
inline const native::HCommandBuffer_* native_cast(const impl::CommandBufferBase_* object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const impl::CommandBufferBase_& to const native::HCommandBuffer_*
\return Return const native::HCommandBuffer_*
***********************************************************************************************************************/
inline native::HCommandBuffer_* native_cast(CommandBufferBase& object) { return &native_cast(*object); }

/*!*********************************************************************************************************************
\brief Cast from const impl::CommandBufferBase_& to const native::HCommandBuffer_*
\return Return const native::HCommandBuffer_*
***********************************************************************************************************************/
inline native::HCommandBuffer_* native_cast(impl::CommandBufferBase_* object) { return &native_cast(*object); }

}//namespace api
}//namespace pvr
