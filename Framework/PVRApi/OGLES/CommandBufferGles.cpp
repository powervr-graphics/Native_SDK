/*!*********************************************************************************************************************
\file         PVRApi/OGLES/CommandBufferGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        OpenGL ES Implementation details CommandBuffer class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/CommandBuffer.h"
#include "PVRCore/StackTrace.h"
#include "PVRCore/ListOfInterfaces.h"
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/OGLES/ApiCommands.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/ApiObjects/CommandPool.h"
#include "PVRApi/ApiObjects/Sync.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"

namespace pvr {
namespace api {
namespace impl {
class CommandBufferBaseImplementationDetails : public native::HCommandBuffer_
{
public:
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;

	typedef ListOfInterfaces<ApiCommand> CommandQueue; //!< Special class optimizing the memory layout of the command queue
	CommandQueue queue;
	GraphicsContext context;
	bool m_isRecording;
	CommandBufferBaseImplementationDetails(GraphicsContext& context) : context(context), queue(1024), m_isRecording(false)
	{

	}

	bool valdidateRecordState()
	{
#ifdef DEBUG
		if (!m_isRecording)
		{
			Log("Attempted to submit into the commandBuffer without calling beginRecording first.");
			assertion(false , "You must call beginRecording before starting to submit commands into the commandBuffer.");
			return false;
		}
#endif
		return true;
	}

	void beginRecording()
	{
		if (m_isRecording)
		{
			Log("Called CommandBuffer::beginRecording while a recording was already in progress. Call CommandBuffer::endRecording first");
			assertion(0);
		}
		queue.clear();
		m_isRecording = true;
	}

	//Enqueue a parameterless object
	template<typename ClassType, typename ObjectType> void enqueue_internal(const ObjectType& obj)
	{
		if (!valdidateRecordState()) { return; }

		queue.emplace_back<ClassType>(obj);
#ifdef DEBUG
		queue.last()->interfacePtr->debug_commandCallSiteStackTrace = getStackTraceInfo(2);
#endif
	}
	void submit(CommandBufferBase_& cmdBuf)
	{
		assertion(context.isValid() , "No context has been set");
		for (auto it = queue.begin(); it != queue.end(); ++it)
		{
			it->execute(cmdBuf);
		}
	}
};

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
	void execute_private(CommandBufferBase_& cmdBuf) {	me->pImpl->submit(cmdBuf);	}
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

const native::HCommandBuffer_& CommandBufferBase_::getNativeObject() const { return *pImpl; }
native::HCommandBuffer_& CommandBufferBase_::getNativeObject() { return *pImpl; }

CommandBufferBase_::CommandBufferBase_(GraphicsContext& context, CommandPool& pool, native::HCommandBuffer_& cmdBuff)
{
	pImpl.construct(context);
}

CommandBufferBase_::~CommandBufferBase_()
{
	clear();
}

GraphicsContext& CommandBufferBase_::getContext() { return pImpl->context; }

void CommandBuffer_::beginRecording()
{
	pImpl->beginRecording();
}

void CommandBufferBase_::endRecording()
{
	if (!pImpl->m_isRecording)
	{
		Log("Called CommandBuffer::endRecording while a recording was already in progress. Call CommandBuffer::beginRecording first");
		assertion(0);
	}
	pImpl->m_isRecording = false;
}

void CommandBufferBase_::bindPipeline(GraphicsPipeline& pipeline)
{
	pImpl->enqueue_internal<PackagedBindable<GraphicsPipeline> >(pipeline);
}

void CommandBufferBase_::bindPipeline(ParentableGraphicsPipeline& pipeline)
{
	pImpl->enqueue_internal<PackagedBindable<ParentableGraphicsPipeline> >(pipeline);
}

void CommandBufferBase_::bindPipeline(ComputePipeline& pipeline)
{
	pImpl->enqueue_internal<PackagedBindable<ComputePipeline> >(pipeline);
}

void CommandBufferBase_::bindDescriptorSet(const api::PipelineLayout& pipelineLayout, pvr::uint32 firstSet,
    const DescriptorSet& set, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	bindDescriptorSets(types::PipelineBindPoint::Graphics, pipelineLayout, firstSet, &set, 1, dynamicOffsets, numDynamicOffset);
}

void CommandBufferBase_::bindDescriptorSets(types::PipelineBindPoint::Enum bindingPoint, const api::PipelineLayout& pipelineLayout,
    uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	pImpl->enqueue_internal<BindDescriptorSets>(BindDescriptorSets(bindingPoint, pipelineLayout, sets, numDescSets, dynamicOffsets, numDynamicOffset));
}

void CommandBufferBase_::bindDescriptorSetCompute(const api::PipelineLayout& pipelineLayout, pvr::uint32 firstSet,
    const DescriptorSet& set, const uint32* dynamicOffsets, uint32 numDynamicOffset)
{
	pImpl->enqueue_internal<BindDescriptorSets>(BindDescriptorSets(types::PipelineBindPoint::Compute, pipelineLayout, &set, 1, dynamicOffsets, numDynamicOffset));
}

void CommandBufferBase_::clearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 const* clearColors, const pvr::Rectanglei* rects)
{
	pImpl->enqueue_internal<ClearColorAttachment>(ClearColorAttachment(attachmentCount, clearColors, rects));
}

void CommandBufferBase_::clearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 clearColor, const pvr::Rectanglei rect)
{
	pImpl->enqueue_internal<ClearColorAttachment>(ClearColorAttachment(attachmentCount, clearColor, rect));
}

void CommandBufferBase_::clearDepthAttachment(const pvr::Rectanglei& clearRect, float32 depth)
{
	pImpl->enqueue_internal<ClearDepthStencilAttachment>(ClearDepthStencilAttachment(depth, clearRect));
}

void CommandBufferBase_::clearStencilAttachment(const pvr::Rectanglei& clearRect, pvr::int32 stencil)
{
	pImpl->enqueue_internal<ClearDepthStencilAttachment>(ClearDepthStencilAttachment(stencil, clearRect));
}

void CommandBufferBase_::clearDepthStencilAttachment(const pvr::Rectanglei& clearRect, float32 depth, pvr::int32 stencil)
{
	pImpl->enqueue_internal<ClearDepthStencilAttachment>(ClearDepthStencilAttachment(depth, stencil, clearRect));
}

void CommandBufferBase_::drawIndexed(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset, uint32 firstInstance, uint32 instanceCount)
{
	pImpl->enqueue_internal<DrawIndexed>(DrawIndexed(firstIndex, indexCount, vertexOffset, firstInstance, instanceCount));
}

void CommandBufferBase_::drawArrays(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance, uint32 instanceCount)
{
	pImpl->enqueue_internal<DrawArrays>(DrawArrays(firstVertex, vertexCount, firstInstance, instanceCount));
}

void CommandBufferBase_::updateBuffer(pvr::api::Buffer& buffer, const void* data, pvr::uint32 offset, pvr::uint32 length)
{
	pImpl->enqueue_internal<UpdateBuffer>(UpdateBuffer(buffer, offset, length, data));
}

void CommandBufferBase_::drawIndexedIndirect(Buffer& buffer)
{
	assertion(0, "DrawIndexedIndirect does not supported Currently");
	Log(Log.Critical, "DrawIndexedIndirect not implemented");
}

void CommandBufferBase_::bindVertexBuffer(const Buffer& buffer, uint32 offset, uint16 bindingIndex)
{
	pImpl->enqueue_internal<BindVertexBuffer>(BindVertexBuffer(buffer, offset, bindingIndex));
}

void CommandBufferBase_::bindVertexBuffer(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding, uint16 bindingCount)
{
	pImpl->enqueue_internal<BindVertexBuffer>(BindVertexBuffer(buffers, offsets, numBuffers, startBinding, bindingCount));
}

void CommandBufferBase_::bindIndexBuffer(const api::Buffer& buffer, uint32 offset, types::IndexType::Enum indexType)
{
	pImpl->enqueue_internal<BindIndexBuffer>(BindIndexBuffer(buffer, offset, indexType));
}

void CommandBufferBase_::setViewport(const pvr::Rectanglei& viewport)
{
	pImpl->enqueue_internal<SetViewport>(SetViewport(viewport));
}

void CommandBufferBase_::setScissor(const pvr::Rectanglei& scissor)
{
	pImpl->enqueue_internal<SetScissor>(SetScissor(scissor));
}

void CommandBufferBase_::setDepthBound(pvr::float32 min, pvr::float32 max)
{
	pvr::assertion(0, "Not Implemented Yet");
	//pImpl->enqueue_internal<SetDepthBound>(SetDepthBound(min, max));
}

void CommandBufferBase_::setStencilCompareMask(types::StencilFace::Enum face, pvr::uint32 compareMask)
{
	pImpl->enqueue_internal<SetStencilCompareMask>(SetStencilCompareMask(face, compareMask));
}

void CommandBufferBase_::setStencilWriteMask(types::StencilFace::Enum face, pvr::uint32 writeMask)
{
	pImpl->enqueue_internal<SetStencilWriteMask>(SetStencilWriteMask(face, writeMask));
}

void CommandBufferBase_::setStencilReference(types::StencilFace::Enum face, pvr::uint32 ref)
{
	pImpl->enqueue_internal<SetStencilReference>(SetStencilReference(face, ref));
}

void CommandBufferBase_::setDepthBias(pvr::float32 depthBias, pvr::float32 depthBiasClamp, pvr::float32 slopeScaledDepthBias)
{
	Log(Log.Critical, "setDepthBias not implemented");
}

void CommandBufferBase_::setBlendConstants(glm::vec4 rgba)
{
	pImpl->enqueue_internal<SetBlendConstants>(SetBlendConstants(glm::vec4(rgba.r, rgba.g, rgba.b, rgba.a)));
}

void CommandBufferBase_::setLineWidth(float width)
{
	assertion(0, "Set line Width does not supported Currently");
	Log(Log.Critical, "SetLineWidth not implemented");
}

void CommandBufferBase_::drawIndirect(Buffer& buffer, pvr::uint32 offset, pvr::uint32 count, pvr::uint32 stride)
{
	assertion(0, "Draw Indirect does not supported Currently");
	Log(Log.Critical, "DrawIndirect not implemented");
}

void CommandBufferBase_::dispatchCompute(uint32 numGroupsX, uint32 numGroupsY, uint32 numGroupsZ)
{
	pImpl->enqueue_internal<DispatchCompute>(DispatchCompute(numGroupsX, numGroupsY, numGroupsZ));
}

#define SET_UNIFORM_DEFINITION(_type_)\
 template <>void CommandBufferBase_::setUniform<_type_>(int32 location, const _type_& val)\
{ pImpl->enqueue_internal<SetUniform<_type_>/**/>(SetUniform<_type_>(location, val)); }\
 template <>void CommandBufferBase_::setUniformPtr<_type_>(int32 location, pvr::uint32 count, const _type_* ptr) \
{ pImpl->enqueue_internal<SetUniformPtr<_type_>/**/>(SetUniformPtr<_type_>(location, count, ptr)); }

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
SET_UNIFORM_DEFINITION(glm::mat3);
SET_UNIFORM_DEFINITION(glm::mat4);
#undef SET_UNIFORM_DEFINITION

void CommandBufferBase_::pushPipeline()
{
	pImpl->enqueue_internal<PushPipeline>(PushPipeline());
}

void CommandBufferBase_::popPipeline()
{
	pImpl->enqueue_internal<PopPipeline>(PopPipeline());
}

void CommandBufferBase_::resetPipeline()
{
	pImpl->enqueue_internal<ResetPipeline>(ResetPipeline());
}
//
//void CommandBufferBase_::setMemoryBarrier(const PipelineBarrier& barrier)
//{
//	pImpl->enqueue_internal<PipelineBarrier>(barrier);
//}
//
//Sync CommandBufferBase_::insertFenceSync()
//{
//	CreateFenceSyncImpl fenceSyncImpl;
//	pImpl->enqueue_internal<CreateFenceSyncImpl>(fenceSyncImpl);
//	return fenceSyncImpl.syncObject;
//}


#ifdef DEBUG
void CommandBufferBase_::logCommandStackTraces()
{
	for (auto it = pImpl->queue.begin(); it != pImpl->queue.end(); ++it)
	{
		Log(Log.Debug, it->debug_commandCallSiteStackTrace.c_str());
	}
}
#endif


SecondaryCommandBuffer_::SecondaryCommandBuffer_(GraphicsContext& context, CommandPool& pool, native::HCommandBuffer_& cmdBuff) : CommandBufferBase_(context, pool, cmdBuff)
{
}

void SecondaryCommandBuffer_::beginRecording(const RenderPass& rp, uint32 subPass)
{
	pImpl->beginRecording();
}

void SecondaryCommandBuffer_::beginRecording(const Fbo& rp, uint32 subPass)
{
	pImpl->beginRecording();
}

CommandBuffer_::CommandBuffer_(GraphicsContext& context, CommandPool& pool, native::HCommandBuffer_& cmdBuff) : CommandBufferBase_(context, pool, cmdBuff)
{
}

void CommandBuffer_::submit()
{
	pImpl->submit(*this);
}

void CommandBuffer_::enqueueSecondaryCmds(SecondaryCommandBuffer& secondaryCmdBuffer)
{
	pImpl->enqueue_internal<SecondaryCommandBufferPackager>(secondaryCmdBuffer);
}

void CommandBuffer_::beginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineSubpass, const glm::vec4& clearColor
                                     , float32 clearDepth, uint32 clearStencil)
{
	pImpl->enqueue_internal<BeginRenderPass>(BeginRenderPass(fbo, renderArea, clearColor, clearDepth, clearStencil));
}

void CommandBuffer_::beginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass,
                                     const glm::vec4* clearColor, uint32 numClearColor, float32 clearDepth, uint32 clearStencil)
{
	pImpl->enqueue_internal<BeginRenderPass>(BeginRenderPass(fbo, renderArea, clearColor, numClearColor, clearDepth, clearStencil));
}


void CommandBuffer_::endRenderPass()
{
	pImpl->enqueue_internal<EndRenderPass>(EndRenderPass());
}

void CommandBuffer_::nextSubPassInline() {}

void CommandBuffer_::nextSubPassSecondaryCmds(pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	enqueueSecondaryCmds(cmdBuffer);
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

bool CommandBufferBase_::isRecording() { return pImpl->m_isRecording; }

void CommandBufferBase_::clear(bool releaseResources)
{
	pImpl->queue.clear();
}

void CommandBufferBase_::pipelineBarrier(
  types::PipelineStageFlags::Bits srcStage, types::PipelineStageFlags::Bits dstStage,
  const MemoryBarrierSet& barriers, bool dependencyByRegion /*= true*/)
{
	if (getContext()->getApiType() < Api::OpenGLES31) { return; }

	PipelineBarrier barrier;
	barrier.barrier = (GLenum)(size_t)barriers.getNativeMemoryBarriers();
	pImpl->enqueue_internal<PipelineBarrier>(barrier);
}

}
}
}
//!\endcond
