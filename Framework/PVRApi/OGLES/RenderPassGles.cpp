/*!*********************************************************************************************************************
\file         PVRApi\OGLES\RenderPassGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Definitions of the OpenGL ES implementation of the RenderPass.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/RenderPassGles.h"
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"

namespace pvr {
using namespace types;
namespace api {
namespace gles {
void RenderPassGles_::destroy()
{
	m_desc.clear();
}

bool RenderPassGles_::init(const RenderPassCreateParam& descriptor)
{
	m_desc = descriptor;
	return true;
}

void RenderPassGles_::begin(IGraphicsContext& device, const api::Fbo& fbo, const pvr::Rectanglei& renderArea,
                            glm::vec4* clearColor, pvr::uint32 numClearColor, pvr::float32 clearDepth,
                            pvr::int32 clearStencil) const
{
	platform::ContextGles& deviceEs = static_cast<platform::ContextGles&>(device);
	platform::ContextGles::RenderStatesTracker& renderStates = deviceEs.getCurrentRenderStates();
	GLbitfield clears = 0;
	assertion(fbo.isValid() , "Null Fbo");
	int myint;
#ifndef GL_DRAW_FRAMEBUFFER_BINDING
	gl::GetIntegerv(GL_FRAMEBUFFER_BINDING, &myint);
#else
	gl::GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &myint);
#endif
	bool isFrameBufferZero = (myint == 0);
	if (renderStates.viewport != renderArea)
	{
		debugLogApiError("RenderPass_::begin begin set view port");
		gl::Viewport(renderArea.x, renderArea.y, renderArea.width, renderArea.height);
		debugLogApiError("RenderPass_::begin end set view port");
		renderStates.viewport = renderArea;
	}

	if (renderStates.scissor != renderArea)
	{
		debugLogApiError("RenderPass_::begin begin set scissor");
		gl::Scissor(renderArea.x, renderArea.y, renderArea.width, renderArea.height);
		debugLogApiError("RenderPass_::begin end set scissor");
	}

	if (m_desc.getNumSubPass() > 1)
	{
#ifdef GL_SHADER_PIXEL_LOCAL_STORAGE_EXT
		if (m_context->isExtensionSupported("GL_EXT_shader_pixel_local_storage"))
		{
			gl::Enable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT);
		}
#endif
	}

	if (device.getApiType() <= Api::OpenGLES2 && m_desc.getNumColorInfo())
	{
		const LoadOp& loadOpColor = m_desc.getColorInfo(0).loadOpColor;
		const StoreOp& storeOpColor  = m_desc.getColorInfo(0).storeOpColor;

		for (uint32 i = 1; i < m_desc.getNumColorInfo(); ++i)
		{
			if (loadOpColor != m_desc.getColorInfo(i).loadOpColor)
			{
				Log(Log.Error, "Different LoadOps defined for attachments of an FBO. OpenGL ES 2 cannot support"
				    " different ops per attachment - defaulting to LoadOp of Attachment 0");
			}
			if (storeOpColor != m_desc.getColorInfo(i).storeOpColor)
			{
				Log(Log.Error, "Different StoreOps defined for attachments of an FBO. OpenGL ES 2 cannot support"
				    " different ops per attachment - defaulting to StoreOp of Attachment 0");
			}
		}
	}

	std::vector<GLenum> invalidateAttachments;
	//Weird for condition is so that for <=GLES2, run only once
	for (uint32 i = 0; i < m_desc.getNumColorInfo() && (i == 0 || device.getApiType() >= Api::OpenGLES3); ++i)
	{
		const LoadOp& loadOp = m_desc.getColorInfo(i).loadOpColor;
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
					debugLogApiError("RenderPass_::begin begin set clear-color");
					gl::ClearColor(clearColor[0].r, clearColor[0].g, clearColor[0].b, clearColor[0].a);
					debugLogApiError("RenderPass_::begin end set clear-color");
					clears |= GL_COLOR_BUFFER_BIT;
					// set the color write mask if the current state is disabled.
				}
			}
			else
			{
				debugLogApiError("RenderPass_::begin begin clear-color");
				gl::ClearBufferfv(GL_COLOR, i, &clearColor[0].r);
				debugLogApiError("RenderPass_::begin end clearcolor");
			}
			break;
		case LoadOp::Load:/*Default OpenGL*/ break;
		}
	}// next color op

	// apply depth load op
	switch (m_desc.getDepthStencilInfo().loadOpDepth)
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
		debugLogApiError("RenderPass_::begin begin clear depth");
		gl::ClearDepthf(clearDepth);
		debugLogApiError("RenderPass_::begin end clear color");
		if (!renderStates.depthStencil.depthWrite) { gl::DepthMask(true); }
		clears |= GL_DEPTH_BUFFER_BIT;
		break;
	}
	debugLogApiError("RenderPass_::begin depth");

	// apply stencil load op
	switch (m_desc.getDepthStencilInfo().loadOpStencil)
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
		if (!renderStates.depthStencil.stencilWriteMask) {	gl::StencilMask(1); }
		debugLogApiError("RenderPass_::begin begin clear stencil");
		gl::ClearStencil(clearStencil);
		debugLogApiError("RenderPass_::begin end clear stencil");
		clears |= GL_STENCIL_BUFFER_BIT;
		break;
	}

	// do fbo's invalidate operation
	if (invalidateAttachments.size())
	{
		debugLogApiError("RenderPass_::begin invalidate bind");
		gl::InvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
		debugLogApiError("RenderPass_::begin invalidate");
	}
	if (clears)
	{
		debugLogApiError("RenderPass_::begin begin clear");
		gl::Clear(clears);
		debugLogApiError("RenderPass_::begin end clear");
	}

	// unset the states.
	if (clears & GL_DEPTH_BUFFER_BIT && !renderStates.depthStencil.depthWrite)
	{
		debugLogApiError("RenderPass_::begin begin depthmask");
		gl::DepthMask(GL_FALSE);
		debugLogApiError("RenderPass_::begin end depthmask");
	}

	if (clears & GL_COLOR_BUFFER_BIT && renderStates.colorWriteMask != glm::bvec4(true))
	{
		debugLogApiError("RenderPass_::begin begin colormask");
		gl::ColorMask(renderStates.colorWriteMask.r, renderStates.colorWriteMask.g, renderStates.colorWriteMask.b,
		              renderStates.colorWriteMask.a);
		debugLogApiError("RenderPass_::begin end colormask");
	}

	if (clears & GL_STENCIL_BUFFER_BIT && !renderStates.depthStencil.stencilWriteMask)
	{
		debugLogApiError("RenderPass_::begin begin stencilmask");
		gl::StencilMask(0);
		debugLogApiError("RenderPass_::begin end stencilmask");
	}
	if (renderStates.scissor != renderArea)
	{
		debugLogApiError("RenderPass_::begin begin scissor");
		gl::Scissor(renderStates.scissor.x, renderStates.scissor.y, renderStates.scissor.width, renderStates.scissor.height);
		debugLogApiError("RenderPass_::begin end scissor");
	}
	debugLogApiError("RenderPass_::begin end scissor");
}

void RenderPassGles_::end(IGraphicsContext& context) const
{
	if (m_desc.getNumSubPass() > 1)
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


	if (context.getApiType() <= Api::OpenGLES2 && m_desc.getNumColorInfo())
	{
		const StoreOp& storeOpColor = m_desc.getColorInfo(0).storeOpColor;
		for (uint32 i = 1; i < m_desc.getNumColorInfo(); ++i)
		{
			if (storeOpColor != m_desc.getColorInfo(i).storeOpColor)
			{
				Log(Log.Error, "Different StoreOps defined for attachments of an FBO. OpenGL ES 2 cannot support"
				    " different ops per attachment - defaulting to StoreOp of Attachment 0");
			}
		}
	}

	if (context.hasApiCapability(ApiCapabilities::InvalidateFrameBuffer))
	{
		//ES 2 does not support ignore, and store is the default.
		std::vector<GLenum> invalidateAttachments;
		for (uint32 i = 0; i < m_desc.getNumColorInfo(); ++i)
		{
			if (m_desc.getColorInfo(i).storeOpColor == StoreOp::Ignore)
			{
				invalidateAttachments.push_back(isFrameBufferZero ? GL_COLOR : (GL_COLOR_ATTACHMENT0 + i));
			}
		}
		debugLogApiError("RenderPass_::end colorops store color");

		// apply depth load op

		//ES 2 does not support ignore, and store is the default.
		if (m_desc.getDepthStencilInfo().storeOpDepth == StoreOp::Ignore)
		{
			invalidateAttachments.push_back(isFrameBufferZero ? GL_DEPTH : GL_DEPTH_ATTACHMENT);
		}
		debugLogApiError("RenderPass_::end depth");


		//ES 2 does not support ignore, and store is the default.
		// apply stencil load op
		if (m_desc.getDepthStencilInfo().storeOpStencil == StoreOp::Ignore)
		{
			invalidateAttachments.push_back(isFrameBufferZero ? GL_STENCIL : GL_STENCIL_ATTACHMENT);
		}
		debugLogApiError("RenderPass_::end stencil");

		// do fbo's invalidate operation
		if (invalidateAttachments.size() && context.hasApiCapabilityNatively(ApiCapabilities::InvalidateFrameBuffer))
		{
			gl::InvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
			debugLogApiError("RenderPass_::end invalidate");
		}
		else if (invalidateAttachments.size() && context.hasApiCapabilityExtension(ApiCapabilities::InvalidateFrameBuffer))
		{
			glext::DiscardFramebufferEXT(GL_FRAMEBUFFER, (GLsizei)invalidateAttachments.size(), &invalidateAttachments[0]);
			debugLogApiError("RenderPass_::end discard");
		}
	}
	debugLogApiError("RenderPass_::end exit");
#if defined(TARGET_OS_IPHONE)
	static_cast<platform::ContextGles&>(context).getPlatformContext().presentBackbuffer();
#endif
}
}
}
}
//!\endcond
