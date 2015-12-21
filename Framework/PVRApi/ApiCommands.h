/*!*********************************************************************************************************************
\file         PVRApi\ApiCommands.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Internal classes that are used by the CommandBuffer to represent user commands. Each class corresponds to a
              CommandBuffer command of the same name.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjectTypes.h"
#include "PVRApi/ApiCommand.h"
#include "PVRAssets/Model/Mesh.h"
#include "PVRApi/ApiObjects/RenderPass.h"
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRApi/ApiObjects/DescriptorTable.h"
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
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
};

class ResetPipeline : public ApiCommand
{
public:
	ResetPipeline() {}
private:
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
};

class PopPipeline : public ApiCommand
{
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	static void bindGraphicsPipeline(void* pipeline, IGraphicsContext& context);
	static void bindComputePipeline(void* pipeline, IGraphicsContext& context);
	PopPipeline() {}
};


class BindDescriptorSets : public ApiCommand
{
public:
	BindDescriptorSets(PipelineBindingPoint::Enum bindingPoint, const api::PipelineLayout& pipelineLayout,
	                   const DescriptorSet& set, uint32 dynamicOffset) : set(1, set), dynamicOffsets(1, dynamicOffset),
		pipeLayout(pipelineLayout) {}

	BindDescriptorSets(PipelineBindingPoint::Enum bindingPoint, const api::PipelineLayout& pipelineLayout,
	                   DescriptorSet* set, uint32* dynamicOffsets, uint32 count) :
		set(set, set + count), dynamicOffsets(dynamicOffsets, dynamicOffsets + count) {}
private:
	std::vector<DescriptorSet> set;
	std::vector<uint32> dynamicOffsets;
	api::PipelineLayout pipeLayout;
	void execute_private(impl::CommandBufferBaseImpl& cmd);
};


class SetClearDepthVal : public ApiCommand
{
public:
	SetClearDepthVal(float32 depthVal) : depthVal(depthVal) {}
private:
	float32 depthVal;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
};

class ClearColorAttachment : public ApiCommand
{
public:

	ClearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 const& clearColor, pvr::Rectanglei const& clearRect)
		: clearConst(attachmentCount, clearColor), clearRect(attachmentCount, clearRect)
	{}

	ClearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 const* clearColor, pvr::Rectanglei const* clearRect)
	{}
private:
	std::vector<glm::vec4> clearConst;
	std::vector<pvr::Rectanglei> clearRect;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
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
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
};

class SetClearStencilVal : public ApiCommand
{
	int32 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
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
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
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
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);

	uint16 startBinding;
	uint16 bindingCount;
	std::vector<api::Buffer> buffers;
	std::vector<uint32> offsets;
};

class BindIndexBuffer : public ApiCommand
{
public:
	BindIndexBuffer(const api::Buffer& buffer, uint32 offset, IndexType::Enum indexType)
		: buffer(buffer), offset(offset), indexType(indexType) {}

private:
	void execute_private(impl::CommandBufferBaseImpl&);
	api::Buffer buffer;
	uint32 offset;
	IndexType::Enum indexType;
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
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
};

class BeginRenderPass : public ApiCommand
{
public:
	BeginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, const glm::vec4& clearColor
	                = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0) :
		m_fbo(fbo), m_renderArea(renderArea), m_clearDepth(clearDepth), m_clearStencil(clearStencil)
	{
		m_clearColor.push_back(clearColor);
	}

private:
	api::Fbo m_fbo;
	std::vector<glm::vec4> m_clearColor;
	const Rectanglei m_renderArea;
	float32 m_clearDepth;
	uint32 m_clearStencil;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
};

class EndRenderPass : public ApiCommand
{
public:
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
};

class SetScissor : public ApiCommand
{
	const Rectanglei m_scissor;
public:
	SetScissor(pvr::Rectanglei const& scissor) : m_scissor(scissor) {}
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
};

class SetViewport : public ApiCommand
{
	const Rectanglei m_viewport;
public:
	SetViewport(pvr::Rectanglei const& viewport) : m_viewport(viewport) {}
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
};

class SetDepthBound : public ApiCommand
{
	const pvr::float32 m_min, m_max;
public:
	SetDepthBound(pvr::float32 min, pvr::float32 max) : m_min(min), m_max(max) {}
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
};

class SetStencilCompareMask : public ApiCommand
{
	const pvr::api::Face::Enum m_face;
	const pvr::uint32 m_mask;
public:
	SetStencilCompareMask(pvr::api::Face::Enum face, pvr::uint32 mask) : m_face(face), m_mask(mask) {}
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
};

class SetStencilWriteMask : public ApiCommand
{
	const pvr::api::Face::Enum m_face;
	const pvr::uint32 m_mask;
public:
	SetStencilWriteMask(pvr::api::Face::Enum face, pvr::uint32 mask) : m_face(face), m_mask(mask) {}
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
};

class SetStencilReference : public ApiCommand
{
	const pvr::api::Face::Enum m_face;
	const pvr::uint32 m_ref;
public:
	SetStencilReference(pvr::api::Face::Enum face, pvr::uint32 ref) : m_face(face), m_ref(ref) {}
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
};

class SetLineWidth : public ApiCommand
{
	const pvr::float32 m_lineWidth;
public:
	SetLineWidth(pvr::float32 lineWidth) : m_lineWidth(lineWidth) {}
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
};

class SetBlendConstants : public ApiCommand
{
	const glm::vec4 m_constants;
public:
	SetBlendConstants(glm::vec4 const& constants) : m_constants(constants) {}
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
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
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
	uint32 m_numGroupXYZ[3];
};


#ifndef PVR_NO_UNIFORM_SUPPORT

#pragma region Uniforms

template<typename _type> class SetUniformPtr;
template<typename _type> class SetUniform;


template<>
class SetUniform<float32> : public ApiCommand
{
	int32 location;
	float32 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, float32 val) : location(location), val(val) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<int32> : public ApiCommand
{
	int32 location;
	int32 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, int32 val) : location(location), val(val) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<uint32> : public ApiCommand
{
	int32 location;
	uint32 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(uint32 location, uint32 val) : location(location), val(val) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniformPtr<float32> : public ApiCommand
{
	const float32* val;
	int32 location;
	int32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, int32 count, const float32* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<int32> : public ApiCommand
{
	const int32* val;
	int32 location;
	int32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);

public:
	SetUniformPtr(int32 location, int32 count, const int32* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<uint32> : public ApiCommand
{
	const uint32* val;
	int32 location;
	int32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(uint32 location, uint32 count, const uint32* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniform<glm::ivec2> : public ApiCommand
{
	int32 location;
	glm::ivec2 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::ivec2& val) : location(location), val(val) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<glm::uvec2> : public ApiCommand
{
	int32 location;
	glm::uvec2 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::uvec2& val) : location(location), val(val) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<glm::vec2> : public ApiCommand
{
	int32 location;
	glm::vec2 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::vec2& val) : location(location), val(val) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniformPtr<glm::ivec2> : public ApiCommand
{
	int32 location;
	int32 count;
	const glm::ivec2* val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, int32 count, const glm::ivec2* val) : location(location), count(count), val(val) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<glm::uvec2> : public ApiCommand
{
	const glm::uvec2* val;
	int32 location;
	int32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, int32 count, const glm::uvec2* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<glm::vec2> : public ApiCommand
{
	const glm::vec2* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count, const glm::vec2* val) : val(val), location(location), count(count) {}

	virtual ~SetUniformPtr() {}
};

template<>
class SetUniform<glm::ivec3> : public ApiCommand
{
	int32 location;
	glm::ivec3 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::ivec3& val) : location(location), val(val) {}

	SetUniform(int32 location, int32 x, int32 y, int32 z) : location(location), val(x, y, z) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<glm::uvec3> : public ApiCommand
{
	int32 location;
	glm::uvec3 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::uvec3& val) : location(location), val(val) {}

	SetUniform(int32 location, uint32 x, uint32 y, uint32 z) : location(location), val(x, y, z) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<glm::vec3> : public ApiCommand
{
	int32 location;
	glm::vec3 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::vec3& val) : location(location), val(val) {}

	SetUniform(int32 location, float32 x, float32 y, float32 z) : location(location), val(x, y, z) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniformPtr<glm::ivec3> : public ApiCommand
{
	const glm::ivec3* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count, const glm::ivec3* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<glm::uvec3> : public ApiCommand
{
	const glm::uvec3* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count, const glm::uvec3* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<glm::vec3> : public ApiCommand
{
	const glm::vec3* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count, const glm::vec3* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniform<glm::ivec4> : public ApiCommand
{
	int32 location;
	glm::ivec4 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::ivec4& val) : location(location), val(val) {}

	SetUniform(int32 location, int32 x, int32 y, int32 z, int32 w) :
		location(location), val(x, y, z, w) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<glm::uvec4> : public ApiCommand
{
	int32 location;
	glm::uvec4 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::uvec4& val) : location(location), val(val) {}

	SetUniform(int32 location, uint32 x, uint32 y, uint32 z, uint32 w) :
		location(location), val(x, y, z, w) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<glm::vec4> : public ApiCommand
{
	int32 location;
	glm::vec4 val;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::vec4& val) : location(location), val(val) {}

	SetUniform(int32 location, float32 x, float32 y, float32 z, float32 w) :
		location(location), val(x, y, z, w) {}

	virtual ~SetUniform() {}
};

template<>
class SetUniform<glm::mat2> : public ApiCommand
{
	glm::mat2 val;
	int32 location;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::mat2& val) : val(val), location(location){}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<glm::mat3> : public ApiCommand
{
	glm::mat3 val;
	int32 location;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::mat3& val) : val(val), location(location) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniform<glm::mat4> : public ApiCommand
{
	glm::mat4 val;
	int32 location;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniform(int32 location, const glm::mat4& val) : val(val), location(location) {}
	virtual ~SetUniform() {}
};

template<>
class SetUniformPtr<glm::ivec4> : public ApiCommand
{
	const glm::ivec4* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count, const glm::ivec4* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<glm::uvec4> : public ApiCommand
{
	const glm::uvec4* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count, const glm::uvec4* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<glm::vec4> : public ApiCommand
{
	const glm::vec4* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count, const glm::vec4* val) : val(val), location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<glm::mat2> : public ApiCommand
{
	const glm::mat2* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count, const glm::mat2* val) : val(val),
		location(location), count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<glm::mat3> : public ApiCommand
{
	const glm::mat3* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count,const glm::mat3* val) : val(val), location(location),
		count(count) {}
	virtual ~SetUniformPtr() {}
};

template<>
class SetUniformPtr<glm::mat4> : public ApiCommand
{
	const glm::mat4* val;
	int32 location;
	uint32 count;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuff);
public:
	SetUniformPtr(int32 location, uint32 count, const glm::mat4* val) : val(val), location(location), count(count) { }
	virtual ~SetUniformPtr() {}
};
#pragma endregion

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

class CreateFenceSyncImpl;

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
class SyncImpl
{
	friend class ::pvr::api::impl::CreateFenceSyncImpl;
	friend class ::pvr::api::impl::CommandBufferBaseImpl;
	template<typename>  friend struct ::pvr::RefCountEntryIntrusive;
	pvr::RingBuffer<void*> pimpl;
	uint32 maxSize;
	SyncImpl();
	~SyncImpl();
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
	SyncWaitResult::Enum clientWait(uint32 which, uint64 timeout = 0);
};
}

/*!****************************************************************************************************************
\brief Reference-counted handle to a Sync object.
Default constructor returns an empty handle that wraps a NULL object.
Use the CommandBuffer's insertFenceSync to construct a Sync.
As with all reference-counted handles, access with the arrow operator.
*******************************************************************************************************************/
typedef RefCountedResource<impl::SyncImpl> Sync;

namespace impl {
//!\cond NO_DOXYGEN
class CreateFenceSyncImpl : public ApiCommand
{
	friend class ::pvr::api::impl::CommandBufferBaseImpl;
	Sync syncObject;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
	CreateFenceSyncImpl() { syncObject.construct(); }
};
//!\endcond
}

/*!*********************************************************************************************************************
\brief  A memory barrier into the command stream. Used to signify that some types of pending operations from
before the barrier must have finished before the commands after the barrier start executing.
***********************************************************************************************************************/
class PipelineBarrier : public ApiCommand
{
private:
	struct Barrier
	{
		enum Type
		{
			ImageMemoryBarrier,
			MemoryBarrier,
			BufferMemoryBarrier
		};
		uint32 m_inputMask, m_outputMask;
		Type		m_type;
	};
	std::vector<Barrier> m_barriers;
	uint32 m_eventCount;
	void execute_private(impl::CommandBufferBaseImpl& cmdBuffer);
public:
	/*!*********************************************************************************************************************
	\brief  A memory barrier into the command stream. Used to signify that some types of pending operations from
	before the barrier must have finished before the commands after the barrier start executing.
	***********************************************************************************************************************/
	void addMemoryBarrier(uint32 inputMask, uint32 outPutMask)
	{
		m_barriers.push_back(Barrier());
		Barrier& barrier = m_barriers.back();
		barrier.m_type = Barrier::MemoryBarrier;
		barrier.m_inputMask = inputMask;
		barrier.m_outputMask = outPutMask;
	}
};
}
}