/*!*********************************************************************************************************************
\file         PVRApi/OGLES/CommandBufferVk.cpp
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
#include "PVRApi/ApiObjects/ComputePipeline.h"
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

typedef RefCountedWeakReference<vulkan::CommandPoolVk_> CommandPoolVkWeakRef;
class CommandBufferBaseImplementationDetails : public native::HCommandBuffer_
{
public:
	GraphicsContext context;
	CommandPoolVkWeakRef pool;
	bool isRecording;
	std::vector<VkCommandBuffer> multiEnqueueCache;

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

	types::RenderPassContents::Enum m_nextSubPassContent;
};

CommandBufferBase_::CommandBufferBase_(GraphicsContext& context, CommandPool& cmdPool, native::HCommandBuffer_& hCmdBuffer)
{
	pImpl.construct(context, cmdPool);

	platform::ContextVk& vkctx = native_cast(*context);
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
				Log(Log.Debug, "Command buffer released AFTER its pool was destroyed");
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

void CommandBufferBase_::pipelineBarrier(types::PipelineStageFlags::Bits srcStage, types::PipelineStageFlags::Bits dstStage, const MemoryBarrierSet& barriers, bool dependencyByRegion)
{
	vk::CmdPipelineBarrier(pImpl->handle, ConvertToVk::pipelineStage(srcStage), ConvertToVk::pipelineStage(dstStage), (dependencyByRegion != 0) * VK_DEPENDENCY_BY_REGION_BIT,
	                       barriers.getNativeMemoryBarriersCount(), (VkMemoryBarrier*)barriers.getNativeMemoryBarriers(),
	                       barriers.getNativeBufferBarriersCount(), (VkBufferMemoryBarrier*)barriers.getNativeBufferBarriers(),
	                       barriers.getNativeImageBarriersCount(), (VkImageMemoryBarrier*)barriers.getNativeImageBarriers());
}

void CommandBufferBase_::waitForEvent(const Event& evt, types::PipelineStageFlags::Bits srcStage, types::PipelineStageFlags::Bits dstStage, const MemoryBarrierSet& barriers)
{
	vk::CmdWaitEvents(pImpl->handle, 1, &native_cast(*evt).handle, ConvertToVk::pipelineStage(srcStage), ConvertToVk::pipelineStage(dstStage),
	                  barriers.getNativeMemoryBarriersCount(), (VkMemoryBarrier*)barriers.getNativeMemoryBarriers(),
	                  barriers.getNativeBufferBarriersCount(), (VkBufferMemoryBarrier*)barriers.getNativeBufferBarriers(),
	                  barriers.getNativeImageBarriersCount(), (VkImageMemoryBarrier*)barriers.getNativeImageBarriers());
}

void CommandBufferBase_::waitForEvents(const EventSet& events, types::PipelineStageFlags::Bits srcStage, types::PipelineStageFlags::Bits dstStage, const MemoryBarrierSet& barriers)
{
	vk::CmdWaitEvents(pImpl->handle, events->getNativeEventsCount(), (VkEvent*)events->getNativeEvents(), ConvertToVk::pipelineStage(srcStage), ConvertToVk::pipelineStage(dstStage),
	                  barriers.getNativeMemoryBarriersCount(), (VkMemoryBarrier*)barriers.getNativeMemoryBarriers(),
	                  barriers.getNativeBufferBarriersCount(), (VkBufferMemoryBarrier*)barriers.getNativeBufferBarriers(),
	                  barriers.getNativeImageBarriersCount(), (VkImageMemoryBarrier*)barriers.getNativeImageBarriers());
}

void CommandBufferBase_::setEvent(Event& evt, types::PipelineStageFlags::Bits stage)
{
	vk::CmdSetEvent(pImpl->handle, native_cast(*evt), ConvertToVk::pipelineStage(stage));
}

void CommandBufferBase_::resetEvent(Event& evt, types::PipelineStageFlags::Bits stage)
{
	vk::CmdResetEvent(pImpl->handle, native_cast(*evt), ConvertToVk::pipelineStage(stage));
}

void CommandBufferBase_::endRecording()
{
	if (!pImpl->isRecording)
	{
		Log("Called CommandBuffer::endRecording while a recording was already in progress. Call CommandBuffer::beginRecording first");
		assertion(0);
	}
	pImpl->isRecording = false;
	vkThrowIfFailed(vk::EndCommandBuffer(pImpl->handle), "CommandBufferBase::endRecording failed");
}

void CommandBufferBase_::bindPipeline(GraphicsPipeline& pipeline)
{
	vk::CmdBindPipeline(*pImpl, VK_PIPELINE_BIND_POINT_GRAPHICS, native_cast(*pipeline));
}

void CommandBufferBase_::bindPipeline(ParentableGraphicsPipeline& pipeline)
{
	vk::CmdBindPipeline(*pImpl, VK_PIPELINE_BIND_POINT_GRAPHICS, native_cast(*pipeline));
}

void CommandBufferBase_::bindPipeline(ComputePipeline& pipeline)
{
	vk::CmdBindPipeline(*pImpl, VK_PIPELINE_BIND_POINT_GRAPHICS, native_cast(*pipeline));
}

void CommandBufferBase_::bindDescriptorSet(const api::PipelineLayout& pipelineLayout,
    uint32 firstSet, const DescriptorSet& set, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	bindDescriptorSets(types::PipelineBindPoint::Graphics, pipelineLayout, firstSet, &set, 1, dynamicOffsets, numDynamicOffset);
}

void CommandBufferBase_::bindDescriptorSetCompute(const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet& set,
    const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	vk::CmdBindDescriptorSets(pImpl->handle, VK_PIPELINE_BIND_POINT_COMPUTE, native_cast(*pipelineLayout), firstSet, 1, &(native_cast(*set).handle), numDynamicOffset, dynamicOffsets);
}

void CommandBufferBase_::bindDescriptorSets(types::PipelineBindPoint::Enum bindingPoint, const api::PipelineLayout& pipelineLayout,
    uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	if (numDescSets < 10)
	{
		VkDescriptorSet native_sets[10] = { VK_NULL_HANDLE };
		for (uint32 i = 0; i < numDescSets; ++i)
		{
			native_sets[i] = native_cast(*sets[i]);
		}
		vk::CmdBindDescriptorSets(pImpl->handle, ConvertToVk::pipelineBindPoint(bindingPoint), native_cast(*pipelineLayout).handle, firstSet, numDescSets, native_sets, numDynamicOffset, dynamicOffsets);
	}
	else
	{
		std::vector<VkDescriptorSet> native_sets;
		native_sets.resize(numDescSets);
		for (uint32 i = 0; i < numDescSets; ++i)
		{
			native_sets[i] = native_cast(*sets[i]);
		}
		vk::CmdBindDescriptorSets(pImpl->handle, ConvertToVk::pipelineBindPoint(bindingPoint), native_cast(*pipelineLayout), firstSet, numDescSets, native_sets.data(), numDynamicOffset, dynamicOffsets);
	}
}

void CommandBufferBase_::updateBuffer(Buffer& buffer, const void* data, uint32 offset, uint32 length)
{
	vk::CmdUpdateBuffer(getNativeObject(), native_cast(*buffer).buffer, offset, length, (const uint32*)data);
}

void CommandBufferBase_::clearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 const* clearColors, const pvr::Rectanglei* rects)
{
	assertion(attachmentCount <= 8);
	VkClearAttachment att[8];
	VkClearRect rec[8];
	for (uint32 i = 0; i < attachmentCount; ++i)
	{
		att[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		memcpy(att[i].clearValue.color.float32, glm::value_ptr(clearColors[i]), 4 * 4);
		att[i].colorAttachment = i;
		rec[i].baseArrayLayer = 0;
		rec[i].layerCount = 1;
		rec[i].rect.offset.x = rects[i].x;
		rec[i].rect.offset.y = rects[i].y;
		rec[i].rect.extent.width = rects[i].width;
		rec[i].rect.extent.height = rects[i].height;
	}

	vk::CmdClearAttachments(pImpl->handle, attachmentCount, att, attachmentCount, rec);
}

void CommandBufferBase_::clearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 clearColor, const pvr::Rectanglei rect)
{
	assertion(attachmentCount <= 8);
	VkClearAttachment att[8];
	VkClearRect rec[8];
	for (uint32 i = 0; i < attachmentCount; ++i)
	{
		att[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		memcpy(att[i].clearValue.color.float32, glm::value_ptr(clearColor), 4 * 4);
		att[i].colorAttachment = i;
		rec[i].baseArrayLayer = 0;
		rec[i].layerCount = 1;
		rec[i].rect.offset.x = rect.x;
		rec[i].rect.offset.y = rect.y;
		rec[i].rect.extent.width = rect.width;
		rec[i].rect.extent.height = rect.height;
	}
	vk::CmdClearAttachments(pImpl->handle, attachmentCount, att, attachmentCount, rec);
}

void CommandBufferBase_::clearDepthAttachment(const pvr::Rectanglei& clearRect, float32 depth)
{
	VkClearAttachment att;
	VkClearRect rec;
	att.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	att.clearValue.depthStencil.depth = depth;
	rec.rect.offset.x = clearRect.x;
	rec.rect.offset.y = clearRect.y;
	rec.rect.extent.width = clearRect.width;
	rec.rect.extent.height = clearRect.height;
	vk::CmdClearAttachments(pImpl->handle, 1, &att, 1, &rec);
}

void CommandBufferBase_::clearStencilAttachment(const pvr::Rectanglei& clearRect, pvr::int32 stencil)
{
	VkClearAttachment att;
	VkClearRect rec;
	att.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
	att.clearValue.depthStencil.stencil = stencil;
	rec.rect.offset.x = clearRect.x;
	rec.rect.offset.y = clearRect.y;
	rec.rect.extent.width = clearRect.width;
	rec.rect.extent.height = clearRect.height;
	vk::CmdClearAttachments(pImpl->handle, 1, &att, 1, &rec);
}

void CommandBufferBase_::clearDepthStencilAttachment(const pvr::Rectanglei& clearRect, float32 depth, pvr::int32 stencil)
{
	VkClearAttachment att;
	VkClearRect rec;
	att.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
	att.clearValue.depthStencil.stencil = stencil;
	att.clearValue.depthStencil.depth = depth;
	rec.rect.offset.x = clearRect.x;
	rec.rect.offset.y = clearRect.y;
	rec.rect.extent.width = clearRect.width;
	rec.rect.extent.height = clearRect.height;
	vk::CmdClearAttachments(pImpl->handle, 1, &att, 1, &rec);
}

void CommandBufferBase_::drawIndexed(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset, uint32 firstInstance, uint32 instanceCount)
{
	vk::CmdDrawIndexed(pImpl->handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBufferBase_::drawArrays(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance, uint32 instanceCount)
{
	vk::CmdDraw(pImpl->handle, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBufferBase_::drawIndexedIndirect(Buffer& buffer)
{
	vk::CmdDrawIndexedIndirect(pImpl->handle, native_cast(buffer)->buffer, 0, 1, 0);
}

void CommandBufferBase_::bindVertexBuffer(const Buffer& buffer, uint32 offset, uint16 bindingIndex)
{
	VkDeviceSize offs = offset;
	vk::CmdBindVertexBuffers(pImpl->handle, bindingIndex, 1, &native_cast(*buffer).buffer, &offs);
}

void CommandBufferBase_::bindVertexBuffer(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding, uint16 bindingCount)
{
	if (numBuffers <= 8)
	{
		VkBuffer buff[8];
		VkDeviceSize sizes[8];
		for (int i = 0; i < numBuffers; ++i)
		{
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
			buff[i] = native_cast(*buffers[i]).buffer;
			sizes[i] = offsets[i];
		}
		vk::CmdBindVertexBuffers(pImpl->handle, startBinding, bindingCount, buff, sizes);
		delete buff;
		delete sizes;
	}
}

void CommandBufferBase_::bindIndexBuffer(const api::Buffer& buffer, uint32 offset, types::IndexType::Enum indexType)
{
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

void CommandBufferBase_::setStencilCompareMask(types::StencilFace::Enum face, pvr::uint32 compareMask)
{
	vk::CmdSetStencilCompareMask(pImpl->handle, (VkStencilFaceFlagBits)face, compareMask);
}

void CommandBufferBase_::setStencilWriteMask(types::StencilFace::Enum face, pvr::uint32 writeMask)
{
	vk::CmdSetStencilWriteMask(pImpl->handle, (VkStencilFaceFlagBits)face, writeMask);
}

void CommandBufferBase_::setStencilReference(types::StencilFace::Enum face, pvr::uint32 ref)
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
	vk::CmdDrawIndirect(pImpl->handle, native_cast(*buffer).buffer, offset, count, stride);
}

void CommandBufferBase_::dispatchCompute(uint32 numGroupsX, uint32 numGroupsY, uint32 numGroupsZ)
{
	vk::CmdDispatch(pImpl->handle, numGroupsX, numGroupsY, numGroupsZ);
}

bool CommandBufferBase_::isRecording() { return pImpl->isRecording; }

void CommandBufferBase_::clear(bool releaseResources)
{
	vk::ResetCommandBuffer(pImpl->handle, (releaseResources != 0) * VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

#define SET_UNIFORM_DECLARATION(_type_)\
 template <>void CommandBufferBase_::setUniform<_type_>(int32 location, const _type_& val)\
{ Log(Log.Error, "Free uniforms not supported in Vulkan implementation. Please use a Buffer instead.");  assertion(0); }\
 template <>void CommandBufferBase_::setUniformPtr<_type_>(int32 location, pvr::uint32 count, const _type_* ptr) \
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
SET_UNIFORM_DECLARATION(glm::mat3);
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

//void SecondaryCommandBuffer_::beginRecording()
//{
//	if (pImpl->isRecording)
//	{
//		Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
//		assertion(0);
//	}
//	clear();
//	pImpl->isRecording = true;
//	VkCommandBufferBeginInfo info;
//	VkCommandBufferInheritanceInfo inheritInfo;
//	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	info.pNext = NULL;
//	info.flags = 0;
//	inheritInfo.pNext = NULL;
//	inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
//	inheritInfo.renderPass = VK_NULL_HANDLE;
//	inheritInfo.framebuffer = VK_NULL_HANDLE;
//	inheritInfo.subpass = 0;
//	inheritInfo.occlusionQueryEnable = VK_FALSE;
//	inheritInfo.queryFlags = 0;
//	inheritInfo.pipelineStatistics = 0;
//	info.pInheritanceInfo = &inheritInfo;
//	vkThrowIfFailed(vk::BeginCommandBuffer(pImpl->handle, &info), "CommandBufferBase::beginRecording(void) failed");
//}

void SecondaryCommandBuffer_::beginRecording(const Fbo& fbo, uint32 subPass)
{
	if (pImpl->isRecording)
	{
		Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
		assertion(0);
	}
	clear();
	pImpl->isRecording = true;
	VkCommandBufferBeginInfo info;
	VkCommandBufferInheritanceInfo inheritanceInfo;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = NULL;
	info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	inheritanceInfo.pNext = NULL;
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
	pImpl->isRecording = true;
	VkCommandBufferBeginInfo info;
	VkCommandBufferInheritanceInfo inheritInfo;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = NULL;
	info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	inheritInfo.pNext = NULL;
	inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritInfo.renderPass = native_cast(*renderPass);
	inheritInfo.framebuffer = VK_NULL_HANDLE;
	inheritInfo.subpass = subPass;
	inheritInfo.occlusionQueryEnable = VK_FALSE;
	inheritInfo.queryFlags = 0;
	inheritInfo.pipelineStatistics = 0;
	info.pInheritanceInfo = &inheritInfo;
	vkThrowIfFailed(vk::BeginCommandBuffer(pImpl->handle, &info), "CommandBufferBase::beginRecording(renderpass, [subpass]) failed");
}

SecondaryCommandBuffer_::SecondaryCommandBuffer_(GraphicsContext& context, CommandPool& cmdPool, native::HCommandBuffer_& hBuff) : CommandBufferBase_(context, cmdPool, hBuff) { }

inline static void submit_command_buffers(VkQueue queue, VkDevice device, VkCommandBuffer* cmdBuffs, uint32 numCmdBuffs = 1, VkSemaphore* waitSems = NULL, uint32 numWaitSems = 0,
    VkSemaphore* signalSems = NULL, uint32 numSignalSems = 0, VkFence fence = VK_NULL_HANDLE)
{
	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkSubmitInfo nfo;
	nfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	nfo.pNext = 0;
	nfo.waitSemaphoreCount = numWaitSems;
	nfo.pWaitSemaphores = waitSems;
	nfo.pWaitDstStageMask = &pipeStageFlags;
	nfo.pCommandBuffers = cmdBuffs;
	nfo.commandBufferCount = numCmdBuffs;
	nfo.pSignalSemaphores =  signalSems;
	nfo.signalSemaphoreCount = numSignalSems;

	vkThrowIfFailed(vk::QueueSubmit(queue, 1, &nfo, fence), "CommandBufferBase::submitCommandBuffers failed");
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
	VkCommandBufferBeginInfo info;
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = NULL;
	info.flags = 0;
	info.pInheritanceInfo = NULL;
	vkThrowIfFailed(vk::BeginCommandBuffer(pImpl->handle, &info), "CommandBuffer::beginRecording(void) failed");
}

CommandBuffer_::CommandBuffer_(GraphicsContext& context, CommandPool& cmdPool, native::HCommandBuffer_& hCmdBuff) : CommandBufferBase_(context, cmdPool, hCmdBuff)
{ }


void CommandBuffer_::submit(Semaphore& waitSemaphore, Semaphore& signalSemaphore, const Fence& fence)
{
	auto handles = getContext()->getPlatformContext().getNativePlatformHandles();
	VkSemaphore* waitSems = NULL;
	VkSemaphore* signalSems = NULL;
	VkFence vkfence = fence.isValid() ? native_cast(*fence).handle : VK_NULL_HANDLE;
	if (waitSemaphore.isValid())
	{
		waitSems = &*native_cast(*waitSemaphore);
	}
	if (signalSemaphore.isValid())
	{
		signalSems = &*native_cast(*signalSemaphore);
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
	submit_command_buffers(handles.graphicsQueue, handles.context.device, &pImpl->handle, 1, &handles.semaphoreCanBeginRendering[swapIndex], handles.semaphoreCanBeginRendering[swapIndex] != 0,
	                       &handles.semaphoreFinishedRendering[swapIndex], handles.semaphoreFinishedRendering[swapIndex] != 0, vkfence);
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

native::HCommandBuffer_& CommandBufferBase_::getNativeObject() { return *pImpl; }
const native::HCommandBuffer_& CommandBufferBase_::getNativeObject() const { return *pImpl; }

void CommandBuffer_::enqueueSecondaryCmds(SecondaryCommandBuffer& secondaryCmdBuffer)
{
	vk::CmdExecuteCommands(pImpl->handle, 1, &native_cast(*secondaryCmdBuffer).handle);
}


void CommandBuffer_::enqueueSecondaryCmds(SecondaryCommandBuffer* secondaryCmdBuffer, uint32 numCmdBuffs)
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
		cmdBuffs[i] = native_cast(*secondaryCmdBuffer[i]).handle;
	}

	vk::CmdExecuteCommands(pImpl->handle, numCmdBuffs, cmdBuffs);
}

void CommandBuffer_::enqueueSecondaryCmds_BeginMultiple(uint32 expectedNumber)
{
	pImpl->multiEnqueueCache.resize(0);
	pImpl->multiEnqueueCache.reserve(expectedNumber);
}

void CommandBuffer_::enqueueSecondaryCmds_EnqueueMultiple(SecondaryCommandBuffer* secondaryCmdBuffer, uint32 numCmdBuffers)
{
	pImpl->multiEnqueueCache.reserve(pImpl->multiEnqueueCache.size() + numCmdBuffers);
	for (uint32 i = 0; i < numCmdBuffers; ++i)
	{
		pImpl->multiEnqueueCache.push_back(native_cast(*secondaryCmdBuffer[i]).handle);
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

void CommandBuffer_::beginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass,
                                     const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	beginRenderPass(fbo, renderArea, inlineFirstSubpass, &clearColor, 1, clearDepth, clearStencil);
}

void CommandBuffer_::beginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColor
                                     , uint32 numClearColor, float32 clearDepth, uint32 clearStencil)
{
	VkRenderPassBeginInfo nfo;
	nfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	nfo.pNext = 0;
	std::vector<VkClearValue> clearValues(numClearColor + 1);
	uint32 i = 0;
	for (; i < numClearColor; ++i)
	{
		clearValues[i].color.float32[0] = clearColor[i].r;
		clearValues[i].color.float32[1] = clearColor[i].g;
		clearValues[i].color.float32[2] = clearColor[i].b;
		clearValues[i].color.float32[3] = clearColor[i].a;
	}
	clearValues[i].depthStencil.depth = clearDepth;
	clearValues[i].depthStencil.stencil = clearStencil;

	nfo.pClearValues = clearValues.data();
	nfo.clearValueCount = (uint32)clearValues.size();
	nfo.framebuffer = native_cast(*fbo);
	copyRectangleToVulkan(renderArea, nfo.renderArea);
	nfo.renderPass = native_cast(*fbo->getRenderPass());
	vk::CmdBeginRenderPass(pImpl->handle, &nfo, inlineFirstSubpass ? VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
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
