/*!
\brief Vulkan Implementation of the CommandBuffer class.
\file PVRApi/Vulkan/CommandBufferVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/CommandBuffer.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"

namespace pvr {
namespace api {
//The class definition
namespace impl {
class CommandBufferImplVk_ : public ICommandBufferImpl_, public native::HCommandBuffer_
{
public:
	CommandBufferImplVk_(GraphicsContext context, CommandPool pool, native::HCommandBuffer_& myHandle) :
		ICommandBufferImpl_(context, pool)
	{
		handle = myHandle;
	}
	~CommandBufferImplVk_();

	bool valdidateRecordState()
	{
#ifdef DEBUG
		if (!_isRecording)
		{
			Log("Attempted to submit into the commandBuffer without calling beginRecording first.");
			assertion(false, "You must call beginRecording before starting to submit commands into the commandBuffer.");
			return false;
		}
#endif
		return true;
	}

	types::RenderPassContents _nextSubPassContent;


	// MISCELLANEOUS
	void pushPipeline_();
	void popPipeline_();
	void resetPipeline_();

#ifdef DEBUG
	void logCommandStackTraces_() { debug_assertion(false, "Not implemented for Vulkan"); }
#endif

	// SYNCHRONIZATION
	void pipelineBarrier_(types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers, bool dependencyByRegion = true);
	void waitForEvent_(const Event& evt, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);
	void waitForEvents_(const EventSet& evts, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);
	void setEvent_(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands);
	void resetEvent_(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands);

	// PIPELINES & SETS
	void bindPipeline_(GraphicsPipeline& pipeline);
	void bindPipeline_(ComputePipeline& pipeline);
	void bindPipeline_(VertexRayPipeline& pipeline);
	void bindPipeline_(SceneTraversalPipeline& pipeline);

	void bindDescriptorSet_(const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0)
	{
		bindDescriptorSets_(types::PipelineBindPoint::Graphics, pipelineLayout, firstSet, &set, 1, dynamicOffsets, numDynamicOffsets);
	}

	void bindDescriptorSetCompute_(const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0)
	{
		bindDescriptorSets_(types::PipelineBindPoint::Compute, pipelineLayout, firstSet, &set, 1, dynamicOffsets, numDynamicOffsets);
	}

	void bindDescriptorSetRayTracing_(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0);
	void bindDescriptorSetSHG_(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0);
	void bindDescriptorSets_(types::PipelineBindPoint bindingPoint, const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0);
	void bindVertexBuffer_(const Buffer& buffer, uint32 offset, uint16 bindingIndex);
	void bindVertexBuffer_(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding, uint16 bindingCount);
	void bindIndexBuffer_(const api::Buffer& buffer, uint32 offset, types::IndexType indexType);

	// RECORDING - SUBMITTING
	void beginRecording_();
	void beginRecording_(const RenderPass& rp, uint32 subPass = 0);
	void beginRecording_(const Fbo& fbo, uint32 subPass = 0);

	void endRecording_();
	void clear_(bool releaseAllResources = false);
	void submit_(const Semaphore& waitSemaphore, const Semaphore& signalSemaphore, const Fence& fence = Fence());
	void submit_(SemaphoreSet& waitSemaphores, SemaphoreSet& signalSemaphores, const Fence& fence = Fence());
	void submit_(Fence& fence);
	void submit_();
	void submitEndOfFrame_(Semaphore& waitSemaphore);
	void submitStartOfFrame_(Semaphore& signalSemaphore, const Fence& fence = Fence());

	void enqueueSecondaryCmds_(SecondaryCommandBuffer& secondaryCmdBuffer);
	void enqueueSecondaryCmds_(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers);
	void enqueueSecondaryCmds_BeginMultiple_(uint32 expectedMax = 255);
	void enqueueSecondaryCmds_EnqueueMultiple_(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers);
	void enqueueSecondaryCmds_SubmitMultiple_(bool keepAllocated = false);

	// RENDERPASSES - SUBPASSES
	void beginRenderPass_(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0);
	void beginRenderPass_(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32 clearDepth = 1.f, uint32 clearStencil = 0);
	void beginRenderPass_(api::Fbo& fbo, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0);
	void beginRenderPass_(api::Fbo& fbo, const RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0);
	void beginRenderPass_(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32* clearDepth, uint32* clearStencil, uint32 numClearDepthStencil);
	void beginRenderPass_(api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0);

	void endRenderPass_();
	void nextSubPassInline_();
	void nextSubPassSecondaryCmds_(SecondaryCommandBuffer& cmdBuffer);

	// BUFFERS - IMAGES - TEXTURES
	void updateBuffer_(Buffer& buffer, const void* data, uint32 offset, uint32 length);
	void copyBuffer_(api::Buffer src, api::Buffer dest, uint32 srcOffset, uint32 destOffset, uint32 sizeInBytes);
	void blitImage_(api::TextureStore& src, api::TextureStore& dest, types::ImageLayout srcLayout, types::ImageLayout dstLayout, types::ImageBlitRange* regions, uint32 numRegions, types::SamplerFilter filter);
	void copyImageToBuffer_(api::TextureStore& srcImage, types::ImageLayout srcImageLayout, api::Buffer& dstBuffer, types::BufferImageCopy* regions, uint32 numRegions);

	// DYNAMIC COMMANDS
	void clearColorAttachment_(uint32 const* attachmentIndices, glm::vec4 const* clearColors, uint32 attachmentCount, const Rectanglei* rects, const uint32* baseArrayLayers, const uint32* layerCounts, uint32 rectCount);
	void clearColorAttachment_(uint32 attachmentIndex, glm::vec4 clearColor, const Rectanglei rect, const uint32 baseArrayLayer = 0u, const uint32 layerCount = 1u);
	void clearColorAttachment_(api::Fbo fbo, glm::vec4 clearColor);
	void clearDepthAttachment_(const Rectanglei& clearRect, float32 depth = 1.f);
	void clearStencilAttachment_(const Rectanglei& clearRect, int32 stencil = 0);
	void clearDepthStencilAttachment_(const Rectanglei& clearRect, float32 depth = 1.f, int32 stencil = 0);
	void clearColorImage_(api::TextureView& image, glm::vec4 clearColor, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayer = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General);
	void clearColorImage_(api::TextureView& image, glm::vec4 clearColor, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General);
	void clearDepthImage_(api::TextureView& image, float clearDepth, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayer = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General);
	void clearDepthImage_(api::TextureView& image, float clearDepth, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General);
	void clearStencilImage_(api::TextureView& image, uint32 clearStencil, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayers = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General);
	void clearStencilImage_(api::TextureView& image, uint32 clearStencil, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General);
	void clearDepthStencilImage_(api::TextureView& image, float clearDepth, uint32 clearStencil, const uint32 baseMipLevel = 0u, const uint32 levelCount = 1u, const uint32 baseArrayLayers = 0u, const uint32 layerCount = 1u, types::ImageLayout layout = types::ImageLayout::General);
	void clearDepthStencilImage_(api::TextureView& image, float clearDepth, uint32 clearStencil, const uint32* baseMipLevel, const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount, types::ImageLayout layout = types::ImageLayout::General);
	void setViewport_(const Rectanglei& viewport);
	void setScissor_(const Rectanglei& scissor);
	void setDepthBound_(float32 min = 0.0f, float32 max = 1);
	void setStencilCompareMask_(types::StencilFace face, uint32 compareMask);
	void setStencilWriteMask_(types::StencilFace face, uint32 writeMask);
	void setStencilReference_(types::StencilFace face, uint32 ref);
	void setDepthBias_(float32 depthBiasConstantFactor, float32 depthBiasClamp, float32 depthBiasSlopeFactor);
	void setBlendConstants_(glm::vec4 rgba);
	void setLineWidth_(float32 lineWidth);

	// DRAWING COMMANDS
	void drawIndexed_(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset = 0, uint32 firstInstance = 0, uint32 instanceCount = 1);
	void drawArrays_(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance = 0, uint32 instanceCount = 1);
	void drawArraysIndirect_(api::Buffer& buffer, uint32 offset, uint32 drawCount, uint32 stride);
	void drawIndexedIndirect_(Buffer& buffer);
	void drawIndirect_(Buffer& buffer, uint32 offset, uint32 count, uint32 stride);
	void dispatchCompute_(uint32 numGroupsX, uint32 numGroupsY = 1, uint32 numGroupsZ = 1);

	void beginSceneHierarchy_(const SceneHierarchy& sceneHierarchy, math::AxisAlignedBox& extents);
	void endSceneHierarchy_();
	void mergeSceneHierarchies_(const SceneHierarchy& destinationSceneHierarchy, math::AxisAlignedBox& extents, const SceneHierarchy* sourceSceneHierarchies, const uint32 numberOfSourceSceneHierarchies, const uint32 mergeQuality);
	void bindSceneHierarchies_(const SceneHierarchy* sceneHierarchies, uint32 firstBinding, const uint32 numberOfSceneHierarchies);
	void dispatchRays_(uint32 xOffset, uint32 yOffset, uint32 frameWidth, uint32 frameHeight);
	void bindAccumulationImages_(uint32 startBinding, uint32 bindingCount, const TextureView* imageViews);
	void sceneHierarchyAppend_(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
	void sceneHierarchyAppendIndexed_(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance);
	void sceneHierarchyAppendIndirect_(api::BufferView& indirectBuffer, uint32 offset, uint32 drawCount, uint32 stride);
	void sceneHierarchyAppendIndexedIndirect_(api::BufferView& indirectBuffer, uint32 offset, uint32 drawCount, uint32 stride);
	void pushSharedRayConstants_(uint32 offset, uint32 size, const void* pValues);
	void setRaySizes_(uint32 raySizeCount, const uint32* pRaySizes);
	void setRayBounceLimit_(uint32 limit);

	//UNIFORMS
#define SET_UNIFORM_DECLARATION(_type_)\
  void setUniform_(int32, const _type_& ); \
  void setUniformPtr_(int32, uint32, const _type_*);

	SET_UNIFORM_DECLARATION(float32);
	SET_UNIFORM_DECLARATION(uint32);
	SET_UNIFORM_DECLARATION(int32);
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
	SET_UNIFORM_DECLARATION(glm::mat4);
#undef SET_UNIFORM_DECLARATION

	std::vector<VkCommandBuffer> multiEnqueueCache;
	std::vector<RefCountedResource<void>/**/> objectRefs;
	api::GraphicsPipeline lastBoundGraphicsPipe;
	api::ComputePipeline lastBoundComputePipe;
	api::Fbo lastBoundFbo;
	api::RenderPass lastBoundRenderPass;
private:
	void beginRenderPass__(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32* clearDepth, uint32* clearStencil, uint32 numClearDepthStencil);


};

}
}
}
