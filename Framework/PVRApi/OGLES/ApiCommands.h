/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ApiCommands.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Internal classes that are used by the CommandBuffer to represent user commands. Each class corresponds to a
              CommandBuffer command of the same name.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRApi/OGLES/ApiCommand.h"
#include "PVRAssets/Model/Mesh.h"
#include "PVRApi/ApiObjects/RenderPass.h"
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRCore/RingBuffer.h"
#include <vector>
namespace pvr {
class IGraphicsContext;
namespace api {
namespace impl {

/*! \cond NO_DOXYGEN*/
class PushPipeline : public ApiCommand
{
public:
	PushPipeline() {}
private:
	void execute_private(impl::CommandBufferBase_& cmdBuff);
};

class ResetPipeline : public ApiCommand
{
public:
	ResetPipeline() {}
private:
	void execute_private(impl::CommandBufferBase_& cmdBuff);
};

class PopPipeline : public ApiCommand
{
	void execute_private(impl::CommandBufferBase_& cmdBuff);
public:
	static void bindGraphicsPipeline(void* pipeline, IGraphicsContext& context);
	static void bindComputePipeline(void* pipeline, IGraphicsContext& context);
	PopPipeline() {}
};


class BindDescriptorSets : public ApiCommand
{
public:
	BindDescriptorSets(types::PipelineBindPoint bindingPoint, const api::PipelineLayout& pipelineLayout,
	                   const DescriptorSet* set, uint32 numSet, const uint32* dynamicOffsets, uint32 numDynamicOffset) :
		set(set, set + numSet), dynamicOffsets(dynamicOffsets, dynamicOffsets + numDynamicOffset) {}
private:
	std::vector<DescriptorSet> set;
	std::vector<uint32> dynamicOffsets;
	api::PipelineLayout pipeLayout;
	void execute_private(impl::CommandBufferBase_& cmd);
};


class SetClearDepthVal : public ApiCommand
{
public:
	SetClearDepthVal(float32 depthVal) : depthVal(depthVal) {}
private:
	float32 depthVal;
	void execute_private(impl::CommandBufferBase_& cmdBuff);
};

class ClearColorImage : public ApiCommand
{
public:

	ClearColorImage(pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32 baseMipLevel = 0u,
	                const pvr::uint32 baseArrayLayer = 0u, const pvr::uint32 layerCount = 1u)
		: imageToClear(image), clearColor(clearColor), baseMipLevel(baseMipLevel),
		  baseArrayLayer(baseArrayLayer), layerCount(layerCount)
	{}

private:
	pvr::api::TextureView& imageToClear;
	glm::vec4 clearColor;
	pvr::uint32 baseMipLevel;
	pvr::uint32 baseArrayLayer;
	pvr::uint32 layerCount;
	void execute_private(impl::CommandBufferBase_& cmdBuff);
};

class ClearDepthStencilImage : public ApiCommand
{
public:

	ClearDepthStencilImage(pvr::api::TextureView& image, float clearDepth, pvr::uint32 clearStencil, const pvr::uint32 baseMipLevel = 0u,
	                       const pvr::uint32 baseArrayLayer = 0u, const pvr::uint32 layerCount = 1u)
		: imageToClear(image), clearDepth(clearDepth), clearStencil(clearStencil),
		  baseMipLevel(baseMipLevel),
		  baseArrayLayer(baseArrayLayer), layerCount(layerCount)
	{}

private:
	pvr::api::TextureView& imageToClear;
	float clearDepth;
	pvr::uint32 clearStencil;
	pvr::uint32 baseMipLevel;
	pvr::uint32 baseArrayLayer;
	pvr::uint32 layerCount;
	void execute_private(impl::CommandBufferBase_& cmdBuff);
};

class ClearColorAttachment : public ApiCommand
{
public:

	ClearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 const& clearColor, pvr::uint32 rectCount, pvr::Rectanglei const& clearRect)
		: clearConst(attachmentCount, clearColor), clearRect(rectCount, clearRect)
	{}

	ClearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 const* clearColor, pvr::uint32 rectCount, pvr::Rectanglei const* clearRect)
	{
		for (unsigned int i = 0; i < attachmentCount; i++)
		{
			this->clearConst.push_back(clearColor[i]);
		}
		for (unsigned int i = 0; i < rectCount; i++)
		{
			this->clearRect.push_back(clearRect[i]);
		}
	}
private:
	std::vector<glm::vec4> clearConst;
	std::vector<pvr::Rectanglei> clearRect;
	void execute_private(impl::CommandBufferBase_& cmdBuff);
};

class ClearDepthStencilAttachment : public ApiCommand
{
public:
	enum ClearBits
	{
		Depth = 0x01, Stencil = 0x02
	};

	ClearDepthStencilAttachment(float32 depth, pvr::Rectanglei const& clearRect) :
		clearDepth(depth), clearBits(ClearBits::Depth), rect(clearRect) {}

	ClearDepthStencilAttachment(pvr::int32 stencil, pvr::Rectanglei const& clearRect) :
		clearStencil(stencil), clearBits(ClearBits::Stencil), rect(clearRect) {}

	ClearDepthStencilAttachment(pvr::float32 depth, pvr::int32 stencil, pvr::Rectanglei const& clearRect) :
		clearDepth(depth), clearStencil(stencil), clearBits(ClearBits::Depth | ClearBits::Stencil), rect(clearRect) {}
private:
	pvr::float32 clearDepth;
	pvr::int32 clearStencil;
	pvr::uint32 clearBits;
	pvr::Rectanglei rect;
	void execute_private(impl::CommandBufferBase_& cmdBuff);
};

class SetClearStencilVal : public ApiCommand
{
	int32 val;
	void execute_private(impl::CommandBufferBase_& cmdBuff);
public:
	SetClearStencilVal(int32 stencilval) : val(stencilval) {}
};

class DrawIndexed : public ApiCommand
{
public:
	DrawIndexed(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset = 0,
	            uint32 firstInstance = 0, uint32 instanceCount = 1) : firstIndex(firstIndex),
		indexCount(indexCount), vertexOffset(vertexOffset), firstInstance(firstInstance),
		instanceCount(instanceCount) {}
private:
	void execute_private(impl::CommandBufferBase_& cmdBuff);
	uint32 firstIndex;
	uint32 indexCount;
	uint32 vertexOffset;
	uint32 firstInstance;
	uint32 instanceCount;
};

class BindVertexBuffer : public ApiCommand
{

public:
	BindVertexBuffer(const Buffer& buffer, uint32 offset, uint16 bindingIndex) :
		startBinding(bindingIndex), bindingCount(1)
	{
		buffers.push_back(buffer);
		offsets.push_back(offset);
	}

	BindVertexBuffer(Buffer const* buffers, uint32* offsets,
	                 uint16 numBuffers, uint16 startBinding,
	                 uint16 bindingCount) :
		startBinding(startBinding), bindingCount(bindingCount),
		buffers(buffers, buffers + numBuffers),
		offsets(offsets, offsets + numBuffers)
	{
	}
private:
	void execute_private(impl::CommandBufferBase_& cmdBuff);

	uint16 startBinding;
	uint16 bindingCount;
	std::vector<api::Buffer> buffers;
	std::vector<uint32> offsets;
};

class BindIndexBuffer : public ApiCommand
{
public:
	BindIndexBuffer(const api::Buffer& buffer, uint32 offset, types::IndexType indexType)
		: buffer(buffer), offset(offset), indexType(indexType) {}

private:
	void execute_private(impl::CommandBufferBase_&);
	api::Buffer buffer;
	uint32 offset;
	types::IndexType indexType;
};

class DrawArrays : public ApiCommand
{
public:
	DrawArrays(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance,
	           uint32 instanceCount) : firstVertex(firstVertex), vertexCount(vertexCount),
		firstInstance(firstInstance), instanceCount(instanceCount) {}
private:
	uint32						firstVertex;
	uint32						vertexCount;
	uint32						firstInstance;
	uint32						instanceCount;
	void execute_private(impl::CommandBufferBase_& cmdBuff);
};

class BeginRenderPass : public ApiCommand
{
public:
	BeginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea,
	                const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	                float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	                uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue) :
		m_fbo(fbo), m_renderArea(renderArea), m_clearDepth(clearDepth), m_clearStencil(clearStencil)
	{
		m_clearColor.push_back(clearColor);
	}

	BeginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, const glm::vec4*& clearColor, uint32 numClearColor,
	                float32 clearDepth = 1.f, uint32 clearStencil = 0) :
		m_fbo(fbo), m_renderArea(renderArea), m_clearDepth(clearDepth), m_clearStencil(clearStencil),
		m_clearColor(clearColor, clearColor + numClearColor)
	{}

private:
	api::Fbo m_fbo;
	std::vector<glm::vec4> m_clearColor;
	const Rectanglei m_renderArea;
	float32 m_clearDepth;
	uint32 m_clearStencil;
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class EndRenderPass : public ApiCommand
{
public:
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class SetScissor : public ApiCommand
{
	const Rectanglei m_scissor;
public:
	SetScissor(pvr::Rectanglei const& scissor) : m_scissor(scissor) {}
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class SetViewport : public ApiCommand
{
	const Rectanglei m_viewport;
public:
	SetViewport(pvr::Rectanglei const& viewport) : m_viewport(viewport) {}
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class SetDepthBound : public ApiCommand
{
	const pvr::float32 m_min, m_max;
public:
	SetDepthBound(pvr::float32 min, pvr::float32 max) : m_min(min), m_max(max) {}
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class UpdateBuffer : public ApiCommand
{
public:
	UpdateBuffer(api::Buffer& buffer, uint32 offset, uint32 length, const void* data) :
		buffer(buffer), offset(offset), data(data), length(length) {}

	void execute_private(impl::CommandBufferBase_& cmdBuffer);
	api::Buffer buffer;
	pvr::uint32 offset;
	pvr::uint32 length;
	const void* data;
};

class SetStencilCompareMask : public ApiCommand
{
	const types::StencilFace m_face;
	const pvr::uint32 m_mask;
public:
	SetStencilCompareMask(types::StencilFace face, pvr::uint32 mask) : m_face(face), m_mask(mask) {}
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class SetStencilWriteMask : public ApiCommand
{
	const types::StencilFace m_face;
	const pvr::uint32 m_mask;
public:
	SetStencilWriteMask(types::StencilFace face, pvr::uint32 mask) : m_face(face), m_mask(mask) {}
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class SetStencilReference : public ApiCommand
{
	const types::StencilFace m_face;
	const pvr::uint32 m_ref;
public:
	SetStencilReference(types::StencilFace face, pvr::uint32 ref) : m_face(face), m_ref(ref) {}
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class SetLineWidth : public ApiCommand
{
	const pvr::float32 m_lineWidth;
public:
	SetLineWidth(pvr::float32 lineWidth) : m_lineWidth(lineWidth) {}
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class SetBlendConstants : public ApiCommand
{
	const glm::vec4 m_constants;
public:
	SetBlendConstants(glm::vec4 const& constants) : m_constants(constants) {}
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};

class DispatchCompute : public ApiCommand
{
public:
	DispatchCompute(uint32 numGroupDispatchX, uint32 numGroupDispatchY = 1, uint32 numGroupDispatchZ = 1)
	{
		m_numGroupXYZ[0] = numGroupDispatchX, m_numGroupXYZ[1] = numGroupDispatchY,
		                   m_numGroupXYZ[2] = numGroupDispatchZ;
	}
private:
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
	uint32 m_numGroupXYZ[3];
};


#ifndef PVR_NO_UNIFORM_SUPPORT

template<typename _type> class SetUniformPtr;
template<typename _type> class SetUniform;


#define SET_UNIFORM_CLASS_DEFINITION(type) \
template<> class SetUniform<type> : public ApiCommand \
{	int32 location;  type val; \
	void execute_private(impl::CommandBufferBase_& cmdBuff); \
public: \
	SetUniform(int32 location, type val) : location(location), val(val) {} \
	virtual ~SetUniform() {} }; \
\
template<> class SetUniformPtr<type> : public ApiCommand \
{ 	const type* val; int32 location; int32 count; \
	void execute_private(impl::CommandBufferBase_& cmdBuff); \
public: \
	SetUniformPtr(int32 location, uint32 count, const type* val) : val(val), location(location), count(count) {} \
	virtual ~SetUniformPtr() {} \
}; \

#define SET_UNIFORM_CLASS_DEFINITION_LARGE(type) \
template<> class SetUniform<type> : public ApiCommand \
{	int32 location;  type val; \
	void execute_private(impl::CommandBufferBase_& cmdBuff); \
public: \
	SetUniform(int32 location, const type& val) : location(location), val(val) {} \
	virtual ~SetUniform() {} }; \
\
template<> class SetUniformPtr<type> : public ApiCommand \
{ 	const type* val; int32 location; int32 count; \
	void execute_private(impl::CommandBufferBase_& cmdBuff); \
public: \
	SetUniformPtr(int32 location, uint32 count, const type* val) : val(val), location(location), count(count) {} \
	virtual ~SetUniformPtr() {} \
}; \

SET_UNIFORM_CLASS_DEFINITION(float32)
SET_UNIFORM_CLASS_DEFINITION(int32)
SET_UNIFORM_CLASS_DEFINITION(uint32)
SET_UNIFORM_CLASS_DEFINITION(glm::vec2)
SET_UNIFORM_CLASS_DEFINITION(glm::ivec2)
SET_UNIFORM_CLASS_DEFINITION(glm::uvec2)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::vec3)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::ivec3)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::uvec3)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::vec4)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::ivec4)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::uvec4)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::mat2)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::mat2x3)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::mat2x4)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::mat3x2)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::mat3x3)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::mat3x4)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::mat4x2)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::mat4x3)
SET_UNIFORM_CLASS_DEFINITION_LARGE(glm::mat4x4)

#endif

/*! \endcond */


struct SyncWaitResult
{
	enum Enum
	{
		Ok = 0,
		SyncPointNotCreatedYet,
		TimeoutExpired,
		Failed
	};
};

class CreateFenceSync_;

/*!*********************************************************************************************************************
\brief  The Sync object can be used both as an API command, or to be directly queried by the application.
Each time the CommandBuffer is submitted, an underlying sync object is added to the list. If the MaxSize is reached, the
first sync object to be submitted is discarder. The user normally queries/uses the syncs last-to-first with an index. This
technique makes it trivial to use frame-lag techniques, like double/triple buffering.
Each Sync starts in a non-signaled state, and becomes Signaled as soon as the GPU actually executes it previous command.
All of the functions work on a last-to-first indexing - passing 0 refers to the last command buffer submission, and from
there backwards in time, until getMaxSize() - 1, which is the earliest sync point used.
Use:
Get a sync object with insertFenceSync().
2) Use clientWait() to CPU-block until the Sync becomes Signaled.
3) Use isSignaled() to query if the Sync is Signaled, without blocking.
4) Use commandBuffer.serverSync() to force the implementation to wait for the specified sync object before proceeding.
***********************************************************************************************************************/
class Sync_
{
	friend class ::pvr::api::impl::CreateFenceSync_;
	friend class ::pvr::api::impl::CommandBufferBase_;
	template<typename>  friend struct ::pvr::RefCountEntryIntrusive;
	pvr::RingBuffer<void*> pimpl;
	uint32 maxSize;
	Sync_();
	~Sync_();
	void serverWait(uint32 which);
public:

	/*!*********************************************************************************************************************
	\brief  Manually discards the last howMany sync points. Specialised use only.
	\param  howMany The number of sync points to discard. Non positive numbers have no effect.
	***********************************************************************************************************************/
	void discardLast(int32 howMany = 1);

	/*!*********************************************************************************************************************
	\brief  Get the maximum number of syncs that will be created. Each time the command buffer that created this sync object
	(with the insertFenceSync call) is submitted, another sync is created. When MaxSize is reached, the first sync to be
	created is discarded.
	\return The maximum number of sync points that this object represents.
	***********************************************************************************************************************/
	uint32 getMaxSize() { return this->maxSize; }

	/*!*********************************************************************************************************************
	\brief  Set the maximum number of syncs that will be created. Each time the command buffer that created this sync object
	(with the insertFenceSync call) is submitted, another sync is created. When MaxSize is reached, the first sync to be
	created is discarded.
	\param maxSize The maximum number of sync points that this object represents will be set to this number
	***********************************************************************************************************************/
	void setMaxSize(uint32 maxSize) { this->maxSize = maxSize; }

	/*!*********************************************************************************************************************
	\brief  Call this function to test if a Sync is signalled, without blocking for it. Each call to "submit" on the command
			buffer that created this sync, will push a new sync point to the front of this queue.
	\param which The ordinal, newest-to-oldest, of the sync point to query (0=last submit, 1=previous frame etc.)
	\return True if the Sync is signaled (hence its previous commands are complete), false otherwise
	***********************************************************************************************************************/
	bool isSignaled(uint32 which);

	/*!*********************************************************************************************************************
	\brief  Call this function to wait on the sync object (i.e. wait for it to become Signalled, meaning that any commands
			preceding it are complete)
	\param timeout Optional: Provide a timeout. If 0 is passed, the wait is infinite.
	\param which The fence sync to wait on (0 is the latest, maxSize is the earliest).
	\return * SyncResult::Ok if the Sync was either already signaled or became signaled within the timeout (the preceding
	        commands have finished)
			* SyncResult::CommandBufferNotSubmitted if the commandBuffer that would generat the Sync has not yet been
			submitted, hence the sync will never be signalled (so waiting on it would probably result in a deadlock).
			* SyncResult::TimeoutExpired, if the Sync was NOT signalled, but the timeout expired.
			* SyncResult::Failed, if the implementation failed to wait (for example, the sync was already destroyed)
	***********************************************************************************************************************/
	SyncWaitResult clientWait(uint32 which, uint64 timeout = 0);
};
}

/*!****************************************************************************************************************
\brief Reference-counted handle to a Sync object.
Default constructor returns an empty handle that wraps a NULL object.
Use the CommandBuffer's insertFenceSync to construct a Sync.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef RefCountedResource<impl::Sync_> Sync;

namespace impl {
//!\cond NO_DOXYGEN
class CreateFenceSync_ : public ApiCommand
{
	friend class ::pvr::api::impl::CommandBufferBase_;
	Sync syncObject;
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
	CreateFenceSync_() { syncObject.construct(); }
};
//!\endcond
}
/*!*********************************************************************************************************************
\brief  A memory barrier into the command stream. Used to signify that some types of pending operations from
before the barrier must have finished before the commands after the barrier start executing.
***********************************************************************************************************************/
class PipelineBarrier : public ApiCommand
{
public:
	uint32 barrier;
	void execute_private(impl::CommandBufferBase_& cmdBuffer);
};
}
}
//!\endcond