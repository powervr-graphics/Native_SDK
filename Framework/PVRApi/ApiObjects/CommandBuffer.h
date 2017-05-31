/*!
\brief Contains the CommandBuffer implementation. Heavily uses the Template method and the Bridge pattern.
\file PVRApi/ApiObjects/CommandBuffer.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRCore/Math/AxisAlignedBox.h"

namespace pvr {
namespace api {
/// <summary>Cast this object into its Api-specific implementation. In order to use this function,
/// a specific NativeApi/[MYAPI]/NativeObjects.h must be included</summary>
/// <param name="object">The Command buffer to cast</param>
/// <returns> A container of the native handle (Api specific, e.g. VkCommandBuffer)</returns>
native::HCommandBuffer_& native_cast(::pvr::api::impl::CommandBufferBase_& object);
/// <summary>Cast this object into its Api-specific implementation. In order to use this function,
/// a specific NativeApi/[MYAPI]/NativeObjects.h must be included</summary>
/// <param name="object">The Command buffer to cast</param>
/// <returns> A container of the native handle (Api specific, e.g. VkCommandBuffer)</returns>
const native::HCommandBuffer_& native_cast(const ::pvr::api::impl::CommandBufferBase_& object);
namespace impl {
//!\cond NO_DOXYGEN
//This is our bridge interface. All calls from the CommandBuffer(CommandBufferBase, [Primary]CommandBuffer, SecondaryCommandBuffer) gets delegated to the bridge,
//which will carry the different implementations between different APIS. Any fixed code (the contexts, some flags etc.) will stay with the respective objects.
class ICommandBufferImpl_ // INTERFACE CLASS
{
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
protected:
	typedef RefCountedWeakReference<CommandPool_> CommandPoolWeakRef;
public:

	virtual void beginRecording_() = 0;
	virtual void beginRecording_(const RenderPass& rp, uint32 subPass = 0) = 0;
	virtual void beginRecording_(const Fbo& fbo, uint32 subPass = 0) = 0;

	virtual void endRecording_() = 0;
	virtual void clear_(bool releaseAllResources = false) = 0;

	virtual void submit_(const Semaphore& waitSemaphore, const Semaphore& signalSemaphore, const Fence& fence = Fence()) = 0;
	virtual void submit_(SemaphoreSet& waitSemaphores, SemaphoreSet& signalSemaphores, const Fence& fence = Fence()) = 0;
	virtual void submit_(Fence& fence) = 0;
	virtual void submit_() = 0;
	virtual void submitEndOfFrame_(Semaphore& waitSemaphore) = 0;
	virtual void submitStartOfFrame_(Semaphore& signalSemaphore, const Fence& fence = Fence()) = 0;
	virtual void enqueueSecondaryCmds_(SecondaryCommandBuffer& secondaryCmdBuffer) = 0;
	virtual void enqueueSecondaryCmds_(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers) = 0;
	virtual void enqueueSecondaryCmds_BeginMultiple_(uint32 expectedMax = 255) = 0;
	virtual void enqueueSecondaryCmds_EnqueueMultiple_(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers) = 0;
	virtual void enqueueSecondaryCmds_SubmitMultiple_(bool keepAllocated = false) = 0;

	virtual void beginRenderPass_(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0) = 0;
	virtual void beginRenderPass_(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32 clearDepth = 1.f, uint32 clearStencil = 0) = 0;
	virtual void beginRenderPass_(api::Fbo& fbo, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0) = 0;
	virtual void beginRenderPass_(api::Fbo& fbo, const RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0) = 0;
	virtual void beginRenderPass_(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32* clearDepth, uint32* clearStencil, uint32 numClearDepthStencil) = 0;
	virtual void beginRenderPass_(api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0) = 0;

	virtual void endRenderPass_() = 0;
	virtual void nextSubPassInline_() = 0;
	virtual void nextSubPassSecondaryCmds_(SecondaryCommandBuffer& cmdBuffer) = 0;

	virtual void bindPipeline_(GraphicsPipeline& pipeline) = 0;
	virtual void bindPipeline_(ComputePipeline& pipeline) = 0;
	virtual void bindPipeline_(SceneTraversalPipeline& pipeline) = 0;
	virtual void bindPipeline_(VertexRayPipeline& pipeline) = 0;
	virtual void bindDescriptorSet_(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0) = 0;
	virtual void bindDescriptorSetCompute_(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0) = 0;
	virtual void bindDescriptorSetRayTracing_(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0) = 0;
	virtual void bindDescriptorSetSHG_(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0) = 0;
	virtual void bindDescriptorSets_(types::PipelineBindPoint bindingPoint, const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0) = 0;
	virtual void bindVertexBuffer_(const Buffer& buffer, uint32 offset, uint16 bindingIndex) = 0;
	virtual void bindVertexBuffer_(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding, uint16 bindingCount) = 0;
	virtual void bindIndexBuffer_(const api::Buffer& buffer, uint32 offset, types::IndexType indexType) = 0;

	virtual void clearColorAttachment_(uint32 const* attachmentIndices, glm::vec4 const* clearColors, uint32 attachmentCount, const Rectanglei* rects, const uint32* baseArrayLayers, const uint32* layerCounts, uint32 rectCount) = 0;
	virtual void clearColorAttachment_(uint32 attachmentIndex, glm::vec4 clearColor, const Rectanglei rect, const uint32 baseArrayLayer = 0u, const uint32 layerCount = 1u) = 0;
	virtual void clearColorAttachment_(api::Fbo fbo, glm::vec4 clearColor) = 0;
	virtual void clearDepthAttachment_(const Rectanglei& clearRect, float32 depth = 1.f) = 0;
	virtual void clearStencilAttachment_(const Rectanglei& clearRect, int32 stencil = 0) = 0;
	virtual void clearDepthStencilAttachment_(const Rectanglei& clearRect, float32 depth = 1.f, int32 stencil = 0) = 0;
	virtual void clearColorImage_(api::TextureView& image, glm::vec4 clearColor, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayer = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General) = 0;
	virtual void clearColorImage_(api::TextureView& image, glm::vec4 clearColor, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General) = 0;
	virtual void clearDepthImage_(api::TextureView& image, float clearDepth, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayer = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General) = 0;
	virtual void clearDepthImage_(api::TextureView& image, float clearDepth, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General) = 0;
	virtual void clearStencilImage_(api::TextureView& image, uint32 clearStencil, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayers = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General) = 0;
	virtual void clearStencilImage_(api::TextureView& image, uint32 clearStencil, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General) = 0;
	virtual void clearDepthStencilImage_(api::TextureView& image, float clearDepth, uint32 clearStencil, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayers = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General) = 0;
	virtual void clearDepthStencilImage_(api::TextureView& image, float clearDepth, uint32 clearStencil, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General) = 0;
	virtual void setViewport_(const Rectanglei& viewport) = 0;
	virtual void setScissor_(const Rectanglei& scissor) = 0;
	virtual void setDepthBound_(float32 min = 0.0f, float32 max = 1) = 0;
	virtual void setStencilCompareMask_(types::StencilFace face, uint32 compareMask) = 0;
	virtual void setStencilWriteMask_(types::StencilFace face, uint32 writeMask) = 0;
	virtual void setStencilReference_(types::StencilFace face, uint32 ref) = 0;
	virtual void setDepthBias_(float32 depthBiasConstantFactor, float32 depthBiasClamp, float32 depthBiasSlopeFactor) = 0;
	virtual void setBlendConstants_(glm::vec4 rgba) = 0;
	virtual void setLineWidth_(float32 lineWidth) = 0;

	virtual void drawIndexed_(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset = 0, uint32 firstInstance = 0, uint32 instanceCount = 1) = 0;
	virtual void drawArrays_(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance = 0, uint32 instanceCount = 1) = 0;
	virtual void drawArraysIndirect_(api::Buffer& buffer, uint32 offset, uint32 drawCount, uint32 stride) = 0;
	virtual void drawIndexedIndirect_(Buffer& buffer) = 0;
	virtual void drawIndirect_(Buffer& buffer, uint32 offset, uint32 count, uint32 stride) = 0;
	virtual void dispatchCompute_(uint32 numGroupsX, uint32 numGroupsY = 1, uint32 numGroupsZ = 1) = 0;

	virtual void updateBuffer_(Buffer& buffer, const void* data, uint32 offset, uint32 length) = 0;
	virtual void copyBuffer_(api::Buffer src, api::Buffer dest, uint32 srcOffset, uint32 destOffset, uint32 sizeInBytes) = 0;
	virtual void blitImage_(api::TextureStore& src, api::TextureStore& dest, types::ImageLayout srcLayout, types::ImageLayout dstLayout, types::ImageBlitRange* regions, uint32 numRegions, types::SamplerFilter filter) = 0;
	virtual void copyImageToBuffer_(api::TextureStore& srcImage, types::ImageLayout srcImageLayout, api::Buffer& dstBuffer, types::BufferImageCopy* regions, uint32 numRegions) = 0;

	virtual void pushPipeline_() = 0;
	virtual void popPipeline_() = 0;
	virtual void resetPipeline_() = 0;

	virtual void pipelineBarrier_(types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers, bool dependencyByRegion = true) = 0;
	virtual void waitForEvent_(const Event& evt, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers) = 0;
	virtual void waitForEvents_(const EventSet& evts, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers) = 0;
	virtual void setEvent_(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands) = 0;
	virtual void resetEvent_(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands) = 0;

	virtual void beginSceneHierarchy_(const SceneHierarchy& sceneHierarchy, math::AxisAlignedBox& extents) = 0;
	virtual void endSceneHierarchy_() = 0;
	virtual void mergeSceneHierarchies_(const SceneHierarchy& destinationSceneHierarchy, math::AxisAlignedBox& extents, const SceneHierarchy* sourceSceneHierarchies, const uint32 numberOfSourceSceneHierarchies, const uint32 mergeQuality) = 0;
	virtual void bindSceneHierarchies_(const SceneHierarchy* sceneHierarchies, uint32 firstBinding, const uint32 numberOfSceneHierarchies) = 0;
	virtual void dispatchRays_(uint32 xOffset, uint32 yOffset,  uint32 frameWidth, uint32 frameHeight) = 0;
	virtual void bindAccumulationImages_(uint32 startBinding, uint32 bindingCount, const TextureView* imageViews) = 0;
	virtual void sceneHierarchyAppend_(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) = 0;
	virtual void sceneHierarchyAppendIndexed_(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) = 0;
	virtual void sceneHierarchyAppendIndirect_(api::BufferView& indirectBuffer, uint32 offset, uint32 drawCount, uint32 stride) = 0;
	virtual void sceneHierarchyAppendIndexedIndirect_(api::BufferView& indirectBuffer, uint32 offset, uint32 drawCount, uint32 stride) = 0;
	virtual void pushSharedRayConstants_(uint32 offset, uint32 size, const void* pValues) = 0;
	virtual void setRaySizes_(uint32 raySizeCount, const uint32* pRaySizes) = 0;
	virtual void setRayBounceLimit_(uint32 limit) = 0;

#ifdef DEBUG
	virtual void logCommandStackTraces_() = 0;
#endif

#define SET_UNIFORM_DECLARATION(_type_)\
  virtual void setUniform_(int32 location, const _type_& val) = 0; \
  virtual void setUniformPtr_(int32 location, uint32 count, const _type_* ptr) = 0;

	SET_UNIFORM_DECLARATION(uint32);
	SET_UNIFORM_DECLARATION(int32);
	SET_UNIFORM_DECLARATION(float32);
	SET_UNIFORM_DECLARATION(glm::vec2);
	SET_UNIFORM_DECLARATION(glm::ivec2);
	SET_UNIFORM_DECLARATION(glm::uvec2);
	SET_UNIFORM_DECLARATION(glm::vec3);
	SET_UNIFORM_DECLARATION(glm::ivec3);
	SET_UNIFORM_DECLARATION(glm::uvec3);
	SET_UNIFORM_DECLARATION(glm::vec4);
	SET_UNIFORM_DECLARATION(glm::ivec4);
	SET_UNIFORM_DECLARATION(glm::uvec4);
	SET_UNIFORM_DECLARATION(glm::mat2);
	SET_UNIFORM_DECLARATION(glm::mat2x3);
	SET_UNIFORM_DECLARATION(glm::mat2x4);
	SET_UNIFORM_DECLARATION(glm::mat3x2);
	SET_UNIFORM_DECLARATION(glm::mat3);
	SET_UNIFORM_DECLARATION(glm::mat3x4);
	SET_UNIFORM_DECLARATION(glm::mat4x2);
	SET_UNIFORM_DECLARATION(glm::mat4x3);
	SET_UNIFORM_DECLARATION(glm::mat4x4);
#undef SET_UNIFORM_DECLARATION

	// NON VIRTUAL MEMBERS //
	bool isRecording_() const { return _isRecording; }
	GraphicsContext& getContext_() { return _context; }
	const GraphicsContext& getContext_() const { return _context; }
	CommandPoolWeakRef& getCommandPool_() { return _pool; }
	const CommandPoolWeakRef& getCommandPool_() const { return _pool; }

	virtual ~ICommandBufferImpl_() {}
protected:
	GraphicsContext _context;
	CommandPoolWeakRef _pool;
	bool _isRecording;
	ICommandBufferImpl_(GraphicsContext context, CommandPool pool):
		_context(context), _pool(pool), _isRecording(false) {}
};
//!\endcond

/// <summary>Contains all the commands and states that need to be submitted to the gpu, including pipeline, texture,
/// and samplers. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer. In debug
/// builds (define DEBUG or define PVR_STORE_STACK_TRACE_WITH_API_COMMANDS), a limited stack trace is stored with
/// each command so that if an error occurs, the site where the command was actually added to the command buffer
/// can be determined. <para>Primary command buffers can contain RenderPasses, and can be submitted to the GPU.
/// Secondary command buffers cannot contain RenderPasses, and can be enqueued to PrimaryCommandBuffers.</para>
/// <para>-It is invalid to submit commands to a command buffer while it is not being recorded.</para> <para>-It is
/// invalid to reset a command buffer while it is being recorded.</para> <para>-It is invalid to submit a command
/// buffer more than once if it is one time submit command buffer</para></summary>
class CommandBufferBase_
{
	friend native::HCommandBuffer_& ::pvr::api::native_cast(pvr::api::impl::CommandBufferBase_& object);
	friend const native::HCommandBuffer_& ::pvr::api::native_cast(const pvr::api::impl::CommandBufferBase_& object);
public:
<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief Destructor. Frees all resources.
	***********************************************************************************************************************/
	virtual ~CommandBufferBase_();

	/*!*********************************************************************************************************************
	\brief Get a reference to the context used by this CommandBuffer.
	***********************************************************************************************************************/
	GraphicsContext& getContext();

	/*!****************************************************************************************************************
	\brief	Call this function when you are done recording commands. BeginRecording must be called first.
	*******************************************************************************************************************/
	void endRecording();

	/*!****************************************************************************************************************
	\brief	Queries if a command buffer is in the recording state
	\return	True if recording, false otherwise
	*******************************************************************************************************************/
	bool isRecording();

	/*!****************************************************************************************************************
	\brief	Bind a graphics pipeline.
	\param	pipeline The GraphicsPipeline to bind.
	*******************************************************************************************************************/
	void bindPipeline(GraphicsPipeline pipeline);

	/*!****************************************************************************************************************
	\brief	Bind a compute pipeline
	\param	pipeline The ComputePipeline to bind
	*******************************************************************************************************************/
	void bindPipeline(ComputePipeline& pipeline);

	/*!****************************************************************************************************************
	\brief	Bind a single DescriptorSet to the Graphics Pipeline binding point
	\param	pipelineLayout The pipelineLayout that the GraphicsPipeline will have
	\param	index The index to which to bind the descriptor set
	\param	set The descriptorSet to bind to the binding point bindingPoint
	\param	dynamicOffsets A c-style array of unsigned integers, each of which is consecutively applied as a Dynamic
	Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.
	\param	numDynamicOffsets The number of dynamic offsets in \p dynamicOffsets. Must exactly much the number of Dynamic
	        objects in the \p set.
	*******************************************************************************************************************/
	void bindDescriptorSet(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set,
	                       const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0);

	/*!****************************************************************************************************************
	\brief	Bind a single DescriptorSet to the Compute Pipeline binding point
	\param	pipelineLayout The pipelineLayout that the ComputePipeline will have
	\param	index The index to which to bind the descriptor set
	\param	set The descriptorSet to bind to the binding point bindingPoint
	\param	dynamicOffsets A c-style array of unsigned integers, each of which is consecutively applied as a Dynamic
	Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.
	\param	numDynamicOffsets The number of dynamic offsets in \p dynamicOffsets. Must exactly much the number of Dynamic
	objects in the \p set.
	*******************************************************************************************************************/
	void bindDescriptorSetCompute(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set,
	                              const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0);

	/*!****************************************************************************************************************
	\brief	Bind multiple DescriptorSets
	\param	bindingPoint The index where the first descriptor set will bind to. The rest will be bound successively.
	\param	pipelineLayout The pipelineLayout that the GraphicsPipeline will have
	\param	firstSet The first index to start binding descriptor sets to.
	\param	sets The array of descriptorSets. The first item in the array will be bound to binding point \p firstSet and 
			each one after to the next binding point
	\param	dynamicOffsets An array of Offsets that will be used when binding items of this descriptor set respectively
	\param	numDescSets The number of descriptor sets in the array
	\param	dynamicOffsets A c-style array of unsigned integers, each of which is consecutively applied as a Dynamic
	Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.
	\param	numDynamicOffsets The number of dynamic offsets in \p dynamicOffsets. Must exactly much the number of Dynamic
	objects in the \p set.
	*******************************************************************************************************************/
	void bindDescriptorSets(types::PipelineBindPoint bindingPoint, const api::PipelineLayout& pipelineLayout,
	                        uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets = NULL,
	                        uint32 numDynamicOffsets = 0);

	/*!****************************************************************************************************************
	\brief	Clear multiple attachments with separate clear colors and clear rectangle for each.
	\param	attachmentIndices Attachments indices to clear
	\param	clearColors An array of colors to clear to, each corresponding to an attachment
	\param	attachmentCount Number of attachments to clear
	\param  rects An array of rectangles, each corresponding to the clear area of an attachment
	\param  baseArrayLayers An array of base array layers corresponding to the first layer to be cleared for the attachment
	\param  layerCounts An array of layer counts corresponding to number of layers to clear for the attachment
	\param  rectCount The number of rectangles to clear (the number of rectangles in \p rects)
	*******************************************************************************************************************/
	void clearColorAttachment(pvr::uint32 const* attachmentIndices, glm::vec4 const* clearColors, pvr::uint32 attachmentCount,
	                          const pvr::Rectanglei* rects, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCounts, pvr::uint32 rectCount);

	/*!****************************************************************************************************************
	\param	attachmentIndex The index for the attachment to clear
	\param	clearColor The clear area
	\param	rect The rectangle to clear
	\param  baseArrayLayer The base array layer corresponding to the first layer to be cleared for the attachment
	\param  layerCount The number of layers to clear for the attachment
	*******************************************************************************************************************/
	void clearColorAttachment(pvr::uint32 attachmentIndex, glm::vec4 clearColor, const pvr::Rectanglei rect,
	                          const pvr::uint32 baseArrayLayer = 0u, const pvr::uint32 layerCount = 1u);

	/*!****************************************************************************************************************
	\brief	Clear all attachment for a single fbo with a single clear color.
			NOTE: This clear operation must be called inside the render pass
	\param	fbo The fbo to clear attachments
	\param	clearColor The clear area
	*******************************************************************************************************************/
	void clearColorAttachment(pvr::api::Fbo fbo, glm::vec4 clearColor);

	/*!****************************************************************************************************************
	\brief	Clear the depth attachment of an fbo.
			This clear operation must be called inside the render pass
	\brief	Clear the depth attachment of an fbo.
			NOTE: This clear operation must be called inside the render pass
	\brief	Clear the depth attachment of an fbo
	\param	clearRect The clear area
	\param	depth The clear value
	*******************************************************************************************************************/
	void clearDepthAttachment(const pvr::Rectanglei& clearRect, float32 depth = 1.f);

	/*!****************************************************************************************************************
	\brief	Clear the stencil attachment of an fbo.
			NOTE: This clear operation must be called inside the render pass
	\param	clearRect The clear area
	\param	stencil The clear value
	*******************************************************************************************************************/
	void clearStencilAttachment(const pvr::Rectanglei& clearRect, pvr::int32 stencil = 0);

	/*!****************************************************************************************************************
	\brief	Clear the depth stencil attachment
	\param	clearRect clear area
	\param	depth The depth clear value
	\param	stencil The stencil clear value
	*******************************************************************************************************************/
	void clearDepthStencilAttachment(const pvr::Rectanglei& clearRect, float32 depth = 1.f, pvr::int32 stencil = 0);

	/*!****************************************************************************************************************
	\brief	Clears the specified color image using the clear color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearColor The clear color to use for the clear
	\param	baseMipLevel base mip levels
	\param	levelCount level counts
	\param	baseArrayLayer base array layers
	\param	layerCount number of layers to clear
	*******************************************************************************************************************/
	void clearColorImage(pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32 baseMipLevel = 0u,
	                     const pvr::uint32 levelCount = 1u, const pvr::uint32 baseArrayLayer = 0u,
	                     const pvr::uint32 layerCount = 1u, pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears rangeCount sub resource ranges of the specified color image using the clear color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearColor The clear color to use for the clear
	\param	baseMipLevel rangeCount base mip levels
	\param	levelCount rangeCount level counts
	\param	baseArrayLayers rangeCount base array layers
	\param	layerCount rangeCount number of layers to clear
	\param	rangeCount The number of sub resource ranges to clear
	*******************************************************************************************************************/
	void clearColorImage(pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32* baseMipLevel,
	                     const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount,
	                     pvr::uint32 rangeCount, pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears the specified depth image using the clear depth color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearDepth The clear color to use for the clear
	\param	baseMipLevel base mip levels
	\param	levelCount level counts
	\param	baseArrayLayer base array layer
	\param	layerCount number of layers to clear
	*******************************************************************************************************************/
	void clearDepthImage(pvr::api::TextureView& image, float clearDepth, const pvr::uint32 baseMipLevel = 0u,
	                     const pvr::uint32 levelCount = 1u, const pvr::uint32 baseArrayLayer = 0u,
	                     const pvr::uint32 layerCount = 1u, pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears rangeCount sub resource ranges of the specified depth image using the clear depth color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearDepth The clear color to use for the clear
	\param	baseMipLevel rangeCount base mip levels
	\param	levelCount rangeCount level counts
	\param	baseArrayLayers rangeCount base array layers
	\param	layerCount rangeCount number of layers to clear
	\param	rangeCount The number of sub resource ranges to clear
	*******************************************************************************************************************/
	void clearDepthImage(pvr::api::TextureView& image, float clearDepth, const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount,
	                     const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount,
	                     pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears the specified stencil image using the clear stencil color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearStencil The clear color to use for the clear
	\param	baseMipLevel base mip levels
	\param	levelCount level counts
	\param	baseArrayLayers base array layers
	\param	layerCount number of layers to clear
	*******************************************************************************************************************/
	void clearStencilImage(pvr::api::TextureView& image, pvr::uint32 clearStencil, const pvr::uint32 baseMipLevel = 0u,
	                       const pvr::uint32 levelCount = 1u, const pvr::uint32 baseArrayLayers = 0u, const pvr::uint32 layerCount = 1u,
	                       pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears rangeCount sub resource ranges of the specified stencil image using the clear stencil color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearStencil The clear color to use for the clear
	\param	baseMipLevel rangeCount base mip levels
	\param	levelCount rangeCount level counts
	\param	baseArrayLayers rangeCount base array layers
	\param	layerCount rangeCount number of layers to clear
	\param	rangeCount The number of sub resource ranges to clear
	*******************************************************************************************************************/
	void clearStencilImage(pvr::api::TextureView& image, pvr::uint32 clearStencil, const pvr::uint32* baseMipLevel,
	                       const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount,
	                       pvr::uint32 rangeCount, pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears the specified depth stencil image using the clear depth and stencil color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearDepth The clear depth color to use for the clear
	\param	clearStencil The clear color to use for the clear
	\param	baseMipLevel base mip levels
	\param	levelCount level counts
	\param	baseArrayLayers base array layers
	\param	layerCount number of layers to clear
	*******************************************************************************************************************/
	void clearDepthStencilImage(pvr::api::TextureView& image, float clearDepth, pvr::uint32 clearStencil,
	                            const pvr::uint32 baseMipLevel = 0u, const pvr::uint32 levelCount = 1u,
	                            const pvr::uint32 baseArrayLayers = 0u, const pvr::uint32 layerCount = 1u,
	                            pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears rangeCount sub resource ranges of the specified depth stencil image using the clear depth and stencil color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearDepth The clear depth color to use for the clear
	\param	clearStencil The clear color to use for the clear
	\param	baseMipLevel rangeCount base mip levels
	\param	levelCount rangeCount level counts
	\param	baseArrayLayers rangeCount base array layers
	\param	layerCount rangeCount number of layers to clear
	\param	rangeCount The number of sub resource ranges to clear
	*******************************************************************************************************************/
	void clearDepthStencilImage(pvr::api::TextureView& image, float clearDepth, pvr::uint32 clearStencil,
	                            const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers,
	                            const pvr::uint32* layerCount, pvr::uint32 rangeCount,
	                            pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Draw command. Use the current state in the command buffer (pipelines, buffers, descriptor sets) to execute
	        a drawing command. Uses the currently bound IndexBuffer for indexes into the currently bound VBOs.
	\param	firstIndex Offset into the Index buffer to start drawing from
	\param	indexCount The number of indexes to draw
	\param	vertexOffset Index into the Vertex Buffer to start drawing from
	\param	firstInstance The Instance from which to draw from
	\param	instanceCount The number of instances to draw
	*******************************************************************************************************************/
	void drawIndexed(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset = 0,
	                 uint32 firstInstance = 0, uint32 instanceCount = 1);

	/*!****************************************************************************************************************
	\brief	Draw command. Use the current state in the command buffer (pipelines, buffers, descriptor sets) to execute
	        a drawing command. Does not use an IndexBuffer.
	\param	firstVertex The vertex from where to start drawing
	\param	vertexCount The number of vertices to draw
	\param	firstInstance The Instance from which to draw from
	\param	instanceCount The number of instances to draw
	*******************************************************************************************************************/
	void drawArrays(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance = 0, uint32 instanceCount = 1);

	/*!*****************************************************************************************************************
	\brief drawArraysIndirect
	\param buffer Buffer containing draw parameters. The parameters of each draw must be encoded in an array of
		   DrawIndirectCmd structures
	\param offset Offset in bytes in the buffer where draw parameters begin
	\param drawCount Number of draws to execute, can be zero. If the the drawCount is greater than 1 then the
		   stride must be multiple of 4.
	\param stride Stride in byte between sets of draw parameters
	********************************************************************************************************************/
	void drawArraysIndirect(api::Buffer& buffer, uint32 offset, uint32 drawCount, uint32 stride);

	/*!****************************************************************************************************************
	\brief	Indirect Draw command. Use buffer to obtain the draw call parameters.
	\param	buffer A buffer containing the draw call parameters in the form of a DrawIndexedIndirectCommand
	*******************************************************************************************************************/
	void drawIndexedIndirect(Buffer& buffer);

	/*!****************************************************************************************************************
	\brief	Update the buffer.
	\param[in]	buffer The buffer to update
	\param[in] offset Offset in the buffer updates begin
	\param[in] length Buffer update length
	\param[in] data New update data.
	*******************************************************************************************************************/
	void updateBuffer(Buffer& buffer, const void* data, pvr::uint32 offset, pvr::uint32 length);


	/*!****************************************************************************************************************
	\brief	Bind a Vertex Buffer for drawing
	\param	buffer The vertex buffer to bind
	\param	offset The offset into the vertex buffer to bind
	\param	bindingIndex The Vertex Buffer index to bind the vertex buffer to
	*******************************************************************************************************************/
	void bindVertexBuffer(const Buffer& buffer, uint32 offset, uint16 bindingIndex);

	/*!****************************************************************************************************************
	\brief	Bind an array of Vertex Buffers
	\param	buffers The array of buffers
	\param	offsets The array of offsets into the vertex buffer, each corresponding to a vertex buffer
	\param	startBinding The binding index that the first buffer will be bound
	\param	numBuffers The number of buffers to bind
	\param	bindingCount The number of bindings
	*******************************************************************************************************************/
	void bindVertexBuffer(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding,
	                      uint16 bindingCount);

	/*!****************************************************************************************************************
	\brief	Bind an index buffer for drawing
	\param	buffer The buffer to bind as an IndexBuffer
	\param	offset The offset into the Index buffer to bind
	\param	indexType the type of indices the buffer contains
	*******************************************************************************************************************/
	void bindIndexBuffer(const api::Buffer& buffer, uint32 offset, types::IndexType indexType);

	/*!****************************************************************************************************************
	\brief	Set the viewport rectangle
	\param	viewport The viewport rectangle
	*******************************************************************************************************************/
	void setViewport(const pvr::Rectanglei& viewport);

	/*!****************************************************************************************************************
	\brief	Set the scissor rectangle
	\param	scissor The scissor rectangle
	*******************************************************************************************************************/
	void setScissor(const pvr::Rectanglei& scissor);

	/*!****************************************************************************************************************
	\brief	Set minimum and maximum depth
	\param	min Minimum depth (default 0.0f)
	\param	max Maximum depth (default 1.0f)
	*******************************************************************************************************************/
	void setDepthBound(pvr::float32 min = 0.0f, pvr::float32 max = 1);

	/*!****************************************************************************************************************
	\brief	Set the stencil comparison mask
	\param	face The face/faces for which to set the stencil mask
	\param	compareMask A uint32 which will mask both the values and the reference before stencil comparisons
	*******************************************************************************************************************/
	void setStencilCompareMask(types::StencilFace face, pvr::uint32 compareMask);

	/*!****************************************************************************************************************
	\brief	Set the stencil write mask
	\param	face The face/faces for which to set the stencil write mask
	\param	writeMask A uint32 which will mask the values when writing the stencil
	*******************************************************************************************************************/
	void setStencilWriteMask(types::StencilFace face, pvr::uint32 writeMask);

	/*!****************************************************************************************************************
	\brief	Set stencil reference value
	\param	face The face/faces for which to set the stencil reference value
	\param	ref The stencil reference value
	*******************************************************************************************************************/
	void setStencilReference(types::StencilFace face, pvr::uint32 ref);

	/*!****************************************************************************************************************
	\brief	This is a dynamic command which controll the offset of depth values of all fragments generated by the rasterization of a polygon.
			NOTE: If depthBiasEnable is set to false in pipelineCreation::RasterStateCreateParam then no depth bias
			is applied and the fragment's depth values are unchanged.
	\param	depthBiasConstantFactor Scalar factor controlling the constant depth value added to each fragment.
	\param	depthBiasClamp  Maximum or minimum depth bias of a fragment
	\param	depthBiasSlopeFactor Scalar factor applied to a fragment's slope in depth bias calculations
	*******************************************************************************************************************/
	void setDepthBias(pvr::float32 depthBiasConstantFactor, pvr::float32 depthBiasClamp, pvr::float32 depthBiasSlopeFactor);

	/*!****************************************************************************************************************
	\brief	Set blend constants for blend operation using constant colors
	\param	rgba Red Green Blue Alpha blend constant
	*******************************************************************************************************************/
	void setBlendConstants(glm::vec4 rgba);

	/*!****************************************************************************************************************
	\brief	Set the line width
	\param lineWidth the new width of lines drawn
	*******************************************************************************************************************/
	void setLineWidth(float32 lineWidth);

	/*!****************************************************************************************************************
	\brief	Copy buffer
	\param src Source buffer to copy from
	\param dest Destination buffer to copy in to
	\param srcOffset Source buffer offset
	\param destOffset Destination buffer offset
	\param sizeInBytes Data size in bytes
	*******************************************************************************************************************/
	void copyBuffer(pvr::api::Buffer src, pvr::api::Buffer dest, pvr::uint32 srcOffset, pvr::uint32 destOffset, pvr::uint32 sizeInBytes);

	/*!****************************************************************************************************************
	\brief	Blit Image
	\param src Source image to blit from
	\param dest Destination image to blit in to
	\param srcLayout layout of the source image subresources for the blit
	\param dstLayout Layout of the destination image subresources for the blit
	\param regions An array of regions to blit
	\param numRegions Number of regions
	\param filter Sampler Filter to apply if the blits require scaling
	*******************************************************************************************************************/
	void blitImage(api::TextureStore& src, api::TextureStore& dest, types::ImageLayout srcLayout, types::ImageLayout dstLayout,
	               types::ImageBlitRange* regions, uint32 numRegions, types::SamplerFilter filter);

	/*!****************************************************************************************************************
	\brief Copy image to a buffer
	\param srcImage Source image to copy from
	\param srcImageLayout Layout of the source image subresources for the copy
	\param dstBuffer Buffer to copy into
	\param regions An array of regions to copy
	\param numRegions Number of regions in the array

	*******************************************************************************************************************/
	void copyImageToBuffer(api::TextureStore& srcImage, types::ImageLayout srcImageLayout,
	                       api::Buffer& dstBuffer, types::BufferImageCopy* regions, uint32 numRegions);

	/*!****************************************************************************************************************
	\brief	This draw command behaves similarly to drawArray except that the parameters are read by the device from a buffer during execution.
	\param buffer Buffer which contains the parameters.
	\param offset Offset in to the buffer where parameters begin
	\param count Number of draws to execute, and can be zero
	\param stride Byte stride between successive sets of draw parameters
	*******************************************************************************************************************/
	void drawIndirect(Buffer& buffer, pvr::uint32 offset, pvr::uint32 count, pvr::uint32 stride);

	/*!****************************************************************************************************************
	\brief	Enqueues a ComputeShader execution using the ComputeShader that is in the currently bound ComputePipeline.
	\param numGroupsX  The number of workgroups enqueued in the X direction.
	\param numGroupsY  The number of workgroups enqueued in the Y direction (default 1).
	\param numGroupsZ  The number of workgroups enqueued in the Z direction (default 1).
	*******************************************************************************************************************/
	void dispatchCompute(uint32 numGroupsX, uint32 numGroupsY = 1, uint32 numGroupsZ = 1);

	/*!****************************************************************************************************************
	\brief	If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value will be
	        copied at the time of calling this function, and will be fixed until set to another value. Usually used it
			is needed to alternate between a few fixed, known in advance, values. Since changing the value would actually
			require re-recording the command buffer, this has limited use.
	\tparam _type The type of item to upload to the uniform. Supported types depend on the underlying API and shader
	        glsl version. Vectors and matrices should be uploaded using the glm::vec/mat types (not float)
	\param location The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	\param val The value. Although passed by const-ref, it will immediately be copied and stored internally
	\description Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
			is limited. See setUniformPtr.
	*******************************************************************************************************************/
	template<typename _type> void setUniform(int32 location, const _type& val);

	/*!****************************************************************************************************************
	\brief	This is the function of choice for updating uniforms (if supported by the underlying API). This function
	        sets a Uniform variable to be updated from a memory location every time the command buffer is submitted, so
			that updating the value in that memory location is properly updated in the shader uniform.
	\tparam _type The type of item to upload to the uniform. Supported types depend on the underlying API and shader
	        glsl version. Vectors and matrices should be uploaded using the glm::vec/mat types (not float)
	\param location The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	\param count The number of values to read from the pointer
	\param ptr A pointer to a location that shall remain fixed as long as the command buffer is used (unless cleared).
	       The value will be read from ptr and the uniform updated every time the command buffer is submitted.
	\description WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer is
		   used, since every time the command buffer is submitted, this memory location will be read. In synchronous
		   underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
		   command buffer or after it returns. Uploading arrays of values is supported.
	*******************************************************************************************************************/
	template<typename _type> void setUniformPtr(int32 location, pvr::uint32 count, const _type* ptr);

	/*!****************************************************************************************************************
	\brief	Store which pipeline is currently bound, so that it can later be retrieved and bound with a popPipeline command.
	*******************************************************************************************************************/
	void pushPipeline();

	/*!****************************************************************************************************************
	\brief	Bind the previously pushed pipeline (See pushGraphicsPipeline)
	*******************************************************************************************************************/
	void popPipeline();

	/*!****************************************************************************************************************
	\brief	INTERNAL. reset the currently bound pipeline.
	*******************************************************************************************************************/
	void resetPipeline();

	/*!****************************************************************************************************************
	\brief	Add a memory barrier to the command stream, forcing preceeding commands to be written before succeeding
	commands are executed.
	*******************************************************************************************************************/
	void pipelineBarrier(types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage,
	                     const MemoryBarrierSet& barriers, bool dependencyByRegion = true);

	void waitForEvent(const Event& evt, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);
	void waitForEvents(const EventSet& evts, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);
	void setEvent(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands);
	void resetEvent(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands);

	/*!*********************************************************************************************************************
	\brief Clear the command queue. It is invalid to clear the command buffer while it is being recorded.
	***********************************************************************************************************************/
	void clear(bool releaseAllResources = false);

	const native::HCommandBuffer_& getNativeObject() const;
	native::HCommandBuffer_& getNativeObject();
=======
	CommandBufferBase_(std::auto_ptr<ICommandBufferImpl_> impl) : pimpl(impl) {}

	/// <summary>Destructor.</summary>
	virtual ~CommandBufferBase_() {}

	/// <summary>Get a reference to the context used by this CommandBuffer.</summary>
	GraphicsContext& getContext() { return pimpl->getContext_(); }

	/// <summary>Call this function when you are done recording commands. BeginRecording must be called first.
	/// </summary>
	void endRecording() { return pimpl->endRecording_(); }

	/// <summary>Queries if a command buffer is in the recording state</summary>
	/// <returns>True if recording, false otherwise</returns>
	bool isRecording() { return pimpl->isRecording_(); }

	/// <summary>Bind a graphics pipeline.</summary>
	/// <param name="pipeline">The GraphicsPipeline to bind.</param>
	void bindPipeline(GraphicsPipeline pipeline)
	{ pimpl->bindPipeline_(pipeline); }

	/// <summary>Bind a compute pipeline</summary>
	/// <param name="pipeline">The ComputePipeline to bind</param>
	void bindPipeline(ComputePipeline& pipeline)
	{ pimpl->bindPipeline_(pipeline); }

	/// <summary>Bind a Scene Traversal pipeline</summary>
	/// <param name="pipeline">The SceneTraversalPipeline to bind</param>
	void bindPipeline(SceneTraversalPipeline& pipeline)
	{ pimpl->bindPipeline_(pipeline); }

	/// <summary>Bind a Vertex Ray pipeline</summary>
	/// <param name="pipeline">The VertexRayPipeline to bind</param>
	void bindPipeline(VertexRayPipeline& pipeline)
	{ pimpl->bindPipeline_(pipeline); }

	/// <summary>Bind a single DescriptorSet to the Graphics Pipeline binding point</summary>
	/// <param name="pipelineLayout">The pipelineLayout that the GraphicsPipeline will have</param>
	/// <param name="index">The index to which to bind the descriptor set</param>
	/// <param name="set">The descriptorSet to bind to the binding point bindingPoint</param>
	/// <param name="dynamicOffsets">A c-style array of unsigned integers, each of which is consecutively applied as
	/// a Dynamic Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.</param>
	/// <param name="numDynamicOffsets">The number of dynamic offsets in <paramref name="dynamicOffsets."/>Must
	/// exactly much the number of Dynamic objects in the <paramref name="set."/></param>
	void bindDescriptorSet(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0)
	{ pimpl->bindDescriptorSet_(pipelineLayout, index, set, dynamicOffsets, numDynamicOffsets); }

	/// <summary>Bind a single DescriptorSet to the Compute Pipeline binding point</summary>
	/// <param name="pipelineLayout">The pipelineLayout that the ComputePipeline will have</param>
	/// <param name="index">The index to which to bind the descriptor set</param>
	/// <param name="set">The descriptorSet to bind to the binding point bindingPoint</param>
	/// <param name="dynamicOffsets">A c-style array of unsigned integers, each of which is consecutively applied as
	/// a Dynamic Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.</param>
	/// <param name="numDynamicOffsets">The number of dynamic offsets in <paramref name="dynamicOffsets."/>Must
	/// exactly much the number of Dynamic objects in the <paramref name="set."/></param>
	void bindDescriptorSetCompute(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0)
	{ pimpl->bindDescriptorSetCompute_(pipelineLayout, index, set, dynamicOffsets, numDynamicOffsets); }

	/// <summary>Bind a single DescriptorSet to the Ray Tracing Pipeline binding point</summary>
	/// <param name="pipelineLayout">The pipelineLayout that the Ray Tracing Pipeline will have</param>
	/// <param name="index">The index to which to bind the descriptor set</param>
	/// <param name="set">The descriptorSet to bind to the binding point bindingPoint</param>
	/// <param name="dynamicOffsets">A c-style array of unsigned integers, each of which is consecutively applied as
	/// a Dynamic Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.</param>
	/// <param name="numDynamicOffsets">The number of dynamic offsets in <paramref name="dynamicOffsets."/>Must
	/// exactly much the number of Dynamic objects in the <paramref name="set."/></param>
	void bindDescriptorSetRayTracing(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0)
	{ pimpl->bindDescriptorSetRayTracing_(pipelineLayout, index, set, dynamicOffsets, numDynamicOffsets); }

	/// <summary>Bind a single DescriptorSet to the Scene Hierarchy Generator Pipeline binding point</summary>
	/// <param name="pipelineLayout">The pipelineLayout that the Scene Hierarchy Generator Pipeline will have</param>
	/// <param name="index">The index to which to bind the descriptor set</param>
	/// <param name="set">The descriptorSet to bind to the binding point bindingPoint</param>
	/// <param name="dynamicOffsets">A c-style array of unsigned integers, each of which is consecutively applied as
	/// a Dynamic Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.</param>
	/// <param name="numDynamicOffsets">The number of dynamic offsets in <paramref name="dynamicOffsets."/>Must
	/// exactly much the number of Dynamic objects in the <paramref name="set."/></param>
	void bindDescriptorSetSHG(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0)
	{ pimpl->bindDescriptorSetSHG_(pipelineLayout, index, set, dynamicOffsets, numDynamicOffsets); }

	/// <summary>Bind multiple DescriptorSets</summary>
	/// <param name="bindingPoint">The index where the first descriptor set will bind to. The rest will be bound
	/// successively.</param>
	/// <param name="pipelineLayout">The pipelineLayout that the GraphicsPipeline will have</param>
	/// <param name="firstSet">The first index to start binding descriptor sets to.</param>
	/// <param name="sets">The array of descriptorSets. The first item in the array will be bound to binding point
	/// <paramref name="firstSet"/>and each one after to the next binding point</param>
	/// <param name="dynamicOffsets">An array of Offsets that will be used when binding items of this descriptor set
	/// respectively</param>
	/// <param name="numDescSets">The number of descriptor sets in the array</param>
	/// <param name="dynamicOffsets">A c-style array of unsigned integers, each of which is consecutively applied as
	/// a Dynamic Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.</param>
	/// <param name="numDynamicOffsets">The number of dynamic offsets in <paramref name="dynamicOffsets."/>Must
	/// exactly much the number of Dynamic objects in the <paramref name="set."/></param>
	void bindDescriptorSets(types::PipelineBindPoint bindingPoint, const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0)
	{ pimpl->bindDescriptorSets_(bindingPoint, pipelineLayout, firstSet, sets, numDescSets, dynamicOffsets, numDynamicOffsets); }

	/// <summary>Clear multiple attachments with separate clear colors and clear rectangle for each. NOTE: This clear operation must be
	/// called inside the render pass</summary>
	/// <param name="attachmentIndices">Current subpass color attachment binding indices to clear.</param>
	/// <param name="clearColors">An array of colors to clear to, each corresponding to an attachment</param>
	/// <param name="attachmentCount">Number of attachments to clear</param>
	/// <param name="rects">An array of rectangles, each corresponding to the clear area of an attachment</param>
	/// <param name="baseArrayLayers">An array of base array layers corresponding to the first layer to be cleared
	/// for the attachment</param>
	/// <param name="layerCounts">An array of layer counts corresponding to number of layers to clear for the
	/// attachment</param>
	/// <param name="rectCount">The number of rectangles to clear (the number of rectangles in
	/// <paramref name="rects"/>)</param>
	void clearColorAttachment(uint32 const* attachmentIndices, glm::vec4 const* clearColors, uint32 attachmentCount, const Rectanglei* rects, const uint32* baseArrayLayers, const uint32* layerCounts, uint32 rectCount)
	{ pimpl->clearColorAttachment_(attachmentIndices, clearColors, attachmentCount, rects, baseArrayLayers, layerCounts, rectCount); }

	/// <summary>Clear a color attachment. NOTE: This clear operation must be
	/// called inside the render pass</summary>
	/// <param name="attachmentIndex">Current subpass color attachment binding index to clear.</param>
	/// <param name="clearColor">The color to which to clear</param>
	/// <param name="rect">The rectangle to clear</param>
	/// <param name="baseArrayLayer">The array layer corresponding to the first layer to be cleared for the attachment
	/// </param>
	/// <param name="layerCount">The number of layers to clear for the attachment</param>
	void clearColorAttachment(uint32 attachmentIndex, glm::vec4 clearColor, const Rectanglei rect, const uint32 baseArrayLayer = 0u, const uint32 layerCount = 1u)
	{ pimpl->clearColorAttachment_(attachmentIndex, clearColor, rect, baseArrayLayer, layerCount); }

	/// <summary>Clear all attachment for a single fbo with a single clear color. NOTE: This clear operation must be
	/// called inside the render pass</summary>
	/// <param name="fbo">The fbo to clear attachments</param>
	/// <param name="clearColor">The clear area</param>
	void clearColorAttachment(api::Fbo fbo, glm::vec4 clearColor)
	{ pimpl->clearColorAttachment_(fbo, clearColor); }

	/// <summary>Clear the depth attachment of an fbo. NOTE: This clear operation must be called inside the render pass
	/// </summary>
	/// <param name="clearRect">The clear area</param>
	/// <param name="depth">The clear value</param>
	void clearDepthAttachment(const Rectanglei& clearRect, float32 depth = 1.f)
	{ pimpl->clearDepthAttachment_(clearRect, depth); }

	/// <summary>Clear the stencil attachment of an fbo. NOTE: This clear operation must be called inside the render
	/// pass</summary>
	/// <param name="clearRect">The clear area</param>
	/// <param name="stencil">The clear value</param>
	void clearStencilAttachment(const Rectanglei& clearRect, int32 stencil = 0)
	{ pimpl->clearStencilAttachment_(clearRect, stencil); }

	/// <summary>Clear the depth stencil attachment</summary>
	/// <param name="clearRect">clear area</param>
	/// <param name="depth">The depth clear value</param>
	/// <param name="stencil">The stencil clear value</param>
	void clearDepthStencilAttachment(const Rectanglei& clearRect, float32 depth = 1.f, int32 stencil = 0)
	{ pimpl->clearDepthStencilAttachment_(clearRect, depth, stencil); }

	/// <summary>Clears the specified color image using the clear color specified. NOTE: This clear operation must be called
	/// outside the render pass</summary>
	/// <param name="image">the image to clear</param>
	/// <param name="layout">The layout of the image</param>
	/// <param name="clearColor">The clear color to use for the clear</param>
	/// <param name="baseMipLevel">base mip levels</param>
	/// <param name="levelCount">level counts</param>
	/// <param name="baseArrayLayer">base array layers</param>
	/// <param name="layerCount">number of layers to clear</param>
	void clearColorImage(api::TextureView& image, glm::vec4 clearColor, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayer = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General)
	{ pimpl->clearColorImage_(image, clearColor, baseMipLevel, levelCount, baseArrayLayer, layerCount, layout); }

	/// <summary>Clears rangeCount sub resource ranges of the specified color image using the clear color specified.
	/// NOTE: This clear operation must be called outside the render pass</summary>
	/// <param name="image">the image to clear</param>
	/// <param name="layout">The layout of the image</param>
	/// <param name="clearColor">The clear color to use for the clear</param>
	/// <param name="baseMipLevel">rangeCount base mip levels</param>
	/// <param name="levelCount">rangeCount level counts</param>
	/// <param name="baseArrayLayers">rangeCount base array layers</param>
	/// <param name="layerCount">rangeCount number of layers to clear</param>
	/// <param name="rangeCount">The number of sub resource ranges to clear</param>
	void clearColorImage(api::TextureView& image, glm::vec4 clearColor, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General)
	{ pimpl->clearColorImage_(image, clearColor, baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount, layout); }

	/// <summary>Clears the specified depth image using the clear depth color specified.
	/// NOTE: This clear operation must be called outside the render pass</summary>
	/// <param name="image">the image to clear</param>
	/// <param name="layout">The layout of the image</param>
	/// <param name="clearDepth">The clear color to use for the clear</param>
	/// <param name="baseMipLevel">base mip levels</param>
	/// <param name="levelCount">level counts</param>
	/// <param name="baseArrayLayer">base array layer</param>
	/// <param name="layerCount">number of layers to clear</param>
	void clearDepthImage(api::TextureView& image, float clearDepth, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayer = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General)
	{ pimpl->clearDepthImage_(image, clearDepth, baseMipLevel, levelCount, baseArrayLayer, layerCount, layout); }

	/// <summary>Clears rangeCount sub resource ranges of the specified depth image using the clear depth color
	/// specified. NOTE This clear operation must be called outside the render pass</summary>
	/// <param name="image">the image to clear</param>
	/// <param name="layout">The layout of the image</param>
	/// <param name="clearDepth">The clear color to use for the clear</param>
	/// <param name="baseMipLevel">rangeCount base mip levels</param>
	/// <param name="levelCount">rangeCount level counts</param>
	/// <param name="baseArrayLayers">rangeCount base array layers</param>
	/// <param name="layerCount">rangeCount number of layers to clear</param>
	/// <param name="rangeCount">The number of sub resource ranges to clear</param>
	void clearDepthImage(api::TextureView& image, float clearDepth, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General)
	{ pimpl->clearDepthImage_(image, clearDepth, baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount, layout); }

	/// <summary>Clears the specified stencil image using the clear stencil color specified.
	/// NOTE This clear operation must be called outside the render pass</summary>
	/// <param name="image">the image to clear</param>
	/// <param name="layout">The layout of the image</param>
	/// <param name="clearStencil">The clear color to use for the clear</param>
	/// <param name="baseMipLevel">base mip levels</param>
	/// <param name="levelCount">level counts</param>
	/// <param name="baseArrayLayers">base array layers</param>
	/// <param name="layerCount">number of layers to clear</param>
	void clearStencilImage(api::TextureView& image, uint32 clearStencil, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayers = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General)
	{ pimpl->clearStencilImage_(image, clearStencil, baseMipLevel, levelCount, baseArrayLayers, layerCount, layout); }

	/// <summary>Clears rangeCount sub resource ranges of the specified stencil image using the clear stencil color
	/// specified. NOTE: This clear operation must be called outside the render pass</summary>
	/// <param name="image">the image to clear</param>
	/// <param name="layout">The layout of the image</param>
	/// <param name="clearStencil">The clear color to use for the clear</param>
	/// <param name="baseMipLevel">rangeCount base mip levels</param>
	/// <param name="levelCount">rangeCount level counts</param>
	/// <param name="baseArrayLayers">rangeCount base array layers</param>
	/// <param name="layerCount">rangeCount number of layers to clear</param>
	/// <param name="rangeCount">The number of sub resource ranges to clear</param>
	void clearStencilImage(api::TextureView& image, uint32 clearStencil, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General)
	{ pimpl->clearStencilImage_(image, clearStencil, baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount, layout); }

	/// <summary>Clears the specified depth stencil image using the clear depth and stencil color specified.
	/// NOTE: This clear operation must be called outside the render pass</summary>
	/// <param name="image">the image to clear</param>
	/// <param name="layout">The layout of the image</param>
	/// <param name="clearDepth">The clear depth color to use for the clear</param>
	/// <param name="clearStencil">The clear color to use for the clear</param>
	/// <param name="baseMipLevel">base mip levels</param>
	/// <param name="levelCount">level counts</param>
	/// <param name="baseArrayLayers">base array layers</param>
	/// <param name="layerCount">number of layers to clear</param>
	void clearDepthStencilImage(api::TextureView& image, float clearDepth, uint32 clearStencil, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayers = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General)
	{ pimpl->clearDepthStencilImage_(image, clearDepth, clearStencil, baseMipLevel, levelCount, baseArrayLayers, layerCount, layout); }

	/// <summary>Clears rangeCount sub resource ranges of the specified depth stencil image using the clear depth and
	/// stencil color specified. NOTE: This clear operation must be called outside the render pass</summary>
	/// <param name="image">the image to clear</param>
	/// <param name="layout">The layout of the image</param>
	/// <param name="clearDepth">The clear depth color to use for the clear</param>
	/// <param name="clearStencil">The clear color to use for the clear</param>
	/// <param name="baseMipLevel">rangeCount base mip levels</param>
	/// <param name="levelCount">rangeCount level counts</param>
	/// <param name="baseArrayLayers">rangeCount base array layers</param>
	/// <param name="layerCount">rangeCount number of layers to clear</param>
	/// <param name="rangeCount">The number of sub resource ranges to clear</param>
	void clearDepthStencilImage(api::TextureView& image, float clearDepth, uint32 clearStencil, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General)
	{ pimpl->clearDepthStencilImage_(image, clearDepth, clearStencil, baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount, layout); }

	/// <summary>Draw command. Use the current state in the command buffer (pipelines, buffers, descriptor sets) to
	/// execute a drawing command. Uses the currently bound IndexBuffer for indexes into the currently bound VBOs.
	/// </summary>
	/// <param name="firstIndex">Offset into the Index buffer to start drawing from</param>
	/// <param name="indexCount">The number of indexes to draw</param>
	/// <param name="vertexOffset">Index into the Vertex Buffer to start drawing from</param>
	/// <param name="firstInstance">The Instance from which to draw from</param>
	/// <param name="instanceCount">The number of instances to draw</param>
	void drawIndexed(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset = 0, uint32 firstInstance = 0, uint32 instanceCount = 1)
	{ pimpl->drawIndexed_(firstIndex, indexCount, vertexOffset, firstInstance, instanceCount); }

	/// <summary>Draw command. Use the current state in the command buffer (pipelines, buffers, descriptor sets) to
	/// execute a drawing command. Does not use an IndexBuffer.</summary>
	/// <param name="firstVertex">The vertex from where to start drawing</param>
	/// <param name="vertexCount">The number of vertices to draw</param>
	/// <param name="firstInstance">The Instance from which to draw from</param>
	/// <param name="instanceCount">The number of instances to draw</param>
	void drawArrays(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance = 0, uint32 instanceCount = 1)
	{ pimpl->drawArrays_(firstVertex, vertexCount, firstInstance, instanceCount); }

	/// <summary>drawArraysIndirect</summary>
	/// <param name="buffer">Buffer containing draw parameters. The parameters of each draw must be encoded in an array
	/// of DrawIndirectCmd structures</param>
	/// <param name="offset">Offset in bytes in the buffer where draw parameters begin</param>
	/// <param name="drawCount">Number of draws to execute, can be zero. If the the drawCount is greater than 1 then the
	/// stride must be multiple of 4.</param>
	/// <param name="stride">Stride in byte between sets of draw parameters</param>
	void drawArraysIndirect(api::Buffer& buffer, uint32 offset, uint32 drawCount, uint32 stride)
	{ pimpl->drawArraysIndirect_(buffer, offset, drawCount, stride); }


	/// <summary>Indirect Draw command. Use buffer to obtain the draw call parameters.</summary>
	/// <param name="buffer">A buffer containing the draw call parameters in the form of a
	/// DrawIndexedIndirectCommand</param>
	void drawIndexedIndirect(Buffer& buffer)
	{ pimpl->drawIndexedIndirect_(buffer); }

	/// <summary>Update the buffer.</summary>
	/// <param name="buffer">The buffer to update</param>
	/// <param name="offset">Offset in the buffer updates begin</param>
	/// <param name="length">Buffer update length</param>
	/// <param name="data">New update data.</param>
	void updateBuffer(Buffer& buffer, const void* data, uint32 offset, uint32 length)
	{ pimpl->updateBuffer_(buffer, data, offset, length); }


	/// <summary>Bind a Vertex Buffer for drawing</summary>
	/// <param name="buffer">The vertex buffer to bind</param>
	/// <param name="offset">The offset into the vertex buffer to bind</param>
	/// <param name="bindingIndex">The Vertex Buffer index to bind the vertex buffer to</param>
	void bindVertexBuffer(const Buffer& buffer, uint32 offset, uint16 bindingIndex)
	{ pimpl->bindVertexBuffer_(buffer, offset, bindingIndex); }

	/// <summary>Bind an array of Vertex Buffers</summary>
	/// <param name="buffers">The array of buffers</param>
	/// <param name="offsets">The array of offsets into the vertex buffer, each corresponding to a vertex buffer
	/// </param>
	/// <param name="startBinding">The binding index that the first buffer will be bound</param>
	/// <param name="numBuffers">The number of buffers to bind</param>
	/// <param name="bindingCount">The number of bindings</param>
	void bindVertexBuffer(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding, uint16 bindingCount)
	{ pimpl->bindVertexBuffer_(buffers, offsets, numBuffers, startBinding, bindingCount); }

	/// <summary>Bind an index buffer for drawing</summary>
	/// <param name="buffer">The buffer to bind as an IndexBuffer</param>
	/// <param name="offset">The offset into the Index buffer to bind</param>
	/// <param name="indexType">the type of indices the buffer contains</param>
	void bindIndexBuffer(const api::Buffer& buffer, uint32 offset, types::IndexType indexType)
	{ pimpl->bindIndexBuffer_(buffer, offset, indexType); }

	/// <summary>Set the viewport rectangle</summary>
	/// <param name="viewport">The viewport rectangle</param>
	void setViewport(const Rectanglei& viewport)
	{ pimpl->setViewport_(viewport); }

	/// <summary>Set the scissor rectangle</summary>
	/// <param name="scissor">The scissor rectangle</param>
	void setScissor(const Rectanglei& scissor)
	{ pimpl->setScissor_(scissor); }

	/// <summary>Set minimum and maximum depth</summary>
	/// <param name="min">Minimum depth (default 0.0f)</param>
	/// <param name="max">Maximum depth (default 1.0f)</param>
	void setDepthBound(float32 min = 0.0f, float32 max = 1)
	{ pimpl->setDepthBound_(min, max); }

	/// <summary>Set the stencil comparison mask</summary>
	/// <param name="face">The face/faces for which to set the stencil mask</param>
	/// <param name="compareMask">A uint32 which will mask both the values and the reference before stencil
	/// comparisons</param>
	void setStencilCompareMask(types::StencilFace face, uint32 compareMask)
	{ pimpl->setStencilCompareMask_(face, compareMask); }

	/// <summary>Set the stencil write mask</summary>
	/// <param name="face">The face/faces for which to set the stencil write mask</param>
	/// <param name="writeMask">A uint32 which will mask the values when writing the stencil</param>
	void setStencilWriteMask(types::StencilFace face, uint32 writeMask)
	{ pimpl->setStencilWriteMask_(face, writeMask); }

	/// <summary>Set stencil reference value</summary>
	/// <param name="face">The face/faces for which to set the stencil reference value</param>
	/// <param name="ref">The stencil reference value</param>
	void setStencilReference(types::StencilFace face, uint32 ref)
	{ pimpl->setStencilReference_(face, ref); }

	/// <summary>This is a dynamic command which controll the offset of depth values of all fragments generated by the
	/// rasterization of a polygon. NOTE: If depthBiasEnable is set to false in pipelineCreation::RasterStateCreateParam
	/// then no depth bias is applied and the fragment's depth values are unchanged.</summary>
	/// <param name="depthBiasConstantFactor">Scalar factor controlling the constant depth value added to each fragment.
	/// </param>
	/// <param name="depthBiasClamp">Maximum or minimum depth bias of a fragment</param>
	/// <param name="depthBiasSlopeFactor">Scalar factor applied to a fragment's slope in depth bias calculations
	/// </param>
	void setDepthBias(float32 depthBiasConstantFactor, float32 depthBiasClamp, float32 depthBiasSlopeFactor)
	{ pimpl->setDepthBias_(depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor); }

	/// <summary>Set blend constants for blend operation using constant colors</summary>
	/// <param name="rgba">Red Green Blue Alpha blend constant</param>
	void setBlendConstants(glm::vec4 rgba)
	{ pimpl->setBlendConstants_(rgba); }

	/// <summary>Set the line width</summary>
	/// <param name="lineWidth">the new width of lines drawn</param>
	void setLineWidth(float32 lineWidth)
	{ pimpl->setLineWidth_(lineWidth); }

	/// <summary>Copy buffer</summary>
	/// <param name="src">Source buffer to copy from</param>
	/// <param name="dest">Destination buffer to copy in to</param>
	/// <param name="srcOffset">Source buffer offset</param>
	/// <param name="destOffset">Destination buffer offset</param>
	/// <param name="sizeInBytes">Data size in bytes</param>
	void copyBuffer(api::Buffer src, api::Buffer dest, uint32 srcOffset, uint32 destOffset, uint32 sizeInBytes)
	{ pimpl->copyBuffer_(src, dest, srcOffset, destOffset, sizeInBytes); }

	/// <summary>Blit Image</summary>
	/// <param name="src">Source image to blit from</param>
	/// <param name="dest">Destination image to blit in to</param>
	/// <param name="srcLayout">layout of the source image subresources for the blit</param>
	/// <param name="dstLayout">Layout of the destination image subresources for the blit</param>
	/// <param name="regions">An array of regions to blit</param>
	/// <param name="numRegions">Number of regions</param>
	/// <param name="filter">Sampler Filter to apply if the blits require scaling</param>
	void blitImage(api::TextureStore& src, api::TextureStore& dest, types::ImageLayout srcLayout, types::ImageLayout dstLayout, types::ImageBlitRange* regions, uint32 numRegions, types::SamplerFilter filter)
	{ pimpl->blitImage_(src, dest, srcLayout, dstLayout, regions, numRegions, filter); }

	/// <summary>Copy image to a buffer</summary>
	/// <param name="srcImage">Source image to copy from</param>
	/// <param name="srcImageLayout">Layout of the source image subresources for the copy</param>
	/// <param name="dstBuffer">Buffer to copy into</param>
	/// <param name="regions">An array of regions to copy</param>
	/// <param name="numRegions">Number of regions in the array</param>
	void copyImageToBuffer(api::TextureStore& srcImage, types::ImageLayout srcImageLayout, api::Buffer& dstBuffer, types::BufferImageCopy* regions, uint32 numRegions)
	{ pimpl->copyImageToBuffer_(srcImage, srcImageLayout, dstBuffer, regions, numRegions); }

	/// <summary>This draw command behaves similarly to drawArray except that the parameters are read by the device
	/// from a buffer during execution.</summary>
	/// <param name="buffer">Buffer which contains the parameters.</param>
	/// <param name="offset">Offset in to the buffer where parameters begin</param>
	/// <param name="count">Number of draws to execute, and can be zero</param>
	/// <param name="stride">Byte stride between successive sets of draw parameters</param>
	void drawIndirect(Buffer& buffer, uint32 offset, uint32 count, uint32 stride)
	{ pimpl->drawIndirect_(buffer, offset, count, stride); }

	/// <summary>Enqueues a ComputeShader execution using the ComputeShader that is in the currently bound ComputePipeline.
	/// </summary>
	/// <param name="numGroupsX">The number of workgroups enqueued in the X direction.</param>
	/// <param name="numGroupsY">The number of workgroups enqueued in the Y direction (default 1).</param>
	/// <param name="numGroupsZ">The number of workgroups enqueued in the Z direction (default 1).</param>
	void dispatchCompute(uint32 numGroupsX, uint32 numGroupsY = 1, uint32 numGroupsZ = 1)
	{ pimpl->dispatchCompute_(numGroupsX, numGroupsY, numGroupsZ); }

	/// <summary>Begins the recording of scene hierarchy generation commands.
	/// </summary>
	/// <param name="sceneHierarchy">The scene hierarchy which will be target of build commands in the command buffer.</param>
	/// <param name="extents">The scene hierarchy extents to use when building the hierarchy.</param>
	void beginSceneHierarchy(const SceneHierarchy& sceneHierarchy, pvr::math::AxisAlignedBox& extents)
	{ pimpl->beginSceneHierarchy_(sceneHierarchy, extents); }

	/// <summary>Ends the recording of scene hierarchy generation commands.</summary>
	void endSceneHierarchy()
	{ pimpl->endSceneHierarchy_(); }

	/// <summary>Merges a number of scene hierarchies into a single merged hierarchy.</summary>
	/// <param name="destinationSceneHierarchy">The scene hierarchy which will be target of the merge.</param>
	/// <param name="extents">The scene hierarchy extents to use when merging the hierarchy.</param>
	/// <param name="sourceSceneHierarchies">The source hierarchies to use in the merge.</param>
	/// <param name="numberOfSourceSceneHierarchies">The number of source hierarchies to use in the merge.</param>
	/// <param name="mergeQuality">The merge quality to use (Ranges between 0 and 1).</param>
	void mergeSceneHierarchies(const SceneHierarchy& destinationSceneHierarchy, pvr::math::AxisAlignedBox& extents, const SceneHierarchy* sourceSceneHierarchies, const pvr::uint32 numberOfSourceSceneHierarchies, const pvr::uint32 mergeQuality)
	{ pimpl->mergeSceneHierarchies_(destinationSceneHierarchy, extents, sourceSceneHierarchies, numberOfSourceSceneHierarchies, mergeQuality); }

	/// <summary>Binds a number of scene hierarchies making it possible to use them in subsequent dispatches.</summary>
	/// <param name="sceneHierarchies">The scene hierarchies to bind.</param>
	/// <param name="firstBinding">The index to use for the first binding.</param>
	/// <param name="numberOfSceneHierarchies">The number of scene hierarchies to bind.</param>
	void bindSceneHierarchies(const SceneHierarchy* sceneHierarchies, pvr::uint32 firstBinding, const pvr::uint32 numberOfSceneHierarchies)
	{ pimpl->bindSceneHierarchies_(sceneHierarchies, firstBinding, numberOfSceneHierarchies); }

	/// <summary>Runs a frame shader for each pixel in the box [x, y, x+width, y+height].</summary>
	/// <param name="xOffset">The offset for the x axis.</param>
	/// <param name="yOffset">The offset for the y axis.</param>
	/// <param name="frameWidth">Specifies the number of pixels in the x axis.</param>
	/// <param name="frameHeight">Specifies the number of pixels in the y axis.</param>
	void dispatchRays(pvr::uint32 xOffset, pvr::uint32 yOffset, pvr::uint32 frameWidth, pvr::uint32 frameHeight)
	{ pimpl->dispatchRays_(xOffset, yOffset, frameWidth, frameHeight); }

	/// <summary>Binds a number of accumulation images making accumulation to them possible in subsequent commands which dispatch rays.</summary>
	/// <param name="startBinding">The initial binding index to use.</param>
	/// <param name="bindingCount">The accumulation image count.</param>
	/// <param name="imageViews">The accumulation images to bind.</param>
	void bindAccumulationImages(pvr::uint32 startBinding, pvr::uint32 bindingCount, const TextureView* imageViews)
	{ pimpl->bindAccumulationImages_(startBinding, bindingCount, imageViews); }

	/// <summary>Appends geometry for use in scene hierarchy building.</summary>
	/// <param name="vertexCount">The number of vertices.</param>
	/// <param name="instanceCount">The number of instances.</param>
	/// <param name="firstVertex">The first vertex.</param>
	/// <param name="firstInstance">The first instance.</param>
	void sceneHierarchyAppend(pvr::uint32 vertexCount, pvr::uint32 instanceCount, pvr::uint32 firstVertex, pvr::uint32 firstInstance)
	{ pimpl->sceneHierarchyAppend_(vertexCount, instanceCount, firstVertex, firstInstance); }

	/// <summary>Appends indexed geometry for use in scene hierarchy building.</summary>
	/// <param name="indexCount">The number of indices.</param>
	/// <param name="instanceCount">The number of instances.</param>
	/// <param name="firstIndex">The first index.</param>
	/// <param name="vertexOffset">The vertex offset.</param>
	/// <param name="firstInstance">The first instance.</param>
	void sceneHierarchyAppendIndexed(pvr::uint32 indexCount, pvr::uint32 instanceCount, pvr::uint32 firstIndex, pvr::uint32 vertexOffset, pvr::uint32 firstInstance)
	{ pimpl->sceneHierarchyAppendIndexed_(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance); }

	/// <summary>Indirectly appends geometry for use in scene hierarchy building.</summary>
	/// <param name="indirectBuffer">The buffer to use as the source of indirect information.</param>
	/// <param name="offset">The offset into the buffer.</param>
	/// <param name="drawCount">The number of draws.</param>
	/// <param name="stride">The stride used in the buffer.</param>
	void sceneHierarchyAppendIndirect(pvr::api::BufferView& indirectBuffer, pvr::uint32 offset, pvr::uint32 drawCount, pvr::uint32 stride)
	{ pimpl->sceneHierarchyAppendIndirect_(indirectBuffer, offset, drawCount, stride); }

	/// <summary>Indirectly appends indexed geometry for use in scene hierarchy building.</summary>
	/// <param name="indirectBuffer">The buffer to use as the source of indirect information.</param>
	/// <param name="offset">The offset into the buffer.</param>
	/// <param name="drawCount">The number of draws.</param>
	/// <param name="stride">The stride used in the buffer.</param>
	void sceneHierarchyAppendIndexedIndirect(pvr::api::BufferView& indirectBuffer, pvr::uint32 offset, pvr::uint32 drawCount, pvr::uint32 stride)
	{ pimpl->sceneHierarchyAppendIndexedIndirect_(indirectBuffer, offset, drawCount, stride); }

	/// <summary>Push shared ray constant data.</summary>
	/// <param name="offset">The offset into the ray constant data.</param>
	/// <param name="size">The size of ray constant data.</param>
	/// <param name="pValues">The data to use as shared ray data.</param>
	void pushSharedRayConstants(uint32 offset, uint32 size, const void* pValues)
	{
		pimpl->pushSharedRayConstants_(offset, size, pValues);
	}

	/// <summary>Set the sizes of rays used in a renderpass.</summary>
	/// <param name="raySizeCount">The number of ray sizes.</param>
	/// <param name="pRaySizes">The size of ray user data.</param>
	void setRaySizes(uint32 raySizeCount, const uint32* pRaySizes)
	{
		pimpl->setRaySizes_(raySizeCount, pRaySizes);
	}

	/// <summary>Sets the ray bounce limit.</summary>
	/// <param name="limit">The maximum number of times any ray may bounce in a renderpass.</param>
	void setRayBounceLimit(uint32 limit)
	{
		pimpl->setRayBounceLimit_(limit);
	}

	// UNIFORMS

	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const uint32& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const uint32* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const int32& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const int32* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const float32& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const float32* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::vec2& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::vec2* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::ivec2& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::ivec2* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::uvec2& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::uvec2* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::vec3& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::vec3* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::ivec3& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::ivec3* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::uvec3& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::uvec3* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::vec4& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::vec4* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::ivec4& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::ivec4* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::uvec4& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::uvec4* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::mat2& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::mat2* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::mat2x3& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::mat2x3* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::mat2x4& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::mat2x4* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::mat3x2& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::mat3x2* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::mat3& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::mat3* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::mat3x4& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::mat3x4* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::mat4x2& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::mat4x2* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::mat4x3& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::mat4x3* ptr) { pimpl->setUniformPtr_(location, count, ptr); }
	/// <summary>If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value
	/// will be copied at the time of calling this function, and will be fixed until set to another value. Usually
	/// used it is needed to alternate between a few fixed, known in advance, values. Since changing the value would
	/// actually require re-recording the command buffer, this has limited use.</summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="val">The value. Although passed by const-ref, it will immediately be copied and stored internally
	/// </param>
	/// <remarks>Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
	/// is limited. See setUniformPtr.</remarks>
	void setUniform(int32 location, const glm::mat4x4& val) { pimpl->setUniform_(location, val); }
	/// <summary>This is the function of choice for updating uniforms (if supported by the underlying API). This
	/// function sets a Uniform variable to be updated from a memory location every time the command buffer is
	/// submitted, so that updating the value in that memory location is properly updated in the shader uniform.
	/// </summary>
	/// <param name="location">The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	/// </param>
	/// <param name="count">The number of values to read from the pointer</param>
	/// <param name="ptr">A pointer to a location that shall remain fixed as long as the command buffer is used
	/// (unless cleared). The value will be read from ptr and the uniform updated every time the command buffer is
	/// submitted.</param>
	/// <remarks>WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer
	/// is used, since every time the command buffer is submitted, this memory location will be read. In synchronous
	/// underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
	/// command buffer or after it returns. Uploading arrays of values is supported.</remarks>
	void setUniformPtr(int32 location, uint32 count, const glm::mat4x4* ptr) { pimpl->setUniformPtr_(location, count, ptr); }


	/// <summary>Store which pipeline is currently bound, so that it can later be retrieved and bound with a popPipeline
	/// command.</summary>
	void pushPipeline()
	{ pimpl->pushPipeline_(); }

	/// <summary>Bind the previously pushed pipeline (See pushGraphicsPipeline)</summary>
	void popPipeline()
	{ pimpl->popPipeline_(); }

	/// <summary>INTERNAL. reset the currently bound pipeline.</summary>
	void resetPipeline()
	{ pimpl->resetPipeline_(); }

	/// <summary>Add a memory barrier to the command stream, forcing preceeding commands to be written before
	/// succeeding commands are executed.</summary>
	void pipelineBarrier(types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers, bool dependencyByRegion = true)
	{ pimpl->pipelineBarrier_(srcStage, dstStage, barriers, dependencyByRegion); }


	void waitForEvent(const Event& evt, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
	{ pimpl->waitForEvent_(evt, srcStage, dstStage, barriers); }
	void waitForEvents(const EventSet& evts, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
	{ pimpl->waitForEvents_(evts, srcStage, dstStage, barriers); }
	void setEvent(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands)
	{ pimpl->setEvent_(evt, pipelineFlags); }
	void resetEvent(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands)
	{ pimpl->resetEvent_(evt, pipelineFlags); }

	/// <summary>Clear the command queue. It is invalid to clear the command buffer while it is being recorded.
	/// </summary>
	void clear(bool releaseAllResources = false)
	{ pimpl->clear_(releaseAllResources); }
>>>>>>> 1776432f... 4.3

#ifdef DEBUG
	void logCommandStackTraces()
	{ pimpl->logCommandStackTraces_(); }
#endif
protected:

	friend class SecondaryCommandBufferPackager;
	std::auto_ptr<ICommandBufferImpl_> pimpl;
};

<<<<<<< HEAD
SET_UNIFORM_DECLARATION(uint32);
SET_UNIFORM_DECLARATION(int32);
SET_UNIFORM_DECLARATION(float32);
SET_UNIFORM_DECLARATION(glm::vec2);
SET_UNIFORM_DECLARATION(glm::ivec2);
SET_UNIFORM_DECLARATION(glm::uvec2);
SET_UNIFORM_DECLARATION(glm::vec3);
SET_UNIFORM_DECLARATION(glm::ivec3);
SET_UNIFORM_DECLARATION(glm::uvec3);
SET_UNIFORM_DECLARATION(glm::vec4);
SET_UNIFORM_DECLARATION(glm::ivec4);
SET_UNIFORM_DECLARATION(glm::uvec4);
SET_UNIFORM_DECLARATION(glm::mat2);
SET_UNIFORM_DECLARATION(glm::mat2x3);
SET_UNIFORM_DECLARATION(glm::mat2x4);
SET_UNIFORM_DECLARATION(glm::mat3x2);
SET_UNIFORM_DECLARATION(glm::mat3);
SET_UNIFORM_DECLARATION(glm::mat3x4);
SET_UNIFORM_DECLARATION(glm::mat4x2);
SET_UNIFORM_DECLARATION(glm::mat4x3);
SET_UNIFORM_DECLARATION(glm::mat4x4);


/*!*********************************************************************************************************************
\brief Contains all the commands and states that need to be submitted to the gpu, including pipeline, texture, and
samplers. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer.

\description Secondary command buffers cannot contain RenderPasses, and cannot be submitted to the GPU.
	SecondaryCommandBuffers can be submitted to the primaryCommandBuffer
-It is invalid to submit commands to a command buffer while it is not being recorded.
-It is invalid to reset a command buffer while it is being recorded.
-It is invalid to submit a command buffer more than once if it is one time submit command buffer
-Draw commands must be between a BeginRenderpass and an EndRenderpass command
***********************************************************************************************************************/
=======



/// <summary>Contains all the commands and states that need to be submitted to the gpu, including pipeline, texture,
/// and samplers. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer.
/// </summary>
/// <remarks>Secondary command buffers cannot contain RenderPasses, and cannot be submitted to the GPU.
/// SecondaryCommandBuffers can be submitted to the primaryCommandBuffer -It is invalid to submit commands to a
/// command buffer while it is not being recorded. -It is invalid to reset a command buffer while it is being
/// recorded. -It is invalid to submit a command buffer more than once if it is one time submit command buffer
/// -Draw commands must be between a BeginRenderpass and an EndRenderpass command</remarks>
>>>>>>> 1776432f... 4.3
class SecondaryCommandBuffer_ : public CommandBufferBase_
{
public:
	SecondaryCommandBuffer_(std::auto_ptr<ICommandBufferImpl_> impl) : CommandBufferBase_(impl) {}

	/// <summary>Call this function before beginning to record commands.</summary>
	void beginRecording()
	{
		pimpl->beginRecording_();
	}

	/// <summary>Call this function before beginning to record commands. If the Fbo is known, prefer the Fbo overload
	/// as it may offer better performance.</summary>
	void beginRecording(const RenderPass& rp, uint32 subPass = 0)
	{ pimpl->beginRecording_(rp, subPass); }

	/// <summary>Call this function before beginning to record commands.</summary>
	void beginRecording(const Fbo& fbo, uint32 subPass = 0)
	{ pimpl->beginRecording_(fbo, subPass); }

private:
	// privated inherited members
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
	template<typename MyClass_> friend class PackagedBindable;
};

/// <summary>Contains all the commands and states that need to be submitted to the gpu, including pipeline, texture,
/// and samplers. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer.
/// </summary>
/// <remarks>Primary command buffers can contain RenderPasses, and can be submitted to the GPU.
/// SecondaryCommandBuffers can be submitted to the primaryCommandBuffer -It is invalid to submit commands to a
/// command buffer while it is not being recorded. -It is invalid to reset a command buffer while it is being
/// recorded. -It is invalid to submit a command buffer more than once if it is one time submit command buffer
/// -Draw commands must be between a BeginRenderpass and an EndRenderpass command</remarks>
class CommandBuffer_ : public CommandBufferBase_
{
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
public:
<<<<<<< HEAD
	/*!****************************************************************************************************************
	\brief	Call this function before beginning to record commands.
	*******************************************************************************************************************/
	void beginRecording();

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(const Semaphore& waitSemaphore, const Semaphore& signalSemaphore, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(SemaphoreSet& waitSemaphores, SemaphoreSet& signalSemaphores, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(Semaphore& waitSemaphore, SemaphoreSet& signalSemaphores, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(SemaphoreSet& waitSemaphores, Semaphore& signalSemaphore, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(Fence& fence);

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit();

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submitEndOfFrame(Semaphore& waitSemaphore);

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submitStartOfFrame(Semaphore& signalSemaphore, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Record commands from the secondary command buffer.
	\param secondaryCmdBuffer Record all commands from a secondary command buffer
	***********************************************************************************************************************/
	void enqueueSecondaryCmds(SecondaryCommandBuffer& secondaryCmdBuffer);

	/*!*********************************************************************************************************************
	\brief Record commands from an array of secondary command buffer
	\param secondaryCmdBuffers A c-style array of SecondaryCommandBuffers
	\param numCmdBuffers The number of SecondaryCommandBuffers in secondaryCmdBuffers
	***********************************************************************************************************************/
	void enqueueSecondaryCmds(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers);

	/*!*********************************************************************************************************************
	\brief Record commands from an  secondary command buffer. Multiple enqueueing mode.
		   This is an optimized version where the user is expected to be enqueueing
	       multiple secondary command buffers, but does not necessarily immediately have them available.
	\param expectedMax The number of command buffers that are expected to be enqueued. This number is only a hint and can
	       be overrun.
	***********************************************************************************************************************/
	void enqueueSecondaryCmds_BeginMultiple(uint32 expectedMax = 255);

	/*!*********************************************************************************************************************
	\brief Collect commands for the multiple enqueueing mode. Must be called after enqueueSecondaryCmds_BeginMultiple.
	\param secondaryCmdBuffers A c-style array of secondaryCmdBuffer
	\param numCmdBuffers The number of command buffers
	***********************************************************************************************************************/
	void enqueueSecondaryCmds_EnqueueMultiple(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers);

	/*!*********************************************************************************************************************
	\brief Submit the commands collected in multiple enqueueing mode.
	\param keepAllocated Attempt to keep any allocated memory for the next enqueueMultiple call.
	***********************************************************************************************************************/
	void enqueueSecondaryCmds_SubmitMultiple(bool keepAllocated = false);

	/*!*********************************************************************************************************************
	\brief Begin a RenderPass, i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.
	\param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	\param renderArea The area of the FBO to write to
	\param inlineFirstSubpass Set to 'true' if the commands of the first subpass will be provided Inline. Set to 'false' if
	the commands of the first subpass will be submitted through a SecondaryCommandBuffer
	\param clearColor If the Color attachment LoadOp is Clear, the color to clear to
	\param clearDepth If the Depth attachment LoadOp is Clear, the depth value to clear to
	\param clearStencil If the Stencil attachment LoadOp is Clear, the stencil value to clear to
	***********************************************************************************************************************/
	void beginRenderPass(
	  api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass,
	  const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*********************************************************************************************************************
	\brief Begin a RenderPass, i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.
	\param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	\param renderArea The area of the FBO to write to
	\param inlineFirstSubpass Set to 'true' if the commands of the first subpass will be provided Inline. Set to 'false' if
	the commands of the first subpass will be submitted through a SecondaryCommandBuffer
	\param clearColors If the Color attachment LoadOp is Clear, the color to clear to each attachment
	\param numClearColors Number of colour attachments
	\param clearDepth If the Depth attachment LoadOp is Clear, the depth value to clear to
	\param clearStencil If the Stencil attachment LoadOp is Clear, the stencil value to clear to
	***********************************************************************************************************************/
	void beginRenderPass(
	  api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors,
	  uint32 numClearColors, float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!
	   \brief Begin a renderPass i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.
	   \param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	   \param inlineFirstSubpass Begin the first subpass commands inline in this commandbuffer if true.
	   \param clearColor If the Color attachment LoadOp is Clear, the color to clear to each attachment
	   \param clearDepth  If the Depth attachment LoadOp is Clear, the depth value to clear to
	   \param clearStencil If the Stencil attachment LoadOp is Clear, the stencil value to clear to
	 */
	void beginRenderPass(
	  api::Fbo& fbo, bool inlineFirstSubpass,
	  const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!
	   \brief Begin a renderPass i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.
	   \param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	   \param inlineFirstSubpass Begin the first subpass commands inline in this commandbuffer if true.
	   \param clearColor If the Color attachment LoadOp is Clear, the color to clear to each attachment
	   \param numClearColor Number of clear colors. The colors are one to one mapped with the fbo attachments
	   \param clearDepth  f the Depth attachment LoadOp is Clear, the depth value to clear to
	   \param clearStencil If the Stencil attachment LoadOp is Clear, the stencil value to clear to
	 */
	void beginRenderPass(
	  api::Fbo& fbo, bool inlineFirstSubpass, const glm::vec4* clearColor,
	  uint32 numClearColor,
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);


	/*!*
	\brief beginRenderPass
	\param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	\param renderPass The render pass object. Renderpass must be compatible with the one the fbo created with
	\param renderArea The area of the FBO to write to
	\param inlineFirstSubpass True if the first supass commands will be in this commandbuffer, false
		   if the commands will be in the secondary commandbuffer
	\param clearColor Color initial clear values. Takes effect only if the renderpass's color load op is clear
	\param clearDepth Depth intial clear value. Takes effect only if the renderpass's depth load op is clear
	\param clearStencil Stencil intial clear value. Takes effect only if the renderpass's stencil load op is clear
	 */
	void beginRenderPass(
	  api::Fbo& fbo, const RenderPass& renderPass, const Rectanglei& renderArea,
	  bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*
	\brief beginRenderPass
	\param fbo
	\param renderPass
	\param renderArea
	\param inlineFirstSubpass
	\param clearColors
	\param numClearColors
	\param clearDepth
	\param clearStencil
	 */
	void beginRenderPass(
	  api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea,
	  bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors,
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*
	\brief beginRenderPass
	\param fbo
	\param renderPass
	\param inlineFirstSubpass
	\param clearColor
	\param clearDepth
	\param clearStencil
	 */
	void beginRenderPass(
	  api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass,
	  const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*
	\brief beginRenderPass
	\param fbo
	\param renderPass
	\param inlineFirstSubpass
	\param clearColor
	\param numClearColor
	\param clearDepth
	\param clearStencil
	 */
	void beginRenderPass(
	  api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass, const glm::vec4* clearColor,
	  uint32 numClearColor, float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*********************************************************************************************************************
	\brief Finish the a renderpass (executes the StoreOp).
	***********************************************************************************************************************/
	void endRenderPass();

	/*!*********************************************************************************************************************
	\brief Begin recording commands for the next surpass in this render pass.
	***********************************************************************************************************************/
	void nextSubPassInline();

	/*!****************************************************************************************************************
	\brief	Record next sub pass commands from a secondary-commandbuffer.
	\param	cmdBuffer The commands in this will be used to record the next subPass.
	*******************************************************************************************************************/
	void nextSubPassSecondaryCmds(SecondaryCommandBuffer& cmdBuffer);
};

}//impl
inline native::HCommandBuffer_& native_cast(pvr::api::impl::CommandBuffer_& object)
{
	return object.getNativeObject();
}
inline const native::HCommandBuffer_& native_cast(const pvr::api::impl::CommandBuffer_& object)
{
	return object.getNativeObject();
}
}//api
}//pvr
#undef SET_UNIFORM_DECLARATION
=======
	CommandBuffer_(std::auto_ptr<ICommandBufferImpl_> impl) : CommandBufferBase_(impl) {}

	/// <summary>Call this function before beginning to record commands.</summary>
	void beginRecording()
	{ pimpl->beginRecording_(); }

	/// <summary>Submit this command buffer to the GPU</summary>
	void submit(const Semaphore& waitSemaphore, const Semaphore& signalSemaphore, const Fence& fence = Fence())
	{ pimpl->submit_(waitSemaphore, signalSemaphore, fence); }

	/// <summary>Submit this command buffer to the GPU</summary>
	void submit(SemaphoreSet& waitSemaphores, SemaphoreSet& signalSemaphores, const Fence& fence = Fence())
	{ pimpl->submit_(waitSemaphores, signalSemaphores, fence); }

	/// <summary>Submit this command buffer to the GPU</summary>
	void submit(Fence& fence)
	{ pimpl->submit_(fence); }

	/// <summary>Submit this command buffer to the GPU</summary>
	void submit()
	{ pimpl->submit_(); }

	/// <summary>Submit this command buffer to the GPU</summary>
	void submitEndOfFrame(Semaphore& waitSemaphore)
	{ pimpl->submitEndOfFrame_(waitSemaphore); }

	/// <summary>Submit this command buffer to the GPU</summary>
	void submitStartOfFrame(Semaphore& signalSemaphore, const Fence& fence = Fence())
	{ pimpl->submitStartOfFrame_(signalSemaphore, fence); }

	/// <summary>Record commands from the secondary command buffer.</summary>
	/// <param name="secondaryCmdBuffer">Record all commands from a secondary command buffer</param>
	void enqueueSecondaryCmds(SecondaryCommandBuffer& secondaryCmdBuffer)
	{ pimpl->enqueueSecondaryCmds_(secondaryCmdBuffer); }

	/// <summary>Record commands from an array of secondary command buffer</summary>
	/// <param name="secondaryCmdBuffers">A c-style array of SecondaryCommandBuffers</param>
	/// <param name="numCmdBuffers">The number of SecondaryCommandBuffers in secondaryCmdBuffers</param>
	void enqueueSecondaryCmds(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers)
	{ pimpl->enqueueSecondaryCmds_(secondaryCmdBuffers, numCmdBuffers); }

	/// <summary>Record commands from an secondary command buffer. Multiple enqueueing mode. This is an optimized version
	/// where the user is expected to be enqueueing multiple secondary command buffers, but does not necessarily
	/// immediately have them available.</summary>
	/// <param name="expectedMax">The number of command buffers that are expected to be enqueued. This number is only
	/// a hint and can be overrun.</param>
	void enqueueSecondaryCmds_BeginMultiple(uint32 expectedMax = 255)
	{ pimpl->enqueueSecondaryCmds_BeginMultiple_(expectedMax); }

	/// <summary>Collect commands for the multiple enqueueing mode. Must be called after enqueueSecondaryCmds_BeginMultiple.
	/// </summary>
	/// <param name="secondaryCmdBuffers">A c-style array of secondaryCmdBuffer</param>
	/// <param name="numCmdBuffers">The number of command buffers</param>
	void enqueueSecondaryCmds_EnqueueMultiple(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers)
	{ pimpl->enqueueSecondaryCmds_EnqueueMultiple_(secondaryCmdBuffers, numCmdBuffers); }

	/// <summary>Submit the commands collected in multiple enqueueing mode.</summary>
	/// <param name="keepAllocated">Attempt to keep any allocated memory for the next enqueueMultiple call.</param>
	void enqueueSecondaryCmds_SubmitMultiple(bool keepAllocated = false)
	{ pimpl->enqueueSecondaryCmds_SubmitMultiple_(keepAllocated); }

	/// <summary>Begin a RenderPass, i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.</summary>
	/// <param name="fbo">The FramebufferObject to draw to. All draw commands will write into this FBO.</param>
	/// <param name="renderArea">The area of the FBO to write to</param>
	/// <param name="inlineFirstSubpass">Set to 'true' if the commands of the first subpass will be provided Inline.
	/// Set to 'false' if the commands of the first subpass will be submitted through a SecondaryCommandBuffer
	/// </param>
	/// <param name="clearColor">If the Color attachment LoadOp is Clear, the color to clear to</param>
	/// <param name="clearDepth">If the Depth attachment LoadOp is Clear, the depth value to clear to</param>
	/// <param name="clearStencil">If the Stencil attachment LoadOp is Clear, the stencil value to clear to</param>
	void beginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0)
	{ pimpl->beginRenderPass_(fbo, renderArea, inlineFirstSubpass, clearColor, clearDepth, clearStencil); }

	/// <summary>Begin a RenderPass, i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.</summary>
	/// <param name="fbo">The FramebufferObject to draw to. All draw commands will write into this FBO.</param>
	/// <param name="renderArea">The area of the FBO to write to</param>
	/// <param name="inlineFirstSubpass">Set to 'true' if the commands of the first subpass will be provided Inline.
	/// Set to 'false' if the commands of the first subpass will be submitted through a SecondaryCommandBuffer
	/// </param>
	/// <param name="clearColors">If the Color attachment LoadOp is Clear, the color to clear to each attachment
	/// </param>
	/// <param name="numClearColors">Number of colour attachments</param>
	/// <param name="clearDepth">If the Depth attachment LoadOp is Clear, the depth value to clear to</param>
	/// <param name="clearStencil">If the Stencil attachment LoadOp is Clear, the stencil value to clear to</param>
	void beginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32 clearDepth = 1.f, uint32 clearStencil = 0)
	{ pimpl->beginRenderPass_(fbo, renderArea, inlineFirstSubpass, clearColors, numClearColors, clearDepth, clearStencil); }

	/// <summary>Begin a renderPass i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.</summary>
	/// <param name="fbo">The FramebufferObject to draw to. All draw commands will write into this FBO.</param>
	/// <param name="inlineFirstSubpass">Begin the first subpass commands inline in this commandbuffer if true.</param>
	/// <param name="clearColor">If the Color attachment LoadOp is Clear, the color to clear to each attachment
	/// </param>
	/// <param name="clearDepth">If the Depth attachment LoadOp is Clear, the depth value to clear to</param>
	/// <param name="clearStencil">If the Stencil attachment LoadOp is Clear, the stencil value to clear to</param>
	void beginRenderPass(api::Fbo& fbo, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0)
	{ pimpl->beginRenderPass_(fbo, inlineFirstSubpass, clearColor, clearDepth, clearStencil); }

	/// <summary>Begin a renderPass i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.</summary>
	/// <param name="fbo">The FramebufferObject to draw to. All draw commands will write into this FBO.</param>
	/// <param name="renderPass">The render pass object. Renderpass must be compatible with the one the fbo created
	/// with</param>
	/// <param name="renderArea">The area of the FBO to write to</param>
	/// <param name="inlineFirstSubpass">True if the first supass commands will be in this commandbuffer, false if the
	/// commands will be in the secondary commandbuffer</param>
	/// <param name="clearColor">Color initial clear values. Takes effect only if the renderpass's color load op is
	/// clear</param>
	/// <param name="clearDepth">Depth intial clear value. Takes effect only if the renderpass's depth load op is clear
	/// </param>
	/// <param name="clearStencil">Stencil intial clear value. Takes effect only if the renderpass's stencil load op is
	/// clear</param>
	void beginRenderPass(api::Fbo& fbo, const RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0)
	{ pimpl->beginRenderPass_(fbo, renderPass, renderArea, inlineFirstSubpass, clearColor, clearDepth, clearStencil); }

	/// <summary>Begin a renderPass i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.</summary>
	/// <param name="fbo">The FramebufferObject to draw to. All draw commands will write into this FBO.</param>
	/// <param name="renderPass">The render pass object. Renderpass must be compatible with the one the fbo created
	/// with</param>
	/// <param name="renderArea">The area of the FBO to write to</param>
	/// <param name="inlineFirstSubpass">True if the first supass commands will be in this commandbuffer, false if the
	/// commands will be in the secondary commandbuffer</param>
	/// <param name="clearColors">If the Color attachment LoadOp is Clear, the color to clear to each attachment
	/// </param>
	/// <param name="numClearColors">Number of colour attachments</param>
	/// <param name="numClearDepthStencil">Number of depth/stencil attachments</param>
	/// <param name="clearDepth">Depth intial clear value. Takes effect only if the renderpass's depth load op is clear
	/// </param>
	/// <param name="clearStencil">Stencil intial clear value. Takes effect only if the renderpass's stencil load op is
	/// clear</param>
	void beginRenderPass(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32* clearDepth, uint32* clearStencil, uint32 numClearDepthStencil)
	{ pimpl->beginRenderPass_(fbo, renderPass, renderArea, inlineFirstSubpass, clearColors, numClearColors, clearDepth, clearStencil, numClearDepthStencil); }

	/// <summary>Begin a renderPass i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.</summary>
	/// <param name="fbo">The FramebufferObject to draw to. All draw commands will write into this FBO.</param>
	/// <param name="renderPass">The render pass object. Renderpass must be compatible with the one the fbo created
	/// with</param>
	/// <param name="inlineFirstSubpass">True if the first supass commands will be in this commandbuffer, false if the
	/// commands will be in the secondary commandbuffer</param>
	/// <param name="clearColor">Color initial clear values. Takes effect only if the renderpass's color load op is
	/// clear</param>
	/// <param name="clearDepth">Depth intial clear value. Takes effect only if the renderpass's depth load op is clear
	/// </param>
	/// <param name="clearStencil">Stencil intial clear value. Takes effect only if the renderpass's stencil load op is
	/// clear</param>
	void beginRenderPass(api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0)
	{ pimpl->beginRenderPass_(fbo, renderPass, inlineFirstSubpass, clearColor, clearDepth, clearStencil); }

	/// <summary>Finish the a renderpass (executes the StoreOp).</summary>
	void endRenderPass()
	{ pimpl->endRenderPass_(); }

	/// <summary>Begin recording commands for the next surpass in this render pass.</summary>
	void nextSubPassInline()
	{ pimpl->nextSubPassInline_(); }

	/// <summary>Record next sub pass commands from a secondary-commandbuffer.</summary>
	/// <param name="cmdBuffer">The commands in this will be used to record the next subPass.</param>
	void nextSubPassSecondaryCmds(SecondaryCommandBuffer& cmdBuffer)
	{ pimpl->nextSubPassSecondaryCmds_(cmdBuffer); }
};

}//impl
}//api
}//pvr
>>>>>>> 1776432f... 4.3
