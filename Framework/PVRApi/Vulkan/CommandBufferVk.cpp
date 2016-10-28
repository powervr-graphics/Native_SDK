/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/CommandBufferVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        OpenGL ES Implementation details CommandBuffer class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/CommandBuffer.h"
#include "PVRApi/Vulkan/CommandPoolVk.h"
#include "PVRCore/StackTrace.h"
#include "PVRCore/ListOfInterfaces.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRApi/Vulkan/GraphicsPipelineVk.h"
#include "PVRApi/Vulkan/ComputePipelineVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRApi/Vulkan/BufferVk.h"
#include "PVRApi/Vulkan/RenderPassVk.h"
#include "PVRApi/Vulkan/PipelineLayoutVk.h"
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/Vulkan/SyncVk.h"

namespace pvr {
namespace api {
namespace impl {

//Check that there is a list of void pointers in the command buffers (Vulkan and GLES, especially Vulkan) keeping all required objects alive
//Check that the DescriptorSets are keeping alive any objects added to them.

typedef RefCountedWeakReference<vulkan::CommandPoolVk_> CommandPoolVkWeakRef;
class CommandBufferBaseImplementationDetails : public native::HCommandBuffer_
{
public:
	GraphicsContext context;
	CommandPoolVkWeakRef pool;
	bool isRecording;
	std::vector<VkCommandBuffer> multiEnqueueCache;
	std::vector<RefCountedResource<void>/**/> objectRefs;
	api::GraphicsPipeline lastBoundGraphicsPipe;
	api::ComputePipeline lastBoundComputePipe;
	CommandBufferBaseImplementationDetails(const GraphicsContext& context, CommandPool& pool) :
		context(static_cast<vulkan::ContextVkWeakRef>(context)),
		pool(native_cast(pool)->getWeakReference()), isRecording(false)
	{
	}
	bool valdidateRecordState()
	{
#ifdef DEBUG
		if (!isRecording)
		{
			Log("Attempted to submit into the commandBuffer without calling beginRecording first.");
			assertion(false ,  "You must call beginRecording before starting to submit commands into the commandBuffer.");
			return false;
		}
#endif
		return true;
	}

	types::RenderPassContents m_nextSubPassContent;
};

CommandBufferBase_::CommandBufferBase_(GraphicsContext& context, CommandPool& cmdPool, native::HCommandBuffer_& hCmdBuffer)
{
	pImpl.construct(context, cmdPool);
	pImpl->handle = hCmdBuffer;
}

CommandBufferBase_::~CommandBufferBase_()
{
	if (pImpl->context.isValid())
	{
		if (pImpl->handle != VK_NULL_HANDLE)
		{
			if (pImpl->pool.isValid())
			{
				vk::FreeCommandBuffers(native_cast(*pImpl->context).getDevice(), pImpl->pool->handle, 1, &pImpl->handle);
			}
			else
			{
                Log(Log.Debug, "Trying to release a Command buffer AFTER its pool was destroyed");
}
			pImpl->handle = VK_NULL_HANDLE;
		}
	}
	else
	{
		Log(Log.Warning, "WARNING - Command buffer released AFTER its context was destroyed.");
	}
}
GraphicsContext& CommandBufferBase_::getContext() { return pImpl->context; }

void CommandBufferBase_::pipelineBarrier(types::PipelineStageFlags srcStage,
    types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers, bool dependencyByRegion)
{
	vk::CmdPipelineBarrier(pImpl->handle, ConvertToVk::pipelineStage(srcStage), ConvertToVk::pipelineStage(dstStage),
	                       (dependencyByRegion != 0) * VK_DEPENDENCY_BY_REGION_BIT,  barriers.getNativeMemoryBarriersCount(),
	                       (VkMemoryBarrier*)barriers.getNativeMemoryBarriers(), barriers.getNativeBufferBarriersCount(),
	                       (VkBufferMemoryBarrier*)barriers.getNativeBufferBarriers(), barriers.getNativeImageBarriersCount(),
	                       (VkImageMemoryBarrier*)barriers.getNativeImageBarriers());
}

void CommandBufferBase_::waitForEvent(const Event& evt, types::PipelineStageFlags srcStage,
                                      types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
{
	vk::CmdWaitEvents(pImpl->handle, 1, &native_cast(*evt).handle, ConvertToVk::pipelineStage(srcStage),
	                  ConvertToVk::pipelineStage(dstStage), barriers.getNativeMemoryBarriersCount(),
	                  (VkMemoryBarrier*)barriers.getNativeMemoryBarriers(), barriers.getNativeBufferBarriersCount(),
	                  (VkBufferMemoryBarrier*)barriers.getNativeBufferBarriers(), barriers.getNativeImageBarriersCount(),
	                  (VkImageMemoryBarrier*)barriers.getNativeImageBarriers());
}

void CommandBufferBase_::waitForEvents(const EventSet& events, types::PipelineStageFlags srcStage,
                                       types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
{
	vk::CmdWaitEvents(pImpl->handle, events->getNativeEventsCount(), (VkEvent*)events->getNativeEvents(),
	                  ConvertToVk::pipelineStage(srcStage), ConvertToVk::pipelineStage(dstStage),
	                  barriers.getNativeMemoryBarriersCount(), (VkMemoryBarrier*)barriers.getNativeMemoryBarriers(),
	                  barriers.getNativeBufferBarriersCount(), (VkBufferMemoryBarrier*)barriers.getNativeBufferBarriers(),
	                  barriers.getNativeImageBarriersCount(), (VkImageMemoryBarrier*)barriers.getNativeImageBarriers());
}

void CommandBufferBase_::setEvent(Event& evt, types::PipelineStageFlags stage)
{
	pImpl->objectRefs.push_back(evt);
	vk::CmdSetEvent(pImpl->handle, native_cast(*evt), ConvertToVk::pipelineStage(stage));
}

void CommandBufferBase_::resetEvent(Event& evt, types::PipelineStageFlags stage)
{
	vk::CmdResetEvent(pImpl->handle, native_cast(*evt), ConvertToVk::pipelineStage(stage));
}

void CommandBufferBase_::endRecording()
{
	if (!pImpl->isRecording)
	{
		Log("Called CommandBuffer::endRecording while a recording was not in progress. Call CommandBuffer::beginRecording first");
		assertion(0);
	}
	pImpl->isRecording = false;
	vkThrowIfFailed(vk::EndCommandBuffer(pImpl->handle), "CommandBufferBase::endRecording failed");
}

void CommandBufferBase_::bindPipeline(GraphicsPipeline pipeline)
{
	if (!pImpl->lastBoundGraphicsPipe.isValid() || pImpl->lastBoundGraphicsPipe != pipeline)
	{
		pImpl->objectRefs.push_back(pipeline);
		vk::CmdBindPipeline(*pImpl, VK_PIPELINE_BIND_POINT_GRAPHICS, pvr::api::vulkan::native_cast(*pipeline));
		pImpl->lastBoundGraphicsPipe = pipeline;
	}
}

void CommandBufferBase_::blitImage(api::TextureStore& src, api::TextureStore& dst,  types::ImageLayout srcLayout,
                                   types::ImageLayout dstLayout, types::ImageBlitRange* regions, uint32 numRegions,
                                   types::SamplerFilter filter)
{
	pImpl->objectRefs.push_back(src);
	pImpl->objectRefs.push_back(dst);
	std::vector<VkImageBlit> imageBlits(numRegions);
	for (uint32 i = 0; i < numRegions; ++i) {	imageBlits[i] = ConvertToVk::imageBlit(regions[i]); 	}

	vk::CmdBlitImage(*pImpl, src->getNativeObject().image, ConvertToVk::imageLayout(srcLayout),
	                 dst->getNativeObject().image, ConvertToVk::imageLayout(dstLayout), numRegions,
	                 imageBlits.data(), ConvertToVk::samplerFilter(filter));
}

void CommandBufferBase_::copyImageToBuffer(TextureStore& srcImage, types::ImageLayout srcImageLayout,
    Buffer& dstBuffer, types::BufferImageCopy* regions, uint32 numRegions)
{
	// Try to avoid heap allocation
	VkBufferImageCopy regionsArray[10] = {};
	VkBufferImageCopy* pRegions = regionsArray;
	std::vector<VkBufferImageCopy> regionsVec(0);
	if (numRegions > sizeof(ARRAY_SIZE(pRegions)))
{
		regionsVec.resize(numRegions);
		pRegions = regionsVec.data();
	}

	for (uint32 i = 0; i < numRegions; ++i) { pRegions[i] = ConvertToVk::bufferImageCopy(regions[i]); }

	vk::CmdCopyImageToBuffer(*pImpl, srcImage->getNativeObject().image, ConvertToVk::imageLayout(srcImageLayout),
	                         dstBuffer->getNativeObject().buffer, numRegions, pRegions);

}


void CommandBufferBase_::copyBuffer(pvr::api::Buffer src, pvr::api::Buffer dst, pvr::uint32 srcOffset, pvr::uint32 destOffset, pvr::uint32 sizeInBytes)
{
	pImpl->objectRefs.push_back(src);
	pImpl->objectRefs.push_back(dst);
	VkBufferCopy region; region.srcOffset = srcOffset, region.dstOffset = destOffset, region.size = sizeInBytes;
	vk::CmdCopyBuffer(*pImpl, src->getNativeObject().buffer, dst->getNativeObject().buffer, 1, &region);
}


void CommandBufferBase_::bindPipeline(ComputePipeline& pipeline)
{
	if (!pImpl->lastBoundComputePipe.isValid() || pImpl->lastBoundComputePipe != pipeline)
	{
		pImpl->lastBoundComputePipe = pipeline;
		pImpl->objectRefs.push_back(pipeline);
	vk::CmdBindPipeline(*pImpl, VK_PIPELINE_BIND_POINT_COMPUTE, native_cast(*pipeline));
}
}

void CommandBufferBase_::bindDescriptorSet(const api::PipelineLayout& pipelineLayout,
        uint32 firstSet, const DescriptorSet& set, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	pImpl->objectRefs.push_back(set);
	bindDescriptorSets(types::PipelineBindPoint::Graphics, pipelineLayout, firstSet, &set, 1, dynamicOffsets, numDynamicOffset);
}

void CommandBufferBase_::bindDescriptorSetCompute(const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet& set,
        const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	pImpl->objectRefs.push_back(set);
	vk::CmdBindDescriptorSets(pImpl->handle, VK_PIPELINE_BIND_POINT_COMPUTE,
	                          native_cast(*pipelineLayout), firstSet, 1, &(native_cast(*set).handle), numDynamicOffset, dynamicOffsets);
}

void CommandBufferBase_::bindDescriptorSets(types::PipelineBindPoint bindingPoint, const api::PipelineLayout& pipelineLayout,
        uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	assertion(numDescSets < 8);
	if (numDescSets < 8)
	{
		VkDescriptorSet native_sets[8] = { VK_NULL_HANDLE };
		for (uint32 i = 0; i < numDescSets; ++i)
		{
			pImpl->objectRefs.push_back(sets[i]);
			native_sets[i] = native_cast(*sets[i]);
		}
		vk::CmdBindDescriptorSets(pImpl->handle, ConvertToVk::pipelineBindPoint(bindingPoint),
		                          native_cast(*pipelineLayout).handle, firstSet, numDescSets, native_sets, numDynamicOffset, dynamicOffsets);
	}
}

void CommandBufferBase_::updateBuffer(Buffer& buffer, const void* data, uint32 offset, uint32 length)
{
	pImpl->objectRefs.push_back(buffer);
	vk::CmdUpdateBuffer(getNativeObject(), native_cast(*buffer).buffer, offset, length, (const uint32*)data);
}

void CommandBufferBase_::clearColorImage(pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32 baseMipLevel,
    const pvr::uint32 levelCount, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount,
    pvr::types::ImageLayout layout)
{
	pImpl->objectRefs.push_back(image);
	clearColorImage(image, clearColor, &baseMipLevel, &levelCount, &baseArrayLayer, &layerCount, 1u, layout);
}

void CommandBufferBase_::clearColorImage(pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32* baseMipLevel,
    const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount,
    pvr::types::ImageLayout layout)
{
	pImpl->objectRefs.push_back(image);
	assertion(layout == pvr::types::ImageLayout::General || layout == pvr::types::ImageLayout::TransferDstOptimal);

	VkClearColorValue clearColorValue;
	clearColorValue.float32[0] = clearColor.x;
	clearColorValue.float32[1] = clearColor.y;
	clearColorValue.float32[2] = clearColor.z;
	clearColorValue.float32[3] = clearColor.w;

	assertion(rangeCount <= 10);

	VkImageSubresourceRange subResourceRange[10];

	for (uint32 i = 0; i < rangeCount; ++i)
	{
		subResourceRange[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResourceRange[i].baseMipLevel = baseMipLevel[i];
		subResourceRange[i].levelCount = levelCount[i];
		subResourceRange[i].baseArrayLayer = baseArrayLayers[i];
		subResourceRange[i].layerCount = layerCount[i];
	}

	vk::CmdClearColorImage(pImpl->handle, image->getResource()->getNativeObject().image,
	                       ConvertToVk::imageLayout(layout), &clearColorValue, rangeCount, subResourceRange);
}

void clearDepthStencilImageHelper(pvr::native::HCommandBuffer_ nativeCommandBuffer, pvr::api::TextureView& image, pvr::types::ImageLayout layout,
                                  VkImageAspectFlags imageAspect, float clearDepth, pvr::uint32 clearStencil, const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount,
                                  const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount)
{
	assertion(layout == pvr::types::ImageLayout::General || layout == pvr::types::ImageLayout::TransferDstOptimal);

	VkClearDepthStencilValue clearDepthStencilValue;
	clearDepthStencilValue.depth = clearDepth;
	clearDepthStencilValue.stencil = clearStencil;

	VkImageSubresourceRange subResourceRanges[10];

	for (uint32 i = 0; i < rangeCount; ++i)
	{
		subResourceRanges[i].aspectMask = imageAspect;
		subResourceRanges[i].baseMipLevel = baseMipLevel[i];
		subResourceRanges[i].levelCount = levelCount[i];
		subResourceRanges[i].baseArrayLayer = baseArrayLayers[i];
		subResourceRanges[i].layerCount = layerCount[i];
	}

	vk::CmdClearDepthStencilImage(nativeCommandBuffer, image->getResource()->getNativeObject().image,
	                              ConvertToVk::imageLayout(layout), &clearDepthStencilValue, rangeCount, subResourceRanges);
}

void CommandBufferBase_::clearDepthImage(pvr::api::TextureView& image, float clearDepth, const pvr::uint32 baseMipLevel,
    const pvr::uint32 levelCount, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount,
    pvr::types::ImageLayout layout)
{
	pImpl->objectRefs.push_back(image);
	clearDepthStencilImageHelper(pImpl->handle, image, layout, VK_IMAGE_ASPECT_DEPTH_BIT, clearDepth, 0u,
	                             &baseMipLevel, &levelCount, &baseArrayLayer, &layerCount, 1u);
	}

void CommandBufferBase_::clearDepthImage(pvr::api::TextureView& image, float clearDepth, const pvr::uint32* baseMipLevel,
    const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount,
    pvr::uint32 rangeCount, pvr::types::ImageLayout layout)
	{
	pImpl->objectRefs.push_back(image);
	clearDepthStencilImageHelper(pImpl->handle, image, layout, VK_IMAGE_ASPECT_DEPTH_BIT, clearDepth, 0u,
	                             baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount);
}

void CommandBufferBase_::clearStencilImage(pvr::api::TextureView& image, pvr::uint32 clearStencil, const pvr::uint32 baseMipLevel,
    const pvr::uint32 levelCount, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount,
    pvr::types::ImageLayout layout)
		{
	pImpl->objectRefs.push_back(image);
	clearDepthStencilImageHelper(pImpl->handle, image, layout, VK_IMAGE_ASPECT_STENCIL_BIT, 0.0f, clearStencil,
	                             &baseMipLevel, &levelCount, &baseArrayLayer, &layerCount, 1u);
		}

void CommandBufferBase_::clearStencilImage(pvr::api::TextureView& image, pvr::uint32 clearStencil, const pvr::uint32* baseMipLevel,
    const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount,
    pvr::types::ImageLayout layout)
{
	pImpl->objectRefs.push_back(image);
	clearDepthStencilImageHelper(pImpl->handle, image, layout, VK_IMAGE_ASPECT_STENCIL_BIT, 0.0f, clearStencil,
	                             baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount);
	}

void CommandBufferBase_::clearDepthStencilImage(pvr::api::TextureView& image, float clearDepth, pvr::uint32 clearStencil, const pvr::uint32 baseMipLevel,
    const pvr::uint32 levelCount, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount,
    pvr::types::ImageLayout layout)
{
	pImpl->objectRefs.push_back(image);
	clearDepthStencilImageHelper(pImpl->handle, image, layout, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
	                             0.0f, clearStencil, &baseMipLevel, &levelCount, &baseArrayLayer, &layerCount, 1u);
}

void CommandBufferBase_::clearDepthStencilImage(pvr::api::TextureView& image, float clearDepth, pvr::uint32 clearStencil, const pvr::uint32* baseMipLevel,
    const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount,
    pvr::types::ImageLayout layout)
{
	pImpl->objectRefs.push_back(image);
	clearDepthStencilImageHelper(pImpl->handle, image, layout, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0.0f, clearStencil,
	                             baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount);
}

void CommandBufferBase_::clearColorAttachment(pvr::uint32 const* attachmentIndices, glm::vec4 const* clearColors, pvr::uint32 attachmentCount,
    const pvr::Rectanglei* rects, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rectCount)
{
	assertion(attachmentCount <= (uint32)FrameworkCaps::MaxColorAttachments);
	assertion(rectCount <= 10);
	VkClearAttachment clearAttachments[(uint32)FrameworkCaps::MaxColorAttachments];
	VkClearRect clearRectangles[10];
	for (uint32 i = 0; i < attachmentCount; ++i)
	{
		clearAttachments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		memcpy(clearAttachments[i].clearValue.color.float32, glm::value_ptr(clearColors[i]), 4 * 4);
		clearAttachments[i].colorAttachment = attachmentIndices[i];
	}

	for (uint32 i = 0; i < rectCount; ++i)
	{
		clearRectangles[i].baseArrayLayer = baseArrayLayers[i];
		clearRectangles[i].layerCount = layerCount[i];
		clearRectangles[i].rect.offset.x = rects[i].x;
		clearRectangles[i].rect.offset.y = rects[i].y;
		clearRectangles[i].rect.extent.width = rects->getDimension().x;
		clearRectangles[i].rect.extent.height = rects->getDimension().y;
	}

	vk::CmdClearAttachments(pImpl->handle, attachmentCount, clearAttachments, rectCount, clearRectangles);
	}

void CommandBufferBase_::clearColorAttachment(pvr::uint32 attachmentIndex, glm::vec4 clearColor,
    const pvr::Rectanglei rect, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount)
{
	clearColorAttachment(&attachmentIndex, &clearColor, 1u, &rect, &baseArrayLayer, &layerCount, 1u);
}

void CommandBufferBase_::clearColorAttachment(pvr::api::Fbo fbo, glm::vec4 clearColor)
{
	pImpl->objectRefs.push_back(fbo);
    pvr::uint32 attachmentIndices[(uint32)FrameworkCaps::MaxColorAttachments];
	for (uint32 i = 0; i < fbo->getNumColorAttachments(); ++i)
	{
		attachmentIndices[i] = i;
	}

	pvr::Rectanglei rect(0u, 0u, fbo->getDimensions().x, fbo->getDimensions().y);

	const pvr::uint32 baseArrayLayer = 0u;
	const pvr::uint32 layerCount = 1u;

	clearColorAttachment(attachmentIndices, &clearColor, 1u, &rect, &baseArrayLayer, &layerCount, 1u);
	}

void clearDepthStencilAttachmentHelper(pvr::native::HCommandBuffer_ nativeCommandBuffer, const pvr::Rectanglei& clearRect,
                                       VkImageAspectFlags imageAspect, float32 depth, pvr::int32 stencil)
{
	VkClearAttachment clearAttachment = {};
	VkClearRect clearRectangle = {};
	clearAttachment.aspectMask = imageAspect;
	clearAttachment.clearValue.depthStencil.depth = depth;
	clearAttachment.clearValue.depthStencil.stencil = stencil;

	clearRectangle.rect.offset.x = clearRect.x;
	clearRectangle.rect.offset.y = clearRect.y;
	clearRectangle.rect.extent.width = clearRect.width;
	clearRectangle.rect.extent.height = clearRect.height;
	clearRectangle.layerCount = 1;
	vk::CmdClearAttachments(nativeCommandBuffer, 1u, &clearAttachment, 1u, &clearRectangle);
}

void CommandBufferBase_::clearDepthAttachment(const pvr::Rectanglei& clearRect, float32 depth)
{
	clearDepthStencilAttachmentHelper(pImpl->handle, clearRect, VK_IMAGE_ASPECT_DEPTH_BIT, depth, 0u);
}

void CommandBufferBase_::clearStencilAttachment(const pvr::Rectanglei& clearRect, pvr::int32 stencil)
{
	clearDepthStencilAttachmentHelper(pImpl->handle, clearRect, VK_IMAGE_ASPECT_STENCIL_BIT, 0.0f, stencil);
}

void CommandBufferBase_::clearDepthStencilAttachment(const pvr::Rectanglei& clearRect, float32 depth, pvr::int32 stencil)
{
	clearDepthStencilAttachmentHelper(pImpl->handle, clearRect, VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT, depth, stencil);
}

void CommandBufferBase_::drawIndexed(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset, uint32 firstInstance, uint32 instanceCount)
{
	vk::CmdDrawIndexed(pImpl->handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBufferBase_::drawArrays(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance, uint32 instanceCount)
{
	vk::CmdDraw(pImpl->handle, vertexCount, instanceCount, firstVertex, firstInstance);
}


void CommandBufferBase_::drawArraysIndirect(api::Buffer& buffer, uint32 offset, uint32 drawCount, uint32 stride)
{
	vk::CmdDrawIndirect(pImpl->handle, buffer->getNativeObject().buffer, offset, drawCount, stride);
}

void CommandBufferBase_::drawIndexedIndirect(Buffer& buffer)
{
	pImpl->objectRefs.push_back(buffer);
	vk::CmdDrawIndexedIndirect(pImpl->handle, native_cast(buffer)->buffer, 0, 1, 0);
}

void CommandBufferBase_::bindVertexBuffer(const Buffer& buffer, uint32 offset, uint16 bindingIndex)
{
	pImpl->objectRefs.push_back(buffer);
	VkDeviceSize offs = offset;
	vk::CmdBindVertexBuffers(pImpl->handle, bindingIndex, 1, &native_cast(*buffer).buffer, &offs);
}

void CommandBufferBase_::bindVertexBuffer(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding, uint16 bindingCount)
{
	if (numBuffers <= 8)
	{
		pImpl->objectRefs.push_back(buffers[numBuffers]);
		VkBuffer buff[8];
		VkDeviceSize sizes[8];
		for (int i = 0; i < numBuffers; ++i)
		{
			pImpl->objectRefs.push_back(buffers[i]);
			buff[i] = native_cast(*buffers[i]).buffer;
			sizes[i] = offsets[i];
		}
		vk::CmdBindVertexBuffers(pImpl->handle, startBinding, bindingCount, buff, sizes);
	}
	else
	{
		VkBuffer* buff = new VkBuffer[numBuffers];
		VkDeviceSize* sizes = new VkDeviceSize[numBuffers];
		for (int i = 0; i < numBuffers; ++i)
		{
			pImpl->objectRefs.push_back(buffers[i]);
			buff[i] = native_cast(*buffers[i]).buffer;
			sizes[i] = offsets[i];
		}
		vk::CmdBindVertexBuffers(pImpl->handle, startBinding, bindingCount, buff, sizes);
		delete[] buff;
		delete[] sizes;
	}
}

void CommandBufferBase_::bindIndexBuffer(const api::Buffer& buffer, uint32 offset, types::IndexType indexType)
{
	pImpl->objectRefs.push_back(buffer);
	vk::CmdBindIndexBuffer(pImpl->handle, native_cast(*buffer).buffer, offset, indexType == types::IndexType::IndexType16Bit ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void CommandBufferBase_::setViewport(const pvr::Rectanglei& viewport)
{
	VkViewport vp;
	vp.height = (float)viewport.height;
	vp.width = (float)viewport.width;
	vp.x = (float)viewport.x;
	vp.y = (float)viewport.y;
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	vk::CmdSetViewport(pImpl->handle, 0, 1, &vp);
}

void CommandBufferBase_::setScissor(const pvr::Rectanglei& scissor)
{
	VkRect2D sc;
	sc.offset.x = scissor.x;
	sc.offset.y = scissor.y;
	sc.extent.height = scissor.height;
	sc.extent.width = scissor.width;
	vk::CmdSetScissor(pImpl->handle, 0, 1, &sc);
}

void CommandBufferBase_::setDepthBound(pvr::float32 minDepth, pvr::float32 maxDepth)
{
	vk::CmdSetDepthBounds(pImpl->handle, minDepth, maxDepth);
}

void CommandBufferBase_::setStencilCompareMask(types::StencilFace face, pvr::uint32 compareMask)
{
	vk::CmdSetStencilCompareMask(pImpl->handle, (VkStencilFaceFlagBits)face, compareMask);
}

void CommandBufferBase_::setStencilWriteMask(types::StencilFace face, pvr::uint32 writeMask)
{
	vk::CmdSetStencilWriteMask(pImpl->handle, (VkStencilFaceFlagBits)face, writeMask);
}

void CommandBufferBase_::setStencilReference(types::StencilFace face, pvr::uint32 ref)
{
	vk::CmdSetStencilReference(pImpl->handle, (VkStencilFaceFlagBits)face, ref);
}

void CommandBufferBase_::setDepthBias(pvr::float32 depthBias, pvr::float32 depthBiasClamp, pvr::float32 slopeScaledDepthBias)
{
	vk::CmdSetDepthBias(pImpl->handle, depthBias, depthBiasClamp, slopeScaledDepthBias);
}

void CommandBufferBase_::setBlendConstants(glm::vec4 rgba)
{
	vk::CmdSetBlendConstants(pImpl->handle, glm::value_ptr(rgba));
}

void CommandBufferBase_::setLineWidth(float32 lineWidth)
{
	vk::CmdSetLineWidth(pImpl->handle, lineWidth);
}

void CommandBufferBase_::drawIndirect(Buffer& buffer, pvr::uint32 offset, pvr::uint32 count, pvr::uint32 stride)
{
	pImpl->objectRefs.push_back(buffer);
	vk::CmdDrawIndirect(pImpl->handle, native_cast(*buffer).buffer, offset, count, stride);
}

void CommandBufferBase_::dispatchCompute(uint32 numGroupsX, uint32 numGroupsY, uint32 numGroupsZ)
{
	vk::CmdDispatch(pImpl->handle, numGroupsX, numGroupsY, numGroupsZ);
}

bool CommandBufferBase_::isRecording() { return pImpl->isRecording; }

void CommandBufferBase_::clear(bool releaseResources)
{
	pImpl->objectRefs.clear();
	pImpl->lastBoundComputePipe.reset();
	pImpl->lastBoundGraphicsPipe.reset();
	vk::ResetCommandBuffer(pImpl->handle, (releaseResources != 0) * VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

#define SET_UNIFORM_DECLARATION(_type_)\
 template <>void CommandBufferBase_::setUniform<_type_>(int32, const _type_& )\
{ Log(Log.Error, "Free uniforms not supported in Vulkan implementation. Please use a Buffer instead.");  assertion(0); }\
 template <>void CommandBufferBase_::setUniformPtr<_type_>(int32 , pvr::uint32 , const _type_* ) \
{ Log(Log.Error, "Free uniforms not supported in Vulkan implementation. Please use a Buffer instead.");  assertion(0); }

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

void CommandBufferBase_::pushPipeline()
{
	return;
	//"Push/Pop pipeline not supported in vulkan"
}

void CommandBufferBase_::popPipeline()
{
	return;
	//"Push/Pop pipeline not supported/required in vulkan"
	}

void CommandBufferBase_::resetPipeline()
{
	return;
	//"Reset graphics pipeline has no effect on Vulkan underlying API"
}

#ifdef DEBUG
void CommandBufferBase_::logCommandStackTraces()
{
	assertion(false, "Not implemented yet");
}
#endif

void SecondaryCommandBuffer_::beginRecording(const Fbo& fbo, uint32 subPass)
{
	if (pImpl->isRecording)
	{
		Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
		assertion(0);
	}
	clear();
	pImpl->objectRefs.push_back(fbo);
	pImpl->isRecording = true;
	VkCommandBufferBeginInfo info = {};
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = native_cast(*fbo->getRenderPass());
	inheritanceInfo.framebuffer = native_cast(*fbo);
	inheritanceInfo.subpass = subPass;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.queryFlags = 0;
	inheritanceInfo.pipelineStatistics = 0;
	info.pInheritanceInfo = &inheritanceInfo;
	vkThrowIfFailed(vk::BeginCommandBuffer(pImpl->handle, &info), "CommandBufferBase::beginRecording(fbo, [subpass]) failed");
}

void SecondaryCommandBuffer_::beginRecording(const RenderPass& renderPass, uint32 subPass)
{
	if (pImpl->isRecording)
	{
		Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
		assertion(0);
	}
	clear();
	pImpl->objectRefs.push_back(renderPass);
	pImpl->isRecording = true;
	VkCommandBufferBeginInfo info = {};
	VkCommandBufferInheritanceInfo inheritInfo = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritInfo.renderPass = native_cast(*renderPass);
	inheritInfo.subpass = subPass;
	inheritInfo.occlusionQueryEnable = VK_FALSE;
	info.pInheritanceInfo = &inheritInfo;
	vkThrowIfFailed(vk::BeginCommandBuffer(pImpl->handle, &info), "CommandBufferBase::beginRecording(renderpass, [subpass]) failed");
}

SecondaryCommandBuffer_::SecondaryCommandBuffer_(GraphicsContext& context, CommandPool& cmdPool, native::HCommandBuffer_& hBuff) : CommandBufferBase_(context, cmdPool, hBuff) { }

inline static void submit_command_buffers(VkQueue queue, VkDevice device, VkCommandBuffer* cmdBuffs,
    uint32 numCmdBuffs = 1, VkSemaphore* waitSems = NULL, uint32 numWaitSems = 0,
    VkSemaphore* signalSems = NULL, uint32 numSignalSems = 0, VkFence signalFence = VK_NULL_HANDLE)
{
	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkSubmitInfo nfo = {};
	nfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	nfo.waitSemaphoreCount = numWaitSems;
	nfo.pWaitSemaphores = waitSems;
	nfo.pWaitDstStageMask = &pipeStageFlags;
	nfo.pCommandBuffers = cmdBuffs;
	nfo.commandBufferCount = numCmdBuffs;
	nfo.pSignalSemaphores =  signalSems;
	nfo.signalSemaphoreCount = numSignalSems;
	vkThrowIfFailed(vk::QueueSubmit(queue, 1, &nfo, signalFence), "CommandBufferBase::submitCommandBuffers failed");
	}

void CommandBuffer_::beginRecording()
{
	if (pImpl->isRecording)
	{
		Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
		assertion(0);
	}
	clear();
	pImpl->isRecording = true;
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = NULL;
	info.pInheritanceInfo = NULL;
	vkThrowIfFailed(vk::BeginCommandBuffer(pImpl->handle, &info), "CommandBuffer::beginRecording(void) failed");
}

CommandBuffer_::CommandBuffer_(GraphicsContext& context, CommandPool& cmdPool, native::HCommandBuffer_& hCmdBuff) : CommandBufferBase_(context, cmdPool, hCmdBuff)
{ }


void CommandBuffer_::submit(const Semaphore& waitSemaphore, const Semaphore& signalSemaphore, const Fence& fence)
{
	auto handles = getContext()->getPlatformContext().getNativePlatformHandles();
	VkSemaphore* waitSems = NULL;
	VkSemaphore* signalSems = NULL;
	VkFence vkfence = fence.isValid() ? native_cast(*fence).handle : VK_NULL_HANDLE;
	if (waitSemaphore.isValid())
	{
		waitSems = (VkSemaphore*)&*native_cast(*waitSemaphore);
	}
	if (signalSemaphore.isValid())
	{
		signalSems = (VkSemaphore*)&*native_cast(*signalSemaphore);
	}
	submit_command_buffers(handles.graphicsQueue, handles.context.device, &pImpl->handle, 1, waitSems, waitSems != 0, signalSems, signalSems != 0, vkfence);
}

void CommandBuffer_::submit(SemaphoreSet& waitSemaphores, SemaphoreSet& signalSemaphores, const Fence& fence)
{
	auto handles = getContext()->getPlatformContext().getNativePlatformHandles();
	VkSemaphore* waitSems = (VkSemaphore*)(waitSemaphores.isValid() ? waitSemaphores->getNativeSemaphores() : 0);
	uint32 waitSemsSize = (waitSemaphores.isValid() ? waitSemaphores->getNativeSemaphoresCount() : 0);
	VkSemaphore* signalSems = (VkSemaphore*)(signalSemaphores.isValid() ? signalSemaphores->getNativeSemaphores() : 0);
	uint32 signalSemsSize = (signalSemaphores.isValid() ? signalSemaphores->getNativeSemaphoresCount() : 0);
	VkFence vkfence = fence.isValid() ? native_cast(*fence).handle : VK_NULL_HANDLE;

	submit_command_buffers(handles.graphicsQueue, handles.context.device, &pImpl->handle, 1, waitSems, waitSemsSize, signalSems, signalSemsSize, vkfence);
}

void CommandBuffer_::submit(Fence& fence)
{
	auto handles = getContext()->getPlatformContext().getNativePlatformHandles();
	uint32 swapIndex = getContext()->getPlatformContext().getSwapChainIndex();
	VkFence vkfence = (fence.isValid() ? native_cast(*fence).handle : VK_NULL_HANDLE);
	submit_command_buffers(handles.graphicsQueue, handles.context.device, &pImpl->handle, 1, &handles.semaphoreCanBeginRendering[swapIndex],
	                       handles.semaphoreCanBeginRendering[swapIndex] != 0, &handles.semaphoreFinishedRendering[swapIndex],
	                       handles.semaphoreFinishedRendering[swapIndex] != 0, vkfence);
}

void CommandBuffer_::submit()
{
	uint32 swapIndex = getContext()->getPlatformContext().getSwapChainIndex();
	auto handles = getContext()->getPlatformContext().getNativePlatformHandles();
	submit_command_buffers(handles.graphicsQueue, handles.context.device, &pImpl->handle, 1,
	                       &handles.semaphoreCanBeginRendering[swapIndex], handles.semaphoreCanBeginRendering[swapIndex] != 0,
	                       &handles.semaphoreFinishedRendering[swapIndex], handles.semaphoreFinishedRendering[swapIndex] != 0,
	                       handles.fenceRender[swapIndex]);
}

void CommandBuffer_::submitEndOfFrame(Semaphore& waitSemaphore)
{
	auto handles = getContext()->getPlatformContext().getNativePlatformHandles();
	uint32 swapIndex = getContext()->getPlatformContext().getSwapChainIndex();
	VkFence vkfence = handles.fenceRender[swapIndex];
	assertion(waitSemaphore.isValid() && "CommandBuffer_::submitWait Invalid semaphore to wait on");
    VkSemaphore* waitSems = &*native_cast(*waitSemaphore);
	submit_command_buffers(handles.graphicsQueue, handles.context.device, &pImpl->handle, 1,
	                       waitSems, waitSems != 0, &handles.semaphoreFinishedRendering[swapIndex],
	                       handles.semaphoreFinishedRendering[swapIndex] != 0, vkfence);
}

void CommandBuffer_::submitStartOfFrame(Semaphore& signalSemaphore, const Fence& fence)
{
	auto handles = getContext()->getPlatformContext().getNativePlatformHandles();
	uint32 swapIndex = getContext()->getPlatformContext().getSwapChainIndex();
	VkSemaphore* pSignalSemaphore = NULL;
	VkFence vkfence = fence.isValid() ? native_cast(*fence).handle : VK_NULL_HANDLE;
	assertion(signalSemaphore.isValid() && "CommandBuffer_::submitWait Invalid semaphore to wait on");
	pSignalSemaphore = &*native_cast(*signalSemaphore);
	submit_command_buffers(handles.graphicsQueue, handles.context.device, &pImpl->handle, 1,
	                       &handles.semaphoreCanBeginRendering[swapIndex], handles.semaphoreCanBeginRendering[swapIndex] != 0,
	                       pSignalSemaphore, pSignalSemaphore != 0, vkfence);
}



native::HCommandBuffer_& CommandBufferBase_::getNativeObject() { return *pImpl; }
const native::HCommandBuffer_& CommandBufferBase_::getNativeObject() const { return *pImpl; }

void CommandBuffer_::enqueueSecondaryCmds(SecondaryCommandBuffer& secondaryCmdBuffer)
{
	pImpl->objectRefs.push_back(secondaryCmdBuffer);
	assertion(secondaryCmdBuffer.isValid());
	vk::CmdExecuteCommands(pImpl->handle, 1, &native_cast(*secondaryCmdBuffer).handle);
}


void CommandBuffer_::enqueueSecondaryCmds(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffs)
{
	std::vector<VkCommandBuffer> cmdBuffsHeap;
	VkCommandBuffer cmdBuffsStack[32];
	VkCommandBuffer* cmdBuffs = cmdBuffsStack;
	if (numCmdBuffs > 32)
	{
		cmdBuffsHeap.resize(numCmdBuffs);
		cmdBuffs = cmdBuffsHeap.data();
	}
	for (uint32 i = 0; i < numCmdBuffs; ++i)
	{
		pImpl->objectRefs.push_back(secondaryCmdBuffers[i]);
		cmdBuffs[i] = native_cast(*secondaryCmdBuffers[i]).handle;
	}

	vk::CmdExecuteCommands(pImpl->handle, numCmdBuffs, cmdBuffs);
}

void CommandBuffer_::enqueueSecondaryCmds_BeginMultiple(uint32 expectedNumber)
{
	pImpl->multiEnqueueCache.resize(0);
	pImpl->multiEnqueueCache.reserve(expectedNumber);
}

void CommandBuffer_::enqueueSecondaryCmds_EnqueueMultiple(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers)
{
	pImpl->multiEnqueueCache.reserve(pImpl->multiEnqueueCache.size() + numCmdBuffers);
	for (uint32 i = 0; i < numCmdBuffers; ++i)
	{
		pImpl->objectRefs.push_back(secondaryCmdBuffers[i]);
		pImpl->multiEnqueueCache.push_back(native_cast(*secondaryCmdBuffers[i]).handle);
	}
}

void CommandBuffer_::enqueueSecondaryCmds_SubmitMultiple(bool keepAllocated)
{
	vk::CmdExecuteCommands(pImpl->handle, (uint32)pImpl->multiEnqueueCache.size(), pImpl->multiEnqueueCache.data());
	pImpl->multiEnqueueCache.resize(0);
}

inline void copyRectangleToVulkan(const Rectanglei& renderArea, VkRect2D&  vulkanRenderArea)
{
	memcpy(&vulkanRenderArea, &renderArea, sizeof(renderArea));
}

void CommandBuffer_::beginRenderPass(api::Fbo& fbo, bool inlineFirstSubpass,
                                     const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	beginRenderPass(fbo, Rectanglei(glm::ivec2(0, 0), fbo->getDimensions()), inlineFirstSubpass, clearColor, clearDepth, clearStencil);
}

void CommandBuffer_::beginRenderPass(api::Fbo& fbo, bool inlineFirstSubpass, const glm::vec4* clearColors
                                     , uint32 numClearColors, float32 clearDepth, uint32 clearStencil)
{
	beginRenderPass(fbo, Rectanglei(glm::ivec2(0, 0), fbo->getDimensions()), inlineFirstSubpass, clearColors, numClearColors, clearDepth, clearStencil);
}

void CommandBuffer_::beginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass,
                                     const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	beginRenderPass(fbo, fbo->getRenderPass(), renderArea, inlineFirstSubpass, clearColor,
	                clearDepth, clearStencil);
}

void CommandBuffer_::beginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass,
                                     const glm::vec4* clearColor , uint32 numClearColor, float32 clearDepth,
                                     uint32 clearStencil)
{
	beginRenderPass(fbo, fbo->getRenderPass(), renderArea, inlineFirstSubpass, clearColor, numClearColor,
	                clearDepth, clearStencil);
}


void  CommandBuffer_::beginRenderPass(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea,
                                      bool inlineFirstSubpass, const glm::vec4& clearColor,
                                      float32 clearDepth, uint32 clearStencil)
{
	glm::vec4 clearColors[4];
	assertion(fbo->getNumColorAttachments() < ARRAY_SIZE(clearColor));
	for (uint32 i = 0; i < fbo->getNumColorAttachments(); ++i)
	{
		clearColors[i] = clearColor;
	}
	beginRenderPass(fbo, renderPass, renderArea, inlineFirstSubpass, clearColors, fbo->getNumColorAttachments(),
	                clearDepth, clearStencil);
}


void  CommandBuffer_::beginRenderPass(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea,
                                      bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors,
                                      float32 clearDepth, uint32 clearStencil)
{
	pImpl->objectRefs.push_back(fbo);
	VkRenderPassBeginInfo nfo = {};
	nfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	std::vector<VkClearValue> clearValues(numClearColors + 1);
	uint32 i = 0;
	for (; i < numClearColors; ++i)
	{
		memcpy(clearValues[i].color.float32, &clearColors[i], sizeof(float32) * 4);
	}
	clearValues[i].depthStencil.depth = clearDepth;
	clearValues[i].depthStencil.stencil = clearStencil;

	nfo.pClearValues = clearValues.data();
	nfo.clearValueCount = (uint32)clearValues.size();
	nfo.framebuffer = native_cast(*fbo);
	copyRectangleToVulkan(renderArea, nfo.renderArea);
	nfo.renderPass = native_cast(*renderPass);
	vk::CmdBeginRenderPass(pImpl->handle, &nfo, inlineFirstSubpass ?
	                       VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void  CommandBuffer_::beginRenderPass(api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass,
                                      const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	beginRenderPass(fbo, renderPass, Rectanglei(0, 0, fbo->getDimensions().x, fbo->getDimensions().y),
	                inlineFirstSubpass, clearColor, clearDepth, clearStencil);
}

void  CommandBuffer_::beginRenderPass(api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass,
                                      const glm::vec4* clearColor, uint32 numClearColor, float32 clearDepth,
                                      uint32 clearStencil)
{
	beginRenderPass(fbo, renderPass, Rectanglei(0, 0, fbo->getDimensions().x, fbo->getDimensions().y),
	                inlineFirstSubpass, clearColor, numClearColor, clearDepth, clearStencil);
}


void CommandBuffer_::endRenderPass()
{
	vk::CmdEndRenderPass(pImpl->handle);
}

void CommandBuffer_::nextSubPassInline()
{
	vk::CmdNextSubpass(pImpl->handle, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer_::nextSubPassSecondaryCmds(pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	vk::CmdNextSubpass(pImpl->handle, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	enqueueSecondaryCmds(cmdBuffer);
}

}//impl
inline native::HCommandBuffer_& native_cast(pvr::api::impl::CommandBufferBase_& object) { return object.getNativeObject(); }
inline const native::HCommandBuffer_& native_cast(const pvr::api::impl::CommandBufferBase_& object) { return object.getNativeObject(); }
}//api
}//pvr
//!\endcond
