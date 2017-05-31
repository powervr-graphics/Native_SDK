/*!
\brief Definitions of the OpenGL ES implementation of the RenderPass.
\file PVRApi/OGLES/RenderPassGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/RenderPassGles.h"
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"

namespace pvr {
using namespace types;
namespace api {
namespace gles {
void RenderPassGles_::destroy()
{
<<<<<<< HEAD
	m_desc.clear();
=======
	_desc.clear();
>>>>>>> 1776432f... 4.3
}

bool RenderPassGles_::init(const RenderPassCreateParam& descriptor)
{
	_desc = descriptor;
	return true;
}

void RenderPassGles_::begin(IGraphicsContext& device, const api::Fbo& fbo, const pvr::Rectanglei& renderArea,
                            glm::vec4* clearColor, pvr::uint32 numClearColor, pvr::float32 clearDepth,
                            pvr::int32 clearStencil) const
{
	platform::ContextGles& deviceEs = native_cast(device);
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

	if (_desc.getNumSubPass() > 1)
	{
#ifdef GL_SHADER_PIXEL_LOCAL_STORAGE_EXT
		if (_context->isExtensionSupported("GL_EXT_shader_pixel_local_storage"))
		{
			gl::Enable(GL_SHADER_PIXEL_LOCAL_STORAGE_EXT);
		}
#endif
	}

	if (device.getApiType() <= Api::OpenGLES2 && _desc.getNumColorInfo())
	{
<<<<<<< HEAD
		const LoadOp& loadOpColor = m_desc.getColorInfo(0).loadOpColor;
		const StoreOp& storeOpColor  = m_desc.getColorInfo(0).storeOpColor;
=======
		const LoadOp& loadOpColor = _desc.getColorInfo(0).loadOpColor;
		const StoreOp& storeOpColor  = _desc.getColorInfo(0).storeOpColor;
>>>>>>> 1776432f... 4.3

		for (uint32 i = 1; i < _desc.getNumColorInfo(); ++i)
		{
			if (loadOpColor != _desc.getColorInfo(i).loadOpColor)
			{
				Log(Log.Error, "Different LoadOps defined for attachments of an FBO. OpenGL ES 2 cannot support"
				    " different ops per attachment - defaulting to LoadOp of Attachment 0");
			}
			if (storeOpColor != _desc.getColorInfo(i).storeOpColor)
			{
				Log(Log.Error, "Different StoreOps defined for attachments of an FBO. OpenGL ES 2 cannot support"
				    " different ops per attachment - defaulting to StoreOp of Attachment 0");
			}
		}
	}

	std::vector<GLenum> invalidateAttachments;
	//Weird for condition is so that for <=GLES2, run only once
	for (uint32 i = 0; i < _desc.getNumColorInfo() && (i == 0 || device.getApiType() >= Api::OpenGLES3); ++i)
	{
<<<<<<< HEAD
		const LoadOp& loadOp = m_desc.getColorInfo(i).loadOpColor;
=======
		const LoadOp& loadOp = _desc.getColorInfo(i).loadOpColor;
>>>>>>> 1776432f... 4.3
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
	if(_desc.getNumDepthStencilInfo() > 0)
	{
		switch (_desc.getDepthStencilInfo(0).loadOpDepth)
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
	}
	debugLogApiError("RenderPass_::begin depth");

	// apply stencil load op
	if(_desc.getNumDepthStencilInfo() > 0)
	{
		switch (_desc.getDepthStencilInfo(0).loadOpStencil)
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
	if (_desc.getNumSubPass() > 1)
	{
#ifdef GL_SHADER_PIXEL_LOCAL_STORAGE_EXT
		if (_context->isExtensionSupported("GL_EXT_shader_pixel_local_storage"))
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


	if (context.getApiType() <= Api::OpenGLES2 && _desc.getNumColorInfo())
	{
<<<<<<< HEAD
		const StoreOp& storeOpColor = m_desc.getColorInfo(0).storeOpColor;
		for (uint32 i = 1; i < m_desc.getNumColorInfo(); ++i)
=======
		const StoreOp& storeOpColor = _desc.getColorInfo(0).storeOpColor;
		for (uint32 i = 1; i < _desc.getNumColorInfo(); ++i)
>>>>>>> 1776432f... 4.3
		{
			if (storeOpColor != _desc.getColorInfo(i).storeOpColor)
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
		for (uint32 i = 0; i < _desc.getNumColorInfo(); ++i)
		{
			if (_desc.getColorInfo(i).storeOpColor == StoreOp::Ignore)
			{
				invalidateAttachments.push_back(isFrameBufferZero ? GL_COLOR : (GL_COLOR_ATTACHMENT0 + i));
			}
		}
		debugLogApiError("RenderPass_::end colorops store color");

		// apply depth load op

		//ES 2 does not support ignore, and store is the default.
		if(_desc.getNumDepthStencilInfo() > 0)
		{
			if (_desc.getDepthStencilInfo(0).storeOpDepth == StoreOp::Ignore)
			{
				invalidateAttachments.push_back(isFrameBufferZero ? GL_DEPTH : GL_DEPTH_ATTACHMENT);
			}
			
			debugLogApiError("RenderPass_::end depth");
			
			
			//ES 2 does not support ignore, and store is the default.
			// apply stencil load op
			if (_desc.getDepthStencilInfo(0).storeOpStencil == StoreOp::Ignore)
			{
				invalidateAttachments.push_back(isFrameBufferZero ? GL_STENCIL : GL_STENCIL_ATTACHMENT);
			}
			debugLogApiError("RenderPass_::end stencil");
		}
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
	native_cast(context).getPlatformContext().presentBackbuffer();
#endif
}
}
}
}
