/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ApiCommandsGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES 2/3 implementations Command objects that are used in the PVRApi project of the Framework.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/ApiCommand.h"
#include "PVRApi/OGLES/ApiCommands.h"
#include "PVRCore/IGraphicsContext.h"
#include "PVRApi/OGLES/GraphicsPipelineGles.h"
#include "PVRApi/OGLES/ComputePipelineGles.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRNativeApi/OGLES/LibraryLoaderGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/RenderPassGles.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRApi/OGLES/BufferGles.h"
#include "PVRApi/ApiObjects/CommandBuffer.h"
#include "PVRApi/OGLES/DescriptorSetGles.h"

namespace pvr {
namespace api {
namespace impl {

void pvr::api::impl::UpdateBuffer::execute_private(pvr::api::impl::CommandBufferBase_ &cmdBuffer)
{
	//Safe downcast. We already KNOW that this is our class.
	static_cast<gles::BufferGles_&>(*buffer).update(data, offset, length);
}

void bindVertexBuffer(platform::ContextGles& context)
{
	impl::GraphicsPipeline_* pipeline = context.getBoundGraphicsPipeline_();
	assertion(pipeline != NULL, "valid Graphics pipeline need to bound");
	platform::ContextGles::RenderStatesTracker& renderStates = context.getCurrentRenderStates();
	for (platform::ContextGles::VboBindingMap::iterator it = renderStates.vboBindings.begin();
	     it != renderStates.vboBindings.end(); ++it)
	{
		const uint16& bindIndex = it->first;
		renderStates.lastBoundVbo = it->second;
		gl::BindBuffer(GL_ARRAY_BUFFER, *it->second->getNativeObject());

		VertexInputBindingInfo const* bindingInfo = pipeline->getInputBindingInfo(bindIndex);
		if (!bindingInfo) { break; }
		const pvr::uint32 numAttrib = pipeline->getNumAttributes(bindIndex);
		const VertexAttributeInfoWithBinding* attributes = pipeline->getAttributesInfo(bindIndex);
		for (pvr::uint32 i = 0; i < numAttrib; ++i)
		{
			const VertexAttributeInfo& attrib = attributes[i];
			context.enableAttribute(attrib.index);
			gl::VertexAttribPointer(attrib.index, attrib.width, ConvertToGles::dataType(attrib.format),
			                        types::dataTypeIsNormalised(attrib.format), bindingInfo->strideInBytes,
			                        (void*)(intptr_t)attrib.offsetInBytes);
		}
	}
	context.disableUnneededAttributes();
}

void DrawArrays::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	platform::ContextGles& context = static_cast<platform::ContextGles&>(*cmdBuff.getContext());
	platform::ContextGles::RenderStatesTracker& renderStates = context.getCurrentRenderStates();
	bindVertexBuffer(context);
	if (instanceCount > 1 && context.getApiCapabilities().supports(ApiCapabilities::Instancing))
	{
		gl::DrawArraysInstanced(ConvertToGles::drawPrimitiveType(renderStates.primitiveTopology), firstVertex, vertexCount, instanceCount);
	}
	else
	{
		gl::DrawArrays(ConvertToGles::drawPrimitiveType(renderStates.primitiveTopology), firstVertex, vertexCount);
	}
}

void DrawIndexed::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	pvr::platform::ContextGles& context = static_cast<pvr::platform::ContextGles&>(*cmdBuff.getContext());
	platform::ContextGles::RenderStatesTracker& renderStates = context.getCurrentRenderStates();
	bindVertexBuffer(context);
	if (instanceCount > 1 && context.getApiCapabilities().supports(ApiCapabilities::Instancing))
	{
		gl::DrawElementsInstanced(ConvertToGles::drawPrimitiveType(renderStates.primitiveTopology), indexCount,
		                          renderStates.iboState.indexArrayFormat == types::IndexType::IndexType16Bit ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
		                          (void*)(intptr_t)(firstIndex * (renderStates.iboState.indexArrayFormat == types::IndexType::IndexType16Bit ? 2 : 4)), instanceCount);
	}
	else
	{
		gl::DrawElements(ConvertToGles::drawPrimitiveType(renderStates.primitiveTopology), indexCount,
		                 renderStates.iboState.indexArrayFormat == types::IndexType::IndexType16Bit ?  GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
		                 (void*)(intptr_t)(firstIndex * (renderStates.iboState.indexArrayFormat == types::IndexType::IndexType16Bit ? 2 : 4)));
	}
}

void BindIndexBuffer::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	assertion(static_cast<pvr::uint32>(buffer->getBufferUsage() & types::BufferBindingUse::IndexBuffer) != 0, "Invalid Buffer Usage");
	platform::ContextGles::RenderStatesTracker& currentStates = static_cast<platform::ContextGles&>
	    (*cmdBuffer.getContext()).getCurrentRenderStates();

	if (!currentStates.iboState.buffer.isValid() ||
	    (currentStates.iboState.buffer->getNativeObject() != buffer->getNativeObject()))
	{
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buffer->getNativeObject());
		currentStates.iboState.buffer = buffer;
	}
	currentStates.iboState.indexArrayFormat = indexType;
	currentStates.iboState.offset = offset;
}

void BindDescriptorSets::execute_private(impl::CommandBufferBase_& cmd)
{
	uint32* dynamicOffset = dynamicOffsets.size() ? dynamicOffsets.data() : 0;
	for (pvr::uint32 i = 0; i < set.size(); ++i)
	{
		assertion(set[i].isValid(), "Invalid Descriptor Set");
		if (!set[i].isNull())
		{
			static_cast<gles::DescriptorSetGles_&>(*(set[i])).bind(*cmd.getContext(), dynamicOffset);
		}
	}
}

void SetClearStencilVal::execute_private(impl::CommandBufferBase_&)
{
	gl::ClearStencil(val);
}

void PushPipeline::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	pvr::platform::ContextGles& contextES = static_cast<platform::ContextGles&>(*cmdBuff.getContext());
	if (contextES.isLastBoundPipelineGraphics())
	{
		contextES.pushPipeline(&PopPipeline::bindGraphicsPipeline, cmdBuff.getContext()->getBoundGraphicsPipeline_());
	}
	else if (contextES.isLastBoundPipelineCompute())
	{
		contextES.pushPipeline(&PopPipeline::bindComputePipeline, cmdBuff.getContext()->getBoundComputePipeline());
	}
	else// assuming that no pipeline has been bound currently so push a null
	{
		contextES.pushPipeline(&PopPipeline::bindGraphicsPipeline, cmdBuff.getContext()->getBoundGraphicsPipeline_());
	}
}

void PopPipeline::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	pvr::platform::ContextGles& contextES = static_cast<platform::ContextGles&>(*cmdBuff.getContext());
	contextES.popPipeline();
}

void PopPipeline::bindGraphicsPipeline(void* pipeline, IGraphicsContext& context)
{
	static_cast<gles::GraphicsPipelineImplGles&>(
	  static_cast<impl::GraphicsPipeline_*>(pipeline)->getImpl()).bind();
}

void PopPipeline::bindComputePipeline(void* pipeline, IGraphicsContext& context)
{
	static_cast<gles::ComputePipelineImplGles&>(
	  static_cast<impl::ComputePipeline_*>(pipeline)->getImpl()).bind();
}

void ResetPipeline::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	platform::ContextGles& ctx = static_cast<platform::ContextGles&>(*cmdBuff.getContext());
	if (ctx.isLastBoundPipelineGraphics() && ctx.getBoundGraphicsPipeline_())
	{
		ctx.setBoundGraphicsPipeline(NULL);

	}
	else if (ctx.isLastBoundPipelineGraphics() && ctx.getBoundComputePipeline())
	{
		ctx.setBoundComputePipeline(NULL);
	}
}

void ClearColorImage::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	pvr::uint32 glInternalFormat;
	pvr::uint32 glFormat;
	pvr::uint32 glType;
	pvr::uint32 glTypeSize;
	bool isCompressedFormat;

	pvr::api::ConvertToGles::getOpenGLFormat(imageToClear->getResource()->getFormat().format,
	    imageToClear->getResource()->getFormat().colorSpace, imageToClear->getResource()->getFormat().dataType, glInternalFormat, glFormat, glType, glTypeSize, isCompressedFormat);

	glext::ClearTexSubImageIMG(imageToClear->getResource()->getNativeObject().handle,
	                           baseMipLevel, 0, 0, baseArrayLayer, imageToClear->getResource()->getWidth(),
	                           imageToClear->getResource()->getHeight(), layerCount, glFormat,
	                           glType, glm::value_ptr(clearColor));
}

void ClearDepthStencilImage::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	pvr::uint32 glInternalFormat;
	pvr::uint32 glFormat;
	pvr::uint32 glType;
	pvr::uint32 glTypeSize;
	bool isCompressedFormat;

	pvr::api::ConvertToGles::getOpenGLFormat(imageToClear->getResource()->getFormat().format,
	    imageToClear->getResource()->getFormat().colorSpace, imageToClear->getResource()->getFormat().dataType, glInternalFormat, glFormat, glType, glTypeSize, isCompressedFormat);

	if (glFormat == GL_DEPTH_COMPONENT)
	{
		glext::ClearTexSubImageIMG(imageToClear->getResource()->getNativeObject().handle,
		                           baseMipLevel, 0, 0, baseArrayLayer, imageToClear->getResource()->getWidth(),
		                           imageToClear->getResource()->getHeight(), layerCount, glFormat,
		                           glType, &clearDepth);
	}
#if !defined(TARGET_OS_IPHONE)
	else if (glFormat == GL_STENCIL_INDEX_OES)
	{
		glext::ClearTexSubImageIMG(imageToClear->getResource()->getNativeObject().handle,
		                           baseMipLevel, 0, 0, baseArrayLayer, imageToClear->getResource()->getWidth(),
		                           imageToClear->getResource()->getHeight(), layerCount, glFormat,
		                           glType, &clearStencil);
	}
#endif
	else if (glFormat == GL_DEPTH_STENCIL)
	{
		float data[2];
		data[0] = (float32)clearDepth;
		data[1] = (float32)clearStencil;

		glext::ClearTexSubImageIMG(imageToClear->getResource()->getNativeObject().handle,
		                           baseMipLevel, 0, 0, baseArrayLayer, imageToClear->getResource()->getWidth(),
		                           imageToClear->getResource()->getHeight(), layerCount, glFormat,
		                           glType, data);
	}
}

void ClearColorAttachment::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	platform::ContextGles::RenderStatesTracker& currentStates = static_cast<platform::ContextGles&>
	    (*cmdBuffer.getContext()).getCurrentRenderStates();
	// check if the color mask is disabled
	if (currentStates.colorWriteMask != glm::bvec4(true))
	{
		gl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	if (!currentStates.enabledScissorTest) { gl::Enable(GL_SCISSOR_TEST); }

	// Can only support single clear colour
	//for (pvr::uint32 i = 0; i < clearConst.size(); ++i)
	if (clearConst.size())
	{
		//gl::Scissor(clearRect[i].x, clearRect[i].y, clearRect[i].width, clearRect[i].height);
		gl::ClearColor(clearConst[0].r, clearConst[0].g, clearConst[0].b, clearConst[0].a);
		gl::Clear(GL_COLOR_BUFFER_BIT);
	}

	// unset the state
	if (currentStates.colorWriteMask != glm::bvec4(true))
	{
		gl::ColorMask(currentStates.colorWriteMask.r, currentStates.colorWriteMask.g,
		              currentStates.colorWriteMask.b, currentStates.colorWriteMask.a);
	}
	if (!currentStates.enabledScissorTest) { gl::Disable(GL_SCISSOR_TEST); }
}

void ClearDepthStencilAttachment::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	if (clearBits & ClearBits::Depth)
	{
		gl::ClearDepthf(clearDepth);
	}
	if (clearBits & ClearBits::Stencil)
	{
		gl::ClearStencil(clearStencil);
	}
	gl::Clear((clearBits & ClearBits::Depth ? GL_DEPTH_BUFFER_BIT : 0) |
	          (clearBits & ClearBits::Stencil ? GL_STENCIL_BUFFER_BIT : 0));
}

void SetClearDepthVal::execute_private(impl::CommandBufferBase_&)
{
	gl::ClearDepthf(depthVal);
}

void BeginRenderPass::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	assertion(m_fbo.isValid(), "Invalid Fbo");
	gles::FboGles_& fboGles =  static_cast<gles::FboGles_&>(*m_fbo);
	static_cast<platform::ContextGles&>(*cmdBuff.getContext()).getCurrentRenderStates().boundFbo = m_fbo;
	static_cast<gles::FboGles_&>(fboGles).bind(*cmdBuff.getContext(), types::FboBindingTarget::Write);
	static_cast<gles::RenderPassGles_&>(*static_cast<gles::FboGles_&>(fboGles).getRenderPass()).begin(*cmdBuff.getContext(), m_fbo, m_renderArea, m_clearColor.data(),
	    (uint32)m_clearColor.size(), m_clearDepth, m_clearStencil);
}

void EndRenderPass::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	// make sure they are begin/ end
	platform::ContextGles& contextGles =  static_cast<platform::ContextGles&>(*cmdBuff.getContext());
	assertion(contextGles.getBoundFbo().isValid(), "endRenderPass: Invalid context");
	// bind our proxy fbo to let the driver that we have finish rednering to the currently bound fbo.
	static_cast<const gles::RenderPassGles_&>(*static_cast<const gles::FboGles_&>(*contextGles.getBoundFbo()).getRenderPass()).end(*cmdBuff.getContext());
	// Unbind the framebuffer.
	static_cast<platform::ContextGles&>(*cmdBuff.getContext()).getCurrentRenderStates().boundFbo.reset();
}

void SetBlendConstants::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	gl::BlendColor(m_constants.r, m_constants.g, m_constants.b, m_constants.a);
}

void SetLineWidth::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	gl::LineWidth(m_lineWidth);
}

void SetViewport::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	platform::ContextGles::RenderStatesTracker& recordedStates = static_cast<platform::ContextGles&>
	    (*cmdBuffer.getContext()).getCurrentRenderStates();
	if (recordedStates.viewport == m_viewport) { return; }
	gl::Viewport(m_viewport.x, m_viewport.y, m_viewport.width, m_viewport.height);
	recordedStates.viewport = m_viewport;
}

void SetStencilCompareMask::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	platform::ContextGles::RenderStatesTracker& recordedStates = static_cast<platform::ContextGles&>
	    (*cmdBuffer.getContext()).getCurrentRenderStates();
	switch (m_face)
	{
	case types::StencilFace::Front:
		gl::StencilFuncSeparate(GL_FRONT, ConvertToGles::comparisonMode(recordedStates.depthStencil.stencilOpFront),
		                        recordedStates.depthStencil.refFront, m_mask);
		recordedStates.depthStencil.readMaskFront = m_mask;

		break;
	case types::StencilFace::Back:
		gl::StencilFuncSeparate(GL_BACK, ConvertToGles::comparisonMode(recordedStates.depthStencil.stencilOpBack),
		                        recordedStates.depthStencil.refBack, m_mask);
		recordedStates.depthStencil.readMaskBack = m_mask;

		break;
	case types::StencilFace::FrontBack:
		gl::StencilFuncSeparate(GL_FRONT, ConvertToGles::comparisonMode(recordedStates.depthStencil.stencilOpFront),
		                        recordedStates.depthStencil.refFront, m_mask);

		gl::StencilFuncSeparate(GL_BACK, ConvertToGles::comparisonMode(recordedStates.depthStencil.stencilOpBack),
		                        recordedStates.depthStencil.refBack, m_mask);

		recordedStates.depthStencil.readMaskFront = recordedStates.depthStencil.readMaskBack = m_mask;
		break;
	}
}

void SetScissor::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	gl::Scissor(m_scissor.x, m_scissor.y, m_scissor.width, m_scissor.height);
}


void SetStencilReference::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	platform::ContextGles::RenderStatesTracker& recordedStates = static_cast<platform::ContextGles&>
	    (*cmdBuffer.getContext()).getCurrentRenderStates();
	switch (m_face)
	{
	case types::StencilFace::Front:
		gl::StencilFuncSeparate(GL_FRONT, ConvertToGles::comparisonMode(recordedStates.depthStencil.stencilOpFront),
		                        m_ref, recordedStates.depthStencil.readMaskFront);
		recordedStates.depthStencil.refFront = m_ref;

		break;
	case types::StencilFace::Back:
		gl::StencilFuncSeparate(GL_BACK, ConvertToGles::comparisonMode(recordedStates.depthStencil.stencilOpBack),
		                        m_ref, recordedStates.depthStencil.readMaskBack);
		recordedStates.depthStencil.refBack = m_ref;

		break;
	case types::StencilFace::FrontBack:
		gl::StencilFuncSeparate(GL_FRONT, ConvertToGles::comparisonMode(recordedStates.depthStencil.stencilOpFront),
		                        m_ref, recordedStates.depthStencil.readMaskFront);

		gl::StencilFuncSeparate(GL_BACK, ConvertToGles::comparisonMode(recordedStates.depthStencil.stencilOpBack),
		                        m_ref, recordedStates.depthStencil.readMaskBack);

		recordedStates.depthStencil.refFront = recordedStates.depthStencil.refBack = m_ref;
		break;
	}
}

void SetStencilWriteMask::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	platform::ContextGles::RenderStatesTracker& recordedStates = static_cast<platform::ContextGles&>
	    (*cmdBuffer.getContext()).getCurrentRenderStates();
	switch (m_face)
	{
	case types::StencilFace::Front:
		gl::StencilMaskSeparate(GL_FRONT, m_mask);
		recordedStates.depthStencil.writeMaskFront = m_mask;
		break;
	case types::StencilFace::Back:
		gl::StencilMaskSeparate(GL_BACK, m_mask);
		recordedStates.depthStencil.writeMaskBack = m_mask;
		break;
	case types::StencilFace::FrontBack:
		gl::StencilMaskSeparate(GL_FRONT_AND_BACK, m_mask);
		recordedStates.depthStencil.writeMaskBack = recordedStates.depthStencil.writeMaskFront = m_mask;
		break;
	}
}


//////////////////////////// COMPUTE COMMANDS ////////////////////////////
void DispatchCompute::execute_private(impl::CommandBufferBase_& cmdBuffer)
{
	if (cmdBuffer.getContext()->hasApiCapability(ApiCapabilities::ComputeShader))
	{
		assertion(cmdBuffer.getContext()->isQueueSupported(DeviceQueueType::Compute),
		          "Compute Queue Not supported by the Context");
		gl::DispatchCompute(m_numGroupXYZ[0], m_numGroupXYZ[1], m_numGroupXYZ[2]);
	}
}

Sync_::Sync_() {}
Sync_::~Sync_() {}

//////////////////////////////// FENCE/SYNC COMMANDS ////////////////////////////////
//impl::SyncImpl::SyncImpl() : maxSize(10) { }
//impl::SyncImpl::~SyncImpl()
//{
//  discardLast(maxSize - (uint32)pimpl.size());
//}
//
//void impl::SyncImpl::discardLast(int32 howMany)
//{
//  while (howMany-- > 0 && pimpl.size())
//  {
//    gl::DeleteSync((GLsync)pimpl.front());
//    pimpl.pop_front();
//  }
//}
//
//bool impl::SyncImpl::isSignaled(uint32 which)
//{
//  if (which >= pimpl.size()) { return false; }
//  GLint retval;
//  gl::GetSynciv((GLsync)pimpl[which], GL_SYNC_STATUS, sizeof(GLint), NULL, &retval);
//  return retval == GL_SIGNALED;
//}
//
//
//SyncWaitResult impl::SyncImpl::clientWait(uint32 which, uint64 timeout)
//{
//  if (which >= pimpl.size()) { return SyncWaitResult::SyncPointNotCreatedYet; }
//  uint64 tmp = (timeout ? timeout : 1000000);
//  for (;;)
//  {
//    switch (gl::ClientWaitSync((GLsync)pimpl[which], GL_SYNC_FLUSH_COMMANDS_BIT, tmp))
//    {
//    //If a timeout is set, return the result of the command
//    //If a timeout is NOT set, loop until you get a result OTHER than timeout.
//    case GL_CONDITION_SATISFIED:
//    case GL_ALREADY_SIGNALED:
//      return SyncWaitResult::Ok;
//    case GL_TIMEOUT_EXPIRED:
//      if (timeout) { return SyncWaitResult::TimeoutExpired; }
//      break;
//    case GL_WAIT_FAILED:
//    default:
//      return SyncWaitResult::Failed;
//    }
//  }
//}
//void impl::SyncImpl::serverWait(uint32 which)
//{
//  if (which < pimpl.size())
//  {
//    gl::WaitSync((GLsync)pimpl[which], 0, GL_TIMEOUT_IGNORED);
//  }
//}
//
//void impl::CreateFenceSyncImpl::execute_private(impl::CommandBufferBase_& cmdBuffer)
//{
//  impl::SyncImpl& syncobj = *syncObject;
//  syncobj.discardLast((uint32)syncobj.pimpl.size() - syncobj.maxSize + 1);
//  syncobj.pimpl.push_front(gl::FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
//}

//////////////////////////////// UNIFORM COMMANDS ////////////////////////////////
void SetUniform<float32>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform1f(location, val);
}

void SetUniform<int32>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform1i(location, val);
}

void SetUniform<uint32>::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	if (cmdBuff.getContext()->hasApiCapability(ApiCapabilities::UintUniforms))
	{
		gl::Uniform1ui(location, val);
	}
	else
	{
		assertion(0, "UNSIGNED INT UNIFORM NOT SUPPORTED IN OPENGL ES 2");
	}
}

void SetUniformPtr<float32>::execute_private(impl::CommandBufferBase_&)
{
	if (count == 1)
	{
		gl::Uniform1f(location, *val);
	}
	else
	{
		gl::Uniform1fv(location, count, val);
	}
}
void SetUniformPtr<int32>::execute_private(impl::CommandBufferBase_&)
{
	if (count == 1)
	{
		gl::Uniform1i(location, *val);
	}
	else
	{
		gl::Uniform1iv(location, count, val);
	}
}
void SetUniformPtr<uint32>::execute_private(impl::CommandBufferBase_&)
{
	if (count == 1)
	{
		gl::Uniform1ui(location, *val);
	}
	else
	{
		gl::Uniform1uiv(location, count, val);
	}
}

void  SetUniform<glm::ivec2>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform2i(location, val.x, val.y);
}

void SetUniform<glm::uvec2>::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	if (cmdBuff.getContext()->hasApiCapability(ApiCapabilities::UintUniforms))
	{
		gl::Uniform2ui(location, val.x, val.y);
	}
	else
	{
		assertion(0, "UNSIGNED INT UNIFORMS NOT SUPPORTED IN OPENGL ES 2");
		Log(Log.Error, "SetUniform<uvec2>::execute NOT SUPPORTED in OpenGL ES 2");
	}
}

void SetUniform<glm::vec2>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform2f(location, val.x, val.y);
}

void SetUniformPtr<glm::ivec2>::execute_private(impl::CommandBufferBase_&)
{
	if (count == 1)
	{
		gl::Uniform2i(location, val->x, val->y);
	}
	else
	{
		gl::Uniform2iv(location, count, glm::value_ptr(*val));
	}
}

void SetUniformPtr<glm::uvec2>::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	if (cmdBuff.getContext()->hasApiCapability(ApiCapabilities::UintUniforms))
	{
		if (count == 1)
		{
			gl::Uniform2ui(location, val->x, val->y);
		}
		else
		{
			gl::Uniform2uiv(location, count, glm::value_ptr(*val));
		}
	}
	else
	{
		assertion(0, "UNSIGNED INT UNIFORMS NOT SUPPORTED IN OPENGL ES 2");
		Log(Log.Error, "SetUniformPtr<uvec2>::execute NOT SUPPORTED in OpenGL ES 2");
	}
}

void SetUniformPtr<glm::vec2>::execute_private(impl::CommandBufferBase_&)
{
	if (count == 1)
	{
		gl::Uniform2f(location, val->x, val->y);
	}
	else
	{
		gl::Uniform2fv(location, count, glm::value_ptr(*val));
	}
}

void SetUniform<glm::ivec3>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform3i(location, val.x, val.y, val.z);
}

void SetUniform<glm::uvec3>::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	if (cmdBuff.getContext()->hasApiCapability(ApiCapabilities::UintUniforms))
	{
		gl::Uniform3ui(location, val.x, val.y, val.z);
	}
	else
	{
		assertion(0, "UNSIGNED INT UNIFORMS NOT SUPPORTED IN OPENGL ES 2");
		Log(Log.Error, "SetUniformPtr<uvec2>::execute NOT SUPPORTED in OpenGL ES 2");
	}
}

void SetUniform<glm::vec3>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform3f(location, val.x, val.y, val.z);
}

void SetUniformPtr<glm::ivec3>::execute_private(impl::CommandBufferBase_&)
{
	if (count == 1)
	{
		gl::Uniform3i(location, val->x, val->y, val->z);
	}
	else
	{
		gl::Uniform3iv(location, count, glm::value_ptr(*val));
	}
}

void SetUniformPtr<glm::uvec3>::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	if (cmdBuff.getContext()->hasApiCapability(ApiCapabilities::UintUniforms))
	{
		if (count == 1)
		{
			gl::Uniform3ui(location, val->x, val->y, val->z);
		}
		else
		{
			gl::Uniform3uiv(location, count, glm::value_ptr(*val));
		}
	}
	else
	{
		assertion(0, "UNSIGNED INT UNIFORMS NOT SUPPORTED IN OPENGL ES 2");
		Log(Log.Error, "SetUniformPtr<uvec3>::execute NOT SUPPORTED in OpenGL ES 2");
	}
}

void SetUniformPtr<glm::vec3>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform3fv(location, count, glm::value_ptr(*val));
}

void SetUniform<glm::ivec4>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform4i(location, val.x, val.y, val.z, val.w);
}

void SetUniform<glm::uvec4>::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	if (cmdBuff.getContext()->hasApiCapability(ApiCapabilities::UintUniforms))
	{
		gl::Uniform4ui(location, val.x, val.y, val.z, val.w);
	}
	else
	{
		assertion(0, "UNSIGNED INT UNIFORMS NOT SUPPORTED IN OPENGL ES 2");
		Log(Log.Error, "SetUniformPtr<uvec3>::execute NOT SUPPORTED in OpenGL ES 2");
	}
}

void SetUniform<glm::vec4>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform4f(location, val.x, val.y, val.z, val.w);
}

void SetUniform<glm::mat2>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix2fv(location, 1, false, glm::value_ptr(val));
}

void SetUniform<glm::mat2x3>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix2x3fv(location, 1, false, glm::value_ptr(val));
}

void SetUniform<glm::mat2x4>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix2x4fv(location, 1, false, glm::value_ptr(val));
}

void SetUniform<glm::mat3x2>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix3x2fv(location, 1, false, glm::value_ptr(val));
}

void SetUniform<glm::mat3>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix3fv(location, 1, false, glm::value_ptr(val));
}

void SetUniform<glm::mat3x4>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix3x4fv(location, 1, false, glm::value_ptr(val));
}

void SetUniform<glm::mat4x2>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix4x2fv(location, 1, false, glm::value_ptr(val));
}

void SetUniform<glm::mat4x3>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix4x3fv(location, 1, false, glm::value_ptr(val));
}

void SetUniform<glm::mat4>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix4fv(location, 1, false, glm::value_ptr(val));
}

void SetUniformPtr<glm::ivec4>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform4iv(location, count, glm::value_ptr(*val));
}

void SetUniformPtr<glm::uvec4>::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	if (cmdBuff.getContext()->hasApiCapability(ApiCapabilities::UintUniforms))
	{
		gl::Uniform4uiv(location, count, glm::value_ptr(*val));
	}
	else
	{
		assertion(0, "UNSIGNED INT UNIFORMS NOT SUPPORTED IN OPENGL ES 2");
		Log(Log.Error, "SetUniformPtr<uvec4>::execute NOT SUPPORTED in OpenGL ES 2");
	}
}

void SetUniformPtr<glm::vec4>::execute_private(impl::CommandBufferBase_&)
{
	gl::Uniform4fv(location, count, glm::value_ptr(*val));
}

void SetUniformPtr<glm::mat2>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix2fv(location, count, false, glm::value_ptr(*val));
}

void SetUniformPtr<glm::mat2x3>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix2x3fv(location, count, false, glm::value_ptr(*val));
}

void SetUniformPtr<glm::mat2x4>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix2x4fv(location, count, false, glm::value_ptr(*val));
}

void SetUniformPtr<glm::mat3x2>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix3x2fv(location, count, false, glm::value_ptr(*val));
}

void SetUniformPtr<glm::mat3>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix3fv(location, count, false, glm::value_ptr(*val));
}

void SetUniformPtr<glm::mat3x4>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix3x4fv(location, count, false, glm::value_ptr(*val));
}

void SetUniformPtr<glm::mat4x2>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix4x2fv(location, count, false, glm::value_ptr(*val));
}
void SetUniformPtr<glm::mat4x3>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix4x3fv(location, count, false, glm::value_ptr(*val));
}
void SetUniformPtr<glm::mat4>::execute_private(impl::CommandBufferBase_&)
{
	gl::UniformMatrix4fv(location, count, false, glm::value_ptr(*val));
}

void BindVertexBuffer::execute_private(impl::CommandBufferBase_& cmdBuff)
{
	platform::ContextGles& context = static_cast<platform::ContextGles&>(*cmdBuff.getContext());
	for (pvr::uint16 i = 0, bindIndex = startBinding; i < (uint16)buffers.size(); ++i, ++bindIndex)
	{
		assertion(static_cast<pvr::uint32>(buffers[i]->getBufferUsage() & types::BufferBindingUse::VertexBuffer) != 0, "bindVertexBuffer: Invalid usage flags");
		context.getCurrentRenderStates().vboBindings[bindIndex] = buffers[i];
	}
}

}

void PipelineBarrier::execute_private(impl::CommandBufferBase_& cb)
{
	gl::MemoryBarrier(barrier);
}

}
}
//!\endcond
