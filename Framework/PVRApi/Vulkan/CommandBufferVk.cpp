/*!
\brief Vulkan Implementation functions of theCommandBuffer class.
\file PVRApi/Vulkan/CommandBufferVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/Vulkan/CommandBufferVk.h"
#include "PVRApi/Vulkan/CommandPoolVk.h"
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
using namespace nativeVk;

//assorted functions
namespace impl {
inline void copyRectangleToVulkan(const Rectanglei& renderArea, VkRect2D&  vulkanRenderArea)
{
	memcpy(&vulkanRenderArea, &renderArea, sizeof(renderArea));
}

CommandBufferImplVk_::~CommandBufferImplVk_()
{
	if (_context.isValid())
	{
		if (handle != VK_NULL_HANDLE)
		{
			if (_pool.isValid())
			{
				vk::FreeCommandBuffers(native_cast(_context)->getDevice(), native_cast(*_pool).handle, 1, &handle);
			}
			else
			{
				Log(Log.Debug, "Trying to release a Command buffer AFTER its pool was destroyed");
			}
			handle = VK_NULL_HANDLE;
		}
	}
	else
	{
		Log(Log.Warning, "WARNING - Command buffer released AFTER its context was destroyed.");
	}
}


void CommandBufferImplVk_::pushPipeline_()
{
	return;
	//"Push/Pop pipeline not supported in vulkan"
}

void CommandBufferImplVk_::popPipeline_()
{
	return;
	//"Push/Pop pipeline not supported/required in vulkan"
}

void CommandBufferImplVk_::resetPipeline_()
{
	return;
	//"Reset graphics pipeline has no effect on Vulkan underlying API"
}
inline static void submit_command_buffers(VkQueue queue, VkDevice device, const VkCommandBuffer* cmdBuffs,
    uint32 numCmdBuffs = 1, const VkSemaphore* waitSems = NULL, uint32 numWaitSems = 0,
    const VkSemaphore* signalSems = NULL, uint32 numSignalSems = 0, VkFence signalFence = VK_NULL_HANDLE)
{
	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkSubmitInfo nfo = {};
	nfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	nfo.waitSemaphoreCount = numWaitSems;
	nfo.pWaitSemaphores = waitSems;
	nfo.pWaitDstStageMask = &pipeStageFlags;
	nfo.pCommandBuffers = cmdBuffs;
	nfo.commandBufferCount = numCmdBuffs;
	nfo.pSignalSemaphores = signalSems;
	nfo.signalSemaphoreCount = numSignalSems;
	vkThrowIfFailed(vk::QueueSubmit(queue, 1, &nfo, signalFence), "CommandBufferBase::submitCommandBuffers failed");
}


}

//synchronization,
namespace impl {

VkMemoryBarrier memoryBarrier(const MemoryBarrier& memBarrier)
{
	VkMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.pNext = 0;
	barrier.srcAccessMask = ConvertToVk::accessFlags(memBarrier.srcMask);
	barrier.dstAccessMask = ConvertToVk::accessFlags(memBarrier.dstMask);
	return barrier;
}

VkBufferMemoryBarrier bufferBarrier(const BufferRangeBarrier& buffBarrier)
{
	VkBufferMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.pNext = 0;
	barrier.srcAccessMask = ConvertToVk::accessFlags(buffBarrier.srcMask);
	barrier.dstAccessMask = ConvertToVk::accessFlags(buffBarrier.dstMask);

	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.buffer = native_cast(*buffBarrier.buffer).buffer;
	barrier.offset = buffBarrier.offset;
	barrier.size = buffBarrier.range;
	return barrier;
}
VkImageMemoryBarrier imageBarrier(const ImageAreaBarrier& imgBarrier)
{
	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = 0;
	barrier.srcAccessMask = ConvertToVk::accessFlags(imgBarrier.srcMask);
	barrier.dstAccessMask = ConvertToVk::accessFlags(imgBarrier.dstMask);

	barrier.dstQueueFamilyIndex = 0;
	barrier.srcQueueFamilyIndex = 0;

	barrier.image = native_cast(*imgBarrier.texture).image;
	barrier.newLayout = ConvertToVk::imageLayout(imgBarrier.newLayout);
	barrier.oldLayout = ConvertToVk::imageLayout(imgBarrier.oldLayout);
	barrier.subresourceRange = ConvertToVk::imageSubResourceRange(imgBarrier.area);
	return barrier;
}
inline uint32 getNativeMemoryBarriersCount(const MemoryBarrierSet& set)
{
	return (uint32)set.getMemoryBarriers().size();
}
inline uint32 getNativeImageBarriersCount(const MemoryBarrierSet& set)
{
	return (uint32)set.getImageBarriers().size();
}
inline uint32 getNativeBufferBarriersCount_(const MemoryBarrierSet& set)
{
	return (uint32)set.getBufferBarriers().size();
}
inline void prepareNativeBarriers_(const MemoryBarrierSet& set, VkMemoryBarrier* mem, VkImageMemoryBarrier* img, VkBufferMemoryBarrier* buf)
{
	for (int i = 0; i < set.getMemoryBarriers().size(); ++i)
	{
		mem[i] = memoryBarrier(set.getMemoryBarriers()[i]);
	}
	for (int i = 0; i < set.getImageBarriers().size(); ++i)
	{
		img[i] = imageBarrier(set.getImageBarriers()[i]);
	}
	for (int i = 0; i < set.getBufferBarriers().size(); ++i)
	{
		buf[i] = bufferBarrier(set.getBufferBarriers()[i]);
	}
}
void CommandBufferImplVk_::pipelineBarrier_(types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers, bool dependencyByRegion)
{
	VkMemoryBarrier mem[16];
	VkImageMemoryBarrier img[16];
	VkBufferMemoryBarrier buf[16];
	uint32 memcnt = (uint32)barriers.getMemoryBarriers().size();
	uint32 imgcnt = (uint32)barriers.getImageBarriers().size();
	uint32 bufcnt = (uint32)barriers.getBufferBarriers().size();
	VkMemoryBarrier* memptr = memcnt > 16 ? new VkMemoryBarrier[memcnt] : mem;
	VkImageMemoryBarrier* imgptr = imgcnt > 16 ? new VkImageMemoryBarrier[imgcnt] : img;
	VkBufferMemoryBarrier* bufptr = bufcnt > 16 ? new VkBufferMemoryBarrier[bufcnt] : buf;

	prepareNativeBarriers_(barriers, memptr, imgptr, bufptr);

	vk::CmdPipelineBarrier(handle, ConvertToVk::pipelineStage(srcStage), ConvertToVk::pipelineStage(dstStage),
	                       (dependencyByRegion != 0) * VK_DEPENDENCY_BY_REGION_BIT,
	                       memcnt, memptr, bufcnt, bufptr, imgcnt, imgptr);
	if (memptr != mem) { delete memptr; }
	if (imgptr != img) { delete imgptr; }
	if (bufptr != buf) { delete bufptr; }
}

void CommandBufferImplVk_::waitForEvent_(const Event& evt, types::PipelineStageFlags srcStage,
    types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
{
	VkMemoryBarrier mem[16];
	VkImageMemoryBarrier img[16];
	VkBufferMemoryBarrier buf[16];
	uint32 memcnt = (uint32)barriers.getMemoryBarriers().size();
	uint32 imgcnt = (uint32)barriers.getImageBarriers().size();
	uint32 bufcnt = (uint32)barriers.getBufferBarriers().size();
	VkMemoryBarrier* memptr = memcnt > 16 ? new VkMemoryBarrier[memcnt] : mem;
	VkImageMemoryBarrier* imgptr = imgcnt > 16 ? new VkImageMemoryBarrier[imgcnt] : img;
	VkBufferMemoryBarrier* bufptr = bufcnt > 16 ? new VkBufferMemoryBarrier[bufcnt] : buf;

	prepareNativeBarriers_(barriers, memptr, imgptr, bufptr);

	vk::CmdWaitEvents(handle, 1, &native_cast(*evt).handle, ConvertToVk::pipelineStage(srcStage),
	                  ConvertToVk::pipelineStage(dstStage),
	                  memcnt, memptr, bufcnt, bufptr, imgcnt, imgptr);

	if (memptr != mem) { delete memptr; }
	if (imgptr != img) { delete imgptr; }
	if (bufptr != buf) { delete bufptr; }
}

void CommandBufferImplVk_::waitForEvents_(const EventSet& events, types::PipelineStageFlags srcStage,
    types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
{

	VkMemoryBarrier mem[16];
	VkImageMemoryBarrier img[16];
	VkBufferMemoryBarrier buf[16];
	uint32 memcnt = (uint32)barriers.getMemoryBarriers().size();
	uint32 imgcnt = (uint32)barriers.getImageBarriers().size();
	uint32 bufcnt = (uint32)barriers.getBufferBarriers().size();
	VkMemoryBarrier* memptr = memcnt > 16 ? new VkMemoryBarrier[memcnt] : mem;
	VkImageMemoryBarrier* imgptr = imgcnt > 16 ? new VkImageMemoryBarrier[imgcnt] : img;
	VkBufferMemoryBarrier* bufptr = bufcnt > 16 ? new VkBufferMemoryBarrier[bufcnt] : buf;

	prepareNativeBarriers_(barriers, memptr, imgptr, bufptr);

	vk::CmdWaitEvents(handle, events->getNativeEventsCount(), (VkEvent*)events->getNativeEvents(),
	                  ConvertToVk::pipelineStage(srcStage), ConvertToVk::pipelineStage(dstStage),
	                  memcnt, memptr, bufcnt, bufptr, imgcnt, imgptr);

	if (memptr != mem) { delete memptr; }
	if (imgptr != img) { delete imgptr; }
	if (bufptr != buf) { delete bufptr; }
}

void CommandBufferImplVk_::setEvent_(Event& evt, types::PipelineStageFlags stage)
{
	objectRefs.push_back(evt);
	vk::CmdSetEvent(handle, native_cast(*evt), ConvertToVk::pipelineStage(stage));
}

void CommandBufferImplVk_::resetEvent_(Event& evt, types::PipelineStageFlags stage)
{
	vk::CmdResetEvent(handle, native_cast(*evt), ConvertToVk::pipelineStage(stage));
}
}

//bind pipelines, sets, vertex/index buffers
namespace impl {

void CommandBufferImplVk_::bindPipeline_(GraphicsPipeline& pipeline)
{
	if (!lastBoundGraphicsPipe.isValid() || lastBoundGraphicsPipe != pipeline)
	{
		objectRefs.push_back(pipeline);
		vk::CmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, native_cast(pipeline));
		lastBoundGraphicsPipe = pipeline;
	}
}

void CommandBufferImplVk_::bindPipeline_(ComputePipeline& pipeline)
{
	if (!lastBoundComputePipe.isValid() || lastBoundComputePipe != pipeline)
	{
		lastBoundComputePipe = pipeline;
		objectRefs.push_back(pipeline);
		vk::CmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_COMPUTE, native_cast(pipeline));
	}
}

void CommandBufferImplVk_::bindPipeline_(SceneTraversalPipeline& pipeline)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::bindPipeline_(VertexRayPipeline& pipeline)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}


void CommandBufferImplVk_::bindDescriptorSetRayTracing_(const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet& set,
    const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::bindDescriptorSetSHG_(const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet& set,
    const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::bindDescriptorSets_(types::PipelineBindPoint bindingPoint, const api::PipelineLayout& pipelineLayout, uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	debug_assertion(numDescSets < 8, "Attempted to bind more than 8 descriptor sets");
	if (numDescSets < 8)
	{
		VkDescriptorSet native_sets[8] = { VK_NULL_HANDLE };
		for (uint32 i = 0; i < numDescSets; ++i)
		{
			objectRefs.push_back(sets[i]);
			native_sets[i] = native_cast(*sets[i]);
		}
		vk::CmdBindDescriptorSets(handle, ConvertToVk::pipelineBindPoint(bindingPoint),
		                          native_cast(*pipelineLayout).handle, firstSet, numDescSets, native_sets, numDynamicOffset, dynamicOffsets);
	}
}

void CommandBufferImplVk_::bindVertexBuffer_(const Buffer& buffer, uint32 offset, uint16 bindingIndex)
{
	objectRefs.push_back(buffer);
	VkDeviceSize offs = offset;
	vk::CmdBindVertexBuffers(handle, bindingIndex, 1, &native_cast(*buffer).buffer, &offs);
}

void CommandBufferImplVk_::bindVertexBuffer_(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding, uint16 bindingCount)
{
	if (numBuffers <= 8)
	{
		objectRefs.push_back(buffers[numBuffers]);
		VkBuffer buff[8];
		VkDeviceSize sizes[8];
		for (int i = 0; i < numBuffers; ++i)
		{
			objectRefs.push_back(buffers[i]);
			buff[i] = native_cast(*buffers[i]).buffer;
			sizes[i] = offsets[i];
		}
		vk::CmdBindVertexBuffers(handle, startBinding, bindingCount, buff, sizes);
	}
	else
	{
		VkBuffer* buff = new VkBuffer[numBuffers];
		VkDeviceSize* sizes = new VkDeviceSize[numBuffers];
		for (int i = 0; i < numBuffers; ++i)
		{
			objectRefs.push_back(buffers[i]);
			buff[i] = native_cast(*buffers[i]).buffer;
			sizes[i] = offsets[i];
		}
		vk::CmdBindVertexBuffers(handle, startBinding, bindingCount, buff, sizes);
		delete[] buff;
		delete[] sizes;
	}
}

void CommandBufferImplVk_::bindIndexBuffer_(const api::Buffer& buffer, uint32 offset, types::IndexType indexType)
{
	objectRefs.push_back(buffer);
	vk::CmdBindIndexBuffer(handle, native_cast(*buffer).buffer, offset, indexType == types::IndexType::IndexType16Bit ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

}

//begin end submit clear reset etc.
namespace impl {
void CommandBufferImplVk_::beginRecording_()
{
	if (_isRecording)
	{
		Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
		assertion(0);
	}
	clear_();
	_isRecording = true;
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = NULL;
	info.pInheritanceInfo = NULL;
	vkThrowIfFailed(vk::BeginCommandBuffer(handle, &info), "CommandBuffer::beginRecording(void) failed");
}

void CommandBufferImplVk_::beginRecording_(const Fbo& fbo, uint32 subPass)
{
	if (_isRecording)
	{
		Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
		assertion(0);
	}
	clear_();
	objectRefs.push_back(fbo);
	_isRecording = true;
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
	vkThrowIfFailed(vk::BeginCommandBuffer(handle, &info), "CommandBufferBase::beginRecording(fbo, [subpass]) failed");
}

void CommandBufferImplVk_::beginRecording_(const RenderPass& renderPass, uint32 subPass)
{
	if (_isRecording)
	{
		Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
		assertion(0);
	}
	clear_();
	objectRefs.push_back(renderPass);
	_isRecording = true;
	VkCommandBufferBeginInfo info = {};
	VkCommandBufferInheritanceInfo inheritInfo = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritInfo.renderPass = native_cast(*renderPass);
	inheritInfo.subpass = subPass;
	inheritInfo.occlusionQueryEnable = VK_FALSE;
	info.pInheritanceInfo = &inheritInfo;
	vkThrowIfFailed(vk::BeginCommandBuffer(handle, &info), "CommandBufferBase::beginRecording(renderpass, [subpass]) failed");
}

void CommandBufferImplVk_::endRecording_()
{
	if (!_isRecording)
	{
		Log("Called CommandBuffer::endRecording while a recording was not in progress. Call CommandBuffer::beginRecording first");
		assertion(0);
	}
	_isRecording = false;
	vkThrowIfFailed(vk::EndCommandBuffer(handle), "CommandBufferBase::endRecording failed");
}

void CommandBufferImplVk_::clear_(bool releaseResources)
{
	objectRefs.clear();
	lastBoundComputePipe.reset();
	lastBoundGraphicsPipe.reset();
	vk::ResetCommandBuffer(handle, (releaseResources != 0) * VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}


void CommandBufferImplVk_::submit_(const Semaphore& waitSemaphore, const Semaphore& signalSemaphore, const Fence& fence)
{
	const auto& handles = getContext_()->getPlatformContext().getNativePlatformHandles();
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
	submit_command_buffers(handles.mainQueue(), handles.context.device, &handle, 1, waitSems, waitSems != 0, signalSems, signalSems != 0, vkfence);
}

void CommandBufferImplVk_::submit_(SemaphoreSet& waitSemaphores, SemaphoreSet& signalSemaphores, const Fence& fence)
{
	const auto& handles = getContext_()->getPlatformContext().getNativePlatformHandles();
	VkSemaphore* waitSems = (VkSemaphore*)(waitSemaphores.isValid() ? waitSemaphores->getNativeSemaphores() : 0);
	uint32 waitSemsSize = (waitSemaphores.isValid() ? waitSemaphores->getNativeSemaphoresCount() : 0);
	VkSemaphore* signalSems = (VkSemaphore*)(signalSemaphores.isValid() ? signalSemaphores->getNativeSemaphores() : 0);
	uint32 signalSemsSize = (signalSemaphores.isValid() ? signalSemaphores->getNativeSemaphoresCount() : 0);
	VkFence vkfence = fence.isValid() ? native_cast(*fence).handle : VK_NULL_HANDLE;

	submit_command_buffers(handles.mainQueue(), handles.context.device, &handle, 1, waitSems, waitSemsSize, signalSems, signalSemsSize, vkfence);
}

void CommandBufferImplVk_::submit_(Fence& fence)
{
	const auto& handles = getContext_()->getPlatformContext().getNativePlatformHandles();
	uint32 swapIndex = getContext_()->getSwapChainIndex();
	VkFence vkfence = (fence.isValid() ? native_cast(*fence).handle : VK_NULL_HANDLE);
	submit_command_buffers(handles.mainQueue(), handles.context.device, &handle, 1, &handles.semaphoreCanBeginRendering[swapIndex],
	                       handles.semaphoreCanBeginRendering[swapIndex] != 0, &handles.semaphoreFinishedRendering[swapIndex],
	                       handles.semaphoreFinishedRendering[swapIndex] != 0, vkfence);
}

void CommandBufferImplVk_::submit_()
{
	uint32 swapIndex = getContext_()->getSwapChainIndex();
	const auto& handles = getContext_()->getPlatformContext().getNativePlatformHandles();
	submit_command_buffers(handles.mainQueue(), handles.context.device, &handle, 1,
	                       &handles.semaphoreCanBeginRendering[swapIndex], handles.semaphoreCanBeginRendering[swapIndex] != 0,
	                       &handles.semaphoreFinishedRendering[swapIndex], handles.semaphoreFinishedRendering[swapIndex] != 0,
	                       handles.fenceRender[swapIndex]);
}

void CommandBufferImplVk_::submitEndOfFrame_(Semaphore& waitSemaphore)
{
	const auto& handles = getContext_()->getPlatformContext().getNativePlatformHandles();
	uint32 swapIndex = getContext_()->getSwapChainIndex();
	VkFence vkfence = handles.fenceRender[swapIndex];
	assertion(waitSemaphore.isValid() && "CommandBuffer_::submitWait Invalid semaphore to wait on");
	VkSemaphore* waitSems = &*native_cast(*waitSemaphore);
	submit_command_buffers(handles.mainQueue(), handles.context.device, &handle, 1,
	                       waitSems, waitSems != 0, &handles.semaphoreFinishedRendering[swapIndex],
	                       handles.semaphoreFinishedRendering[swapIndex] != 0, vkfence);
}

void CommandBufferImplVk_::submitStartOfFrame_(Semaphore& signalSemaphore, const Fence& fence)
{
	const auto& handles = getContext_()->getPlatformContext().getNativePlatformHandles();
	uint32 swapIndex = getContext_()->getSwapChainIndex();
	VkSemaphore* pSignalSemaphore = NULL;
	VkFence vkfence = fence.isValid() ? native_cast(*fence).handle : VK_NULL_HANDLE;
	assertion(signalSemaphore.isValid() && "CommandBuffer_::submitWait Invalid semaphore to wait on");
	pSignalSemaphore = &*native_cast(*signalSemaphore);
	submit_command_buffers(handles.mainQueue(), handles.context.device, &handle, 1,
	                       &handles.semaphoreCanBeginRendering[swapIndex], handles.semaphoreCanBeginRendering[swapIndex] != 0,
	                       pSignalSemaphore, pSignalSemaphore != 0, vkfence);
}


void CommandBufferImplVk_::enqueueSecondaryCmds_(SecondaryCommandBuffer& secondaryCmdBuffer)
{
	objectRefs.push_back(secondaryCmdBuffer);
	assertion(secondaryCmdBuffer.isValid());
	vk::CmdExecuteCommands(handle, 1, &native_cast(*secondaryCmdBuffer).handle);
}

void CommandBufferImplVk_::enqueueSecondaryCmds_(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffs)
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
		objectRefs.push_back(secondaryCmdBuffers[i]);
		cmdBuffs[i] = native_cast(*secondaryCmdBuffers[i]).handle;
	}

	vk::CmdExecuteCommands(handle, numCmdBuffs, cmdBuffs);
}

void CommandBufferImplVk_::enqueueSecondaryCmds_BeginMultiple_(uint32 expectedNumber)
{
	multiEnqueueCache.resize(0);
	multiEnqueueCache.reserve(expectedNumber);
}

void CommandBufferImplVk_::enqueueSecondaryCmds_EnqueueMultiple_(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers)
{
	multiEnqueueCache.reserve(multiEnqueueCache.size() + numCmdBuffers);
	for (uint32 i = 0; i < numCmdBuffers; ++i)
	{
		objectRefs.push_back(secondaryCmdBuffers[i]);
		multiEnqueueCache.push_back(native_cast(*secondaryCmdBuffers[i]).handle);
	}
}

void CommandBufferImplVk_::enqueueSecondaryCmds_SubmitMultiple_(bool keepAllocated)
{
	vk::CmdExecuteCommands(handle, (uint32)multiEnqueueCache.size(), multiEnqueueCache.data());
	multiEnqueueCache.resize(0);
}
}

// Renderpasses, Subpasses
namespace impl {
inline void CommandBufferImplVk_::beginRenderPass__(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32* clearDepth, uint32* clearStencil, uint32 numClearDepthStencil)
{
	objectRefs.push_back(fbo);
	VkRenderPassBeginInfo nfo = {};
	nfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	std::vector<VkClearValue> clearValues(numClearColors + numClearDepthStencil);
	uint32 i = 0;
	for (; i < numClearColors; ++i)
	{
		memcpy(clearValues[i].color.float32, &clearColors[i], sizeof(float32) * 4);
	}
	for (numClearDepthStencil += numClearColors; i < numClearDepthStencil; ++i)
	{
		clearValues[i].depthStencil.depth = clearDepth[i - numClearColors];
		clearValues[i].depthStencil.stencil = clearStencil[i - numClearColors];
	}
	nfo.pClearValues = clearValues.data();
	nfo.clearValueCount = (uint32)clearValues.size();
	nfo.framebuffer = native_cast(*fbo);
	copyRectangleToVulkan(renderArea, nfo.renderArea);
	nfo.renderPass = native_cast(*renderPass);

	lastBoundFbo = fbo;
	lastBoundRenderPass = renderPass;
	vk::CmdBeginRenderPass(handle, &nfo, inlineFirstSubpass ?
	                       VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}


void CommandBufferImplVk_::beginRenderPass_(api::Fbo& fbo, bool inlineFirstSubpass, const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	glm::vec4 clearColors[4];
	glm::float32 clearDepths[4];
	uint32 clearStencils[4];
	assertion(fbo->getNumColorAttachments() <= ARRAY_SIZE(clearColors));
	for (uint32 i = 0; i < fbo->getNumColorAttachments(); ++i)
	{
		clearColors[i] = clearColor;
	}

	for (uint32 i = 0; i < fbo->getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}
	beginRenderPass__(fbo, fbo->getRenderPass(), Rectanglei(glm::ivec2(0, 0), fbo->getDimensions()),
	                  inlineFirstSubpass, clearColors, fbo->getNumColorAttachments(), clearDepths,
	                  clearStencils, fbo->getNumDepthStencilAttachments());
}

void CommandBufferImplVk_::beginRenderPass_(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	glm::vec4 clearColors[4];
	glm::float32 clearDepths[4];
	uint32 clearStencils[4];
	assertion(fbo->getNumColorAttachments() <= ARRAY_SIZE(clearColors));
	for (uint32 i = 0; i < fbo->getNumColorAttachments(); ++i)
	{
		clearColors[i] = clearColor;
	}

	for (uint32 i = 0; i < fbo->getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}

	beginRenderPass__(fbo, fbo->getRenderPass(), renderArea, inlineFirstSubpass, clearColors,
	                  fbo->getNumColorAttachments(), clearDepths, clearStencils, fbo->getNumDepthStencilAttachments());
}

void CommandBufferImplVk_::beginRenderPass_(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32 clearDepth, uint32 clearStencil)
{
	glm::float32 clearDepths[4];
	uint32 clearStencils[4];
	for (uint32 i = 0; i < fbo->getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}
	beginRenderPass__(fbo, fbo->getRenderPass(), renderArea, inlineFirstSubpass, clearColors, numClearColors,
	                  clearDepths, clearStencils, fbo->getNumDepthStencilAttachments());
}

void CommandBufferImplVk_::beginRenderPass_(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	glm::vec4 clearColors[4];
	glm::float32 clearDepths[4];
	uint32 clearStencils[4];
	assertion(fbo->getNumColorAttachments() <= ARRAY_SIZE(clearColors));
	for (uint32 i = 0; i < fbo->getNumColorAttachments(); ++i)
	{
		clearColors[i] = clearColor;
	}

	for (uint32 i = 0; i < fbo->getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}

	beginRenderPass__(fbo, renderPass, renderArea, inlineFirstSubpass, clearColors, fbo->getNumColorAttachments(),
	                  clearDepths, clearStencils, fbo->getNumDepthStencilAttachments());
}

void  CommandBufferImplVk_::beginRenderPass_(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors, float32* clearDepth, uint32* clearStencil, uint32 numClearDepthStencil)
{
	beginRenderPass__(fbo, renderPass, renderArea, inlineFirstSubpass, clearColors, numClearColors, clearDepth, clearStencil, numClearDepthStencil);
}

void  CommandBufferImplVk_::beginRenderPass_(api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass, const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	glm::vec4 clearColors[4];
	glm::float32 clearDepths[4];
	uint32 clearStencils[4];
	assertion(fbo->getNumColorAttachments() <= ARRAY_SIZE(clearColors));
	for (uint32 i = 0; i < fbo->getNumColorAttachments(); ++i)
	{
		clearColors[i] = clearColor;
	}

	for (uint32 i = 0; i < fbo->getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}
	beginRenderPass__(fbo, renderPass, Rectanglei(0, 0, fbo->getDimensions().x, fbo->getDimensions().y),
	                  inlineFirstSubpass, clearColors, fbo->getNumColorAttachments(), clearDepths, clearStencils,
	                  fbo->getNumDepthStencilAttachments());
}

void CommandBufferImplVk_::endRenderPass_()
{
	vk::CmdEndRenderPass(handle);
}

void CommandBufferImplVk_::nextSubPassInline_()
{
	vk::CmdNextSubpass(handle, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBufferImplVk_::nextSubPassSecondaryCmds_(pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	vk::CmdNextSubpass(handle, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	enqueueSecondaryCmds_(cmdBuffer);
}

}

//buffers, textures, images
namespace impl {
void CommandBufferImplVk_::updateBuffer_(Buffer& buffer, const void* data, uint32 offset, uint32 length)
{
	objectRefs.push_back(buffer);
	vk::CmdUpdateBuffer(handle, native_cast(*buffer).buffer, offset, length, (const uint32*)data);
}

void CommandBufferImplVk_::blitImage_(api::TextureStore& src, api::TextureStore& dst, types::ImageLayout srcLayout,
                                      types::ImageLayout dstLayout, types::ImageBlitRange* regions, uint32 numRegions,
                                      types::SamplerFilter filter)
{
	objectRefs.push_back(src);
	objectRefs.push_back(dst);
	std::vector<VkImageBlit> imageBlits(numRegions);
	for (uint32 i = 0; i < numRegions; ++i) { imageBlits[i] = ConvertToVk::imageBlit(regions[i]); }

	vk::CmdBlitImage(handle, native_cast(*src).image, ConvertToVk::imageLayout(srcLayout),
	                 native_cast(*dst).image, ConvertToVk::imageLayout(dstLayout), numRegions,
	                 imageBlits.data(), ConvertToVk::samplerFilter(filter));
}

void CommandBufferImplVk_::copyImageToBuffer_(TextureStore& srcImage, types::ImageLayout srcImageLayout,
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

	vk::CmdCopyImageToBuffer(handle, native_cast(*srcImage).image, ConvertToVk::imageLayout(srcImageLayout),
	                         native_cast(*dstBuffer).buffer, numRegions, pRegions);

}


void CommandBufferImplVk_::copyBuffer_(pvr::api::Buffer src, pvr::api::Buffer dst, pvr::uint32 srcOffset, pvr::uint32 destOffset, pvr::uint32 sizeInBytes)
{
	objectRefs.push_back(src);
	objectRefs.push_back(dst);
	VkBufferCopy region; region.srcOffset = srcOffset, region.dstOffset = destOffset, region.size = sizeInBytes;
	vk::CmdCopyBuffer(handle, native_cast(*src).buffer, native_cast(*dst).buffer, 1, &region);
}


}

//dynamic commands
namespace impl {

void CommandBufferImplVk_::setViewport_(const pvr::Rectanglei& viewport)
{
	VkViewport vp;
	vp.height = (float)viewport.height;
	vp.width = (float)viewport.width;
	vp.x = (float)viewport.x;
	vp.y = (float)viewport.y;
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	vk::CmdSetViewport(handle, 0, 1, &vp);
}

void CommandBufferImplVk_::setScissor_(const pvr::Rectanglei& scissor)
{
	VkRect2D sc;
	sc.offset.x = scissor.x;
	sc.offset.y = scissor.y;
	sc.extent.height = scissor.height;
	sc.extent.width = scissor.width;
	vk::CmdSetScissor(handle, 0, 1, &sc);
}

void CommandBufferImplVk_::setDepthBound_(pvr::float32 minDepth, pvr::float32 maxDepth)
{
	vk::CmdSetDepthBounds(handle, minDepth, maxDepth);
}

void CommandBufferImplVk_::setStencilCompareMask_(types::StencilFace face, pvr::uint32 compareMask)
{
	vk::CmdSetStencilCompareMask(handle, (VkStencilFaceFlagBits)face, compareMask);
}

void CommandBufferImplVk_::setStencilWriteMask_(types::StencilFace face, pvr::uint32 writeMask)
{
	vk::CmdSetStencilWriteMask(handle, (VkStencilFaceFlagBits)face, writeMask);
}

void CommandBufferImplVk_::setStencilReference_(types::StencilFace face, pvr::uint32 ref)
{
	vk::CmdSetStencilReference(handle, (VkStencilFaceFlagBits)face, ref);
}

void CommandBufferImplVk_::setDepthBias_(pvr::float32 depthBias, pvr::float32 depthBiasClamp, pvr::float32 slopeScaledDepthBias)
{
	vk::CmdSetDepthBias(handle, depthBias, depthBiasClamp, slopeScaledDepthBias);
}

void CommandBufferImplVk_::setBlendConstants_(glm::vec4 rgba)
{
	vk::CmdSetBlendConstants(handle, glm::value_ptr(rgba));
}

void CommandBufferImplVk_::setLineWidth_(float32 lineWidth)
{
	vk::CmdSetLineWidth(handle, lineWidth);
}


inline void clearcolorimage(VkCommandBuffer buffer, pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount, pvr::types::ImageLayout layout)
{
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

	vk::CmdClearColorImage(buffer, native_cast(*image->getResource()).image,
	                       ConvertToVk::imageLayout(layout), &clearColorValue, rangeCount, subResourceRange);

}

void CommandBufferImplVk_::clearColorImage_(
  pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32 baseMipLevel,
  const pvr::uint32 levelCount, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount,
  pvr::types::ImageLayout layout)
{
	objectRefs.push_back(image);
	clearcolorimage(handle, image, clearColor, &baseMipLevel, &levelCount, &baseArrayLayer, &layerCount, 1u, layout);
}

void CommandBufferImplVk_::clearColorImage_(
  pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32* baseMipLevel,
  const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount,
  pvr::uint32 rangeCount, pvr::types::ImageLayout layout)
{
	objectRefs.push_back(image);

	clearcolorimage(handle, image, clearColor, baseMipLevel, levelCount,  baseArrayLayers, layerCount, rangeCount, layout);
}

inline static void clearDepthStencilImageHelper(
  pvr::native::HCommandBuffer_ nativeCommandBuffer, pvr::api::TextureView& image,
  pvr::types::ImageLayout layout, VkImageAspectFlags imageAspect, float clearDepth,
  pvr::uint32 clearStencil, const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount)
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

	vk::CmdClearDepthStencilImage(nativeCommandBuffer, native_cast(*image->getResource()).image,
	                              ConvertToVk::imageLayout(layout), &clearDepthStencilValue, rangeCount, subResourceRanges);
}

void CommandBufferImplVk_::clearDepthImage_(pvr::api::TextureView& image, float clearDepth, const pvr::uint32 baseMipLevel, const pvr::uint32 levelCount, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount, pvr::types::ImageLayout layout)
{
	objectRefs.push_back(image);
	clearDepthStencilImageHelper(handle, image, layout, VK_IMAGE_ASPECT_DEPTH_BIT, clearDepth, 0u,
	                             &baseMipLevel, &levelCount, &baseArrayLayer, &layerCount, 1u);
}

void CommandBufferImplVk_::clearDepthImage_(pvr::api::TextureView& image, float clearDepth, const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount, pvr::types::ImageLayout layout)
{
	objectRefs.push_back(image);
	clearDepthStencilImageHelper(handle, image, layout, VK_IMAGE_ASPECT_DEPTH_BIT, clearDepth, 0u,
	                             baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount);
}

void CommandBufferImplVk_::clearStencilImage_(pvr::api::TextureView& image, pvr::uint32 clearStencil, const pvr::uint32 baseMipLevel, const pvr::uint32 levelCount, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount, pvr::types::ImageLayout layout)
{
	objectRefs.push_back(image);
	clearDepthStencilImageHelper(handle, image, layout, VK_IMAGE_ASPECT_STENCIL_BIT, 0.0f, clearStencil,
	                             &baseMipLevel, &levelCount, &baseArrayLayer, &layerCount, 1u);
}

void CommandBufferImplVk_::clearStencilImage_(pvr::api::TextureView& image, pvr::uint32 clearStencil, const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount, pvr::types::ImageLayout layout)
{
	objectRefs.push_back(image);
	clearDepthStencilImageHelper(handle, image, layout, VK_IMAGE_ASPECT_STENCIL_BIT, 0.0f, clearStencil,
	                             baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount);
}

void CommandBufferImplVk_::clearDepthStencilImage_(pvr::api::TextureView& image, float clearDepth, pvr::uint32 clearStencil, const pvr::uint32 baseMipLevel, const pvr::uint32 levelCount, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount, pvr::types::ImageLayout layout)
{
	objectRefs.push_back(image);
	clearDepthStencilImageHelper(handle, image, layout, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
	                             0.0f, clearStencil, &baseMipLevel, &levelCount, &baseArrayLayer, &layerCount, 1u);
}

void CommandBufferImplVk_::clearDepthStencilImage_(pvr::api::TextureView& image, float clearDepth, pvr::uint32 clearStencil, const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount, pvr::types::ImageLayout layout)
{
	objectRefs.push_back(image);
	clearDepthStencilImageHelper(handle, image, layout, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0.0f, clearStencil,
	                             baseMipLevel, levelCount, baseArrayLayers, layerCount, rangeCount);
}

inline void clear_color_attachment(VkCommandBuffer cb, pvr::uint32 const* attachmentIndices, glm::vec4 const* clearColors, pvr::uint32 attachmentCount, const pvr::Rectanglei* rects, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rectCount)
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
		clearRectangles[i].rect.extent.width = rects->width;
		clearRectangles[i].rect.extent.height = rects->height;
	}

	vk::CmdClearAttachments(cb, attachmentCount, clearAttachments, rectCount, clearRectangles);

}


void CommandBufferImplVk_::clearColorAttachment_(
  pvr::uint32 const* attachmentIndices, glm::vec4 const* clearColors, pvr::uint32 attachmentCount,
  const pvr::Rectanglei* rects, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rectCount)
{
	clear_color_attachment(handle, attachmentIndices, clearColors, attachmentCount, rects, baseArrayLayers, layerCount, rectCount);
}

void CommandBufferImplVk_::clearColorAttachment_(pvr::uint32 attachmentIndex, glm::vec4 clearColor, const pvr::Rectanglei rect, const pvr::uint32 baseArrayLayer, const pvr::uint32 layerCount)
{
	clear_color_attachment(handle, &attachmentIndex, &clearColor, 1u, &rect, &baseArrayLayer, &layerCount, 1u);
}

void CommandBufferImplVk_::clearColorAttachment_(pvr::api::Fbo fbo, glm::vec4 clearColor)
{
	objectRefs.push_back(fbo);
	pvr::uint32 attachmentIndices[(uint32)FrameworkCaps::MaxColorAttachments];
	for (uint32 i = 0; i < fbo->getNumColorAttachments(); ++i)
	{
		attachmentIndices[i] = i;
	}

	pvr::Rectanglei rect(0u, 0u, fbo->getDimensions().x, fbo->getDimensions().y);

	const pvr::uint32 baseArrayLayer = 0u;
	const pvr::uint32 layerCount = 1u;

	clear_color_attachment(handle, attachmentIndices, &clearColor, 1u, &rect, &baseArrayLayer, &layerCount, 1u);
}

void clearDepthStencilAttachmentHelper(pvr::native::HCommandBuffer_ nativeCommandBuffer, const pvr::Rectanglei& clearRect, VkImageAspectFlags imageAspect, float32 depth, pvr::int32 stencil)
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

void CommandBufferImplVk_::clearDepthAttachment_(const pvr::Rectanglei& clearRect, float32 depth)
{
	clearDepthStencilAttachmentHelper(handle, clearRect, VK_IMAGE_ASPECT_DEPTH_BIT, depth, 0u);
}

void CommandBufferImplVk_::clearStencilAttachment_(const pvr::Rectanglei& clearRect, pvr::int32 stencil)
{
	clearDepthStencilAttachmentHelper(handle, clearRect, VK_IMAGE_ASPECT_STENCIL_BIT, 0.0f, stencil);
}

void CommandBufferImplVk_::clearDepthStencilAttachment_(const pvr::Rectanglei& clearRect, float32 depth, pvr::int32 stencil)
{
	clearDepthStencilAttachmentHelper(handle, clearRect, VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT, depth, stencil);
}

}

// drawing commands
namespace impl {
void CommandBufferImplVk_::drawIndexed_(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset, uint32 firstInstance, uint32 instanceCount)
{
	vk::CmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBufferImplVk_::drawArrays_(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance, uint32 instanceCount)
{
	vk::CmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBufferImplVk_::drawArraysIndirect_(api::Buffer& buffer, uint32 offset, uint32 drawCount, uint32 stride)
{
	vk::CmdDrawIndirect(handle, native_cast(*buffer).buffer, offset, drawCount, stride);
}

void CommandBufferImplVk_::drawIndexedIndirect_(Buffer& buffer)
{
	objectRefs.push_back(buffer);
	vk::CmdDrawIndexedIndirect(handle, native_cast(buffer)->buffer, 0, 1, 0);
}

void CommandBufferImplVk_::drawIndirect_(Buffer& buffer, pvr::uint32 offset, pvr::uint32 count, pvr::uint32 stride)
{
	objectRefs.push_back(buffer);
	vk::CmdDrawIndirect(handle, native_cast(*buffer).buffer, offset, count, stride);
}

void CommandBufferImplVk_::dispatchCompute_(uint32 numGroupsX, uint32 numGroupsY, uint32 numGroupsZ)
{
	vk::CmdDispatch(handle, numGroupsX, numGroupsY, numGroupsZ);
}
}

// ray tracing
namespace impl {
void CommandBufferImplVk_::beginSceneHierarchy_(const SceneHierarchy& sceneHierarchy, pvr::math::AxisAlignedBox& extents)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::endSceneHierarchy_()
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::mergeSceneHierarchies_(const SceneHierarchy& destinationSceneHierarchy,
    pvr::math::AxisAlignedBox& extents, const SceneHierarchy* sourceSceneHierarchies,
    const pvr::uint32 numberOfSourceSceneHierarchies, const pvr::uint32 mergeQuality)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::bindSceneHierarchies_(const SceneHierarchy* sceneHierarchies, pvr::uint32 firstBinding, const pvr::uint32 numberOfSceneHierarchies)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::dispatchRays_(pvr::uint32 xOffset, pvr::uint32 yOffset,
    pvr::uint32 frameWidth, pvr::uint32 frameHeight)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::bindAccumulationImages_(pvr::uint32 startBinding, pvr::uint32 bindingCount, const TextureView* imageViews)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::sceneHierarchyAppend_(pvr::uint32 vertexCount, pvr::uint32 instanceCount, pvr::uint32 firstVertex, pvr::uint32 firstInstance)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::sceneHierarchyAppendIndexed_(pvr::uint32 indexCount, pvr::uint32 instanceCount, pvr::uint32 firstIndex, pvr::uint32 vertexOffset, pvr::uint32 firstInstance)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::sceneHierarchyAppendIndirect_(pvr::api::BufferView& indirectBuffer, pvr::uint32 offset, pvr::uint32 drawCount, pvr::uint32 stride)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::sceneHierarchyAppendIndexedIndirect_(pvr::api::BufferView& indirectBuffer, pvr::uint32 offset, pvr::uint32 drawCount, pvr::uint32 stride)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::pushSharedRayConstants_(uint32 offset, uint32 size, const void* pValues)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::setRaySizes_(uint32 raySizeCount, const uint32* pRaySizes)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}

void CommandBufferImplVk_::setRayBounceLimit_(uint32 limit)
{
	debug_assertion(getContext_()->getPlatformContext().isRayTracingSupported(),
	                "Context does not support ray tracing");
}
}

//uniforms
namespace impl {
#define SET_UNIFORM_DECLARATION(_type_)\
void CommandBufferImplVk_::setUniform_(int32, const _type_& )\
{ Log(Log.Error, "Free uniforms not supported in Vulkan implementation. Please use a Buffer instead.");  assertion(0); }\
void CommandBufferImplVk_::setUniformPtr_(int32 , pvr::uint32 , const _type_* ) \
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

}//impl
native::HCommandBuffer_& native_cast(pvr::api::impl::CommandBufferBase_& object)
{
	return static_cast<pvr::api::impl::CommandBufferImplVk_&>(*object.pimpl);
}
const native::HCommandBuffer_& native_cast(const pvr::api::impl::CommandBufferBase_& object)
{
	return static_cast<pvr::api::impl::CommandBufferImplVk_&>(*object.pimpl);
}

}//api
}//pvr
