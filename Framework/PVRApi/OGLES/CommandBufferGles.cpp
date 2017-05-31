/*!
\brief OpenGL ES Implementation details CommandBuffer class.
\file PVRApi/OGLES/CommandBufferGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/CommandBufferGles.h"
#include "PVRApi/OGLES/ComputePipelineGles.h"
#include "PVRApi/OGLES/GraphicsPipelineGles.h"
#include "PVRApi/ApiObjects/Sync.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRApi/OGLES/TextureGles.h"

namespace pvr {
namespace api {
namespace impl {
//Special internal class used by the CommandBuffer. It packages an API object that can be bound.
template <typename Resource>
class PackagedBindable : public ApiCommand
{
public:
	Resource res;
	PackagedBindable(const Resource& res) : res(res) {}

	void execute_private(CommandBufferBase_& cmdBuf);
	virtual ~PackagedBindable() { }
};

//Special internal class used by the CommandBuffer. It packages an API object that can be bound.
template <>
class PackagedBindable<GraphicsPipeline> : public ApiCommand
{
public:
	GraphicsPipeline res;
	PackagedBindable(const GraphicsPipeline& res) : res(res) {}

	void execute_private(CommandBufferBase_& cmdBuf);
	virtual ~PackagedBindable() { }
};
template <>
class PackagedBindable<ComputePipeline> : public ApiCommand
{
public:
	ComputePipeline res;
	PackagedBindable(const ComputePipeline& res) : res(res) {}

	void execute_private(CommandBufferBase_& cmdBuf);
	virtual ~PackagedBindable() { }
};

class SecondaryCommandBufferPackager : public ApiCommand
{
public:
	RefCountedResource<SecondaryCommandBuffer_> me;
	SecondaryCommandBufferPackager(const RefCountedResource<SecondaryCommandBuffer_>& me)
	{
		this->me = me;
	}
	SecondaryCommandBufferPackager& operator=(const RefCountedResource<SecondaryCommandBuffer_>& me)
	{
		this->me = me; return *this;
	}
	void execute_private(CommandBufferBase_& cmdBuf) { static_cast<CommandBufferImplGles_&>(*me->pimpl).submit_(); }
	virtual ~SecondaryCommandBufferPackager() { }
};

//Special internal class used by the CommandBuffer. It packages an API object that can be bound taking a single
//       parameter (usually, a binding point).
template <typename Resource, typename ParamType>
class PackagedBindableWithParam : public ApiCommand
{
public:
	Resource res;
	ParamType param;
	PackagedBindableWithParam(const Resource& res, const ParamType& param) : res(res),
		param(param) { }

	void execute_private(CommandBufferBase_& cmdBuf);
	virtual ~PackagedBindableWithParam() {}
};
}

//assorted functions
namespace impl {

CommandBufferImplGles_::~CommandBufferImplGles_()
{
	if (!_context.isValid())
	{
		Log(Log.Warning, "WARNING - Command buffer released AFTER its context was destroyed.");
	}
}

void CommandBufferImplGles_::pushPipeline_()
{
	enqueue_internal<PushPipeline>(PushPipeline());
}

void CommandBufferImplGles_::popPipeline_()
{
	enqueue_internal<PopPipeline>(PopPipeline());
}

void CommandBufferImplGles_::resetPipeline_()
{
	enqueue_internal<ResetPipeline>(ResetPipeline());
}
}

//synchronization,
namespace impl {
void CommandBufferImplGles_::waitForEvent_(const Event& evt, types::PipelineStageFlags srcStage,
    types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
{
	assertion(0, "Events not currently supported in OpenGL ES");
}
void CommandBufferImplGles_::waitForEvents_(const EventSet& evts, types::PipelineStageFlags srcStage,
    types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
{
	assertion(0, "Events not currently supported in OpenGL ES");
}
void CommandBufferImplGles_::setEvent_(Event& evt, types::PipelineStageFlags pipelineFlags)
{
	assertion(0, "Events not currently supported in OpenGL ES");
}
void CommandBufferImplGles_::resetEvent_(Event& evt, types::PipelineStageFlags pipelineFlags)
{
	assertion(0, "Events not currently supported in OpenGL ES");
}

}

//bind pipelines, sets, vertex/index buffers
namespace impl {
void CommandBufferImplGles_::bindPipeline_(GraphicsPipeline& pipeline)
{
	enqueue_internal<PackagedBindable<GraphicsPipeline>>(pipeline);
}

void CommandBufferImplGles_::bindPipeline_(ComputePipeline& pipeline)
{
	enqueue_internal<PackagedBindable<ComputePipeline>>(pipeline);
}

void CommandBufferImplGles_::bindPipeline_(VertexRayPipeline& pipeline)
{
	assertion(0, "Bind VertexRay Pipeline not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::bindPipeline_(SceneTraversalPipeline& pipeline)
{
	assertion(0, "Bind SceneTraversal Pipeline not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::bindDescriptorSet_(const api::PipelineLayout& pipelineLayout, uint32 firstSet,
    const DescriptorSet& set, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	bindDescriptorSets_(types::PipelineBindPoint::Graphics, pipelineLayout, firstSet, &set, 1, dynamicOffsets, numDynamicOffset);
}

void CommandBufferImplGles_::bindDescriptorSets_(types::PipelineBindPoint bindingPoint, const api::PipelineLayout& pipelineLayout,
    uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	enqueue_internal<BindDescriptorSets>(BindDescriptorSets(bindingPoint, pipelineLayout, sets, numDescSets, dynamicOffsets, numDynamicOffset));
}

void CommandBufferImplGles_::bindDescriptorSetCompute_(const api::PipelineLayout& pipelineLayout, uint32 firstSet,
    const DescriptorSet& set, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	enqueue_internal<BindDescriptorSets>(BindDescriptorSets(types::PipelineBindPoint::Compute, pipelineLayout, &set, 1, dynamicOffsets, numDynamicOffset));
}

void CommandBufferImplGles_::bindDescriptorSetRayTracing_(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets, uint32 numDynamicOffsets)
{
	assertion(0, "Bind Ray Tracing Descriptor Set not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::bindDescriptorSetSHG_(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set, const uint32* dynamicOffsets, uint32 numDynamicOffsets)
{
	assertion(0, "Bind Scene Hierarchy Generator Descriptor Set not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::bindVertexBuffer_(const Buffer& buffer, uint32 offset, uint16 bindingIndex)
{
	enqueue_internal<BindVertexBuffer>(BindVertexBuffer(buffer, offset, bindingIndex));
}

void CommandBufferImplGles_::bindVertexBuffer_(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding, uint16 bindingCount)
{
	enqueue_internal<BindVertexBuffer>(BindVertexBuffer(buffers, offsets, numBuffers, startBinding, bindingCount));
}

void CommandBufferImplGles_::bindIndexBuffer_(const api::Buffer& buffer, uint32 offset, types::IndexType indexType)
{
	enqueue_internal<BindIndexBuffer>(BindIndexBuffer(buffer, offset, indexType));
}

}

//begin end submit clear reset etc.
namespace impl {
void CommandBufferImplGles_::endRecording_()
{
	if (!_isRecording)
	{
		Log("Called CommandBuffer::endRecording while a recording was already in progress. Call CommandBuffer::beginRecording first");
		assertion(0);
	}
	_isRecording = false;
}

void CommandBufferImplGles_::clear_(bool releaseResources)
{
	_queue.clear();
}
}

// Renderpasses, Subpasses
namespace impl {
void CommandBufferImplGles_::beginRenderPass_(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineSubpass, const glm::vec4& clearColor
    , float32 clearDepth, uint32 clearStencil)
{
	enqueue_internal<BeginRenderPass>(BeginRenderPass(fbo, renderArea, clearColor, clearDepth, clearStencil));
}

void CommandBufferImplGles_::beginRenderPass_(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass,
    const glm::vec4* clearColor, uint32 numClearColor, float32 clearDepth, uint32 clearStencil)
{
	enqueue_internal<BeginRenderPass>(BeginRenderPass(fbo, renderArea, clearColor, numClearColor, clearDepth, clearStencil));
}


void CommandBufferImplGles_::beginRenderPass_(api::Fbo& fbo, bool inlineFirstSubpass, const glm::vec4& clearColor,
    float32 clearDepth, uint32 clearStencil)
{
	beginRenderPass_(fbo, Rectanglei(glm::ivec2(0, 0), fbo->getDimensions()), inlineFirstSubpass, clearColor, clearDepth, clearStencil);
}

void CommandBufferImplGles_::beginRenderPass_(api::Fbo& fbo, const RenderPass& renderPass, const Rectanglei& renderArea,
    bool inlineFirstSubpass, const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	enqueue_internal<BeginRenderPass>(BeginRenderPass(fbo, renderArea, clearColor, clearDepth, clearStencil));
}

void CommandBufferImplGles_::beginRenderPass_(api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea,
    bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors,
    float32* clearDepth, uint32* clearStencil, uint32 numClearDepthStencil)
{
	if (numClearDepthStencil > 0)
	{
		enqueue_internal<BeginRenderPass>(BeginRenderPass(fbo, renderArea, clearColors, numClearColors, *clearDepth, *clearStencil));
	}
	else
	{
		enqueue_internal<BeginRenderPass>(BeginRenderPass(fbo, renderArea, clearColors, numClearColors));
	}
}

void CommandBufferImplGles_::beginRenderPass_(api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass,
    const glm::vec4& clearColor, float32 clearDepth, uint32 clearStencil)
{
	enqueue_internal<BeginRenderPass>(BeginRenderPass(fbo, Rectanglei(glm::ivec2(0, 0), fbo->getDimensions()),
	                                  clearColor, clearDepth, clearStencil));
}

void CommandBufferImplGles_::endRenderPass_()
{
	enqueue_internal<EndRenderPass>(EndRenderPass());
}

void CommandBufferImplGles_::nextSubPassInline_() {}

void CommandBufferImplGles_::nextSubPassSecondaryCmds_(api::SecondaryCommandBuffer& cmdBuffer)
{
	enqueueSecondaryCmds_(cmdBuffer);
}

}// namespace impl


//buffers, textures, images
namespace impl {
void CommandBufferImplGles_::updateBuffer_(api::Buffer& buffer, const void* data, uint32 offset, uint32 length)
{
	enqueue_internal<UpdateBuffer>(UpdateBuffer(buffer, offset, length, data));
}

void CommandBufferImplGles_::copyBuffer_(api::Buffer src, api::Buffer dest, uint32 srcOffset, uint32 destOffset, uint32 sizeInBytes)
{
	void* dstptr = dest->map(types::MapBufferFlags::Write, destOffset, sizeInBytes);
	void* srcptr = src->map(types::MapBufferFlags::Read, srcOffset, sizeInBytes);
	memcpy(srcptr, dstptr, sizeInBytes);
	dest->unmap();
	src->unmap();
}

void CommandBufferImplGles_::blitImage_(api::TextureStore& src, api::TextureStore& dest,
                                        types::ImageLayout srcLayout, types::ImageLayout dstLayout, types::ImageBlitRange* regions,
                                        uint32 numRegions, types::SamplerFilter filter)
{
//	 create an fbo for source and copy from it.
	api::FboCreateParam fboInfoSrc, fboInfoDest;
	api::RenderPassCreateParam rpInfo;
	GLenum glCopyFrom;

	if (srcLayout == types::ImageLayout::DepthStencilAttachmentOptimal ||
	    srcLayout == types::ImageLayout::DepthStencilReadOnlyOptimal)
	{
		rpInfo.setDepthStencilInfo(0, api::RenderPassDepthStencilInfo(src->getFormat()));
		fboInfoSrc.setDepthStencil(0, _context->createTextureView(src));
		fboInfoDest.setDepthStencil(0, _context->createTextureView(dest));
		glCopyFrom = GL_DEPTH_BUFFER_BIT;
	}
	else
	{
		rpInfo.setColorInfo(0, api::RenderPassColorInfo(src->getFormat()));
		fboInfoSrc.setColor(0, _context->createTextureView(src));
		fboInfoDest.setColor(0, _context->createTextureView(dest));
		glCopyFrom = GL_COLOR_BUFFER_BIT;
	}

	rpInfo.setSubPass(0, api::SubPass().setColorAttachment(0, 0));
	fboInfoSrc.setRenderPass(_context->createRenderPass(rpInfo));
	api::Fbo fboSrc = _context->createFbo(fboInfoSrc);
	if (!fboSrc.isValid())
	{
		Log("CommandBufferImplGles_::blitImage : Failed to create the Source fbo");
		return;
	}

	// create the destination fbo.
	rpInfo.setColorInfo(0, dest->getFormat());
	fboInfoDest.setRenderPass(_context->createRenderPass(rpInfo));
	api::Fbo fboDest = _context->createFbo(fboInfoDest);
	if (!fboDest.isValid())
	{
		Log("CommandBufferImplGles_::blitImage : Failed to create the Destination fbo");
		return;
	}

	static_cast<api::gles::FboGles_&>(*fboSrc).bind(*_context, types::FboBindingTarget::Read);
	static_cast<api::gles::FboGles_&>(*fboDest).bind(*_context, types::FboBindingTarget::Write);


	GLenum samplerFilter = 0;// nativeGles::ConvertToGles::samplerMagFilter(filter);

	samplerFilter =  glm::clamp((uint32)samplerFilter, (uint32)GL_NEAREST, (uint32)GL_LINEAR);

	// if the api supports blit framebuffer then use it, else emulate it.
	if (_context->hasApiCapability(ApiCapabilities::BlitFrameBuffer))
	{
		for (uint32 i = 0; i < numRegions; ++i)
		{
			const types::ImageBlitRange& range = regions[i];
			gl::BlitFramebuffer(range.srcOffset[0].offsetX, range.srcOffset[0].offsetY,
			                    range.srcOffset[1].offsetX, range.srcOffset[1].offsetY,
			                    range.dstOffset[0].offsetX, range.dstOffset[0].offsetY,
			                    range.dstOffset[1].offsetX, range.dstOffset[1].offsetY, glCopyFrom, samplerFilter);
			debugLogApiError("gl::BlitFramebuffer - Failed");
		}
	}
	else
	{
		Log(Log.Debug, "blitImage is  not supported for ES2. Supported for ES3+");
	}
}


void CommandBufferImplGles_::copyImageToBuffer_(api::TextureStore& srcImage, types::ImageLayout srcImageLayout, api::Buffer& dstBuffer, types::BufferImageCopy* regions, uint32 numRegions)
{
	assertion(0, "blitImage not currently supported in OpenGL ES");
}
}

//dynamic commands
namespace impl {
void CommandBufferImplGles_::clearColorImage_(api::TextureView& image, glm::vec4 clearColor, const uint32 baseMipLevel,
    const uint32 levelCount, const uint32 baseArrayLayer, const uint32 layerCount,
    types::ImageLayout layout)
{
	if (this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageEXT) || this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageIMG))
	{
		enqueue_internal<ClearColorImage>(ClearColorImage(image, clearColor, baseMipLevel, baseArrayLayer, layerCount));
	}
	else
	{
		Log(Log.Critical, "Extension ClearTexImage not supported");
	}
}

void CommandBufferImplGles_::clearColorImage_(api::TextureView& image, glm::vec4 clearColor, const uint32* baseMipLevel,
    const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount,
    types::ImageLayout layout)
{
	if (this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageEXT) || this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageIMG))
	{
		for (unsigned int i = 0; i < rangeCount; i++)
		{
			for (unsigned int j = 0; j < levelCount[i]; j++)
			{
				enqueue_internal<ClearColorImage>(ClearColorImage(image, clearColor, baseMipLevel[i] + j, baseArrayLayers[i], layerCount[i]));
			}
		}
	}
	else
	{
		assertion("Extension ClearTexImage not supported");
	}
}

void CommandBufferImplGles_::clearDepthImage_(api::TextureView& image, float clearDepth, const uint32 baseMipLevel,
    const uint32 levelCount, const uint32 baseArrayLayer, const uint32 layerCount,
    types::ImageLayout layout)
{
	if (this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageEXT) || this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageIMG))
	{
		enqueue_internal<ClearDepthStencilImage>(ClearDepthStencilImage(image, clearDepth, 0u, baseMipLevel, baseArrayLayer, layerCount));
	}
	else
	{
		Log(Log.Critical, "Extension ClearTexImage not supported");
	}
}

void CommandBufferImplGles_::clearDepthImage_(api::TextureView& image, float clearDepth, const uint32* baseMipLevel,
    const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount,
    uint32 rangeCount, types::ImageLayout layout)
{
	if (this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageEXT) || this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageIMG))
	{
		for (unsigned int i = 0; i < rangeCount; i++)
		{
			for (unsigned int j = 0; j < levelCount[i]; j++)
			{
				enqueue_internal<ClearDepthStencilImage>(ClearDepthStencilImage(image, clearDepth, 0u, baseMipLevel[i] + j, baseArrayLayers[i], layerCount[i]));
			}
		}
	}
	else
	{
		Log(Log.Critical, "Extension ClearTexImage not supported");
	}
}

void CommandBufferImplGles_::clearStencilImage_(api::TextureView& image, uint32 clearStencil, const uint32 baseMipLevel,
    const uint32 levelCount, const uint32 baseArrayLayer, const uint32 layerCount,
    types::ImageLayout layout)
{
	if (this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageEXT) || this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageIMG))
	{
		enqueue_internal<ClearDepthStencilImage>(ClearDepthStencilImage(image, 0.0f, clearStencil, baseMipLevel, baseArrayLayer, layerCount));
	}
	else
	{
		Log(Log.Critical, "Extension ClearTexImage not supported");
	}
}

void CommandBufferImplGles_::clearStencilImage_(api::TextureView& image, uint32 clearStencil, const uint32* baseMipLevel,
    const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount,
    types::ImageLayout layout)
{
	if (this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageEXT) || this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageIMG))
	{
		for (unsigned int i = 0; i < rangeCount; i++)
		{
			for (unsigned int j = 0; j < levelCount[i]; j++)
			{
				enqueue_internal<ClearDepthStencilImage>(ClearDepthStencilImage(image, 0.0f, clearStencil, baseMipLevel[i] + j, baseArrayLayers[i], layerCount[i]));
			}
		}
	}
	else
	{
		Log(Log.Critical, "Extension ClearTexImage not supported");
	}
}

void CommandBufferImplGles_::clearDepthStencilImage_(api::TextureView& image, float clearDepth, uint32 clearStencil, const uint32 baseMipLevel,
    const uint32 levelCount, const uint32 baseArrayLayer, const uint32 layerCount,
    types::ImageLayout layout)
{
	if (this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageEXT) || this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageIMG))
	{
		enqueue_internal<ClearDepthStencilImage>(ClearDepthStencilImage(image, clearDepth, clearStencil, baseMipLevel, baseArrayLayer, layerCount));
	}
	else
	{
		Log(Log.Critical, "Extension ClearTexImage not supported");
	}
}

void CommandBufferImplGles_::clearDepthStencilImage_(api::TextureView& image, float clearDepth, uint32 clearStencil, const uint32* baseMipLevel,
    const uint32* levelCount, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rangeCount,
    types::ImageLayout layout)
{
	if (this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageEXT) || this->getContext_()->hasApiCapabilityExtension(ApiCapabilities::ClearTexImageIMG))
	{
		for (unsigned int i = 0; i < rangeCount; i++)
		{
			for (unsigned int j = 0; j < levelCount[i]; j++)
			{
				enqueue_internal<ClearDepthStencilImage>(ClearDepthStencilImage(image, clearDepth, clearStencil, baseMipLevel[i] + j, baseArrayLayers[i], layerCount[i]));
			}
		}
	}
	else
	{
		Log(Log.Critical, "Extension ClearTexImage not supported");
	}
}

void CommandBufferImplGles_::clearColorAttachment_(uint32 const* attachmentIndices, glm::vec4 const* clearColors, uint32 attachmentCount,
    const Rectanglei* rects, const uint32* baseArrayLayers, const uint32* layerCount, uint32 rectCount)
{
	enqueue_internal<ClearColorAttachment>(ClearColorAttachment(attachmentCount, clearColors, rectCount, rects));
}

void CommandBufferImplGles_::clearColorAttachment_(uint32 attachmentIndex, glm::vec4 clearColor,
    const Rectanglei rect, const uint32 baseArrayLayer, const uint32 layerCount)
{
	enqueue_internal<ClearColorAttachment>(ClearColorAttachment(1u, clearColor, 1u, rect));
}

void CommandBufferImplGles_::clearColorAttachment_(api::Fbo fbo, glm::vec4 clearColor)
{
	enqueue_internal<ClearColorAttachment>(ClearColorAttachment(fbo->getNumColorAttachments(), clearColor, 1u,
	                                       Rectanglei(0u, 0u, fbo->getDimensions().x, fbo->getDimensions().y)));
}

void CommandBufferImplGles_::clearDepthAttachment_(const Rectanglei& clearRect, float32 depth)
{
	enqueue_internal<ClearDepthStencilAttachment>(ClearDepthStencilAttachment(depth, clearRect));
}

void CommandBufferImplGles_::clearStencilAttachment_(const Rectanglei& clearRect, int32 stencil)
{
	enqueue_internal<ClearDepthStencilAttachment>(ClearDepthStencilAttachment(stencil, clearRect));
}

void CommandBufferImplGles_::clearDepthStencilAttachment_(const Rectanglei& clearRect, float32 depth, int32 stencil)
{
	enqueue_internal<ClearDepthStencilAttachment>(ClearDepthStencilAttachment(depth, stencil, clearRect));
}

void CommandBufferImplGles_::setViewport_(const Rectanglei& viewport)
{
	enqueue_internal<SetViewport>(SetViewport(viewport));
}

void CommandBufferImplGles_::setScissor_(const Rectanglei& scissor)
{
	enqueue_internal<SetScissor>(SetScissor(scissor));
}

void CommandBufferImplGles_::setDepthBound_(float32 min, float32 max)
{
	assertion(0, "Not Implemented Yet");
	//enqueue_internal<SetDepthBound>(SetDepthBound(min, max));
}

void CommandBufferImplGles_::setStencilCompareMask_(types::StencilFace face, uint32 compareMask)
{
	enqueue_internal<SetStencilCompareMask>(SetStencilCompareMask(face, compareMask));
}

void CommandBufferImplGles_::setStencilWriteMask_(types::StencilFace face, uint32 writeMask)
{
	enqueue_internal<SetStencilWriteMask>(SetStencilWriteMask(face, writeMask));
}

void CommandBufferImplGles_::setStencilReference_(types::StencilFace face, uint32 ref)
{
	enqueue_internal<SetStencilReference>(SetStencilReference(face, ref));
}

void CommandBufferImplGles_::setDepthBias_(float32 depthBias, float32 depthBiasClamp, float32 slopeScaledDepthBias)
{
	Log(Log.Critical, "setDepthBias not implemented");
}

void CommandBufferImplGles_::setBlendConstants_(glm::vec4 rgba)
{
	enqueue_internal<SetBlendConstants>(SetBlendConstants(glm::vec4(rgba.r, rgba.g, rgba.b, rgba.a)));
}

void CommandBufferImplGles_::setLineWidth_(float width)
{
	assertion(0, "Set line Width does not supported Currently");
	Log(Log.Critical, "SetLineWidth not implemented");
}
}

// drawing commands
namespace impl {
void CommandBufferImplGles_::drawIndexedIndirect_(Buffer& buffer)
{
	assertion(0, "DrawArraysIndirect is not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::drawArraysIndirect_(Buffer& buffer, uint32 offset, uint32 count, uint32 stride)
{
	assertion(0, "DrawArraysIndirect is not currently supported in OpenGL ES");
}


void CommandBufferImplGles_::drawIndexed_(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset, uint32 firstInstance, uint32 instanceCount)
{
	enqueue_internal<DrawIndexed>(DrawIndexed(firstIndex, indexCount, vertexOffset, firstInstance, instanceCount));
}

void CommandBufferImplGles_::drawArrays_(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance, uint32 instanceCount)
{
	enqueue_internal<DrawArrays>(DrawArrays(firstVertex, vertexCount, firstInstance, instanceCount));
}

void CommandBufferImplGles_::drawIndirect_(Buffer& buffer, uint32 offset, uint32 count, uint32 stride)
{
	assertion(0, "Draw Indirect does not supported Currently");
	Log(Log.Critical, "DrawIndirect not implemented");
}

void CommandBufferImplGles_::dispatchCompute_(uint32 numGroupsX, uint32 numGroupsY, uint32 numGroupsZ)
{
	enqueue_internal<DispatchCompute>(DispatchCompute(numGroupsX, numGroupsY, numGroupsZ));
}

void CommandBufferImplGles_::beginSceneHierarchy_(const SceneHierarchy& sceneHierarchy, math::AxisAlignedBox& extents)
{
	assertion(0, "SHG commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::pushSharedRayConstants_(uint32 offset, uint32 size, const void* pValues)
{
	assertion(0, "Ray Tracing commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::setRaySizes_(uint32 raySizeCount, const uint32* pRaySizes)
{
	assertion(0, "Ray Tracing commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::setRayBounceLimit_(uint32 limit)
{
	assertion(0, "Ray Tracing commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::endSceneHierarchy_()
{
	assertion(0, "SHG commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::mergeSceneHierarchies_(const SceneHierarchy& destinationSceneHierarchy, math::AxisAlignedBox& extents, const SceneHierarchy* sourceSceneHierarchies, const uint32 numberOfSourceSceneHierarchies, const uint32 mergeQuality)
{
	assertion(0, "SHG commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::bindSceneHierarchies_(const SceneHierarchy* sceneHierarchies, uint32 firstBinding, const uint32 numberOfSceneHierarchies)
{
	assertion(0, "SHG commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::dispatchRays_(uint32 xOffset, uint32 yOffset, uint32 frameWidth, uint32 frameHeight)
{
	assertion(0, "Ray Tracing commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::bindAccumulationImages_(uint32 startBinding, uint32 bindingCount, const TextureView* imageViews)
{
	assertion(0, "Accumulation Images not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::sceneHierarchyAppend_(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
{
	assertion(0, "SHG commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::sceneHierarchyAppendIndexed_(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance)
{
	assertion(0, "SHG commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::sceneHierarchyAppendIndirect_(api::BufferView& indirectBuffer, uint32 offset, uint32 drawCount, uint32 stride)
{
	assertion(0, "SHG commands not currently supported in OpenGL ES");
}

void CommandBufferImplGles_::sceneHierarchyAppendIndexedIndirect_(api::BufferView& indirectBuffer, uint32 offset, uint32 drawCount, uint32 stride)
{
	assertion(0, "SHG commands not currently supported in OpenGL ES");
}

}

//uniforms
namespace impl {

#define SET_UNIFORM_DEFINITION(_type_)\
 void CommandBufferImplGles_::setUniform_(int32 location, const _type_& val)\
{ enqueue_internal<SetUniform<_type_>/**/>(SetUniform<_type_>(location, val)); }\
 void CommandBufferImplGles_::setUniformPtr_(int32 location, uint32 count, const _type_* ptr) \
{ enqueue_internal<SetUniformPtr<_type_>/**/>(SetUniformPtr<_type_>(location, count, ptr)); }

SET_UNIFORM_DEFINITION(uint32);
SET_UNIFORM_DEFINITION(int32);
SET_UNIFORM_DEFINITION(float32);
SET_UNIFORM_DEFINITION(glm::vec2);
SET_UNIFORM_DEFINITION(glm::ivec2);
SET_UNIFORM_DEFINITION(glm::uvec2);
SET_UNIFORM_DEFINITION(glm::vec3);
SET_UNIFORM_DEFINITION(glm::ivec3);
SET_UNIFORM_DEFINITION(glm::uvec3);
SET_UNIFORM_DEFINITION(glm::vec4);
SET_UNIFORM_DEFINITION(glm::ivec4);
SET_UNIFORM_DEFINITION(glm::uvec4);
SET_UNIFORM_DEFINITION(glm::mat2);
SET_UNIFORM_DEFINITION(glm::mat2x3);
SET_UNIFORM_DEFINITION(glm::mat2x4);
SET_UNIFORM_DEFINITION(glm::mat3x2);
SET_UNIFORM_DEFINITION(glm::mat3);
SET_UNIFORM_DEFINITION(glm::mat3x4);
SET_UNIFORM_DEFINITION(glm::mat4x2);
SET_UNIFORM_DEFINITION(glm::mat4x3);
SET_UNIFORM_DEFINITION(glm::mat4x4);

#undef SET_UNIFORM_DEFINITION

//
//void CommandBufferImplGles_::setMemoryBarrier(const PipelineBarrier& barrier)
//{
//	enqueue_internal<PipelineBarrier>(barrier);
//}
//
//Sync CommandBufferImplGles_::insertFenceSync()
//{
//	CreateFenceSyncImpl fenceSyncImpl;
//	enqueue_internal<CreateFenceSyncImpl>(fenceSyncImpl);
//	return fenceSyncImpl.syncObject;
//}


#ifdef DEBUG
void CommandBufferImplGles_::logCommandStackTraces_()
{
	for (auto it = _queue.begin(); it != _queue.end(); ++it)
	{
		Log(Log.Debug, it->debug_commandCallSiteStackTrace.c_str());
	}
}
#endif

void CommandBufferImplGles_::beginRecording_(const RenderPass& rp, uint32 subPass)
{
	beginRecording_();
}

void CommandBufferImplGles_::beginRecording_(const Fbo& rp, uint32 subPass)
{
	beginRecording_();
}

void CommandBufferImplGles_::submit_()
{
	debug_assertion(_context.isValid(), "No context has been set");
	for (auto it = _queue.begin(); it != _queue.end(); ++it)
	{
		it->execute(*_myowner);
	}
}

void CommandBufferImplGles_::submit_(const Semaphore& waitSemaphore, const Semaphore& signalSemaphore, const Fence& fence)
{
	submit_();
}

void CommandBufferImplGles_::submit_(Fence& fence)
{
	submit_();
}

void CommandBufferImplGles_::submit_(SemaphoreSet& waitSemaphores, SemaphoreSet& signalSemaphores, const Fence& fence)
{
	submit_();
}

void CommandBufferImplGles_::submitStartOfFrame_(Semaphore& signalSemaphore, const Fence& fence)
{
	submit_();
}

void CommandBufferImplGles_::submitEndOfFrame_(Semaphore& waitSemaphore)
{
	submit_();
}

void CommandBufferImplGles_::enqueueSecondaryCmds_(SecondaryCommandBuffer& secondaryCmdBuffer)
{
	enqueue_internal<SecondaryCommandBufferPackager>(secondaryCmdBuffer);
}

void CommandBufferImplGles_::enqueueSecondaryCmds_(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers)
{
	for (uint32 i = 0; i < numCmdBuffers; ++i)
	{
		enqueueSecondaryCmds_(secondaryCmdBuffers[i]);
	}
}
void CommandBufferImplGles_::enqueueSecondaryCmds_BeginMultiple_(uint32 expectedMax) { }
void CommandBufferImplGles_::enqueueSecondaryCmds_EnqueueMultiple_(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers)
{
	for (uint32 i = 0; i < numCmdBuffers; ++i)
	{
		enqueueSecondaryCmds_(secondaryCmdBuffers[i]);
	}
}
void CommandBufferImplGles_::enqueueSecondaryCmds_SubmitMultiple_(bool keepAllocated) { }


void addToBarrier(MemoryBarrier barrier, GLuint& inOutbarrier)
{
	inOutbarrier |= nativeGles::ConvertToGles::memBarrierFlagOut((uint32)barrier.dstMask);
}
void addToBarrier(BufferRangeBarrier barrier, GLuint& inOutbarrier)
{
	inOutbarrier |= nativeGles::ConvertToGles::memBarrierFlagOut((uint32)barrier.dstMask);
}
void addToBarrier(const ImageAreaBarrier& barrier, GLuint& inOutbarrier)
{
	inOutbarrier |= nativeGles::ConvertToGles::memBarrierFlagOut((uint32)barrier.dstMask);
}

void CommandBufferImplGles_::pipelineBarrier_(
  types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage,
  const MemoryBarrierSet& barriers, bool dependencyByRegion /*= true*/)
{
	if (getContext_()->getApiType() < Api::OpenGLES31) { return; }

	PipelineBarrier barriergl; barriergl.barrier = 0;
	for (auto && barrier : barriers.getMemoryBarriers())
	{
		addToBarrier(barrier, barriergl.barrier);
	}
	for (auto && barrier : barriers.getImageBarriers())
	{
		addToBarrier(barrier, barriergl.barrier);
	}
	for (auto && barrier : barriers.getBufferBarriers())
	{
		addToBarrier(barrier, barriergl.barrier);
	}
	enqueue_internal<PipelineBarrier>(barriergl);
}

template<typename Resource_, typename ParamType_>
void PackagedBindableWithParam<Resource_, ParamType_>::execute_private(impl::CommandBufferBase_& cmdBuf)
{
#ifdef DEBUG
	if (res.isNull())
	{
		Log(Log.Warning, "API Command: Tried to bind NULL object");
		assertion(false);
	}
	else
#endif
	{
		res->bind(cmdBuf.getContext(), param);
	}
}

template<typename Resource_>
void PackagedBindable<Resource_>::execute_private(impl::CommandBufferBase_& cmdBuf)
{
#ifdef DEBUG
	if (res.isNull())
	{
		Log(Log.Warning, "API Command: Tried to bind NULL object");
		assertion(false);
	}
	else
#endif
	{
		res->bind(*cmdBuf.getContext());
	}
}

void PackagedBindable<GraphicsPipeline>::execute_private(impl::CommandBufferBase_& cmdBuf)
{
#ifdef DEBUG
	if (res.isNull())
	{
		Log(Log.Warning, "API Command: Tried to bind NULL object");
		assertion(false);
	}
	else
#endif
	{
		static_cast<gles::GraphicsPipelineImplGles&>(res->getImpl()).bind();
	}
}

void PackagedBindable<ComputePipeline>::execute_private(impl::CommandBufferBase_& cmdBuf)
{
#ifdef DEBUG
	if (res.isNull())
	{
		Log(Log.Warning, "API Command: Tried to bind NULL object");
		assertion(false);
	}
	else
#endif
	{
		static_cast<gles::ComputePipelineImplGles&>(res->getImpl()).bind();
	}
}

}
}
}
