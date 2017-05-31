/*!
\brief OpenGL ES Implementation of the FBO supporting classes (Fbo, Color attachment view etc). See FboGles.h.
\file PVRApi/OGLES/FboGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/FboGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRApi/OGLES/RenderPassGles.h"
namespace pvr {
namespace api {

<<<<<<< HEAD
inline static uint32 fboBindTargetToGlesName(types::FboBindingTarget target)
=======
inline static uint32 fboBindTargetToGlesName(IGraphicsContext& context, types::FboBindingTarget target)
>>>>>>> 1776432f... 4.3
{
	GLenum glTargets[4];
#if BUILD_API_MAX<30
	glTargets[0] = GL_NONE;
	glTargets[1] = GL_FRAMEBUFFER;
	glTargets[2] = GL_FRAMEBUFFER;
	glTargets[3] = GL_FRAMEBUFFER;
#else
	if (context.getPlatformContext().isApiSupported(pvr::Api::OpenGLES3))
	{
		glTargets[0] = GL_NONE;
		glTargets[1] = GL_READ_FRAMEBUFFER;
		glTargets[2] = GL_DRAW_FRAMEBUFFER;
		glTargets[3] = GL_FRAMEBUFFER;
	}
	else
	{
		glTargets[0] = GL_NONE;
		glTargets[1] = GL_FRAMEBUFFER;
		glTargets[2] = GL_FRAMEBUFFER;
		glTargets[3] = GL_FRAMEBUFFER;
	}
#endif
	return glTargets[(uint32)target];
}

namespace impl {
Fbo_::Fbo_(const GraphicsContext& context) : _context(context) {}


}

namespace gles {
void FboGles_::destroy()
{
	if (_context.isValid())
	{
		gl::DeleteFramebuffers(1, &static_cast<gles::FboGles_&>(*this).handle);
		debugLogApiError("Fbo_::destroy exit");
	}
	else
	{
		Log(Log.Warning, "FBO object was not cleaned up before context destruction");
	}
}

FboGles_::FboGles_(const GraphicsContext& context) : Fbo_(context), _target(types::FboBindingTarget::ReadWrite) {}

bool DefaultFboGles_::checkFboStatus(GraphicsContext& context) {  return (handle == 0 ? true : false); }

DefaultFboGles_::DefaultFboGles_(const GraphicsContext& context) :  FboGles_(context) {}

void DefaultFboGles_::bind(IGraphicsContext& context,
                           types::FboBindingTarget target)const
{
	_target = target;
#if defined(TARGET_OS_IPHONE)
	native_cast(context).getPlatformContext().makeCurrent();
	debugLogApiError("FboAttachmentImpl::attachTo exit");
	return;
#endif
	debugLogApiError("DefaultFboGles_::bind enter;");
	gl::BindFramebuffer(fboBindTargetToGlesName(context, _target), 0);
	debugLogApiError("DefaultFboGles_::bind exit;");
}

void FboGles_::bind(IGraphicsContext& context, types::FboBindingTarget target) const
{
	_target = target;
	gl::BindFramebuffer(fboBindTargetToGlesName(context, _target), handle);
	debugLogApiError("FboAttachmentImpl::attachTo exit");
}

bool FboGles_::init(const FboCreateParam& desc)
{
	// validate
<<<<<<< HEAD
	m_desc = desc;
	assertion(desc.getRenderPass().isValid() , "Invalid RenderPass");
	m_target = types::FboBindingTarget::ReadWrite;
=======
	_desc = desc;
	assertion(desc.getRenderPass().isValid(), "Invalid RenderPass");
	_target = types::FboBindingTarget::ReadWrite;
>>>>>>> 1776432f... 4.3
	gl::GenFramebuffers(1, &handle);
	gl::BindFramebuffer(GL_FRAMEBUFFER, handle);
	debugLogApiError("FboAttachmentImpl::init bind fbo");
#if defined (GL_FRAMEBUFFER_DEFAULT_WIDTH) && defined(GL_FRAMEBUFFER_DEFAULT_HEIGHT)
	if (_context->getApiType() >= Api::OpenGLES31)
	{
		gl::FramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, desc.getDimensions().x);
		gl::FramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, desc.getDimensions().y);
	}
#endif
	std::vector<GLenum> drawBuffers;
	if (desc.getNumDepthStencilAttachments() && desc.getDepthStencilAttachment(0).isValid())
	{
<<<<<<< HEAD
		auto& texViewEs = static_cast<gles::TextureViewGles_&>(*m_desc.getDepthStencilAttachment());
=======
		auto& texViewEs = static_cast<gles::TextureViewGles_&>(*_desc.getDepthStencilAttachment(0));
>>>>>>> 1776432f... 4.3
		types::ImageAspect attachment = types::ImageAspect(0);
		if (texViewEs.getResource()->getFormat().format == PixelFormat::Depth16 ||
		    texViewEs.getResource()->getFormat().format == PixelFormat::Depth24 ||
		    texViewEs.getResource()->getFormat().format == PixelFormat::Depth32)
		{
			attachment = types::ImageAspect::Depth;
		}

		else if (texViewEs.getResource()->getFormat().format == PixelFormat::Depth24Stencil8 ||
		         texViewEs.getResource()->getFormat().format == PixelFormat::Depth32Stencil8)
		{
			attachment = types::ImageAspect::DepthAndStencil;
		}
		if ((uint32)attachment)
		{
<<<<<<< HEAD
			m_depthStencilAttachment.push_back(desc.getDepthStencilAttachment());
			if (m_desc.getDepthStencilAttachment()->getViewType() == types::ImageViewType::ImageView2DCube)
			{
				gl::FramebufferTexture2D(GL_FRAMEBUFFER, api::ConvertToGles::imageAspect(attachment),
				                         GL_TEXTURE_CUBE_MAP_POSITIVE_X + texViewEs.getSubResourceRange().arrayLayerOffset,
				                         texViewEs.getResource()->getNativeObject().handle, texViewEs.getSubResourceRange().mipLevelOffset);
			}
			else
			{
				gl::FramebufferTexture2D(GL_FRAMEBUFFER, api::ConvertToGles::imageAspect(attachment), GL_TEXTURE_2D,
				                         texViewEs.getResource()->getNativeObject().handle, texViewEs.getSubResourceRange().mipLevelOffset);
=======
			_depthStencilAttachment.push_back(desc.getDepthStencilAttachment(0));
			if (_desc.getDepthStencilAttachment(0)->getViewType() == types::ImageViewType::ImageView2DCube)
			{
				gl::FramebufferTexture2D(GL_FRAMEBUFFER, nativeGles::ConvertToGles::imageAspect(attachment),
				                         GL_TEXTURE_CUBE_MAP_POSITIVE_X + texViewEs.getSubResourceRange().arrayLayerOffset,
				                         static_cast<TextureStoreGles_&>(*texViewEs.getResource()).handle, texViewEs.getSubResourceRange().mipLevelOffset);
			}
			else
			{
				gl::FramebufferTexture2D(GL_FRAMEBUFFER, nativeGles::ConvertToGles::imageAspect(attachment), GL_TEXTURE_2D,
				                         static_cast<TextureStoreGles_&>(*texViewEs.getResource()).handle, texViewEs.getSubResourceRange().mipLevelOffset);
>>>>>>> 1776432f... 4.3
			}
		}
		else
		{
			Log("Invalid Fbo attachment type");
		}
	}

	const RenderPassCreateParam& renderPassInfo = static_cast<const RenderPassGles_&>(*desc.getRenderPass()).getCreateParam();
	if (renderPassInfo.getNumColorInfo() != desc.getNumColorAttachements())
	{
		Log("Renderpass color info does not match with fbo create info");
		assertion(false, "Renderpass color info does not match with fbo create info");
		return false;
	}

	for (uint32 i = 0; i < (uint32)desc.getNumColorAttachements(); ++i)
	{
		auto& texViewEs = static_cast<const gles::TextureViewGles_&>(*_desc.getColorAttachment(i));
		if (texViewEs.getResource()->getFormat() !=
		    static_cast<const RenderPassGles_&>(*desc.getRenderPass()).getCreateParam().getColorInfo(i).format)
		{
			Log("The renderPass color format does not match with color attachment view.");
			assertion(false, "The renderPass color format does not match with color attachment view.");
			return false;
		}
<<<<<<< HEAD
		m_colorAttachments.push_back(desc.getColorAttachment(i));

		if (texViewEs.getResource()->getDepth() > 1 && m_context->hasApiCapability(ApiCapabilities::FramebufferTextureLayer))
		{
			gl::FramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
			                            texViewEs.getResource()->getNativeObject().handle,
			                            texViewEs.getSubResourceRange().mipLevelOffset, texViewEs.getSubResourceRange().arrayLayerOffset);
		}
		else if (texViewEs.getResource()->getDepth() > 1 && !m_context->hasApiCapability(ApiCapabilities::FramebufferTextureLayer))
		{
			pvr::Log("The texture provided has an unsupported format.");
=======
		_colorAttachments.push_back(desc.getColorAttachment(i));

		if (texViewEs.getResource()->getDepth() > 1 && _context->hasApiCapability(ApiCapabilities::FramebufferTextureLayer))
		{
			gl::FramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
			                            static_cast<const TextureStoreGles_&>(*texViewEs.getResource()).handle,
			                            texViewEs.getSubResourceRange().mipLevelOffset, texViewEs.getSubResourceRange().arrayLayerOffset);
		}
		else if (texViewEs.getResource()->getDepth() > 1 && !_context->hasApiCapability(ApiCapabilities::FramebufferTextureLayer))
		{
			Log("The texture provided has an unsupported format.");
>>>>>>> 1776432f... 4.3
			assertion(false, "The texture provided has an unsupported format.");
			return false;
		}
		else
		{
			gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
<<<<<<< HEAD
			                         texViewEs.getResource()->getNativeObject().handle, texViewEs.getSubResourceRange().mipLevelOffset);
=======
			                         static_cast<const TextureStoreGles_&>(*texViewEs.getResource()).handle, texViewEs.getSubResourceRange().mipLevelOffset);
>>>>>>> 1776432f... 4.3
		}
		drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + (GLenum)i);
	}

#if BUILD_API_MAX >= 30
	if (drawBuffers.size() > 1) { gl::DrawBuffers((GLsizei)drawBuffers.size(), &drawBuffers[0]);  }
#endif
	debugLogApiError("FboAttachmentImpl::glDrawBuffers failed");
	bool fboStatus = checkFboStatus(_context);
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
	debugLogApiError("FboAttachmentImpl::init bind 0");
	return fboStatus ? true : false;
}

bool gles::FboGles_::checkFboStatus(GraphicsContext& context)
{
	// check status
	GLenum fboStatus = gl::CheckFramebufferStatus(fboBindTargetToGlesName(static_cast<IGraphicsContext&>(*context), _target));
	switch (fboStatus)
	{
#ifdef GL_FRAMEBUFFER_UNDEFINED
	case GL_FRAMEBUFFER_UNDEFINED:
		Log(Log.Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_UNDEFINED");
		assertion(0); break;
#endif
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		Log(Log.Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		assertion(0); break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		Log(Log.Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		assertion(0); break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		Log(Log.Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_UNSUPPORTED");
		assertion(0); break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		Log(Log.Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
		assertion(0); break;
#endif
	case GL_FRAMEBUFFER_COMPLETE: return true;
	default: Log(Log.Error, "Fbo_::checkFboStatus UNKNOWN ERROR");
		assertion(0); break;
	}
	return false;
}

}// namespace gles
}// namespace api
}// namespace pvr
