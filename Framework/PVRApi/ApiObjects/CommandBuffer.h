/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\CommandBuffer.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the CommandBuffer implementation.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiCommand.h"
#include "PVRApi/ApiCommands.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRCore/StackTrace.h"
#define USE_NONHOMOGENEOUS_LIST
#ifdef USE_NONHOMOGENEOUS_LIST
#include "PVRCore/ListOfInterfaces.h"
#else
#include <deque>
#endif
namespace pvr {
//Forward Declare
class IGraphicsContext;
namespace api {
namespace impl {

//!\cond NO_DOXYGEN
class CommandBufferBaseImpl;
//Special internal class used by the CommandBuffer. It packages an API object that can be bound.
template <typename Resource>
class PackagedBindable : public ApiCommand
{
public:
	Resource res;
	PackagedBindable(const Resource& res) : res(res) {}

	void execute_private(CommandBufferBaseImpl& cmdBuf);
	virtual ~PackagedBindable() { }
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

	void execute_private(CommandBufferBaseImpl& cmdBuf);
	virtual ~PackagedBindableWithParam() {}
};
//!\endcond
/*!*********************************************************************************************************************
\brief Contains all the commands and states that need to be submitted to the gpu, including pipeline, texture, and
		samplers. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer.
		In debug builds (define DEBUG or define PVR_STORE_STACK_TRACE_WITH_API_COMMANDS), a limited stack trace 
		is stored with each command so that if an error occurs, the site where the command was actually added to the 
		command buffer can be determined.
		
		Primary command buffers can contain RenderPasses, and can be submitted to the GPU.
		Secondary command buffers cannot contain RenderPasses, and can be enqueued to PrimaryCommandBuffers.

		-It is invalid to submit commands to a command buffer while it is not being recorded.
		-It is invalid to reset a command buffer while it is being recorded.
		-It is invalid to submit a command buffer more than once if it is one time submit command buffer
***********************************************************************************************************************/
class CommandBufferBaseImpl
{
public:
	/*!*********************************************************************************************************************
	\brief Destructor. Frees all resources.
	***********************************************************************************************************************/
	virtual ~CommandBufferBaseImpl() { clear(); }

	/*!*********************************************************************************************************************
	\brief Get a reference to the context used by this CommandBuffer.
	***********************************************************************************************************************/
	GraphicsContext& getContext() { return context; }

	/*!****************************************************************************************************************
	\brief	Call this function before beginning to record commands.
	*******************************************************************************************************************/
	void beginRecording();

	/*!****************************************************************************************************************
	\brief	Call this function when you are done recording commands. BeginRecording must be called first.
	*******************************************************************************************************************/
	void endRecording();

	/*!****************************************************************************************************************
	\brief	Queries if a command buffer is in the recording state
	\return	True if recording, false otherwise
	*******************************************************************************************************************/
	bool isRecording() { return m_isRecording; }

	/*!****************************************************************************************************************
	\brief	Bind a graphics pipeline.
	\param	pipeline The GraphicsPipeline to bind.
	*******************************************************************************************************************/
	void bindPipeline(GraphicsPipeline& pipeline)
	{
		enqueue_internal<PackagedBindable<GraphicsPipeline> >(pipeline);
	}

	/*!****************************************************************************************************************
	\brief	Bind a parentable graphics pipeline
	\param	pipeline The ParentableGraphicsPipeline to bind
	*******************************************************************************************************************/
	void bindPipeline(ParentableGraphicsPipeline& pipeline)
	{
		enqueue_internal<PackagedBindable<ParentableGraphicsPipeline> >(pipeline);
	}

	/*!****************************************************************************************************************
	\brief	Bind a compute pipeline
	\param	pipeline The ComputePipeline to bind
	*******************************************************************************************************************/
	void bindPipeline(ComputePipeline& pipeline)
	{
		enqueue_internal<PackagedBindable<ComputePipeline> >(pipeline);
	}

	/*!****************************************************************************************************************
	\brief	Bind a single DescriptorSet
	\param	bindingPoint The index of the descriptor set to bind to
	\param	pipelineLayout The pipelineLayout that the GraphicsPipeline will have
	\param	set The descriptorSet to bind to the binding point bindingPoint
	\param	dynamicOffset The Offset that will be used when binding items of this descriptor set
	*******************************************************************************************************************/
	void bindDescriptorSets(PipelineBindingPoint::Enum bindingPoint, const api::PipelineLayout& pipelineLayout,
	                        const DescriptorSet& set, uint32 dynamicOffset)
	{
		enqueue_internal<BindDescriptorSets>(BindDescriptorSets(bindingPoint, pipelineLayout, set, dynamicOffset));
	}

	/*!****************************************************************************************************************
	\brief	Bind multiple DescriptorSets
	\param	bindingPoint The index where the first descriptor set will bind to. The rest will be bound successively.
	\param	pipelineLayout The pipelineLayout that the GraphicsPipeline will have
	\param	sets The array of descriptorSets to bind to the binding points
	\param	dynamicOffsets An array of Offsets that will be used when binding items of this descriptor set respectively
	\param	count The number of descriptor sets in the array
	*******************************************************************************************************************/
	void bindDescriptorSets(PipelineBindingPoint::Enum bindingPoint, const api::PipelineLayout& pipelineLayout,
	                        DescriptorSet* sets, uint32* dynamicOffsets, uint32 count)
	{
		enqueue_internal<BindDescriptorSets>(BindDescriptorSets(bindingPoint, pipelineLayout, sets, dynamicOffsets, count));
	}

	/*!****************************************************************************************************************
	\brief	Clear multiple attachments with separate clear colors and clear rectangle for each.
	\param	attachmentCount Number of attachments to clear
	\param	clearColors An array of colors to clear to, each corresponding to an attachment
	\param  rects An array of rectangles, each corresponding to the clear area of an attachment
	*******************************************************************************************************************/
	void clearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 const* clearColors, const pvr::Rectanglei* rects)
	{
		enqueue_internal<ClearColorAttachment>(ClearColorAttachment(attachmentCount, clearColors, rects));
	}

	/*!****************************************************************************************************************
	\brief	Clear multiple attachment with a single clear color and a single rectangle
	\param	attachmentCount The number of attachments
	\param	clearColor The clear area
	\param	rect The rectangle to clear
	*******************************************************************************************************************/
	void clearColorAttachment(pvr::uint32 attachmentCount, glm::vec4 clearColor, const pvr::Rectanglei rect)
	{
		enqueue_internal<ClearColorAttachment>(ClearColorAttachment(attachmentCount, clearColor, rect));
	}

	/*!****************************************************************************************************************
	\brief	Clear the depth attachment of an fbo
	\param	clearRect The clear area
	\param	depth The clear value
	*******************************************************************************************************************/
	void clearDepthAttachment(const pvr::Rectanglei & clearRect, float32 depth = 1.f)
	{
		enqueue_internal<ClearDepthStencilAttachment>(ClearDepthStencilAttachment(depth, clearRect));
	}

	/*!****************************************************************************************************************
	\brief	Clear the stencil attachment of an fbo
	\param	clearRect The clear area
	\param	stencil The clear value
	*******************************************************************************************************************/
	void clearStencilAttachment(const pvr::Rectanglei & clearRect, pvr::int32 stencil = 0)
	{
		enqueue_internal<ClearDepthStencilAttachment>(ClearDepthStencilAttachment(stencil, clearRect));
	}

	/*!****************************************************************************************************************
	\brief	Clear the depth stencil attachment
	\param	clearRect clear area
	\param	depth The depth clear value
	\param	stencil The stencil clear value
	*******************************************************************************************************************/
	void clearDepthStencilAttachment(const pvr::Rectanglei & clearRect, float32 depth = 1.f, pvr::int32 stencil = 0)
	{
		enqueue_internal<ClearDepthStencilAttachment>(ClearDepthStencilAttachment(depth, stencil, clearRect));
	}

	/*!****************************************************************************************************************
	\brief	Draw command. Use the current state in the command buffer (pipelines, buffers, descriptor sets) to execute
	        a drawing command. Uses the currently bound IndexBuffer for indexes into the currently bound VBOs.
	\param	firstIndex Offset into the Index buffer to start drawing from
	\param	indexCount The number of indexes to draw
	\param	vertexOffset Index into the Vertex Buffer to start drawing from
	\param	firstInstance The Instance from which to draw from
	\param	instanceCount The number of instances to draw
	*******************************************************************************************************************/
	void drawIndexed(uint32 firstIndex, uint32 indexCount, uint32 vertexOffset = 0,
	                 uint32 firstInstance = 0, uint32 instanceCount = 1)
	{
		enqueue_internal<DrawIndexed>(DrawIndexed(firstIndex, indexCount, vertexOffset, firstInstance, instanceCount));
	}

	/*!****************************************************************************************************************
	\brief	Draw command. Use the current state in the command buffer (pipelines, buffers, descriptor sets) to execute
	        a drawing command. Does not use an IndexBuffer.
	\param	firstVertex The vertex from where to start drawing
	\param	vertexCount The number of vertices to draw
	\param	firstInstance The Instance from which to draw from
	\param	instanceCount The number of instances to draw
	*******************************************************************************************************************/
	void drawArrays(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance = 0, uint32 instanceCount = 1)
	{
		enqueue_internal<DrawArrays>(DrawArrays(firstVertex, vertexCount, firstInstance, instanceCount));
	}

	/*!****************************************************************************************************************
	\brief	Indirect Draw command. Use buffer to obtain the draw call parameters.
	\param	buffer A buffer containing the draw call parameters in the form of a DrawIndexedIndirectCommand
	*******************************************************************************************************************/
	void drawIndexedIndirect(Buffer& buffer)
	{
        PVR_ASSERT("DrawIndexedIndirect does not supported Currently");
		Log(Log.Critical, "DrawIndexedIndirect not implemented");
	}

	/*!****************************************************************************************************************
	\brief	Bind a Vertex Buffer for drawing
	\param	buffer The vertex buffer to bind
	\param	offset The offset into the vertex buffer to bind
	\param	bindingIndex The Vertex Buffer index to bind the vertex buffer to
	*******************************************************************************************************************/
	void bindVertexBuffer(const Buffer& buffer, uint32 offset, uint16 bindingIndex)
	{
		enqueue_internal<BindVertexBuffer>(BindVertexBuffer(buffer, offset, bindingIndex));
	}

	/*!****************************************************************************************************************
	\brief	Bind an array of Vertex Buffers
	\param	buffers The array of buffers
	\param	offsets The array of offsets into the vertex buffer, each corresponding to a vertex buffer
	\param	startBinding The binding index that the first buffer will be bound
	\param	numBuffers The number of buffers to bind
	\param	bindingCount The number of bindings
	*******************************************************************************************************************/
	void bindVertexBuffer(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding,
	                      uint16 bindingCount)
	{
		enqueue_internal<BindVertexBuffer>(BindVertexBuffer(buffers, offsets, numBuffers, startBinding, bindingCount));
	}

	/*!****************************************************************************************************************
	\brief	Bind an index buffer for drawing
	\param	buffer The buffer to bind as an IndexBuffer
	\param	offset The offset into the Index buffer to bind
	\param	indexType the type of indices the buffer contains
	*******************************************************************************************************************/
	void bindIndexBuffer(const api::Buffer& buffer, uint32 offset, IndexType::Enum indexType)
	{
		enqueue_internal<BindIndexBuffer>(BindIndexBuffer(buffer, offset, indexType));
	}

	/*!****************************************************************************************************************
	\brief	Set the viewport rectangle
	\param	viewport The viewport rectangle
	*******************************************************************************************************************/
	void setViewport(const pvr::Rectanglei & viewport)
	{
		enqueue_internal<SetViewport>(SetViewport(viewport));
	}

	/*!****************************************************************************************************************
	\brief	Set the scissor rectangle
	\param	scissor The scissor rectangle
	*******************************************************************************************************************/
	void setScissor(const pvr::Rectanglei & scissor)
	{
		enqueue_internal<SetScissor>(SetScissor(scissor));
	}

	/*!****************************************************************************************************************
	\brief	Set minimum and maximum depth
	\param	min Minimum depth (default 0.0f)
	\param	max Maximum depth (default 1.0f)
	*******************************************************************************************************************/
	void setDepthBound(pvr::float32 min = 0.0f, pvr::float32 max = 1)
	{
		enqueue_internal<SetDepthBound>(SetDepthBound(min, max));
	}

	/*!****************************************************************************************************************
	\brief	Set the stencil comparison mask
	\param	face The face/faces for which to set the stencil mask
	\param	compareMask A uint32 which will mask both the values and the reference before stencil comparisons
	*******************************************************************************************************************/
	void setStencilCompareMask(pvr::api::Face::Enum face, pvr::uint32 compareMask)
	{
		enqueue_internal<SetStencilCompareMask>(SetStencilCompareMask(face, compareMask));
	}

	/*!****************************************************************************************************************
	\brief	Set the stencil write mask
	\param	face The face/faces for which to set the stencil write mask
	\param	writeMask A uint32 which will mask the values when writing the stencil
	*******************************************************************************************************************/
	void setStencilWriteMask(pvr::api::Face::Enum face, pvr::uint32 writeMask)
	{
		enqueue_internal<SetStencilWriteMask>(SetStencilWriteMask(face, writeMask));
	}

	/*!****************************************************************************************************************
	\brief	Set stencil reference value
	\param	face The face/faces for which to set the stencil reference value
	\param	ref The stencil reference value
	*******************************************************************************************************************/
	void setStencilReference(pvr::api::Face::Enum face, pvr::uint32 ref)
	{
		enqueue_internal<SetStencilReference>(SetStencilReference(face, ref));
	}

	void setDepthBias(pvr::float32 depthBias, pvr::float32 depthBiasClamp, pvr::float32 slopeScaledDepthBias)
	{
		Log(Log.Critical, "setDepthBias not implemented");
	}

	/*!****************************************************************************************************************
	\brief	Set blend constants for blend operation using constant colors
	\param	r Red blend constant
	\param	g Green blend constant
	\param	b Blue blend constant
	\param	a Alpha blend constant
	*******************************************************************************************************************/
	void setBlendConstants(pvr::float32 r, pvr::float32 g, pvr::float32 b, pvr::float32 a)
	{
		enqueue_internal<SetBlendConstants>(SetBlendConstants(glm::vec4(r, g, b, a)));
	}

	/*!****************************************************************************************************************
	\brief	Set the line width
	*******************************************************************************************************************/
	void setLineWidth()
	{
        PVR_ASSERT("Set line Width does not supported Currently");
		Log(Log.Critical, "SetLineWidth not implemented");
	}

	/*!****************************************************************************************************************
	\brief	Draw indirect.
	*******************************************************************************************************************/
	void drawIndirect(Buffer& buffer, pvr::uint32 offset, pvr::uint32 count, pvr::uint32 stride)
	{
        PVR_ASSERT("Draw Indirect does not supported Currently");
		Log(Log.Critical, "DrawIndirect not implemented");
	}

	/*!****************************************************************************************************************
	\brief	Enqueues a ComputeShader execution using the ComputeShader that is in the currently bound ComputePipeline.
	\param numGroupsX  The number of workgroups enqueued in the X direction.
	\param numGroupsY  The number of workgroups enqueued in the Y direction (default 1).
	\param numGroupsZ  The number of workgroups enqueued in the Z direction (default 1).
	*******************************************************************************************************************/
	void dispatchCompute(uint32 numGroupsX, uint32 numGroupsY = 1, uint32 numGroupsZ = 1)
	{
		enqueue_internal<DispatchCompute>(DispatchCompute(numGroupsX, numGroupsY, numGroupsZ));
	}

	/*!****************************************************************************************************************
	\brief	If Uniforms are supported by the underlying API, set a Uniform variable to a fixed value. The value will be
	        copied at the time of calling this function, and will be fixed until set to another value. Usually used it
			is needed to alternate between a few fixed, known in advance, values. Since changing the value would actually
			require re-recording the command buffer, this has limited use.
	\tparam _type The type of item to upload to the uniform. Supported types depend on the underlying API and shader
	        glsl version. Vectors and matrices should be uploaded using the glm::vec/mat types (not float)
	\param location The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	\param val The value. Although passed by const-ref, it will immediately be copied and stored internally
	\description Due to the value being fixed until the command buffer is re-recorder, the usefulness of this command
			is limited. See setUniformPtr.
	*******************************************************************************************************************/
	template<typename _type>
	void setUniform(int32 location, const _type& val)
	{
		enqueue_internal<SetUniform<_type>/**/>(SetUniform<_type>(location, val));
	}

	/*!****************************************************************************************************************
	\brief	This is the function of choice for updating uniforms (if supported by the underlying API). This function 
	        sets a Uniform variable to be updated from a memory location every time the command buffer is submitted, so
			that updating the value in that memory location is properly updated in the shader uniform.
	\tparam _type The type of item to upload to the uniform. Supported types depend on the underlying API and shader
	        glsl version. Vectors and matrices should be uploaded using the glm::vec/mat types (not float)
	\param location The location of the uniform variable (as returned by Pipeline->getUniformLocation)
	\param count The number of values to read from the pointer
	\param ptr A pointer to a location that shall remain fixed as long as the command buffer is used (unless cleared).
	       The value will be read from ptr and the uniform updated every time the command buffer is submitted.
	\description WARNING: This memory location is intended to be fixed, and must remain live as long as the command buffer is
		   used, since every time the command buffer is submitted, this memory location will be read. In synchronous
		   underlying APIs (OGLES) the value can be changed freely without any synchronization before submitting the
		   command buffer or after it returns. Uploading arrays of values is supported.
	*******************************************************************************************************************/
	template<typename _type>
	void setUniformPtr(int32 location, pvr::uint32 count, const _type* ptr)
	{
		enqueue_internal<SetUniformPtr<_type>/**/>(SetUniformPtr<_type>(location, count, ptr));
	}

	/*!****************************************************************************************************************
	\brief	Store which pipeline is currently bound, so that it can later be retrieved and bound with a popPipeline command.
	*******************************************************************************************************************/
	void pushPipeline()
	{
		enqueue_internal<PushPipeline>(PushPipeline());
	}

	/*!****************************************************************************************************************
	\brief	Bind the previously pushed pipeline (See pushGraphicsPipeline)
	*******************************************************************************************************************/
	void popPipeline()
	{
		enqueue_internal<PopPipeline>(PopPipeline());
	}

	/*!****************************************************************************************************************
	\brief	INTERNAL. reset the currently bound pipeline.
	*******************************************************************************************************************/
	void resetPipeline()
	{
		enqueue_internal<ResetPipeline>(ResetPipeline());
	}

	/*!****************************************************************************************************************
	\brief	Add a memory barrier to the command stream, forcing preceeding commands to be written before succeeding
	commands are executed.
	*******************************************************************************************************************/
	void setMemoryBarrier(const PipelineBarrier& barrier)
	{
		enqueue_internal<PipelineBarrier>(barrier);
	}

	/*!****************************************************************************************************************
	\brief	Add a synchronization object collection to the command stream and return its handle to the user. The user
	may then use the sync points for a configurable number of command buffer submition.
	\return A Sync object (collection of Sync points) representing (consecutive) submissions of this command buffer
	*******************************************************************************************************************/
	Sync insertFenceSync()
	{
		CreateFenceSyncImpl fenceSyncImpl;
		enqueue_internal<CreateFenceSyncImpl>(fenceSyncImpl);
		return fenceSyncImpl.syncObject;
	}

	/*!*********************************************************************************************************************
	\brief Clear the command queue. It is invalid to clear the command buffer while it is being recorded
	***********************************************************************************************************************/
	void clear() {	queue.clear(); }

#ifdef DEBUG
	void logCommandStackTraces()
	{
		for (auto it = queue.begin(); it != queue.end(); ++it)
		{
			Log(Log.Debug, it->debug_commandCallSiteStackTrace.c_str());
		}
	}
#endif

protected:
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
#ifdef USE_NONHOMOGENEOUS_LIST
	typedef ListOfInterfaces<ApiCommand> CommandQueue; //!< Special class optimizing the memory layout of the command queue
#else
	typedef std::deque<ApiCommand*> CommandQueue;
#endif
	GraphicsContext context;
	CommandQueue queue;

	//Enqueue a parameterless object
	template<typename ClassType, typename ObjectType> void enqueue_internal(const ObjectType& obj)
	{
		if (!valdidateRecordState()) { return; }

#ifdef USE_NONHOMOGENEOUS_LIST
		queue.emplace_back<ClassType>(obj);
#else
		queue.push_back(new ClassType(obj));
#endif
#ifdef DEBUG
		queue.last()->interfacePtr->debug_commandCallSiteStackTrace = getStackTraceInfo(2);
#endif
	}

	//Enqueue an object with a parameter
	template<typename ClassType, typename ObjectType, typename ParamType>
	void enqueue_internal(ObjectType& obj, ParamType& par)
	{
		if (!valdidateRecordState()) { return; }
#ifdef USE_NONHOMOGENEOUS_LIST
		queue.emplace_back<ClassType>(obj, par);
#else
		queue.push_back(new ClassType(obj, par));
#endif
#ifdef DEBUG
		queue.last()->interfacePtr->debug_commandCallSiteStackTrace = getStackTraceInfo(2);
#endif
	}

	template<typename iterator>
	void execute_iterator(iterator& it)
	{
#ifdef USE_NONHOMOGENEOUS_LIST
		it->execute(*this);
#else
		(*it)->execute(*this);
#endif
	}
	CommandBufferBaseImpl(GraphicsContext& context) : context(context)
#ifdef USE_NONHOMOGENEOUS_LIST
		, queue(1024)
#endif
		, m_isRecording(false)
	{ }

	CommandBufferBaseImpl() :
#ifdef USE_NONHOMOGENEOUS_LIST
		queue(1024),
#endif
		m_isRecording(false)
	{ }

	bool valdidateRecordState()
	{
#ifdef DEBUG
		if (!m_isRecording)
		{
			Log("Attempted to submit into the commandBuffer without calling beginRecording first.");
			PVR_ASSERT(false && "You must call beginRecording before starting to submit commands into the commandBuffer.");
			return false;
		}
#endif
		return true;
	}

	void submit()
	{
		PVR_ASSERT(context.isValid() && "No context has been set");
		for (CommandQueue::iterator it = queue.begin(); it != queue.end(); ++it)
		{
			execute_iterator(it);
		}
	}

	native::HCommandBuffer m_cmdBuffer;
	bool m_isRecording;
	RenderPassContents::Enum m_nextSubPassContent;
};

/*!*********************************************************************************************************************
\brief Contains all the commands and states that need to be submitted to the gpu, including pipeline, texture, and
samplers. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer.

\description Secondary command buffers cannot contain RenderPasses, and cannot be submitted to the GPU. 
	SecondaryCommandBuffers can be submitted to the primaryCommandBuffer
-It is invalid to submit commands to a command buffer while it is not being recorded.
-It is invalid to reset a command buffer while it is being recorded.
-It is invalid to submit a command buffer more than once if it is one time submit command buffer
-Draw commands must be between a BeginRenderpass and an EndRenderpass command
***********************************************************************************************************************/
class SecondaryCommandBufferImpl : public CommandBufferBaseImpl
{
private:
	// privated inherited members
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
	template<typename MyClass_> friend class PackagedBindable;
	SecondaryCommandBufferImpl(GraphicsContext& context) : CommandBufferBaseImpl(context) {}
	virtual void bind(IGraphicsContext& ctx) {	submit();	}
};

/*!*********************************************************************************************************************
\brief Contains all the commands and states that need to be submitted to the gpu, including pipeline, texture, and
samplers. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer.

\description Primary command buffers can contain RenderPasses, and can be submitted to the GPU. SecondaryCommandBuffers
can be submitted to the primaryCommandBuffer
-It is invalid to submit commands to a command buffer while it is not being recorded.
-It is invalid to reset a command buffer while it is being recorded.
-It is invalid to submit a command buffer more than once if it is one time submit command buffer
-Draw commands must be between a BeginRenderpass and an EndRenderpass command
***********************************************************************************************************************/
class CommandBufferImpl : public CommandBufferBaseImpl
{
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
protected:
	CommandBufferImpl(GraphicsContext& context) : CommandBufferBaseImpl(context) {}
public:
	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit()
	{
		CommandBufferBaseImpl::submit();
	}

	/*!*********************************************************************************************************************
	\brief Record commands from the secondary command buffer
	\param secondaryCmdBuffer Record all commands from a secondary command buffer
	***********************************************************************************************************************/
	void enqueueSecondaryCmds(SecondaryCommandBuffer& secondaryCmdBuffer)
	{
		enqueue_internal<PackagedBindable<SecondaryCommandBuffer>/**/>(secondaryCmdBuffer);
	}

	/*!*********************************************************************************************************************
	\brief Begin a RenderPass, i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.
	\param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	\param renderArea The area of the FBO to write to
	\param clearColor If the Color attachment LoadOp is Clear, the color to clear to
	\param clearDepth If the Depth attachment LoadOp is Clear, the depth value to clear to
	\param clearStencil If the Stencil attachment LoadOp is Clear, the stencil value to clear to
	***********************************************************************************************************************/
	void beginRenderPass(api::Fbo& fbo, const Rectanglei& renderArea, const glm::vec4& clearColor
	                     = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float32 clearDepth = 1.f, uint32 clearStencil = 0)
	{
		enqueue_internal<BeginRenderPass>(BeginRenderPass(fbo, renderArea, clearColor, clearDepth, clearStencil));
	}

	/*!*********************************************************************************************************************
	\brief Finish the a renderpass (executes the StoreOp).
	***********************************************************************************************************************/
	void endRenderPass() { enqueue_internal<EndRenderPass>(EndRenderPass()); }

	/*!*********************************************************************************************************************
	\brief Begin recording commands for the next surpass in this render pass.
	***********************************************************************************************************************/
	void nextSubPassInline() {}

	/*!****************************************************************************************************************
	\brief	Record next sub pass commands from a secondary-commandbuffer.
	\param	cmdBuffer The commands in this will be used to record the next subPass.
	*******************************************************************************************************************/
	void nextSubPassSecondaryCmds(pvr::api::SecondaryCommandBuffer& cmdBuffer)
	{
		enqueueSecondaryCmds(cmdBuffer);
	}

};

//!\cond NO_DOXYGEN
template<typename Resource_, typename ParamType_>
void PackagedBindableWithParam<Resource_, ParamType_>::execute_private(impl::CommandBufferBaseImpl& cmdBuf)
{
#ifdef DEBUG
	if (res.isNull())
	{
		Log(Log.Warning, "API Command: Tried to bind NULL object");
		PVR_ASSERT(false);
	}
	else
#endif
	{
		res->bind(cmdBuf.getContext(), param);
	}
}

template<typename Resource_>
void PackagedBindable<Resource_>::execute_private(impl::CommandBufferBaseImpl& cmdBuf)
{
#ifdef DEBUG
	if (res.isNull())
	{
		Log(Log.Warning, "API Command: Tried to bind NULL object");
		PVR_ASSERT(false);
	}
	else
#endif
	{
		res->bind(*cmdBuf.getContext());
	}
}
//!\endcond
}
}
}
#undef USE_HOMOGENEOUS_LIST
