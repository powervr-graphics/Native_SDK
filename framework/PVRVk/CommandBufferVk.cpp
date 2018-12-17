/*!
\brief Function implementations for the Command Buffer class
\file PVRVk/CommandBufferVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRVk/HeadersVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/GraphicsPipelineVk.h"
#include "PVRVk/ComputePipelineVk.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/RenderPassVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/MemoryBarrierVk.h"
#include "PVRVk/FramebufferVk.h"
#include "PVRVk/QueryPoolVk.h"

namespace pvrvk {
namespace impl {
inline void copyRectangleToVulkan(const Rect2D& renderArea, VkRect2D& vulkanRenderArea)
{
	memcpy(&vulkanRenderArea, &renderArea, sizeof(renderArea));
}

CommandBufferBase_::~CommandBufferBase_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			if (_pool.isValid())
			{
				_device->getVkBindings().vkFreeCommandBuffers(getDevice()->getVkHandle(), _pool->getVkHandle(), 1, &getVkHandle());
				_vkHandle = VK_NULL_HANDLE;
				_device.reset();
			}
			else
			{
				Log(LogLevel::Debug, "Trying to release a Command buffer after its pool was destroyed");
			}
		}
		else
		{
			reportDestroyedAfterDevice("CommandBuffer");
		}
	}
}

// synchronization
VkMemoryBarrier memoryBarrier(const MemoryBarrier& memBarrier)
{
	VkMemoryBarrier barrier = {};
	barrier.sType = static_cast<VkStructureType>(StructureType::e_MEMORY_BARRIER);
	barrier.srcAccessMask = static_cast<VkAccessFlags>(memBarrier.getSrcAccessMask());
	barrier.dstAccessMask = static_cast<VkAccessFlags>(memBarrier.getDstAccessMask());
	return barrier;
}

VkBufferMemoryBarrier bufferBarrier(const BufferMemoryBarrier& buffBarrier)
{
	VkBufferMemoryBarrier barrier;
	barrier.sType = static_cast<VkStructureType>(StructureType::e_BUFFER_MEMORY_BARRIER);
	barrier.pNext = 0;
	barrier.srcAccessMask = static_cast<VkAccessFlags>(buffBarrier.getSrcAccessMask());
	barrier.dstAccessMask = static_cast<VkAccessFlags>(buffBarrier.getDstAccessMask());

	barrier.dstQueueFamilyIndex = static_cast<uint32_t>(-1);
	barrier.srcQueueFamilyIndex = static_cast<uint32_t>(-1);

	barrier.buffer = buffBarrier.getBuffer()->getVkHandle();
	barrier.offset = buffBarrier.getOffset();
	barrier.size = buffBarrier.getSize();
	return barrier;
}

VkImageMemoryBarrier imageBarrier(const ImageMemoryBarrier& imgBarrier)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = static_cast<VkStructureType>(StructureType::e_IMAGE_MEMORY_BARRIER);
	barrier.srcAccessMask = static_cast<VkAccessFlags>(imgBarrier.getSrcAccessMask());
	barrier.dstAccessMask = static_cast<VkAccessFlags>(imgBarrier.getDstAccessMask());

	barrier.srcQueueFamilyIndex = imgBarrier.getSrcQueueFamilyIndex();
	barrier.dstQueueFamilyIndex = imgBarrier.getDstQueueFamilyIndex();

	barrier.image = imgBarrier.getImage()->getVkHandle();
	barrier.newLayout = static_cast<VkImageLayout>(imgBarrier.getNewLayout());
	barrier.oldLayout = static_cast<VkImageLayout>(imgBarrier.getOldLayout());
	barrier.subresourceRange = imgBarrier.getSubresourceRange().get();
	return barrier;
}

inline uint32_t getNumNativeMemoryBarriers(const MemoryBarrierSet& set)
{
	return static_cast<uint32_t>(set.getMemoryBarriers().size());
}

inline uint32_t getNumNativeImageBarriers(const MemoryBarrierSet& set)
{
	return static_cast<uint32_t>(set.getImageBarriers().size());
}

inline uint32_t getNumNativeBufferBarriers(const MemoryBarrierSet& set)
{
	return static_cast<uint32_t>(set.getBufferBarriers().size());
}

inline void prepareNativeBarriers(const MemoryBarrierSet& set, VkMemoryBarrier* mem, VkImageMemoryBarrier* img, VkBufferMemoryBarrier* buf)
{
	const auto& memBarriers = set.getMemoryBarriers();
	const auto& imageBarriers = set.getImageBarriers();
	const auto& bufferBarriers = set.getBufferBarriers();
	for (uint32_t i = 0; i < memBarriers.size(); ++i)
	{
		mem[i] = memoryBarrier(memBarriers[i]);
	}
	for (uint32_t i = 0; i < imageBarriers.size(); ++i)
	{
		img[i] = imageBarrier(imageBarriers[i]);
	}
	for (uint32_t i = 0; i < bufferBarriers.size(); ++i)
	{
		buf[i] = bufferBarrier(bufferBarriers[i]);
	}
}

void CommandBufferBase_::pipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers, bool dependencyByRegion)
{
	const uint32_t maxArrayBarrier = 16;
	VkMemoryBarrier mem[maxArrayBarrier];
	VkImageMemoryBarrier img[maxArrayBarrier];
	VkBufferMemoryBarrier buf[maxArrayBarrier];
	uint32_t memcnt = static_cast<uint32_t>(barriers.getMemoryBarriers().size());
	uint32_t imgcnt = static_cast<uint32_t>(barriers.getImageBarriers().size());
	uint32_t bufcnt = static_cast<uint32_t>(barriers.getBufferBarriers().size());
	VkMemoryBarrier* memptr = memcnt > maxArrayBarrier ? new VkMemoryBarrier[memcnt] : mem;
	VkImageMemoryBarrier* imgptr = imgcnt > maxArrayBarrier ? new VkImageMemoryBarrier[imgcnt] : img;
	VkBufferMemoryBarrier* bufptr = bufcnt > maxArrayBarrier ? new VkBufferMemoryBarrier[bufcnt] : buf;

	prepareNativeBarriers(barriers, memptr, imgptr, bufptr);

	_device->getVkBindings().vkCmdPipelineBarrier(getVkHandle(), static_cast<VkPipelineStageFlags>(srcStage), static_cast<VkPipelineStageFlags>(dstStage),
		static_cast<VkDependencyFlags>(DependencyFlags::e_BY_REGION_BIT * (dependencyByRegion != 0)), memcnt, memptr, bufcnt, bufptr, imgcnt, imgptr);

	if (memptr != mem)
	{
		delete[] memptr;
	}
	if (imgptr != img)
	{
		delete[] imgptr;
	}
	if (bufptr != buf)
	{
		delete[] bufptr;
	}
}

void CommandBufferBase_::waitForEvent(const Event& event, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
{
	_objectReferences.push_back(event);
	VkMemoryBarrier mem[16];
	VkImageMemoryBarrier img[16];
	VkBufferMemoryBarrier buf[16];
	uint32_t memcnt = static_cast<uint32_t>(barriers.getMemoryBarriers().size());
	uint32_t imgcnt = static_cast<uint32_t>(barriers.getImageBarriers().size());
	uint32_t bufcnt = static_cast<uint32_t>(barriers.getBufferBarriers().size());
	VkMemoryBarrier* memptr = memcnt > 16 ? new VkMemoryBarrier[memcnt] : mem;
	VkImageMemoryBarrier* imgptr = imgcnt > 16 ? new VkImageMemoryBarrier[imgcnt] : img;
	VkBufferMemoryBarrier* bufptr = bufcnt > 16 ? new VkBufferMemoryBarrier[bufcnt] : buf;

	prepareNativeBarriers(barriers, memptr, imgptr, bufptr);

	_device->getVkBindings().vkCmdWaitEvents(getVkHandle(), 1, &event->getVkHandle(), static_cast<VkPipelineStageFlags>(srcStage), static_cast<VkPipelineStageFlags>(dstStage),
		memcnt, memptr, bufcnt, bufptr, imgcnt, imgptr);

	if (memptr != mem)
	{
		delete[] memptr;
	}
	if (imgptr != img)
	{
		delete[] imgptr;
	}
	if (bufptr != buf)
	{
		delete[] bufptr;
	}
}

void CommandBufferBase_::waitForEvents(const Event* events, uint32_t numEvents, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
{
	VkMemoryBarrier mem[16];
	VkImageMemoryBarrier img[16];
	VkBufferMemoryBarrier buf[16];
	uint32_t memcnt = static_cast<uint32_t>(barriers.getMemoryBarriers().size());
	uint32_t imgcnt = static_cast<uint32_t>(barriers.getImageBarriers().size());
	uint32_t bufcnt = static_cast<uint32_t>(barriers.getBufferBarriers().size());
	VkMemoryBarrier* memptr = memcnt > 16 ? new VkMemoryBarrier[memcnt] : mem;
	VkImageMemoryBarrier* imgptr = imgcnt > 16 ? new VkImageMemoryBarrier[imgcnt] : img;
	VkBufferMemoryBarrier* bufptr = bufcnt > 16 ? new VkBufferMemoryBarrier[bufcnt] : buf;

	prepareNativeBarriers(barriers, memptr, imgptr, bufptr);

	ArrayOrVector<VkEvent, 4> vkEvents(numEvents);
	for (uint32_t i = 0; i < numEvents; ++i)
	{
		_objectReferences.push_back(events[i]);
		vkEvents[i] = events[i]->getVkHandle();
	}

	_device->getVkBindings().vkCmdWaitEvents(getVkHandle(), numEvents, vkEvents.get(), static_cast<VkPipelineStageFlags>(srcStage), static_cast<VkPipelineStageFlags>(dstStage),
		memcnt, memptr, bufcnt, bufptr, imgcnt, imgptr);

	if (memptr != mem)
	{
		delete[] memptr;
	}
	if (imgptr != img)
	{
		delete[] imgptr;
	}
	if (bufptr != buf)
	{
		delete[] bufptr;
	}
}

// bind pipelines, sets, vertex/index buffers
void CommandBufferBase_::bindDescriptorSets(PipelineBindPoint bindingPoint, const PipelineLayout& pipelineLayout, uint32_t firstSet, const DescriptorSet* sets,
	uint32_t numDescriptorSets, const uint32_t* dynamicOffsets, uint32_t numDynamicOffsets)
{
	debug_assertion(numDescriptorSets < static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSets), "Attempted to bind more than 8 descriptor sets");
	if (numDescriptorSets < static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSets))
	{
		VkDescriptorSet native_sets[static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSets)] = { VK_NULL_HANDLE };
		for (uint32_t i = 0; i < numDescriptorSets; ++i)
		{
			_objectReferences.push_back(sets[i]);
			native_sets[i] = sets[i]->getVkHandle();
		}
		_device->getVkBindings().vkCmdBindDescriptorSets(getVkHandle(), static_cast<VkPipelineBindPoint>(bindingPoint), pipelineLayout->getVkHandle(), firstSet, numDescriptorSets,
			native_sets, numDynamicOffsets, dynamicOffsets);
	}
	_objectReferences.push_back(pipelineLayout);
}

void CommandBufferBase_::bindVertexBuffer(Buffer const* buffers, uint32_t* offsets, uint16_t numBuffers, uint16_t startBinding, uint16_t numBindings)
{
	if (numBuffers <= 8)
	{
		_objectReferences.push_back(buffers[numBuffers]);
		VkBuffer buff[8];
		VkDeviceSize sizes[8];
		for (uint16_t i = 0; i < numBuffers; ++i)
		{
			_objectReferences.push_back(buffers[i]);
			buff[i] = buffers[i]->getVkHandle();
			sizes[i] = offsets[i];
		}
		_device->getVkBindings().vkCmdBindVertexBuffers(getVkHandle(), startBinding, numBindings, buff, sizes);
	}
	else
	{
		VkBuffer* buff = new VkBuffer[numBuffers];
		VkDeviceSize* sizes = new VkDeviceSize[numBuffers];
		for (uint16_t i = 0; i < numBuffers; ++i)
		{
			_objectReferences.push_back(buffers[i]);
			buff[i] = buffers[i]->getVkHandle();
			sizes[i] = offsets[i];
		}
		_device->getVkBindings().vkCmdBindVertexBuffers(getVkHandle(), startBinding, numBindings, buff, sizes);
		delete[] buff;
		delete[] sizes;
	}
}

// begin end submit clear reset etc.
void CommandBufferBase_::begin(const CommandBufferUsageFlags flags)
{
	if (_isRecording)
	{
		Log("Called CommandBuffer::begin while a recording was already in progress. Call CommandBuffer::end first");
		assertion(0);
	}
	reset(CommandBufferResetFlags(0));
	_isRecording = true;
	VkCommandBufferBeginInfo info = {};
	info.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_BEGIN_INFO);
	info.pNext = NULL;
	info.flags = static_cast<VkCommandBufferUsageFlags>(flags);
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_INHERITANCE_INFO);
	inheritanceInfo.renderPass = VK_NULL_HANDLE;
	inheritanceInfo.framebuffer = VK_NULL_HANDLE;
	inheritanceInfo.subpass = static_cast<uint32_t>(-1);
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.queryFlags = (VkQueryControlFlags)0;
	inheritanceInfo.pipelineStatistics = (VkQueryPipelineStatisticFlags)0;
	info.pInheritanceInfo = &inheritanceInfo;
	vkThrowIfFailed(_device->getVkBindings().vkBeginCommandBuffer(getVkHandle(), &info), "CommandBuffer::begin(void) failed");
}

void SecondaryCommandBuffer_::begin(const Framebuffer& framebuffer, uint32_t subpass, const CommandBufferUsageFlags flags)
{
	if (_isRecording)
	{
		throw ErrorValidationFailedEXT("Called CommandBuffer::begin while a recording was already in progress. Call CommandBuffer::end first");
	}
	reset(CommandBufferResetFlags(0));
	_objectReferences.push_back(framebuffer);
	_isRecording = true;
	VkCommandBufferBeginInfo info = {};
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	info.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_BEGIN_INFO);
	info.flags = static_cast<VkCommandBufferUsageFlags>(flags);
	inheritanceInfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_INHERITANCE_INFO);
	inheritanceInfo.renderPass = framebuffer->getRenderPass()->getVkHandle();
	inheritanceInfo.framebuffer = framebuffer->getVkHandle();
	inheritanceInfo.subpass = subpass;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	info.pInheritanceInfo = &inheritanceInfo;
	vkThrowIfFailed(_device->getVkBindings().vkBeginCommandBuffer(getVkHandle(), &info), "CommandBufferBase::begin(framebuffer, [subpass]) failed");
}

void SecondaryCommandBuffer_::begin(const RenderPass& renderPass, uint32_t subpass, const CommandBufferUsageFlags flags)
{
	if (_isRecording)
	{
		throw ErrorValidationFailedEXT("Called CommandBuffer::begin while a recording was already"
									   " in progress. Call CommandBuffer::end first");
	}
	reset(CommandBufferResetFlags(0));
	_objectReferences.push_back(renderPass);
	_isRecording = true;
	VkCommandBufferBeginInfo info = {};
	VkCommandBufferInheritanceInfo inheritInfo = {};
	info.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_BEGIN_INFO);
	info.flags = static_cast<VkCommandBufferUsageFlags>(flags);
	inheritInfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_INHERITANCE_INFO);
	inheritInfo.renderPass = renderPass->getVkHandle();
	inheritInfo.subpass = subpass;
	inheritInfo.occlusionQueryEnable = VK_FALSE;
	info.pInheritanceInfo = &inheritInfo;
	vkThrowIfFailed(_device->getVkBindings().vkBeginCommandBuffer(getVkHandle(), &info), "CommandBufferBase::begin(renderpass, [subpass]) failed");
}

void CommandBufferBase_::end()
{
	if (!_isRecording)
	{
		throw ErrorValidationFailedEXT("Called CommandBuffer::end while a recording "
									   "was not in progress. Call CommandBuffer::begin first");
	}
	_isRecording = false;
	vkThrowIfFailed(_device->getVkBindings().vkEndCommandBuffer(getVkHandle()), "CommandBufferBase::end failed");

#ifdef DEBUG
	if (_debugRegions.size() > 0)
	{
		Log(LogLevel::Debug, "There were a number of debug regions still open in this Command Buffer:");
		for (uint32_t i = 0; i < _debugRegions.size(); i++)
		{
			Log(LogLevel::Debug, "		'%s%'", _debugRegions[i].c_str());
		}
	}
#endif
}

void CommandBuffer_::executeCommands(const SecondaryCommandBuffer& secondaryCmdBuffer)
{
	if (secondaryCmdBuffer.isNull())
	{
		throw ErrorValidationFailedEXT("Secondary command buffer was NULL for ExecuteCommands");
	}
	_objectReferences.push_back(secondaryCmdBuffer);

	_device->getVkBindings().vkCmdExecuteCommands(getVkHandle(), 1, &secondaryCmdBuffer->getVkHandle());
}

void CommandBuffer_::executeCommands(const SecondaryCommandBuffer* secondaryCmdBuffers, uint32_t numCommandBuffers)
{
	ArrayOrVector<VkCommandBuffer, 16> cmdBuffs(numCommandBuffers);
	for (uint32_t i = 0; i < numCommandBuffers; ++i)
	{
		_objectReferences.push_back(secondaryCmdBuffers[i]);
		cmdBuffs[i] = secondaryCmdBuffers[i]->getVkHandle();
	}

	_device->getVkBindings().vkCmdExecuteCommands(getVkHandle(), numCommandBuffers, cmdBuffs.get());
}

// Renderpasses, Subpasses
void CommandBuffer_::beginRenderPass(
	const Framebuffer& framebuffer, const RenderPass& renderPass, const Rect2D& renderArea, bool inlineFirstSubpass, const ClearValue* clearValues, uint32_t numClearValues)
{
	_objectReferences.push_back(framebuffer);
	_objectReferences.push_back(renderPass);
	VkRenderPassBeginInfo nfo = {};
	nfo.sType = static_cast<VkStructureType>(StructureType::e_RENDER_PASS_BEGIN_INFO);
	nfo.pClearValues = (VkClearValue*)clearValues;
	nfo.clearValueCount = numClearValues;
	nfo.framebuffer = framebuffer->getVkHandle();
	copyRectangleToVulkan(renderArea, nfo.renderArea);
	nfo.renderPass = renderPass->getVkHandle();

	_device->getVkBindings().vkCmdBeginRenderPass(
		getVkHandle(), &nfo, static_cast<VkSubpassContents>(inlineFirstSubpass ? SubpassContents::e_INLINE : SubpassContents::e_SECONDARY_COMMAND_BUFFERS));
}

void CommandBuffer_::beginRenderPass(const Framebuffer& framebuffer, const Rect2D& renderArea, bool inlineFirstSubpass, const ClearValue* clearValues, uint32_t numClearValues)
{
	beginRenderPass(framebuffer, framebuffer->getRenderPass(), renderArea, inlineFirstSubpass, clearValues, numClearValues);
}

void CommandBuffer_::beginRenderPass(const Framebuffer& framebuffer, bool inlineFirstSubpass, const ClearValue* clearValues, uint32_t numClearValues)
{
	beginRenderPass(framebuffer, framebuffer->getRenderPass(), Rect2D(Offset2D(0, 0), Extent2D(framebuffer->getDimensions().getWidth(), framebuffer->getDimensions().getHeight())),
		inlineFirstSubpass, clearValues, numClearValues);
}

// buffers, textures, images, push constants
void CommandBufferBase_::updateBuffer(const Buffer& buffer, const void* data, uint32_t offset, uint32_t length)
{
	_objectReferences.push_back(buffer);
	_device->getVkBindings().vkCmdUpdateBuffer(getVkHandle(), buffer->getVkHandle(), offset, length, (const uint32_t*)data);
}

void CommandBufferBase_::pushConstants(const PipelineLayout& pipelineLayout, ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data)
{
	_objectReferences.push_back(pipelineLayout);
	_device->getVkBindings().vkCmdPushConstants(getVkHandle(), pipelineLayout->getVkHandle(), static_cast<VkShaderStageFlags>(stageFlags), offset, size, data);
}

void CommandBufferBase_::resolveImage(const Image& srcImage, const Image& dstImage, const ImageResolve* regions, uint32_t numRegions, ImageLayout srcLayout, ImageLayout dstLayout)
{
	_objectReferences.push_back(srcImage);
	_objectReferences.push_back(dstImage);
	assert(sizeof(ImageResolve) == sizeof(VkImageResolve));
	_device->getVkBindings().vkCmdResolveImage(getVkHandle(), srcImage->getVkHandle(), static_cast<VkImageLayout>(srcLayout), dstImage->getVkHandle(),
		static_cast<VkImageLayout>(dstLayout), numRegions, (const VkImageResolve*)(regions));
}

void CommandBufferBase_::blitImage(const Image& src, const Image& dst, const ImageBlit* regions, uint32_t numRegions, Filter filter, ImageLayout srcLayout, ImageLayout dstLayout)
{
	_objectReferences.push_back(src);
	_objectReferences.push_back(dst);
	ArrayOrVector<VkImageBlit, 8> imageBlits(numRegions);
	for (uint32_t i = 0; i < numRegions; ++i)
	{
		imageBlits[i] = regions[i].get();
	}

	_device->getVkBindings().vkCmdBlitImage(getVkHandle(), src->getVkHandle(), static_cast<VkImageLayout>(srcLayout), dst->getVkHandle(), static_cast<VkImageLayout>(dstLayout),
		numRegions, imageBlits.get(), static_cast<VkFilter>(filter));
}

void CommandBufferBase_::copyImage(const Image& srcImage, const Image& dstImage, ImageLayout srcImageLayout, ImageLayout dstImageLayout, uint32_t numRegions, const ImageCopy* regions)
{
	_objectReferences.push_back(srcImage);
	_objectReferences.push_back(dstImage);
	// Try to avoid heap allocation
	ArrayOrVector<VkImageCopy, 8> pRegions(numRegions);

	for (uint32_t i = 0; i < numRegions; ++i)
	{
		pRegions[i] = regions[i].get();
	}

	_device->getVkBindings().vkCmdCopyImage(getVkHandle(), srcImage->getVkHandle(), static_cast<VkImageLayout>(srcImageLayout), dstImage->getVkHandle(),
		static_cast<VkImageLayout>(dstImageLayout), numRegions, pRegions.get());
}

void CommandBufferBase_::copyImageToBuffer(const Image& srcImage, ImageLayout srcImageLayout, Buffer& dstBuffer, const BufferImageCopy* regions, uint32_t numRegions)
{
	_objectReferences.push_back(srcImage);
	_objectReferences.push_back(dstBuffer);

	ArrayOrVector<VkBufferImageCopy, 8> pRegions(numRegions);
	// Try to avoid heap allocation

	for (uint32_t i = 0; i < numRegions; ++i)
	{
		pRegions[i] = regions[i].get();
	}

	_device->getVkBindings().vkCmdCopyImageToBuffer(
		getVkHandle(), srcImage->getVkHandle(), static_cast<VkImageLayout>(srcImageLayout), dstBuffer->getVkHandle(), numRegions, pRegions.get());
}

void CommandBufferBase_::copyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, uint32_t numRegions, const BufferCopy* regions)
{
	_objectReferences.push_back(srcBuffer);
	_objectReferences.push_back(dstBuffer);
	_device->getVkBindings().vkCmdCopyBuffer(getVkHandle(), srcBuffer->getVkHandle(), dstBuffer->getVkHandle(), numRegions, (const VkBufferCopy*)regions);
}
void CommandBufferBase_::copyBufferToImage(const Buffer& buffer, const Image& image, ImageLayout dstImageLayout, uint32_t regionsCount, const BufferImageCopy* regions)
{
	ArrayOrVector<VkBufferImageCopy, 8> bufferImageCopy(regionsCount);
	_objectReferences.push_back(buffer);
	_objectReferences.push_back(image);
	for (uint32_t i = 0; i < regionsCount; ++i)
	{
		bufferImageCopy[i] = regions[i].get();
	}
	_device->getVkBindings().vkCmdCopyBufferToImage(
		getVkHandle(), buffer->getVkHandle(), image->getVkHandle(), static_cast<VkImageLayout>(dstImageLayout), regionsCount, bufferImageCopy.get());
}

void CommandBufferBase_::fillBuffer(const Buffer& dstBuffer, uint32_t dstOffset, uint32_t data, uint64_t size)
{
	_objectReferences.push_back(dstBuffer);
	_device->getVkBindings().vkCmdFillBuffer(getVkHandle(), dstBuffer->getVkHandle(), dstOffset, size, data);
}

// dynamic commands
void CommandBufferBase_::setViewport(const Viewport& viewport)
{
	_device->getVkBindings().vkCmdSetViewport(getVkHandle(), 0, 1, &viewport.get());
}

void CommandBufferBase_::setScissor(uint32_t firstScissor, uint32_t numScissors, const Rect2D* scissors)
{
	_device->getVkBindings().vkCmdSetScissor(getVkHandle(), firstScissor, numScissors, (const VkRect2D*)scissors);
}

void CommandBufferBase_::setDepthBounds(float min, float max)
{
	_device->getVkBindings().vkCmdSetDepthBounds(getVkHandle(), min, max);
}

void CommandBufferBase_::setStencilCompareMask(StencilFaceFlags face, uint32_t compareMask)
{
	_device->getVkBindings().vkCmdSetStencilCompareMask(getVkHandle(), static_cast<VkStencilFaceFlags>(face), compareMask);
}

void CommandBufferBase_::setStencilWriteMask(StencilFaceFlags face, uint32_t writeMask)
{
	_device->getVkBindings().vkCmdSetStencilWriteMask(getVkHandle(), static_cast<VkStencilFaceFlags>(face), writeMask);
}

void CommandBufferBase_::setStencilReference(StencilFaceFlags face, uint32_t ref)
{
	_device->getVkBindings().vkCmdSetStencilReference(getVkHandle(), static_cast<VkStencilFaceFlags>(face), ref);
}

void CommandBufferBase_::setDepthBias(float depthBias, float depthBiasClamp, float slopeScaledDepthBias)
{
	_device->getVkBindings().vkCmdSetDepthBias(getVkHandle(), depthBias, depthBiasClamp, slopeScaledDepthBias);
}

void CommandBufferBase_::setBlendConstants(float rgba[4])
{
	_device->getVkBindings().vkCmdSetBlendConstants(getVkHandle(), rgba);
}

void CommandBufferBase_::setLineWidth(float lineWidth)
{
	_device->getVkBindings().vkCmdSetLineWidth(getVkHandle(), lineWidth);
}

inline void clearcolorimage(const DeviceWeakPtr& device, VkCommandBuffer buffer, const ImageView& image, ClearColorValue clearColor, const uint32_t* baseMipLevel,
	const uint32_t* numLevels, const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges, ImageLayout layout)
{
	assertion(layout == ImageLayout::e_GENERAL || layout == ImageLayout::e_TRANSFER_DST_OPTIMAL);

	assertion(numRanges <= 10);

	VkImageSubresourceRange subResourceRanges[10];

	for (uint32_t i = 0; i < numRanges; ++i)
	{
		subResourceRanges[i].aspectMask = static_cast<VkImageAspectFlags>(ImageAspectFlags::e_COLOR_BIT);
		subResourceRanges[i].baseMipLevel = baseMipLevel[i];
		subResourceRanges[i].levelCount = numLevels[i];
		subResourceRanges[i].baseArrayLayer = baseArrayLayers[i];
		subResourceRanges[i].layerCount = numLayers[i];
	}

	device->getVkBindings().vkCmdClearColorImage(buffer, image->getImage()->getVkHandle(), static_cast<VkImageLayout>(layout), &clearColor.getColor(), numRanges, subResourceRanges);
}

void CommandBufferBase_::clearColorImage(const ImageView& image, const ClearColorValue& clearColor, ImageLayout currentLayout, const uint32_t baseMipLevel,
	const uint32_t numLevels, const uint32_t baseArrayLayer, const uint32_t numLayers)
{
	_objectReferences.push_back(image);
	clearcolorimage(_device, getVkHandle(), image, clearColor, &baseMipLevel, &numLevels, &baseArrayLayer, &numLayers, 1u, currentLayout);
}

void CommandBufferBase_::clearColorImage(const ImageView& image, const ClearColorValue& clearColor, ImageLayout layout, const uint32_t* baseMipLevel, const uint32_t* numLevels,
	const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges)
{
	_objectReferences.push_back(image);

	clearcolorimage(_device, getVkHandle(), image, clearColor, baseMipLevel, numLevels, baseArrayLayers, numLayers, numRanges, layout);
}

inline static void clearDepthStencilImageHelper(const DeviceWeakPtr& device, VkCommandBuffer nativeCommandBuffer, const Image& image, ImageLayout layout, ImageAspectFlags imageAspect,
	float clearDepth, uint32_t clearStencil, const uint32_t* baseMipLevel, const uint32_t* numLevels, const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges)
{
	if (!(layout == ImageLayout::e_GENERAL || layout == ImageLayout::e_TRANSFER_DST_OPTIMAL))
	{
		throw ErrorValidationFailedEXT("Cannot clear depth stencil image: It is in neither e_GENERAL or e_TRANSFER_DST_OPTIMAL layout");
	}

	VkClearDepthStencilValue clearDepthStencilValue;
	clearDepthStencilValue.depth = clearDepth;
	clearDepthStencilValue.stencil = clearStencil;

	VkImageSubresourceRange subResourceRanges[10];

	for (uint32_t i = 0; i < numRanges; ++i)
	{
		subResourceRanges[i].aspectMask = static_cast<VkImageAspectFlags>(imageAspect);
		subResourceRanges[i].baseMipLevel = baseMipLevel[i];
		subResourceRanges[i].levelCount = numLevels[i];
		subResourceRanges[i].baseArrayLayer = baseArrayLayers[i];
		subResourceRanges[i].layerCount = numLayers[i];
	}

	device->getVkBindings().vkCmdClearDepthStencilImage(
		nativeCommandBuffer, image->getVkHandle(), static_cast<VkImageLayout>(layout), &clearDepthStencilValue, numRanges, subResourceRanges);
}

void CommandBufferBase_::clearDepthImage(
	const Image& image, float clearDepth, const uint32_t baseMipLevel, const uint32_t numLevels, const uint32_t baseArrayLayer, const uint32_t numLayers, ImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(_device, getVkHandle(), image, layout, ImageAspectFlags::e_DEPTH_BIT, clearDepth, 0u, &baseMipLevel, &numLevels, &baseArrayLayer, &numLayers, 1u);
}

void CommandBufferBase_::clearDepthImage(const Image& image, float clearDepth, const uint32_t* baseMipLevel, const uint32_t* numLevels, const uint32_t* baseArrayLayers,
	const uint32_t* numLayers, uint32_t numRanges, ImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(_device, getVkHandle(), image, layout, ImageAspectFlags::e_DEPTH_BIT, clearDepth, 0u, baseMipLevel, numLevels, baseArrayLayers, numLayers, numRanges);
}

void CommandBufferBase_::clearStencilImage(
	const Image& image, uint32_t clearStencil, const uint32_t baseMipLevel, const uint32_t numLevels, const uint32_t baseArrayLayer, const uint32_t numLayers, ImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(
		_device, getVkHandle(), image, layout, ImageAspectFlags::e_STENCIL_BIT, 0.0f, clearStencil, &baseMipLevel, &numLevels, &baseArrayLayer, &numLayers, 1u);
}

void CommandBufferBase_::clearStencilImage(const Image& image, uint32_t clearStencil, const uint32_t* baseMipLevel, const uint32_t* numLevels, const uint32_t* baseArrayLayers,
	const uint32_t* numLayers, uint32_t numRanges, ImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(
		_device, getVkHandle(), image, layout, ImageAspectFlags::e_STENCIL_BIT, 0.0f, clearStencil, baseMipLevel, numLevels, baseArrayLayers, numLayers, numRanges);
}

void CommandBufferBase_::clearDepthStencilImage(const Image& image, float clearDepth, uint32_t clearStencil, const uint32_t baseMipLevel, const uint32_t numLevels,
	const uint32_t baseArrayLayer, const uint32_t numLayers, ImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(_device, getVkHandle(), image, layout, ImageAspectFlags::e_DEPTH_BIT | ImageAspectFlags::e_STENCIL_BIT, clearDepth, clearStencil, &baseMipLevel,
		&numLevels, &baseArrayLayer, &numLayers, 1u);
}

void CommandBufferBase_::clearDepthStencilImage(const Image& image, float clearDepth, uint32_t clearStencil, const uint32_t* baseMipLevel, const uint32_t* numLevels,
	const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges, ImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(_device, getVkHandle(), image, layout, ImageAspectFlags::e_DEPTH_BIT | ImageAspectFlags::e_STENCIL_BIT, clearDepth, clearStencil, baseMipLevel,
		numLevels, baseArrayLayers, numLayers, numRanges);
}

void CommandBufferBase_::clearAttachments(const uint32_t numAttachments, const ClearAttachment* clearAttachments, uint32_t numRectangles, const ClearRect* clearRectangles)
{
	VkClearAttachment vkClearAttachments[static_cast<uint32_t>(FrameworkCaps::MaxColorAttachments) + static_cast<uint32_t>(FrameworkCaps::MaxDepthStencilAttachments)];
	VkClearRect vkClearRectangles[10];
	for (uint32_t i = 0; i < numAttachments; ++i)
	{
		vkClearAttachments[i].aspectMask = static_cast<VkImageAspectFlags>(ImageAspectFlags::e_COLOR_BIT);
		memcpy(&vkClearAttachments[i], &clearAttachments[i], sizeof(clearAttachments[i]));
	}

	for (uint32_t i = 0; i < numRectangles; ++i)
	{
		vkClearRectangles[i].baseArrayLayer = clearRectangles[i].getBaseArrayLayer();
		vkClearRectangles[i].layerCount = clearRectangles[i].getLayerCount();
		vkClearRectangles[i].rect.offset.x = clearRectangles[i].getRect().getOffset().getX();
		vkClearRectangles[i].rect.offset.y = clearRectangles[i].getRect().getOffset().getY();
		vkClearRectangles[i].rect.extent.width = clearRectangles[i].getRect().getExtent().getWidth();
		vkClearRectangles[i].rect.extent.height = clearRectangles[i].getRect().getExtent().getHeight();
	}

	_device->getVkBindings().vkCmdClearAttachments(getVkHandle(), numAttachments, vkClearAttachments, numRectangles, vkClearRectangles);
}

// drawing commands
void CommandBufferBase_::drawIndexed(uint32_t firstIndex, uint32_t numIndices, uint32_t vertexOffset, uint32_t firstInstance, uint32_t numInstances)
{
	_device->getVkBindings().vkCmdDrawIndexed(getVkHandle(), numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void CommandBufferBase_::draw(uint32_t firstVertex, uint32_t numVertices, uint32_t firstInstance, uint32_t numInstances)
{
	_device->getVkBindings().vkCmdDraw(getVkHandle(), numVertices, numInstances, firstVertex, firstInstance);
}

void CommandBufferBase_::drawIndexedIndirect(const Buffer& buffer, uint32_t offset, uint32_t count, uint32_t stride)
{
	_objectReferences.push_back(buffer);
	_device->getVkBindings().vkCmdDrawIndexedIndirect(getVkHandle(), buffer->getVkHandle(), offset, count, stride);
}

void CommandBufferBase_::drawIndirect(const Buffer& buffer, uint32_t offset, uint32_t count, uint32_t stride)
{
	_objectReferences.push_back(buffer);
	_device->getVkBindings().vkCmdDrawIndirect(getVkHandle(), buffer->getVkHandle(), offset, count, stride);
}

void CommandBufferBase_::dispatch(uint32_t numGroupX, uint32_t numGroupY, uint32_t numGroupZ)
{
	_device->getVkBindings().vkCmdDispatch(getVkHandle(), numGroupX, numGroupY, numGroupZ);
}

void CommandBufferBase_::dispatchIndirect(Buffer& buffer, uint32_t offset)
{
	_device->getVkBindings().vkCmdDispatchIndirect(getVkHandle(), buffer->getVkHandle(), offset);
}

void CommandBufferBase_::resetQueryPool(QueryPool& queryPool, uint32_t firstQuery, uint32_t queryCount)
{
	_objectReferences.push_back(queryPool);
	debug_assertion(firstQuery + queryCount <= queryPool->getNumQueries(), "Attempted to reset a query with index larger than the number of queries available to the QueryPool");

	_device->getVkBindings().vkCmdResetQueryPool(getVkHandle(), queryPool->getVkHandle(), firstQuery, queryCount);
}

void CommandBufferBase_::resetQueryPool(QueryPool& queryPool, uint32_t queryIndex)
{
	_objectReferences.push_back(queryPool);
	resetQueryPool(queryPool, queryIndex, 1);
}

void CommandBufferBase_::beginQuery(QueryPool& queryPool, uint32_t queryIndex, QueryControlFlags flags)
{
	if (queryIndex >= queryPool->getNumQueries())
	{
		throw ErrorValidationFailedEXT("Attempted to begin a query with index larger than the number of queries available to the QueryPool");
	}
	_objectReferences.push_back(queryPool);
	_device->getVkBindings().vkCmdBeginQuery(getVkHandle(), queryPool->getVkHandle(), queryIndex, static_cast<VkQueryControlFlags>(flags));
}

void CommandBufferBase_::endQuery(QueryPool& queryPool, uint32_t queryIndex)
{
	if (queryIndex >= queryPool->getNumQueries())
	{
		throw ErrorValidationFailedEXT("Attempted to end a query with index larger than the number of queries available to the QueryPool");
	}
	_objectReferences.push_back(queryPool);
	_device->getVkBindings().vkCmdEndQuery(getVkHandle(), queryPool->getVkHandle(), queryIndex);
}

void CommandBufferBase_::copyQueryPoolResults(
	QueryPool& queryPool, uint32_t firstQuery, uint32_t queryCount, Buffer& dstBuffer, VkDeviceSize offset, VkDeviceSize stride, QueryResultFlags flags)
{
	if (firstQuery + queryCount >= queryPool->getNumQueries())
	{
		throw ErrorValidationFailedEXT("Attempted to copy query results with index larger than the number of queries available to the QueryPool");
	}
	_objectReferences.push_back(queryPool);
	_device->getVkBindings().vkCmdCopyQueryPoolResults(
		getVkHandle(), queryPool->getVkHandle(), firstQuery, queryCount, dstBuffer->getVkHandle(), offset, stride, static_cast<VkQueryControlFlags>(flags));
}

void CommandBufferBase_::writeTimestamp(QueryPool& queryPool, uint32_t queryIndex, PipelineStageFlags pipelineStage)
{
	if (queryIndex >= queryPool->getNumQueries())
	{
		throw ErrorValidationFailedEXT("Attempted to write a timestamp for a with index larger than the number of queries available to the QueryPool");
	}
	_objectReferences.push_back(queryPool);
	_device->getVkBindings().vkCmdWriteTimestamp(getVkHandle(), static_cast<VkPipelineStageFlagBits>(pipelineStage), queryPool->getVkHandle(), queryIndex);
}
} // namespace impl
} // namespace pvrvk
//!\endcond
