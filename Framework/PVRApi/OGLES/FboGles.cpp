/*!*********************************************************************************************************************
\file         PVRApi/OGLES/FboGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the FBO supporting classes (Fbo, Color attachment view etc). See FboGles.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/FboGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRApi/OGLES/RenderPassGles.h"
namespace pvr {
namespace api {

inline static uint32 fboBindTargetToGlesName(types::FboBindingTarget target)
{
//#if BUILD_API_MAX<30
//	GLenum glTarget[] = { GL_NONE, GL_FRAMEBUFFER, GL_FRAMEBUFFER, GL_FRAMEBUFFER };
//#else
//	GLenum glTarget[] = { GL_NONE, GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER };
//#endif
	return GL_FRAMEBUFFER;
}

namespace impl {
Fbo_::Fbo_(GraphicsContext& context) : m_context(context) {}


}

namespace gles {
void FboGles_::destroy()
{
	if (m_context.isValid())
	{
		gl::DeleteFramebuffers(1, &static_cast<gles::FboGles_&>(*this).handle);
		debugLogApiError("Fbo_::destroy exit");
	}
	else
	{
		Log(Log.Warning, "FBO object was not cleaned up before context destruction");
	}
}

FboGles_::FboGles_(GraphicsContext& context) : Fbo_(context), m_target(types::FboBindingTarget::ReadWrite) {}

bool DefaultFboGles_::checkFboStatus() {	return (handle == 0 ? true : false); }

DefaultFboGles_::DefaultFboGles_(GraphicsContext& context) :	FboGles_(context) {}

void DefaultFboGles_::bind(IGraphicsContext& context,
                           types::FboBindingTarget target)const
{
	m_target = target;
#if defined(TARGET_OS_IPHONE)
	static_cast<platform::ContextGles&>(context).getPlatformContext().makeCurrent();
	debugLogApiError("FboAttachmentImpl::attachTo exit");
	return;
#endif
	debugLogApiError("DefaultFboGles_::bind enter;");
	gl::BindFramebuffer(fboBindTargetToGlesName(m_target), 0);
	debugLogApiError("DefaultFboGles_::bind exit;");
}

void FboGles_::bind(IGraphicsContext& context, types::FboBindingTarget target) const
{
	m_target = target;
	gl::BindFramebuffer(fboBindTargetToGlesName(m_target), handle);
	debugLogApiError("FboAttachmentImpl::attachTo exit");
}

bool FboGles_::init(const FboCreateParam& desc)
{
	// validate
	m_desc = desc;
	assertion(desc.getRenderPass().isValid() , "Invalid RenderPass");
	m_target = types::FboBindingTarget::ReadWrite;
	gl::GenFramebuffers(1, &handle);
	gl::BindFramebuffer(GL_FRAMEBUFFER, handle);
	debugLogApiError("FboAttachmentImpl::init bind fbo");
#if defined (GL_FRAMEBUFFER_DEFAULT_WIDTH) && defined(GL_FRAMEBUFFER_DEFAULT_HEIGHT)
	if (m_context->getApiType() >= pvr::Api::OpenGLES31)
	{
		gl::FramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, desc.getDimensions().x);
		gl::FramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, desc.getDimensions().y);
	}
#endif
	std::vector<GLenum> drawBuffers;
	if (desc.getDepthStencilAttachment().isValid())
	{
		auto& texViewEs = static_cast<gles::TextureViewGles_&>(*m_desc.getDepthStencilAttachment());
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
		pvr::Log("Renderpass color info does not match with fbo create info");
		assertion(false , "Renderpass color info does not match with fbo create info");
		return false;
	}

	for (uint32 i = 0; i < (uint32)desc.getNumColorAttachements(); ++i)
	{
		auto& texViewEs = static_cast<const gles::TextureViewGles_&>(*m_desc.getColorAttachment(i));
		if (texViewEs.getResource()->getFormat() !=
		    static_cast<const RenderPassGles_&>(*desc.getRenderPass()).getCreateParam().getColorInfo(i).format)
		{
			pvr::Log("The renderPass color format does not match with color attachment view.");
			assertion(false , "The renderPass color format does not match with color attachment view.");
			return false;
		}
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
			assertion(false, "The texture provided has an unsupported format.");
			return false;
		}
		else
		{
			gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
			                         texViewEs.getResource()->getNativeObject().handle, texViewEs.getSubResourceRange().mipLevelOffset);
		}
		drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + (GLenum)i);
	}

#if BUILD_API_MAX >= 30
	if (drawBuffers.size() > 1) {	gl::DrawBuffers((GLsizei)drawBuffers.size(), &drawBuffers[0]);	}
#endif
	debugLogApiError("FboAttachmentImpl::glDrawBuffers failed");
	bool fboStatus = checkFboStatus();
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
	debugLogApiError("FboAttachmentImpl::init bind 0");
	return fboStatus ? true : false;
}

bool gles::FboGles_::checkFboStatus()
{
	// check status
	GLenum fboStatus = gl::CheckFramebufferStatus(fboBindTargetToGlesName(m_target));
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
const native::HFbo_& impl::Fbo_::getNativeObject()const
{
	return static_cast<const gles::FboGles_&>(*this);
}

native::HFbo_& impl::Fbo_::getNativeObject()
{
	return static_cast<gles::FboGles_&>(*this);
}

}// namespace api
}// namespace pvr
//!\endcond
