/*!*********************************************************************************************************************
\file         PVRApi\OGLES\RenderPass.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Definitions of the OpenGL ES implementation of the RenderPass.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/RenderPass.h"
#include "PVRApi/ApiObjects/FboCreateParam.h"
#include "PVRApi/ApiErrors.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"

namespace pvr {
namespace api {
namespace impl {
RenderPassImpl::~RenderPassImpl() {}

pvr::Result::Enum RenderPassImpl::init(const RenderPassCreateParam& descriptor)
{
	m_desc = descriptor;
	return pvr::Result::Success;
}

void RenderPassImpl::begin(IGraphicsContext& device, const api::Fbo& fbo, const pvr::Rectanglei& renderArea,
                           glm::vec4* clearColor, pvr::uint32 numClearColor, pvr::float32 clearDepth,
                           pvr::int32 clearStencil) const
{
	platform::ContextGles& deviceEs = static_cast<platform::ContextGles&>(device);
	platform::ContextGles::RenderStatesTracker& renderStates = deviceEs.getCurrentRenderStates();
	GLbitfield clears = 0;
	PVR_ASSERT(fbo.isValid() && "Null Fbo");
	int myint;
#if BUILD_API_MAX<30
	gl::GetIntegerv(GL_FRAMEBUFFER_BINDING, &myint);
#else
	gl::GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &myint);
#endif
	bool isFrameBufferZero = (myint == 0);
	if (renderStates.viewport != renderArea){
		gl::Viewport(renderArea.x, renderArea.y, renderArea.width, renderArea.height);
		renderStates.viewport = renderArea;
	}

	if (renderStates.scissor != renderArea)
	{
		gl::Scissor(renderArea.x, renderArea.y, renderArea.width, renderArea.height);
	}

	if (m_desc.subPass.size() > 1)
	{
#ifdef GL_SHADER_PIXEL_LOCAL_STORAGE_EXT
		if (m_context->isExtensionSupported("GL_EXT_shader_pixel_local_storage"))
		{
			gl::Enable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT);
		}
#endif
	}

	if (device.getApiType() <= Api::OpenGLES2 && m_desc.color.size())
	{
		const LoadOp::Enum& loadOpColor = m_desc.color[0].loadOpColor;
		const StoreOp::Enum& storeOpColor = m_desc.color[0].storeOpColor;

		for (uint32 i = 1; i < m_desc.color.size(); ++i)
		{
			if (loadOpColor != m_desc.color[i].loadOpColor)
			{
				Log(Log.Error, "Different LoadOps defined for attachments of an FBO. OpenGL ES 2 cannot support"
				    " different ops per attachment - defaulting to LoadOp of Attachment 0");
			}
			if (storeOpColor != m_desc.color[i].storeOpColor)
			{
				Log(Log.Error, "Different StoreOps defined for attachments of an FBO. OpenGL ES 2 cannot support"
				    " different ops per attachment - defaulting to StoreOp of Attachment 0");
			}
		}
	}

	std::vector<GLenum> invalidateAttachments;
	//Weird for condition is so that for <=GLES2, run only once
	for (uint32 i = 0; i < m_desc.color.size() && (i == 0 || device.getApiType() >= Api::OpenGLES3); ++i)
	{
		const LoadOp::Enum& loadOp = m_desc.color[i].loadOpColor;
		switch (loadOp)
		{
		case LoadOp::Ignore:
			// invalidate the framebuffer
			if (device.hasApiCapability(ApiCapabilities::InvalidateFrameBuffer))
			{
				invalidateAttachments.push_back(isFrameBufferZero ? GL_COLOR : GL_COLOR_ATTACHMENT0 + i);
			}
			else
			{
				clears |= GL_COLOR_BUFFER_BIT;
			}
			break;
		case LoadOp::Clear:
			if (renderStates.colorWriteMask != glm::bvec4(true))
			{
				gl::ColorMask(true, true, true, true);
			}
			if (fbo->isDefault() || !device.hasApiCapability(ApiCapabilities::ClearBuffer))
			{
				if (i == 0)
				{
					gl::ClearColor(clearColor[0].r, clearColor[0].g, clearColor[0].b, clearColor[0].a);
					clears |= GL_COLOR_BUFFER_BIT;
					// set the color write mask if the current state is disabled.
				}
			}
			else
			{
				gl::ClearBufferfv(GL_COLOR, i, &clearColor[0].r);
			}
			break;
		case LoadOp::Load:/*Default OpenGL*/ break;
		}
	}// next color op
	debugLogApiError("RenderPassImpl::begin color");

	// apply depth load op
	switch (m_desc.depthStencil.loadOpDepth)
	{
	case LoadOp::Load: break;// LoadOp::Load is the default GL behaviour - so don't clear or invalidate.
	case LoadOp::Ignore:
		// enable the depth mask if the current state is disabled
		if (!renderStates.depthStencil.depthWrite) { gl::DepthMask(true); }
		if (device.hasApiCapability(ApiCapabilities::InvalidateFrameBuffer))
		{
			invalidateAttachments.push_back(isFrameBufferZero ? GL_DEPTH : GL_DEPTH_ATTACHMENT);
		}
		else
		{
			clears |= GL_DEPTH_BUFFER_BIT;
		}
		break;
	case LoadOp::Clear:
		gl::ClearDepthf(clearDepth);
		if (!renderStates.depthStencil.depthWrite) { gl::DepthMask(true); }
		clears |= GL_DEPTH_BUFFER_BIT;
		break;
	}
	debugLogApiError("RenderPassImpl::begin depth");

	// apply stencil load op
	switch (m_desc.depthStencil.loadOpStencil)
	{
	case LoadOp::Load: break;// LoadOp::Load is the default GL behaviour - so don't clear or invalidate.
	case LoadOp::Ignore:
		// enable stencil write mask if it is disabled
		if (!renderStates.depthStencil.stencilWriteMask) { gl::StencilMask(1); }
		if (device.hasApiCapability(ApiCapabilities::InvalidateFrameBuffer))
		{
			invalidateAttachments.push_back(isFrameBufferZero ? GL_STENCIL : GL_STENCIL_ATTACHMENT);
		}
		else
		{
			clears |= GL_STENCIL_BUFFER_BIT;
			// enable stencil write mask if it is disabled
		}
		break;
	case LoadOp::Clear:
		if (!renderStates.depthStencil.stencilWriteMask) { gl::StencilMask(1); }
		gl::ClearStencil(clearStencil);
		clears |= GL_STENCIL_BUFFER_BIT;
		break;
	}
	debugLogApiError("RenderPassImpl::begin stencil");

	// do fbo's invalidate operation
	if (invalidateAttachments.size())
	{
		debugLogApiError("RenderPassImpl::begin invalidate bind");
		gl::InvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
		debugLogApiError("RenderPassImpl::begin invalidate");
	}
	if (clears) { gl::Clear(clears); }

	// unset the states.
	if (clears & GL_DEPTH_BUFFER_BIT && !renderStates.depthStencil.depthWrite) { gl::DepthMask(GL_FALSE);}

	if (clears & GL_COLOR_BUFFER_BIT && renderStates.colorWriteMask != glm::bvec4(true))
	{
		gl::ColorMask(renderStates.colorWriteMask.r, renderStates.colorWriteMask.g, renderStates.colorWriteMask.b,
		              renderStates.colorWriteMask.a);
	}

	if (clears & GL_STENCIL_BUFFER_BIT && !renderStates.depthStencil.stencilWriteMask) { gl::StencilMask(0); }
	if (renderStates.scissor != renderArea)
	{
		gl::Scissor(renderStates.scissor.x, renderStates.scissor.y, renderStates.scissor.width, renderStates.scissor.height);
	}

}

void RenderPassImpl::end(IGraphicsContext& context) const
{
	if (m_desc.subPass.size() > 1)
	{
#ifdef GL_SHADER_PIXEL_LOCAL_STORAGE_EXT
		if (m_context->isExtensionSupported("GL_EXT_shader_pixel_local_storage"))
		{
			gl::Disable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT);
		}
#endif
	}

	int myint;
#ifdef GL_DRAW_FRAMEBUFFER_BINDING
	if (context.getApiType() >= Api::OpenGLES3)
	{
		gl::GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &myint);
	}
	else
#endif
	{
		gl::GetIntegerv(GL_FRAMEBUFFER_BINDING, &myint);
	}
	bool isFrameBufferZero = (myint == 0);


	if (context.getApiType() <= Api::OpenGLES2 && m_desc.color.size())
	{
		const StoreOp::Enum& storeOpColor = m_desc.color[0].storeOpColor;
		for (uint32 i = 1; i < m_desc.color.size(); ++i)
		{
			if (storeOpColor != m_desc.color[i].storeOpColor)
			{
				Log(Log.Error, "Different StoreOps defined for attachments of an FBO. OpenGL ES 2 cannot support"
				    " different ops per attachment - defaulting to StoreOp of Attachment 0");
			}
		}
	}

	//ES 2 does not support ignore, and store is the default.
	std::vector<GLenum> invalidateAttachments;
	for (uint32 i = 0; i < m_desc.color.size(); ++i)
	{
		if (m_desc.color[i].storeOpColor == StoreOp::ResolveMsaa)
		{
			Log("MSAA Resolve store op not implemented. Using normal Store operation for Color.");
			PVR_ASSERT(0);

		}
		else if (m_desc.color[i].storeOpColor == StoreOp::Ignore)
		{
			invalidateAttachments.push_back(isFrameBufferZero ? GL_COLOR : (GL_COLOR_ATTACHMENT0 + i));
		}
	}
	debugLogApiError("RenderPassImpl::end colorops store color");

	// apply depth load op

#if BUILD_API_MAX>=30
	//ES 2 does not support ignore, and store is the default.
	if (m_desc.depthStencil.storeOpDepth == StoreOp::ResolveMsaa)
	{
		Log("MSAA Resolve store op not implemented. Using normal Store operation for Depth.");
		PVR_ASSERT(0);
	}
	else if (m_desc.depthStencil.storeOpDepth == StoreOp::Ignore)
	{
		invalidateAttachments.push_back(isFrameBufferZero ? GL_DEPTH : GL_DEPTH_ATTACHMENT);
	}
	debugLogApiError("RenderPassImpl::end depth");
#endif


#if BUILD_API_MAX>=30
	//ES 2 does not support ignore, and store is the default.
	// apply stencil load op
	if (m_desc.depthStencil.storeOpStencil == StoreOp::ResolveMsaa)
	{
		Log("MSAA Resolve store op not implemented. Using normal Store operation for Stencil.");
		PVR_ASSERT(0);
	}
	else if (m_desc.depthStencil.storeOpStencil == StoreOp::Ignore)
	{
		invalidateAttachments.push_back(isFrameBufferZero ? GL_STENCIL : GL_STENCIL_ATTACHMENT);
	}
	debugLogApiError("RenderPassImpl::end stencil");
#endif

	// do fbo's invalidate operation
    if (invalidateAttachments.size() &&context.hasApiCapabilityNatively(ApiCapabilities::InvalidateFrameBuffer))
	{
        gl::InvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
        debugLogApiError("RenderPassImpl::end invalidate");
	}
    else if (invalidateAttachments.size() && context.hasApiCapabilityExtension(ApiCapabilities::InvalidateFrameBuffer))
    {
        glext::DiscardFramebufferEXT(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
        debugLogApiError("RenderPassImpl::end discard");
    }
	debugLogApiError("RenderPassImpl::end exit");
#if defined(TARGET_OS_IPHONE)
	static_cast<platform::ContextGles&>(context).getPlatformContext().presentBackbuffer();
#endif
}

}
}
}
//!\endcond