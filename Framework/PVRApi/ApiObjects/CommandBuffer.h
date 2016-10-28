/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\CommandBuffer.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the CommandBuffer implementation.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"

#define SET_UNIFORM_DECLARATION(_type_)\
	template <>void CommandBufferBase_::setUniform<_type_>(int32 location, const _type_& val); \
	template <>void CommandBufferBase_::setUniformPtr<_type_>(int32 location, pvr::uint32 count, const _type_* ptr);


namespace pvr {
namespace api {
namespace impl {
class CommandBufferBaseImplementationDetails;
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
class CommandBufferBase_
{
public:
	/*!*********************************************************************************************************************
	\brief Destructor. Frees all resources.
	***********************************************************************************************************************/
	virtual ~CommandBufferBase_();

	/*!*********************************************************************************************************************
	\brief Get a reference to the context used by this CommandBuffer.
	***********************************************************************************************************************/
	GraphicsContext& getContext();

	/*!****************************************************************************************************************
	\brief	Call this function when you are done recording commands. BeginRecording must be called first.
	*******************************************************************************************************************/
	void endRecording();

	/*!****************************************************************************************************************
	\brief	Queries if a command buffer is in the recording state
	\return	True if recording, false otherwise
	*******************************************************************************************************************/
	bool isRecording();

	/*!****************************************************************************************************************
	\brief	Bind a graphics pipeline.
	\param	pipeline The GraphicsPipeline to bind.
	*******************************************************************************************************************/
	void bindPipeline(GraphicsPipeline pipeline);

	/*!****************************************************************************************************************
	\brief	Bind a compute pipeline
	\param	pipeline The ComputePipeline to bind
	*******************************************************************************************************************/
	void bindPipeline(ComputePipeline& pipeline);

	/*!****************************************************************************************************************
	\brief	Bind a single DescriptorSet to the Graphics Pipeline binding point
	\param	pipelineLayout The pipelineLayout that the GraphicsPipeline will have
	\param	index The index to which to bind the descriptor set
	\param	set The descriptorSet to bind to the binding point bindingPoint
	\param	dynamicOffsets A c-style array of unsigned integers, each of which is consecutively applied as a Dynamic
	Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.
	\param	numDynamicOffsets The number of dynamic offsets in \p dynamicOffsets. Must exactly much the number of Dynamic
	        objects in the \p set.
	*******************************************************************************************************************/
	void bindDescriptorSet(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set,
	                       const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0);

	/*!****************************************************************************************************************
	\brief	Bind a single DescriptorSet to the Compute Pipeline binding point
	\param	pipelineLayout The pipelineLayout that the ComputePipeline will have
	\param	index The index to which to bind the descriptor set
	\param	set The descriptorSet to bind to the binding point bindingPoint
	\param	dynamicOffsets A c-style array of unsigned integers, each of which is consecutively applied as a Dynamic
	Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.
	\param	numDynamicOffsets The number of dynamic offsets in \p dynamicOffsets. Must exactly much the number of Dynamic
	objects in the \p set.
	*******************************************************************************************************************/
	void bindDescriptorSetCompute(const api::PipelineLayout& pipelineLayout, uint32 index, const DescriptorSet& set,
	                              const uint32* dynamicOffsets = NULL, uint32 numDynamicOffsets = 0);

	/*!****************************************************************************************************************
	\brief	Bind multiple DescriptorSets
	\param	bindingPoint The index where the first descriptor set will bind to. The rest will be bound successively.
	\param	pipelineLayout The pipelineLayout that the GraphicsPipeline will have
	\param	firstSet The first index to start binding descriptor sets to.
	\param	sets The array of descriptorSets. The first item in the array will be bound to binding point \p firstSet and 
			each one after to the next binding point
	\param	dynamicOffsets An array of Offsets that will be used when binding items of this descriptor set respectively
	\param	numDescSets The number of descriptor sets in the array
	\param	dynamicOffsets A c-style array of unsigned integers, each of which is consecutively applied as a Dynamic
	Offset to a Dynamic buffer (uniform/storage) of this descriptor set, in order.
	\param	numDynamicOffsets The number of dynamic offsets in \p dynamicOffsets. Must exactly much the number of Dynamic
	objects in the \p set.
	*******************************************************************************************************************/
	void bindDescriptorSets(types::PipelineBindPoint bindingPoint, const api::PipelineLayout& pipelineLayout,
	                        uint32 firstSet, const DescriptorSet* sets, uint32 numDescSets, const uint32* dynamicOffsets = NULL,
	                        uint32 numDynamicOffsets = 0);

	/*!****************************************************************************************************************
	\brief	Clear multiple attachments with separate clear colors and clear rectangle for each.
	\param	attachmentIndices Attachments indices to clear
	\param	clearColors An array of colors to clear to, each corresponding to an attachment
	\param	attachmentCount Number of attachments to clear
	\param  rects An array of rectangles, each corresponding to the clear area of an attachment
	\param  baseArrayLayers An array of base array layers corresponding to the first layer to be cleared for the attachment
	\param  layerCounts An array of layer counts corresponding to number of layers to clear for the attachment
	\param  rectCount The number of rectangles to clear (the number of rectangles in \p rects)
	*******************************************************************************************************************/
	void clearColorAttachment(pvr::uint32 const* attachmentIndices, glm::vec4 const* clearColors, pvr::uint32 attachmentCount,
	                          const pvr::Rectanglei* rects, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCounts, pvr::uint32 rectCount);

	/*!****************************************************************************************************************
	\param	attachmentIndex The index for the attachment to clear
	\param	clearColor The clear area
	\param	rect The rectangle to clear
	\param  baseArrayLayer The base array layer corresponding to the first layer to be cleared for the attachment
	\param  layerCount The number of layers to clear for the attachment
	*******************************************************************************************************************/
	void clearColorAttachment(pvr::uint32 attachmentIndex, glm::vec4 clearColor, const pvr::Rectanglei rect,
	                          const pvr::uint32 baseArrayLayer = 0u, const pvr::uint32 layerCount = 1u);

	/*!****************************************************************************************************************
	\brief	Clear all attachment for a single fbo with a single clear color.
			NOTE: This clear operation must be called inside the render pass
	\param	fbo The fbo to clear attachments
	\param	clearColor The clear area
	*******************************************************************************************************************/
	void clearColorAttachment(pvr::api::Fbo fbo, glm::vec4 clearColor);

	/*!****************************************************************************************************************
	\brief	Clear the depth attachment of an fbo.
			This clear operation must be called inside the render pass
	\brief	Clear the depth attachment of an fbo.
			NOTE: This clear operation must be called inside the render pass
	\brief	Clear the depth attachment of an fbo
	\param	clearRect The clear area
	\param	depth The clear value
	*******************************************************************************************************************/
	void clearDepthAttachment(const pvr::Rectanglei& clearRect, float32 depth = 1.f);

	/*!****************************************************************************************************************
	\brief	Clear the stencil attachment of an fbo.
			NOTE: This clear operation must be called inside the render pass
	\param	clearRect The clear area
	\param	stencil The clear value
	*******************************************************************************************************************/
	void clearStencilAttachment(const pvr::Rectanglei& clearRect, pvr::int32 stencil = 0);

	/*!****************************************************************************************************************
	\brief	Clear the depth stencil attachment
	\param	clearRect clear area
	\param	depth The depth clear value
	\param	stencil The stencil clear value
	*******************************************************************************************************************/
	void clearDepthStencilAttachment(const pvr::Rectanglei& clearRect, float32 depth = 1.f, pvr::int32 stencil = 0);

	/*!****************************************************************************************************************
	\brief	Clears the specified color image using the clear color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearColor The clear color to use for the clear
	\param	baseMipLevel base mip levels
	\param	levelCount level counts
	\param	baseArrayLayer base array layers
	\param	layerCount number of layers to clear
	*******************************************************************************************************************/
	void clearColorImage(pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32 baseMipLevel = 0u,
	                     const pvr::uint32 levelCount = 1u, const pvr::uint32 baseArrayLayer = 0u,
	                     const pvr::uint32 layerCount = 1u, pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears rangeCount sub resource ranges of the specified color image using the clear color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearColor The clear color to use for the clear
	\param	baseMipLevel rangeCount base mip levels
	\param	levelCount rangeCount level counts
	\param	baseArrayLayers rangeCount base array layers
	\param	layerCount rangeCount number of layers to clear
	\param	rangeCount The number of sub resource ranges to clear
	*******************************************************************************************************************/
	void clearColorImage(pvr::api::TextureView& image, glm::vec4 clearColor, const pvr::uint32* baseMipLevel,
	                     const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount,
	                     pvr::uint32 rangeCount, pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears the specified depth image using the clear depth color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearDepth The clear color to use for the clear
	\param	baseMipLevel base mip levels
	\param	levelCount level counts
	\param	baseArrayLayer base array layer
	\param	layerCount number of layers to clear
	*******************************************************************************************************************/
	void clearDepthImage(pvr::api::TextureView& image, float clearDepth, const pvr::uint32 baseMipLevel = 0u,
	                     const pvr::uint32 levelCount = 1u, const pvr::uint32 baseArrayLayer = 0u,
	                     const pvr::uint32 layerCount = 1u, pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears rangeCount sub resource ranges of the specified depth image using the clear depth color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearDepth The clear color to use for the clear
	\param	baseMipLevel rangeCount base mip levels
	\param	levelCount rangeCount level counts
	\param	baseArrayLayers rangeCount base array layers
	\param	layerCount rangeCount number of layers to clear
	\param	rangeCount The number of sub resource ranges to clear
	*******************************************************************************************************************/
	void clearDepthImage(pvr::api::TextureView& image, float clearDepth, const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount,
	                     const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount, pvr::uint32 rangeCount,
	                     pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears the specified stencil image using the clear stencil color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearStencil The clear color to use for the clear
	\param	baseMipLevel base mip levels
	\param	levelCount level counts
	\param	baseArrayLayers base array layers
	\param	layerCount number of layers to clear
	*******************************************************************************************************************/
	void clearStencilImage(pvr::api::TextureView& image, pvr::uint32 clearStencil, const pvr::uint32 baseMipLevel = 0u,
	                       const pvr::uint32 levelCount = 1u, const pvr::uint32 baseArrayLayers = 0u, const pvr::uint32 layerCount = 1u,
	                       pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears rangeCount sub resource ranges of the specified stencil image using the clear stencil color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearStencil The clear color to use for the clear
	\param	baseMipLevel rangeCount base mip levels
	\param	levelCount rangeCount level counts
	\param	baseArrayLayers rangeCount base array layers
	\param	layerCount rangeCount number of layers to clear
	\param	rangeCount The number of sub resource ranges to clear
	*******************************************************************************************************************/
	void clearStencilImage(pvr::api::TextureView& image, pvr::uint32 clearStencil, const pvr::uint32* baseMipLevel,
	                       const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers, const pvr::uint32* layerCount,
	                       pvr::uint32 rangeCount, pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears the specified depth stencil image using the clear depth and stencil color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearDepth The clear depth color to use for the clear
	\param	clearStencil The clear color to use for the clear
	\param	baseMipLevel base mip levels
	\param	levelCount level counts
	\param	baseArrayLayers base array layers
	\param	layerCount number of layers to clear
	*******************************************************************************************************************/
	void clearDepthStencilImage(pvr::api::TextureView& image, float clearDepth, pvr::uint32 clearStencil,
	                            const pvr::uint32 baseMipLevel = 0u, const pvr::uint32 levelCount = 1u,
	                            const pvr::uint32 baseArrayLayers = 0u, const pvr::uint32 layerCount = 1u,
	                            pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

	/*!****************************************************************************************************************
	\brief	Clears rangeCount sub resource ranges of the specified depth stencil image using the clear depth and stencil color specified.
			This clear operation must be called outside the render pass
	\param	image the image to clear
	\param	layout The layout of the image
	\param	clearDepth The clear depth color to use for the clear
	\param	clearStencil The clear color to use for the clear
	\param	baseMipLevel rangeCount base mip levels
	\param	levelCount rangeCount level counts
	\param	baseArrayLayers rangeCount base array layers
	\param	layerCount rangeCount number of layers to clear
	\param	rangeCount The number of sub resource ranges to clear
	*******************************************************************************************************************/
	void clearDepthStencilImage(pvr::api::TextureView& image, float clearDepth, pvr::uint32 clearStencil,
	                            const pvr::uint32* baseMipLevel, const pvr::uint32* levelCount, const pvr::uint32* baseArrayLayers,
	                            const pvr::uint32* layerCount, pvr::uint32 rangeCount,
	                            pvr::types::ImageLayout layout = pvr::types::ImageLayout::General);

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
	                 uint32 firstInstance = 0, uint32 instanceCount = 1);

	/*!****************************************************************************************************************
	\brief	Draw command. Use the current state in the command buffer (pipelines, buffers, descriptor sets) to execute
	        a drawing command. Does not use an IndexBuffer.
	\param	firstVertex The vertex from where to start drawing
	\param	vertexCount The number of vertices to draw
	\param	firstInstance The Instance from which to draw from
	\param	instanceCount The number of instances to draw
	*******************************************************************************************************************/
	void drawArrays(uint32 firstVertex, uint32 vertexCount, uint32 firstInstance = 0, uint32 instanceCount = 1);

	/*!*****************************************************************************************************************
	\brief drawArraysIndirect
	\param buffer Buffer containing draw parameters. The parameters of each draw must be encoded in an array of
		   DrawIndirectCmd structures
	\param offset Offset in bytes in the buffer where draw parameters begin
	\param drawCount Number of draws to execute, can be zero. If the the drawCount is greater than 1 then the
		   stride must be multiple of 4.
	\param stride Stride in byte between sets of draw parameters
	********************************************************************************************************************/
	void drawArraysIndirect(api::Buffer& buffer, uint32 offset, uint32 drawCount, uint32 stride);

	/*!****************************************************************************************************************
	\brief	Indirect Draw command. Use buffer to obtain the draw call parameters.
	\param	buffer A buffer containing the draw call parameters in the form of a DrawIndexedIndirectCommand
	*******************************************************************************************************************/
	void drawIndexedIndirect(Buffer& buffer);

	/*!****************************************************************************************************************
	\brief	Update the buffer.
	\param[in]	buffer The buffer to update
	\param[in] offset Offset in the buffer updates begin
	\param[in] length Buffer update length
	\param[in] data New update data.
	*******************************************************************************************************************/
	void updateBuffer(Buffer& buffer, const void* data, pvr::uint32 offset, pvr::uint32 length);


	/*!****************************************************************************************************************
	\brief	Bind a Vertex Buffer for drawing
	\param	buffer The vertex buffer to bind
	\param	offset The offset into the vertex buffer to bind
	\param	bindingIndex The Vertex Buffer index to bind the vertex buffer to
	*******************************************************************************************************************/
	void bindVertexBuffer(const Buffer& buffer, uint32 offset, uint16 bindingIndex);

	/*!****************************************************************************************************************
	\brief	Bind an array of Vertex Buffers
	\param	buffers The array of buffers
	\param	offsets The array of offsets into the vertex buffer, each corresponding to a vertex buffer
	\param	startBinding The binding index that the first buffer will be bound
	\param	numBuffers The number of buffers to bind
	\param	bindingCount The number of bindings
	*******************************************************************************************************************/
	void bindVertexBuffer(Buffer const* buffers, uint32* offsets, uint16 numBuffers, uint16 startBinding,
	                      uint16 bindingCount);

	/*!****************************************************************************************************************
	\brief	Bind an index buffer for drawing
	\param	buffer The buffer to bind as an IndexBuffer
	\param	offset The offset into the Index buffer to bind
	\param	indexType the type of indices the buffer contains
	*******************************************************************************************************************/
	void bindIndexBuffer(const api::Buffer& buffer, uint32 offset, types::IndexType indexType);

	/*!****************************************************************************************************************
	\brief	Set the viewport rectangle
	\param	viewport The viewport rectangle
	*******************************************************************************************************************/
	void setViewport(const pvr::Rectanglei& viewport);

	/*!****************************************************************************************************************
	\brief	Set the scissor rectangle
	\param	scissor The scissor rectangle
	*******************************************************************************************************************/
	void setScissor(const pvr::Rectanglei& scissor);

	/*!****************************************************************************************************************
	\brief	Set minimum and maximum depth
	\param	min Minimum depth (default 0.0f)
	\param	max Maximum depth (default 1.0f)
	*******************************************************************************************************************/
	void setDepthBound(pvr::float32 min = 0.0f, pvr::float32 max = 1);

	/*!****************************************************************************************************************
	\brief	Set the stencil comparison mask
	\param	face The face/faces for which to set the stencil mask
	\param	compareMask A uint32 which will mask both the values and the reference before stencil comparisons
	*******************************************************************************************************************/
	void setStencilCompareMask(types::StencilFace face, pvr::uint32 compareMask);

	/*!****************************************************************************************************************
	\brief	Set the stencil write mask
	\param	face The face/faces for which to set the stencil write mask
	\param	writeMask A uint32 which will mask the values when writing the stencil
	*******************************************************************************************************************/
	void setStencilWriteMask(types::StencilFace face, pvr::uint32 writeMask);

	/*!****************************************************************************************************************
	\brief	Set stencil reference value
	\param	face The face/faces for which to set the stencil reference value
	\param	ref The stencil reference value
	*******************************************************************************************************************/
	void setStencilReference(types::StencilFace face, pvr::uint32 ref);

	/*!****************************************************************************************************************
	\brief	This is a dynamic command which controll the offset of depth values of all fragments generated by the rasterization of a polygon.
			NOTE: If depthBiasEnable is set to false in pipelineCreation::RasterStateCreateParam then no depth bias
			is applied and the fragment's depth values are unchanged.
	\param	depthBiasConstantFactor Scalar factor controlling the constant depth value added to each fragment.
	\param	depthBiasClamp  Maximum or minimum depth bias of a fragment
	\param	depthBiasSlopeFactor Scalar factor applied to a fragment's slope in depth bias calculations
	*******************************************************************************************************************/
	void setDepthBias(pvr::float32 depthBiasConstantFactor, pvr::float32 depthBiasClamp, pvr::float32 depthBiasSlopeFactor);

	/*!****************************************************************************************************************
	\brief	Set blend constants for blend operation using constant colors
	\param	rgba Red Green Blue Alpha blend constant
	*******************************************************************************************************************/
	void setBlendConstants(glm::vec4 rgba);

	/*!****************************************************************************************************************
	\brief	Set the line width
	\param lineWidth the new width of lines drawn
	*******************************************************************************************************************/
	void setLineWidth(float32 lineWidth);

	/*!****************************************************************************************************************
	\brief	Copy buffer
	\param src Source buffer to copy from
	\param dest Destination buffer to copy in to
	\param srcOffset Source buffer offset
	\param destOffset Destination buffer offset
	\param sizeInBytes Data size in bytes
	*******************************************************************************************************************/
	void copyBuffer(pvr::api::Buffer src, pvr::api::Buffer dest, pvr::uint32 srcOffset, pvr::uint32 destOffset, pvr::uint32 sizeInBytes);

	/*!****************************************************************************************************************
	\brief	Blit Image
	\param src Source image to blit from
	\param dest Destination image to blit in to
	\param srcLayout layout of the source image subresources for the blit
	\param dstLayout Layout of the destination image subresources for the blit
	\param regions An array of regions to blit
	\param numRegions Number of regions
	\param filter Sampler Filter to apply if the blits require scaling
	*******************************************************************************************************************/
	void blitImage(api::TextureStore& src, api::TextureStore& dest, types::ImageLayout srcLayout, types::ImageLayout dstLayout,
	               types::ImageBlitRange* regions, uint32 numRegions, types::SamplerFilter filter);

	/*!****************************************************************************************************************
	\brief Copy image to a buffer
	\param srcImage Source image to copy from
	\param srcImageLayout Layout of the source image subresources for the copy
	\param dstBuffer Buffer to copy into
	\param regions An array of regions to copy
	\param numRegions Number of regions in the array

	*******************************************************************************************************************/
	void copyImageToBuffer(api::TextureStore& srcImage, types::ImageLayout srcImageLayout,
	                       api::Buffer& dstBuffer, types::BufferImageCopy* regions, uint32 numRegions);

	/*!****************************************************************************************************************
	\brief	This draw command behaves similarly to drawArray except that the parameters are read by the device from a buffer during execution.
	\param buffer Buffer which contains the parameters.
	\param offset Offset in to the buffer where parameters begin
	\param count Number of draws to execute, and can be zero
	\param stride Byte stride between successive sets of draw parameters
	*******************************************************************************************************************/
	void drawIndirect(Buffer& buffer, pvr::uint32 offset, pvr::uint32 count, pvr::uint32 stride);

	/*!****************************************************************************************************************
	\brief	Enqueues a ComputeShader execution using the ComputeShader that is in the currently bound ComputePipeline.
	\param numGroupsX  The number of workgroups enqueued in the X direction.
	\param numGroupsY  The number of workgroups enqueued in the Y direction (default 1).
	\param numGroupsZ  The number of workgroups enqueued in the Z direction (default 1).
	*******************************************************************************************************************/
	void dispatchCompute(uint32 numGroupsX, uint32 numGroupsY = 1, uint32 numGroupsZ = 1);

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
	template<typename _type> void setUniform(int32 location, const _type& val);

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
	template<typename _type> void setUniformPtr(int32 location, pvr::uint32 count, const _type* ptr);

	/*!****************************************************************************************************************
	\brief	Store which pipeline is currently bound, so that it can later be retrieved and bound with a popPipeline command.
	*******************************************************************************************************************/
	void pushPipeline();

	/*!****************************************************************************************************************
	\brief	Bind the previously pushed pipeline (See pushGraphicsPipeline)
	*******************************************************************************************************************/
	void popPipeline();

	/*!****************************************************************************************************************
	\brief	INTERNAL. reset the currently bound pipeline.
	*******************************************************************************************************************/
	void resetPipeline();

	/*!****************************************************************************************************************
	\brief	Add a memory barrier to the command stream, forcing preceeding commands to be written before succeeding
	commands are executed.
	*******************************************************************************************************************/
	void pipelineBarrier(types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage,
	                     const MemoryBarrierSet& barriers, bool dependencyByRegion = true);

	void waitForEvent(const Event& evt, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);
	void waitForEvents(const EventSet& evts, types::PipelineStageFlags srcStage, types::PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);
	void setEvent(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands);
	void resetEvent(Event& evt, types::PipelineStageFlags pipelineFlags = types::PipelineStageFlags::AllCommands);

	/*!*********************************************************************************************************************
	\brief Clear the command queue. It is invalid to clear the command buffer while it is being recorded.
	***********************************************************************************************************************/
	void clear(bool releaseAllResources = false);

	const native::HCommandBuffer_& getNativeObject() const;
	native::HCommandBuffer_& getNativeObject();

#ifdef DEBUG
	void logCommandStackTraces();
#endif
protected:
	friend class SecondaryCommandBufferPackager;
	RefCountedResource<CommandBufferBaseImplementationDetails> pImpl;
	CommandBufferBase_(GraphicsContext& context, CommandPool& pool, native::HCommandBuffer_& myHandle);
};

SET_UNIFORM_DECLARATION(uint32);
SET_UNIFORM_DECLARATION(int32);
SET_UNIFORM_DECLARATION(float32);
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
SET_UNIFORM_DECLARATION(glm::mat4x4);


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
class SecondaryCommandBuffer_ : public CommandBufferBase_
{
	friend class CommandBuffer_;
public:
	/*!****************************************************************************************************************
	\brief	Call this function before beginning to record commands. If you use this overload, you CANNOT enqueue this
	commandbuffer between a beginRenderPass() -> endRenderPass() block.
	*******************************************************************************************************************/
	//void beginRecording();

	/*!****************************************************************************************************************
	\brief	Call this function before beginning to record commands. If the Fbo is known, prefer the Fbo overload as it may
	offer better performance.
	*******************************************************************************************************************/
	void beginRecording(const RenderPass& rp, uint32 subPass = 0);

	/*!****************************************************************************************************************
	\brief	Call this function before beginning to record commands.
	*******************************************************************************************************************/
	void beginRecording(const Fbo& fbo, uint32 subPass = 0);
private:
	// privated inherited members
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
	template<typename MyClass_> friend class PackagedBindable;
	SecondaryCommandBuffer_(GraphicsContext& context, CommandPool& pool, native::HCommandBuffer_& hBuff);
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
class CommandBuffer_ : public CommandBufferBase_
{
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
protected:
	CommandBuffer_(GraphicsContext& context, CommandPool& pool, native::HCommandBuffer_& hCmdBuff);
public:
	/*!****************************************************************************************************************
	\brief	Call this function before beginning to record commands.
	*******************************************************************************************************************/
	void beginRecording();

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(const Semaphore& waitSemaphore, const Semaphore& signalSemaphore, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(SemaphoreSet& waitSemaphores, SemaphoreSet& signalSemaphores, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(Semaphore& waitSemaphore, SemaphoreSet& signalSemaphores, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(SemaphoreSet& waitSemaphores, Semaphore& signalSemaphore, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit(Fence& fence);

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submit();

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submitEndOfFrame(Semaphore& waitSemaphore);

	/*!*********************************************************************************************************************
	\brief Submit this command buffer to the GPU
	***********************************************************************************************************************/
	void submitStartOfFrame(Semaphore& signalSemaphore, const Fence& fence = Fence());

	/*!*********************************************************************************************************************
	\brief Record commands from the secondary command buffer.
	\param secondaryCmdBuffer Record all commands from a secondary command buffer
	***********************************************************************************************************************/
	void enqueueSecondaryCmds(SecondaryCommandBuffer& secondaryCmdBuffer);

	/*!*********************************************************************************************************************
	\brief Record commands from an array of secondary command buffer
	\param secondaryCmdBuffers A c-style array of SecondaryCommandBuffers
	\param numCmdBuffers The number of SecondaryCommandBuffers in secondaryCmdBuffers
	***********************************************************************************************************************/
	void enqueueSecondaryCmds(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers);

	/*!*********************************************************************************************************************
	\brief Record commands from an  secondary command buffer. Multiple enqueueing mode.
		   This is an optimized version where the user is expected to be enqueueing
	       multiple secondary command buffers, but does not necessarily immediately have them available.
	\param expectedMax The number of command buffers that are expected to be enqueued. This number is only a hint and can
	       be overrun.
	***********************************************************************************************************************/
	void enqueueSecondaryCmds_BeginMultiple(uint32 expectedMax = 255);

	/*!*********************************************************************************************************************
	\brief Collect commands for the multiple enqueueing mode. Must be called after enqueueSecondaryCmds_BeginMultiple.
	\param secondaryCmdBuffers A c-style array of secondaryCmdBuffer
	\param numCmdBuffers The number of command buffers
	***********************************************************************************************************************/
	void enqueueSecondaryCmds_EnqueueMultiple(SecondaryCommandBuffer* secondaryCmdBuffers, uint32 numCmdBuffers);

	/*!*********************************************************************************************************************
	\brief Submit the commands collected in multiple enqueueing mode.
	\param keepAllocated Attempt to keep any allocated memory for the next enqueueMultiple call.
	***********************************************************************************************************************/
	void enqueueSecondaryCmds_SubmitMultiple(bool keepAllocated = false);

	/*!*********************************************************************************************************************
	\brief Begin a RenderPass, i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.
	\param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	\param renderArea The area of the FBO to write to
	\param inlineFirstSubpass Set to 'true' if the commands of the first subpass will be provided Inline. Set to 'false' if
	the commands of the first subpass will be submitted through a SecondaryCommandBuffer
	\param clearColor If the Color attachment LoadOp is Clear, the color to clear to
	\param clearDepth If the Depth attachment LoadOp is Clear, the depth value to clear to
	\param clearStencil If the Stencil attachment LoadOp is Clear, the stencil value to clear to
	***********************************************************************************************************************/
	void beginRenderPass(
	  api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass,
	  const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*********************************************************************************************************************
	\brief Begin a RenderPass, i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.
	\param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	\param renderArea The area of the FBO to write to
	\param inlineFirstSubpass Set to 'true' if the commands of the first subpass will be provided Inline. Set to 'false' if
	the commands of the first subpass will be submitted through a SecondaryCommandBuffer
	\param clearColors If the Color attachment LoadOp is Clear, the color to clear to each attachment
	\param numClearColors Number of colour attachments
	\param clearDepth If the Depth attachment LoadOp is Clear, the depth value to clear to
	\param clearStencil If the Stencil attachment LoadOp is Clear, the stencil value to clear to
	***********************************************************************************************************************/
	void beginRenderPass(
	  api::Fbo& fbo, const Rectanglei& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors,
	  uint32 numClearColors, float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!
	   \brief Begin a renderPass i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.
	   \param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	   \param inlineFirstSubpass Begin the first subpass commands inline in this commandbuffer if true.
	   \param clearColor If the Color attachment LoadOp is Clear, the color to clear to each attachment
	   \param clearDepth  If the Depth attachment LoadOp is Clear, the depth value to clear to
	   \param clearStencil If the Stencil attachment LoadOp is Clear, the stencil value to clear to
	 */
	void beginRenderPass(
	  api::Fbo& fbo, bool inlineFirstSubpass,
	  const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!
	   \brief Begin a renderPass i.e. binding an FBO and preparing to draw into it. Executes the LoadOp.
	   \param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	   \param inlineFirstSubpass Begin the first subpass commands inline in this commandbuffer if true.
	   \param clearColor If the Color attachment LoadOp is Clear, the color to clear to each attachment
	   \param numClearColor Number of clear colors. The colors are one to one mapped with the fbo attachments
	   \param clearDepth  f the Depth attachment LoadOp is Clear, the depth value to clear to
	   \param clearStencil If the Stencil attachment LoadOp is Clear, the stencil value to clear to
	 */
	void beginRenderPass(
	  api::Fbo& fbo, bool inlineFirstSubpass, const glm::vec4* clearColor,
	  uint32 numClearColor,
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);


	/*!*
	\brief beginRenderPass
	\param fbo The FramebufferObject to draw to. All draw commands will write into this FBO.
	\param renderPass The render pass object. Renderpass must be compatible with the one the fbo created with
	\param renderArea The area of the FBO to write to
	\param inlineFirstSubpass True if the first supass commands will be in this commandbuffer, false
		   if the commands will be in the secondary commandbuffer
	\param clearColor Color initial clear values. Takes effect only if the renderpass's color load op is clear
	\param clearDepth Depth intial clear value. Takes effect only if the renderpass's depth load op is clear
	\param clearStencil Stencil intial clear value. Takes effect only if the renderpass's stencil load op is clear
	 */
	void beginRenderPass(
	  api::Fbo& fbo, const RenderPass& renderPass, const Rectanglei& renderArea,
	  bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*
	\brief beginRenderPass
	\param fbo
	\param renderPass
	\param renderArea
	\param inlineFirstSubpass
	\param clearColors
	\param numClearColors
	\param clearDepth
	\param clearStencil
	 */
	void beginRenderPass(
	  api::Fbo& fbo, const api::RenderPass& renderPass, const Rectanglei& renderArea,
	  bool inlineFirstSubpass, const glm::vec4* clearColors, uint32 numClearColors,
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*
	\brief beginRenderPass
	\param fbo
	\param renderPass
	\param inlineFirstSubpass
	\param clearColor
	\param clearDepth
	\param clearStencil
	 */
	void beginRenderPass(
	  api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass,
	  const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	  float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*
	\brief beginRenderPass
	\param fbo
	\param renderPass
	\param inlineFirstSubpass
	\param clearColor
	\param numClearColor
	\param clearDepth
	\param clearStencil
	 */
	void beginRenderPass(
	  api::Fbo& fbo, const api::RenderPass& renderPass, bool inlineFirstSubpass, const glm::vec4* clearColor,
	  uint32 numClearColor, float32 clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue,
	  uint32 clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue);

	/*!*********************************************************************************************************************
	\brief Finish the a renderpass (executes the StoreOp).
	***********************************************************************************************************************/
	void endRenderPass();

	/*!*********************************************************************************************************************
	\brief Begin recording commands for the next surpass in this render pass.
	***********************************************************************************************************************/
	void nextSubPassInline();

	/*!****************************************************************************************************************
	\brief	Record next sub pass commands from a secondary-commandbuffer.
	\param	cmdBuffer The commands in this will be used to record the next subPass.
	*******************************************************************************************************************/
	void nextSubPassSecondaryCmds(SecondaryCommandBuffer& cmdBuffer);
};

}//impl
inline native::HCommandBuffer_& native_cast(pvr::api::impl::CommandBuffer_& object)
{
	return object.getNativeObject();
}
inline const native::HCommandBuffer_& native_cast(const pvr::api::impl::CommandBuffer_& object)
{
	return object.getNativeObject();
}
}//api
}//pvr
#undef SET_UNIFORM_DECLARATION
