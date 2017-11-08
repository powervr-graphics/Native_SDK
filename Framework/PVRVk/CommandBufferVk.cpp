/*!
\brief Function implementations for the Command Buffer class
\file PVRVk/CommandBufferVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/GraphicsPipelineVk.h"
#include "PVRVk/ComputePipelineVk.h"
#include "PVRVk/BindingsVk.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/RenderPassVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/SyncVk.h"
#include "PVRVk/FramebufferVk.h"

namespace pvrvk {
namespace impl {
inline void copyRectangleToVulkan(const Rect2Di& renderArea, VkRect2D&  vulkanRenderArea)
{
	memcpy(&vulkanRenderArea, &renderArea, sizeof(renderArea));
}

CommandBufferBase_::~CommandBufferBase_()
{
	if (_device.isValid())
	{
		if (_vkCmdBuffer != VK_NULL_HANDLE)
		{
			if (_pool.isValid())
			{
				vk::FreeCommandBuffers(_device->getNativeObject(), _pool->getNativeObject(), 1, &_vkCmdBuffer);
			}
			else
			{
				Log(LogLevel::Debug, "Trying to release a Command buffer AFTER its pool was destroyed");
			}
			_vkCmdBuffer = VK_NULL_HANDLE;
		}
	}
	else
	{
		Log(LogLevel::Warning, "WARNING - Command buffer released AFTER its context was destroyed.");
	}
}

//synchronization
VkMemoryBarrier memoryBarrier(const MemoryBarrier& memBarrier)
{
	VkMemoryBarrier barrier = {};
	barrier.sType = VkStructureType::e_MEMORY_BARRIER;
	barrier.srcAccessMask = memBarrier.srcMask;
	barrier.dstAccessMask = memBarrier.dstMask;
	return barrier;
}

VkBufferMemoryBarrier bufferBarrier(const BufferMemoryBarrier& buffBarrier)
{
	VkBufferMemoryBarrier barrier;
	barrier.sType = VkStructureType::e_BUFFER_MEMORY_BARRIER;
	barrier.pNext = 0;
	barrier.srcAccessMask = buffBarrier.srcMask;
	barrier.dstAccessMask = buffBarrier.dstMask;

	barrier.dstQueueFamilyIndex = static_cast<uint32_t>(-1);
	barrier.srcQueueFamilyIndex = static_cast<uint32_t>(-1);

	barrier.buffer = buffBarrier.buffer->getNativeObject();
	barrier.offset = buffBarrier.offset;
	barrier.size = buffBarrier.size;
	return barrier;
}

VkImageMemoryBarrier imageBarrier(const ImageMemoryBarrier& imgBarrier)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VkStructureType::e_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = imgBarrier.srcAccessMask;
	barrier.dstAccessMask = imgBarrier.dstAccessMask;

	barrier.srcQueueFamilyIndex = imgBarrier.srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = imgBarrier.dstQueueFamilyIndex;

	barrier.image = imgBarrier.image->getNativeObject();
	barrier.newLayout = imgBarrier.newLayout;
	barrier.oldLayout = imgBarrier.oldLayout;
	barrier.subresourceRange = convertToVk(imgBarrier.subresourceRange);
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

void CommandBufferBase_::pipelineBarrier(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, const MemoryBarrierSet& barriers, bool dependencyByRegion)
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

	vk::CmdPipelineBarrier(_vkCmdBuffer, srcStage, dstStage,
	                       VkDependencyFlags::e_BY_REGION_BIT * (dependencyByRegion != 0),
	                       memcnt, memptr, bufcnt, bufptr, imgcnt, imgptr);

	if (memptr != mem) { delete memptr; }
	if (imgptr != img) { delete imgptr; }
	if (bufptr != buf) { delete bufptr; }
}

void CommandBufferBase_::waitForEvent(const Event& event, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
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

	vk::CmdWaitEvents(_vkCmdBuffer, 1, &event->getNativeObject(),
	                  srcStage, dstStage, memcnt, memptr, bufcnt,
	                  bufptr, imgcnt, imgptr);

	if (memptr != mem) { delete memptr; }
	if (imgptr != img) { delete imgptr; }
	if (bufptr != buf) { delete bufptr; }
}

void CommandBufferBase_::waitForEvents(const Event* events, uint32_t numEvents, VkPipelineStageFlags srcStage,
                                       VkPipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
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

	std::vector<VkEvent> vkEvents(numEvents);
	for (uint32_t i = 0; i < numEvents; ++i)
	{
		vkEvents[i] = events[i]->getNativeObject();
	}

	vk::CmdWaitEvents(_vkCmdBuffer, numEvents, vkEvents.data(),
	                  srcStage, dstStage, memcnt, memptr,
	                  bufcnt, bufptr, imgcnt, imgptr);

	if (memptr != mem) { delete memptr; }
	if (imgptr != img) { delete imgptr; }
	if (bufptr != buf) { delete bufptr; }
}

//bind pipelines, sets, vertex/index buffers
void CommandBufferBase_::bindDescriptorSets(VkPipelineBindPoint bindingPoint, const PipelineLayout& pipelineLayout,
    uint32_t firstSet, const DescriptorSet* sets, uint32_t numDescriptorSets, const uint32_t* dynamicOffsets, uint32_t numDynamicOffsets)
{
	debug_assertion(numDescriptorSets < static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSets),
	                "Attempted to bind more than 8 descriptor sets");
	if (numDescriptorSets < static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSets))
	{
		VkDescriptorSet native_sets[static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSets)] = { VK_NULL_HANDLE };
		for (uint32_t i = 0; i < numDescriptorSets; ++i)
		{
			_objectReferences.push_back(sets[i]);
			native_sets[i] = sets[i]->getNativeObject();
		}
		vk::CmdBindDescriptorSets(_vkCmdBuffer, bindingPoint,
		                          pipelineLayout->getNativeObject(), firstSet, numDescriptorSets, native_sets,
		                          numDynamicOffsets, dynamicOffsets);
	}
}

void CommandBufferBase_::bindVertexBuffer(Buffer const* buffers, uint32_t* offsets, uint16_t numBuffers, uint16_t startBinding, uint16_t numBindings)
{
	if (numBuffers <= 8)
	{
		_objectReferences.push_back(buffers[numBuffers]);
		VkBuffer buff[8];
		VkDeviceSize sizes[8];
		for (int i = 0; i < numBuffers; ++i)
		{
			_objectReferences.push_back(buffers[i]);
			buff[i] = buffers[i]->getNativeObject();
			sizes[i] = offsets[i];
		}
		vk::CmdBindVertexBuffers(_vkCmdBuffer, startBinding, numBindings, buff, sizes);
	}
	else
	{
		VkBuffer* buff = new VkBuffer[numBuffers];
		VkDeviceSize* sizes = new VkDeviceSize[numBuffers];
		for (int i = 0; i < numBuffers; ++i)
		{
			_objectReferences.push_back(buffers[i]);
			buff[i] = buffers[i]->getNativeObject();
			sizes[i] = offsets[i];
		}
		vk::CmdBindVertexBuffers(_vkCmdBuffer, startBinding, numBindings, buff, sizes);
		delete[] buff;
		delete[] sizes;
	}
}

//begin end submit clear reset etc.
void CommandBufferBase_::begin(const VkCommandBufferUsageFlags flags)
{
	if (_isRecording)
	{
		Log("Called CommandBuffer::begin while a recording was already in progress. Call CommandBuffer::end first");
		assertion(0);
	}
	reset(VkCommandBufferResetFlags(0));
	_isRecording = true;
	VkCommandBufferBeginInfo info = {};
	info.sType = VkStructureType::e_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = NULL;
	info.flags = flags;
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VkStructureType::e_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = VK_NULL_HANDLE;
	inheritanceInfo.framebuffer = VK_NULL_HANDLE;
	inheritanceInfo.subpass = static_cast<uint32_t>(-1);
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.queryFlags = (VkQueryControlFlags)0;
	inheritanceInfo.pipelineStatistics = (VkQueryPipelineStatisticFlags)0;
	info.pInheritanceInfo = &inheritanceInfo;
	vkThrowIfFailed(vk::BeginCommandBuffer(_vkCmdBuffer, &info), "CommandBuffer::begin(void) failed");
}

void SecondaryCommandBuffer_::begin(const Framebuffer& framebuffer, uint32_t subpass, const VkCommandBufferUsageFlags flags)
{
	if (_isRecording)
	{
		Log("Called CommandBuffer::begin while a recording was "
		    "already in progress. Call CommandBuffer::end first");
		assertion(0);
	}
	reset(VkCommandBufferResetFlags(0));
	_objectReferences.push_back(framebuffer);
	_isRecording = true;
	VkCommandBufferBeginInfo info = {};
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	info.sType = VkStructureType::e_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = flags;
	inheritanceInfo.sType = VkStructureType::e_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = framebuffer->getRenderPass()->getNativeObject();
	inheritanceInfo.framebuffer = framebuffer->getNativeObject();
	inheritanceInfo.subpass = subpass;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	info.pInheritanceInfo = &inheritanceInfo;
	vkThrowIfFailed(vk::BeginCommandBuffer(_vkCmdBuffer, &info), "CommandBufferBase::begin(framebuffer, [subpass]) failed");
}

void SecondaryCommandBuffer_::begin(const RenderPass& renderPass, uint32_t subpass, const VkCommandBufferUsageFlags flags)
{
	if (_isRecording)
	{
		Log("Called CommandBuffer::begin while a recording was already"
		    " in progress. Call CommandBuffer::end first");
		assertion(0);
	}
	reset(VkCommandBufferResetFlags(0));
	_objectReferences.push_back(renderPass);
	_isRecording = true;
	VkCommandBufferBeginInfo info = {};
	VkCommandBufferInheritanceInfo inheritInfo = {};
	info.sType = VkStructureType::e_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = flags;
	inheritInfo.sType = VkStructureType::e_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritInfo.renderPass = renderPass->getNativeObject();
	inheritInfo.subpass = subpass;
	inheritInfo.occlusionQueryEnable = VK_FALSE;
	info.pInheritanceInfo = &inheritInfo;
	vkThrowIfFailed(vk::BeginCommandBuffer(_vkCmdBuffer, &info), "CommandBufferBase::begin(renderpass, [subpass]) failed");
}

void CommandBufferBase_::end()
{
	if (!_isRecording)
	{
		Log("Called CommandBuffer::end while a recording "
		    "was not in progress. Call CommandBuffer::begin first");
		assertion(0);
	}
	_isRecording = false;
	vkThrowIfFailed(vk::EndCommandBuffer(_vkCmdBuffer), "CommandBufferBase::end failed");
}

void CommandBuffer_::executeCommands(SecondaryCommandBuffer& secondaryCmdBuffer)
{
	_objectReferences.push_back(secondaryCmdBuffer);
	assertion(secondaryCmdBuffer.isValid());
	vk::CmdExecuteCommands(_vkCmdBuffer, 1, &secondaryCmdBuffer->getNativeObject());
}

void CommandBuffer_::executeCommands(SecondaryCommandBuffer* secondaryCmdBuffers, uint32_t numCommandBuffers)
{
	std::vector<VkCommandBuffer> cmdBuffsHeap;
	VkCommandBuffer cmdBuffsStack[32];
	VkCommandBuffer* cmdBuffs = cmdBuffsStack;
	if (numCommandBuffers > 32)
	{
		cmdBuffsHeap.resize(numCommandBuffers);
		cmdBuffs = cmdBuffsHeap.data();
	}
	for (uint32_t i = 0; i < numCommandBuffers; ++i)
	{
		_objectReferences.push_back(secondaryCmdBuffers[i]);
		cmdBuffs[i] = secondaryCmdBuffers[i]->getNativeObject();
	}

	vk::CmdExecuteCommands(_vkCmdBuffer, numCommandBuffers, cmdBuffs);
}

// Renderpasses, Subpasses
void CommandBuffer_::beginRenderPass(const Framebuffer& framebuffer, const RenderPass& renderPass, const Rect2Di& renderArea,
                                     bool inlineFirstSubpass, const ClearValue* clearValues, uint32_t numClearValues)
{
	_objectReferences.push_back(framebuffer);
	VkRenderPassBeginInfo nfo = {};
	nfo.sType = VkStructureType::e_RENDER_PASS_BEGIN_INFO;
	nfo.pClearValues = (VkClearValue*)clearValues;
	nfo.clearValueCount = numClearValues;
	nfo.framebuffer = framebuffer->getNativeObject();
	copyRectangleToVulkan(renderArea, nfo.renderArea);
	nfo.renderPass = renderPass->getNativeObject();

	vk::CmdBeginRenderPass(_vkCmdBuffer, &nfo, inlineFirstSubpass ? VkSubpassContents::e_INLINE : VkSubpassContents::e_SECONDARY_COMMAND_BUFFERS);
}

void CommandBuffer_::beginRenderPass(const Framebuffer& framebuffer, const Rect2Di& renderArea, bool inlineFirstSubpass, const ClearValue* clearValues, uint32_t numClearValues)
{
	beginRenderPass(framebuffer, framebuffer->getRenderPass(), renderArea, inlineFirstSubpass, clearValues, numClearValues);
}

void CommandBuffer_::beginRenderPass(const Framebuffer& framebuffer, bool inlineFirstSubpass, const ClearValue* clearValues, uint32_t numClearValues)
{
	beginRenderPass(framebuffer, framebuffer->getRenderPass(), Rect2Di(0, 0, framebuffer->getDimensions().width, framebuffer->getDimensions().height),
	                inlineFirstSubpass, clearValues, numClearValues);
}

//buffers, textures, images, push constants
void CommandBufferBase_::updateBuffer(Buffer& buffer, const void* data, uint32_t offset, uint32_t length)
{
	_objectReferences.push_back(buffer);
	vk::CmdUpdateBuffer(_vkCmdBuffer, buffer->getNativeObject(), offset, length, (const uint32_t*)data);
}

void CommandBufferBase_::pushConstants(PipelineLayout& pipelineLayout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data)
{
	vk::CmdPushConstants(_vkCmdBuffer, pipelineLayout->getNativeObject(), stageFlags, offset, size, data);
}

void CommandBufferBase_::resolveImage(Image& srcImage, Image& dstImage, const ImageResolve* regions,
                                      uint32_t numRegions, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
	_objectReferences.push_back(srcImage);
	_objectReferences.push_back(dstImage);
	assert(sizeof(ImageResolve) == sizeof(VkImageResolve));
	vk::CmdResolveImage(_vkCmdBuffer, srcImage->getNativeObject(), srcLayout, dstImage->getNativeObject(), dstLayout,
	                    numRegions, reinterpret_cast<const VkImageResolve*>(regions));
}

void CommandBufferBase_::blitImage(const Image& src, pvrvk::Image& dst, const ImageBlitRange* regions, uint32_t numRegions,
                                   VkFilter filter, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
	_objectReferences.push_back(src);
	_objectReferences.push_back(dst);
	std::vector<VkImageBlit> imageBlits(numRegions);
	for (uint32_t i = 0; i < numRegions; ++i) { imageBlits[i] = regions[i]; }

	vk::CmdBlitImage(_vkCmdBuffer, src->getNativeObject(),
	                 srcLayout,
	                 dst->getNativeObject(), dstLayout,
	                 numRegions, imageBlits.data(), filter);
}

void CommandBufferBase_::copyImage(Image& srcImage, Image& dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout,
                                   uint32_t numRegions, const ImageCopy* regions)
{
	enum { num_regions = 10 };
	// Try to avoid heap allocation
	VkImageCopy regionsArray[num_regions] = {};
	VkImageCopy* pRegions = regionsArray;
	std::vector<VkImageCopy> regionsVec(0);
	if (numRegions > num_regions)
	{
		regionsVec.resize(numRegions);
		pRegions = regionsVec.data();
	}

	for (uint32_t i = 0; i < numRegions; ++i)
	{
		pRegions[i] = convertToVk(regions[i]);
	}

	vk::CmdCopyImage(_vkCmdBuffer, srcImage->getNativeObject(), srcImageLayout, dstImage->getNativeObject(), dstImageLayout, numRegions, pRegions);
}

void CommandBufferBase_::copyImageToBuffer(Image& srcImage, VkImageLayout srcImageLayout, Buffer& dstBuffer, const BufferImageCopy* regions, uint32_t numRegions)
{
	enum { num_regions = 10 };
	// Try to avoid heap allocation
	VkBufferImageCopy regionsArray[num_regions] = {};
	VkBufferImageCopy* pRegions = regionsArray;
	std::vector<VkBufferImageCopy> regionsVec(0);
	if (numRegions > num_regions)
	{
		regionsVec.resize(numRegions);
		pRegions = regionsVec.data();
	}

	for (uint32_t i = 0; i < numRegions; ++i)
	{
		pRegions[i] = convertToVk(regions[i]);
	}

	vk::CmdCopyImageToBuffer(_vkCmdBuffer, srcImage->getNativeObject(), srcImageLayout, dstBuffer->getNativeObject(), numRegions, pRegions);
}

void CommandBufferBase_::copyBuffer(Buffer src, Buffer dst, uint32_t srcOffset, uint32_t dstOffset, uint32_t sizeInBytes)
{
	_objectReferences.push_back(src);
	_objectReferences.push_back(dst);
	VkBufferCopy region;
	region.srcOffset = srcOffset;
	region.dstOffset = dstOffset;
	region.size = sizeInBytes;
	vk::CmdCopyBuffer(_vkCmdBuffer, src->getNativeObject(), dst->getNativeObject(), 1, &region);
}

void CommandBufferBase_::copyBufferToImage(const Buffer& buffer, Image& image, VkImageLayout dstImageLayout, uint32_t regionsCount, const BufferImageCopy* regions)
{
	std::vector<VkBufferImageCopy> bufferImageCopy(regionsCount);
	for (uint32_t i = 0; i < regionsCount; ++i)
	{
		bufferImageCopy[i] = impl::convertToVk(regions[i]);
	}
	vk::CmdCopyBufferToImage(getNativeObject(), buffer->getNativeObject(), image->getNativeObject(), dstImageLayout, regionsCount, bufferImageCopy.data());
}

void CommandBufferBase_::fillBuffer(Buffer dstBuffer, uint32_t dstOffset, uint32_t data, uint64_t size)
{
	vk::CmdFillBuffer(getNativeObject(), dstBuffer->getNativeObject(), dstOffset, size, data);
}

//dynamic commands
void CommandBufferBase_::setViewport(const Viewport& viewport)
{
	vk::CmdSetViewport(_vkCmdBuffer, 0, 1, (VkViewport*)&viewport);
}

void CommandBufferBase_::setScissor(uint32_t firstScissor, uint32_t numScissors, const Rect2Di* scissors)
{
	vk::CmdSetScissor(_vkCmdBuffer, firstScissor, numScissors, scissors);
}

void CommandBufferBase_::setDepthBounds(float min, float max)
{
	vk::CmdSetDepthBounds(_vkCmdBuffer, min, max);
}

void CommandBufferBase_::setStencilCompareMask(VkStencilFaceFlags face, uint32_t compareMask)
{
	vk::CmdSetStencilCompareMask(_vkCmdBuffer, face, compareMask);
}

void CommandBufferBase_::setStencilWriteMask(VkStencilFaceFlags face, uint32_t writeMask)
{
	vk::CmdSetStencilWriteMask(_vkCmdBuffer, face, writeMask);
}

void CommandBufferBase_::setStencilReference(VkStencilFaceFlags face, uint32_t ref)
{
	vk::CmdSetStencilReference(_vkCmdBuffer, face, ref);
}

void CommandBufferBase_::setDepthBias(float depthBias, float depthBiasClamp, float slopeScaledDepthBias)
{
	vk::CmdSetDepthBias(_vkCmdBuffer, depthBias, depthBiasClamp, slopeScaledDepthBias);
}

void CommandBufferBase_::setBlendConstants(float rgba[4])
{
	vk::CmdSetBlendConstants(_vkCmdBuffer, rgba);
}

void CommandBufferBase_::setLineWidth(float lineWidth)
{
	vk::CmdSetLineWidth(_vkCmdBuffer, lineWidth);
}

inline void clearcolorimage(VkCommandBuffer buffer, ImageView& image, ClearColorValue clearColor, const uint32_t* baseMipLevel,
                            const uint32_t* numLevels, const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges, VkImageLayout layout)
{
	assertion(layout == VkImageLayout::e_GENERAL || layout == VkImageLayout::e_TRANSFER_DST_OPTIMAL);

	assertion(numRanges <= 10);

	VkImageSubresourceRange subResourceRanges[10];

	for (uint32_t i = 0; i < numRanges; ++i)
	{
		subResourceRanges[i].aspectMask = VkImageAspectFlags::e_COLOR_BIT;
		subResourceRanges[i].baseMipLevel = baseMipLevel[i];
		subResourceRanges[i].levelCount = numLevels[i];
		subResourceRanges[i].baseArrayLayer = baseArrayLayers[i];
		subResourceRanges[i].layerCount = numLayers[i];
	}

	vk::CmdClearColorImage(buffer, image->getImage()->getNativeObject(), layout, &clearColor.color, numRanges, subResourceRanges);
}

void CommandBufferBase_::clearColorImage(ImageView& image, const ClearColorValue& clearColor, VkImageLayout currentLayout,
    const uint32_t baseMipLevel, const uint32_t numLevels, const uint32_t baseArrayLayer, const uint32_t numLayers)
{
	_objectReferences.push_back(image);
	clearcolorimage(_vkCmdBuffer, image, clearColor, &baseMipLevel, &numLevels, &baseArrayLayer, &numLayers, 1u, currentLayout);
}

void CommandBufferBase_::clearColorImage(ImageView& image, const ClearColorValue& clearColor, VkImageLayout layout, const uint32_t* baseMipLevel,
    const uint32_t* numLevels, const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges)
{
	_objectReferences.push_back(image);

	clearcolorimage(_vkCmdBuffer, image, clearColor, baseMipLevel, numLevels, baseArrayLayers, numLayers, numRanges, layout);
}

inline static void clearDepthStencilImageHelper(VkCommandBuffer nativeCommandBuffer, Image& image, VkImageLayout layout, VkImageAspectFlags imageAspect, float clearDepth,
    uint32_t clearStencil, const uint32_t* baseMipLevel, const uint32_t* numLevels, const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges)
{
	assertion(layout == VkImageLayout::e_GENERAL || layout == VkImageLayout::e_TRANSFER_DST_OPTIMAL);

	VkClearDepthStencilValue clearDepthStencilValue;
	clearDepthStencilValue.depth = clearDepth;
	clearDepthStencilValue.stencil = clearStencil;

	VkImageSubresourceRange subResourceRanges[10];

	for (uint32_t i = 0; i < numRanges; ++i)
	{
		subResourceRanges[i].aspectMask = imageAspect;
		subResourceRanges[i].baseMipLevel = baseMipLevel[i];
		subResourceRanges[i].levelCount = numLevels[i];
		subResourceRanges[i].baseArrayLayer = baseArrayLayers[i];
		subResourceRanges[i].layerCount = numLayers[i];
	}

	vk::CmdClearDepthStencilImage(nativeCommandBuffer, image->getNativeObject(), layout, &clearDepthStencilValue, numRanges, subResourceRanges);
}

void CommandBufferBase_::clearDepthImage(Image& image, float clearDepth, const uint32_t baseMipLevel,
    const uint32_t numLevels, const uint32_t baseArrayLayer, const uint32_t numLayers, VkImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(
	  _vkCmdBuffer, image, layout, VkImageAspectFlags::e_DEPTH_BIT, clearDepth, 0u,
	  &baseMipLevel, &numLevels, &baseArrayLayer, &numLayers, 1u);
}

void CommandBufferBase_::clearDepthImage(pvrvk::Image& image, float clearDepth, const uint32_t* baseMipLevel,
    const uint32_t* numLevels, const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges, VkImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(_vkCmdBuffer, image, layout, VkImageAspectFlags::e_DEPTH_BIT, clearDepth, 0u,
	                             baseMipLevel, numLevels, baseArrayLayers, numLayers, numRanges);
}

void CommandBufferBase_::clearStencilImage(Image& image, uint32_t clearStencil, const uint32_t baseMipLevel, const uint32_t numLevels,
    const uint32_t baseArrayLayer, const uint32_t numLayers, VkImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(_vkCmdBuffer, image, layout, VkImageAspectFlags::e_STENCIL_BIT, 0.0f, clearStencil,
	                             &baseMipLevel, &numLevels, &baseArrayLayer, &numLayers, 1u);
}

void CommandBufferBase_::clearStencilImage(Image& image, uint32_t clearStencil, const uint32_t* baseMipLevel, const uint32_t* numLevels, const uint32_t* baseArrayLayers,
    const uint32_t* numLayers, uint32_t numRanges, VkImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(_vkCmdBuffer, image, layout, VkImageAspectFlags::e_STENCIL_BIT, 0.0f, clearStencil,
	                             baseMipLevel, numLevels, baseArrayLayers, numLayers, numRanges);
}

void CommandBufferBase_::clearDepthStencilImage(Image& image, float clearDepth, uint32_t clearStencil, const uint32_t baseMipLevel, const uint32_t numLevels,
    const uint32_t baseArrayLayer, const uint32_t numLayers, VkImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(_vkCmdBuffer, image, layout, VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT,
	                             clearDepth, clearStencil, &baseMipLevel, &numLevels, &baseArrayLayer, &numLayers, 1u);
}

void CommandBufferBase_::clearDepthStencilImage(Image& image, float clearDepth, uint32_t clearStencil, const uint32_t* baseMipLevel, const uint32_t* numLevels,
    const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges, VkImageLayout layout)
{
	_objectReferences.push_back(image);
	clearDepthStencilImageHelper(_vkCmdBuffer, image, layout, VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT, clearDepth, clearStencil,
	                             baseMipLevel, numLevels, baseArrayLayers, numLayers, numRanges);
}

void CommandBufferBase_::clearAttachments(const uint32_t numAttachments, const ClearAttachment* clearAttachments, uint32_t numRectangles, const ClearRect* clearRectangles)
{
	VkClearAttachment vkClearAttachments[static_cast<uint32_t>(FrameworkCaps::MaxColorAttachments) + static_cast<uint32_t>(FrameworkCaps::MaxDepthStencilAttachments)];
	VkClearRect vkClearRectangles[10];
	for (uint32_t i = 0; i < numAttachments; ++i)
	{
		vkClearAttachments[i].aspectMask = VkImageAspectFlags::e_COLOR_BIT;
		memcpy(&vkClearAttachments[i], &clearAttachments[i], sizeof(clearAttachments[i]));
	}

	for (uint32_t i = 0; i < numRectangles; ++i)
	{
		vkClearRectangles[i].baseArrayLayer = clearRectangles[i].baseArrayLayer;
		vkClearRectangles[i].layerCount = clearRectangles[i].layerCount;
		vkClearRectangles[i].rect.offset.x = clearRectangles[i].rect.offset.x;
		vkClearRectangles[i].rect.offset.y = clearRectangles[i].rect.offset.y;
		vkClearRectangles[i].rect.extent.width = clearRectangles[i].rect.extent.width;
		vkClearRectangles[i].rect.extent.height = clearRectangles[i].rect.extent.height;
	}

	vk::CmdClearAttachments(getNativeObject(), numAttachments, vkClearAttachments, numRectangles, vkClearRectangles);
}

// drawing commands
void CommandBufferBase_::drawIndexed(uint32_t firstIndex, uint32_t numIndices, uint32_t vertexOffset, uint32_t firstInstance, uint32_t numInstances)
{
	vk::CmdDrawIndexed(_vkCmdBuffer, numIndices, numInstances,
	                   firstIndex, vertexOffset, firstInstance);
}

void CommandBufferBase_::draw(uint32_t firstVertex, uint32_t numVertices, uint32_t firstInstance, uint32_t numInstances)
{
	vk::CmdDraw(_vkCmdBuffer, numVertices, numInstances, firstVertex, firstInstance);
}

void CommandBufferBase_::drawIndexedIndirect(Buffer& buffer, uint32_t offset, uint32_t count, uint32_t stride)
{
	_objectReferences.push_back(buffer);
	vk::CmdDrawIndexedIndirect(_vkCmdBuffer, buffer->getNativeObject(), offset, count, stride);
}

void CommandBufferBase_::drawIndirect(Buffer& buffer, uint32_t offset, uint32_t count, uint32_t stride)
{
	_objectReferences.push_back(buffer);
	vk::CmdDrawIndirect(_vkCmdBuffer, buffer->getNativeObject(), offset, count, stride);
}

void CommandBufferBase_::dispatch(uint32_t numGroupX, uint32_t numGroupY, uint32_t numGroupZ)
{
	vk::CmdDispatch(_vkCmdBuffer, numGroupX, numGroupY, numGroupZ);
}

void CommandBufferBase_::dispatchIndirect(Buffer& buffer, uint32_t offset)
{
	vk::CmdDispatchIndirect(_vkCmdBuffer, buffer->getNativeObject(), offset);
}

}// impl
}// namespace pvrvk
//!\endcond
