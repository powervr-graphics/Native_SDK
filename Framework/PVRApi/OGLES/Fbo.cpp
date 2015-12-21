/*!*********************************************************************************************************************
\file         PVRApi/OGLES/Fbo.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the FBO supporting classes (Fbo, Color attachment view etc). See FboGles.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/FboGles.h"
#include "PVRApi/OGLES/ConvertToApiTypes.h"
#include "PVRApi/ApiErrors.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRApi/ApiObjects/RenderPass.h"
namespace pvr {
namespace api {
//
//inline static uint32 fboTextureAttachmentTargetToGlesName(FboTextureTarget::Enum
//    textureAttachment)
//{
//	PVR_ASSERT(textureAttachment < FboTextureTarget::Unknown && "FboTextureTarget is Unknown");
//	static const GLenum glFboTexTarget[] = { GL_TEXTURE_2D,
//	                                         GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
//	                                         GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
//	                                         GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
//	                                       };
//	return glFboTexTarget[textureAttachment];
//}

inline static uint32 fboBindTargetToGlesName(FboBindingTarget::Enum target)
{
//#if BUILD_API_MAX<30
//	GLenum glTarget[] = { GL_NONE, GL_FRAMEBUFFER, GL_FRAMEBUFFER, GL_FRAMEBUFFER };
//#else
//	GLenum glTarget[] = { GL_NONE, GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER };
//#endif
	return GL_FRAMEBUFFER;
}


inline static  uint32 fboAttatchmentTypeToGlesName(uint32 type)
{
#if BUILD_API_MAX<30
	static const GLenum glType[] =
	{
		GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, GL_NONE, GL_COLOR_ATTACHMENT0
	};
	if (type == FboAttachmentType::DepthStencil)
	{
		Log(Log.Error, "DEPTH_STENCIL_ATTACHMENT not supported in OpenGL ES 2.0");
	}
	if (type > FboAttachmentType::Color)
	{
		return (glType[FboAttachmentType::Color] + (type - FboAttachmentType::Color));
	}
	return glType[type];
#else
	static const GLenum glType[] =
	{
		GL_DEPTH_ATTACHMENT,
		GL_STENCIL_ATTACHMENT,
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_COLOR_ATTACHMENT0
	};
	if (type > FboAttachmentType::Color)
	{
		return (glType[FboAttachmentType::Color] + (type - FboAttachmentType::Color));
	}
	return glType[type];
#endif
}

void impl::ColorAttachmentViewImpl::attachTo(uint32 attachment)const
{
	if (texture->getTextureType() == pvr::TextureDimension::Texture2DCube)
	{
		gl::FramebufferTexture2D(GL_FRAMEBUFFER, fboAttatchmentTypeToGlesName(attachment),
		                         GL_TEXTURE_CUBE_MAP_POSITIVE_X + baseArraySlice,
		                         native::useNativeHandle(texture->getResource()), mipLevel);
	}
	else
	{
		gl::FramebufferTexture2D(GL_FRAMEBUFFER, fboAttatchmentTypeToGlesName(attachment), GL_TEXTURE_2D,
		                         native::useNativeHandle(texture->getResource()), mipLevel);
	}
	debugLogApiError("FboAttachmentImpl::attachTo exit");
}

void impl::DepthStencilViewImpl::attachTo()const
{
	PVR_ASSERT(type == FboAttachmentType::Depth ||
	           type == FboAttachmentType::DepthStencil ||
	           type == FboAttachmentType::Stencil && "invalid attachment Type");

	if (texture->getTextureType() == pvr::TextureDimension::Texture2DCube)
	{
		gl::FramebufferTexture2D(GL_FRAMEBUFFER, fboAttatchmentTypeToGlesName(type), GL_TEXTURE_CUBE_MAP_POSITIVE_X + baseArraySlice,
		                         native::useNativeHandle(texture->getResource()), mipLevel);
	}
	else
	{
		gl::FramebufferTexture2D(GL_FRAMEBUFFER, fboAttatchmentTypeToGlesName(type), GL_TEXTURE_2D,
		                         native::useNativeHandle(texture->getResource()), mipLevel);
	}
	debugLogApiError("FboAttachmentImpl::attachTo exit");
}

namespace impl {
bool DefaultFboGlesImpl::checkFboStatus() {	return (m_fbo->handle == 0 ? true : false); }

DefaultFboGlesImpl::DefaultFboGlesImpl(GraphicsContext& context) :	FboImpl(context) {}

void DefaultFboGlesImpl::bind(IGraphicsContext& context,
                              api::FboBindingTarget::Enum target)const
{
	m_target = target;
#if defined(TARGET_OS_IPHONE)
	static_cast<platform::ContextGles&>(context).getPlatformContext().makeCurrent();
	debugLogApiError("FboAttachmentImpl::attachTo exit");
	return;
#endif
	debugLogApiError("DefaultFboGlesImpl::bind enter;");
	gl::BindFramebuffer(fboBindTargetToGlesName(m_target), 0);
	debugLogApiError("DefaultFboGlesImpl::bind exit;");
}

FboImpl::FboImpl(GraphicsContext& context) :
	m_target(FboBindingTarget::ReadWrite), m_context(context) {}

FboImpl::FboImpl(const FboCreateParam& desc, GraphicsContext& context) :
	m_context(context)
{
	m_fbo.construct(native::HFbo::ElementType());
	init(desc);
}

void FboImpl::bind(IGraphicsContext& context, api::FboBindingTarget::Enum target) const
{
	m_target = target;
	gl::BindFramebuffer(fboBindTargetToGlesName(m_target), m_fbo->handle);
	debugLogApiError("FboAttachmentImpl::attachTo exit");
}

void FboImpl::destroy()
{
	if (m_context.isValid())
	{
		gl::DeleteFramebuffers(1, &m_fbo->handle);
		debugLogApiError("FboImpl::destroy exit");
	}
	else
	{
		Log(Log.Warning, "FBO object was not cleaned up before context destruction");
	}
}

Result::Enum FboImpl::init(const FboCreateParam& desc)
{
	// validate
	PVR_ASSERT(desc.renderPass.isValid() && "Invalid RenderPass");
	m_desc = desc;
	m_target = FboBindingTarget::ReadWrite;
	gl::GenFramebuffers(1, &m_fbo->handle);
	gl::BindFramebuffer(GL_FRAMEBUFFER, m_fbo->handle);
	debugLogApiError("FboAttachmentImpl::init bind fbo");
#if defined (GL_FRAMEBUFFER_DEFAULT_WIDTH) && defined(GL_FRAMEBUFFER_DEFAULT_HEIGHT)
	if (m_context->getApiType() >= pvr::Api::OpenGLES31)
	{
		gl::FramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, m_desc.width);
		gl::FramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, m_desc.height);
	}
#endif
	std::vector<GLenum> drawBuffers;
	if (desc.depthStencilView.isValid())
	{
		m_depthStencilAttachment.push_back(desc.depthStencilView);
		m_depthStencilAttachment.back()->attachTo();
	}

	const std::vector<pvr::api::RenderPassColorInfo>& colorInfo = desc.renderPass->getColorInfo();
	if (colorInfo.size() != desc.colorViews.size())
	{
                pvr::Log("Renderpass color info does not match with fbo create info");
                PVR_ASSERT(false && "Renderpass color info does not match with fbo create info");
		return pvr::Result::UnknownError;
	}

	for (uint32 i = 0; i < (uint32)desc.colorViews.size(); ++i)
	{
		if (desc.colorViews[i]->texture->getResource()->getFormat() != desc.renderPass->getColorInfo()[i].format)
		{
            pvr::Log("The renderPass color format does not match with color attachment view.");
            PVR_ASSERT(false && "The renderPass color format does not match with color attachment view.");
			return pvr::Result::UnknownError;
		}
		m_colorAttachments.push_back(desc.colorViews[i]);
		m_colorAttachments.back()->attachTo(FboAttachmentType::Color + i);
		drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + (GLenum)i);
	}

#if BUILD_API_MAX >= 30
	if (drawBuffers.size() > 1) {	gl::DrawBuffers((GLsizei)drawBuffers.size(), &drawBuffers[0]);	}
#endif
	debugLogApiError("FboAttachmentImpl::glDrawBuffers failed");
	bool fboStatus = checkFboStatus();
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
	debugLogApiError("FboAttachmentImpl::init bind 0");
	return fboStatus ? Result::Success : Result::UnknownError;
}

bool impl::FboImpl::checkFboStatus()
{
	// check status
	GLenum fboStatus = gl::CheckFramebufferStatus(fboBindTargetToGlesName(m_target));
	switch (fboStatus)
	{
#ifdef GL_FRAMEBUFFER_UNDEFINED
    case GL_FRAMEBUFFER_UNDEFINED:
        Log(Log.Error, "FboImpl::checkFboStatus GL_FRAMEBUFFER_UNDEFINED");
		PVR_ASSERT(0); break;
#endif
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        Log(Log.Error, "FboImpl::checkFboStatus GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		PVR_ASSERT(0); break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        Log(Log.Error,"FboImpl::checkFboStatus GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		PVR_ASSERT(0); break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        Log(Log.Error, "FboImpl::checkFboStatus GL_FRAMEBUFFER_UNSUPPORTED");
		PVR_ASSERT(0); break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		Log(Log.Error, "FboImpl::checkFboStatus GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
		PVR_ASSERT(0); break;
#endif
	case GL_FRAMEBUFFER_COMPLETE: return true;
	default: Log(Log.Error, "FboImpl::checkFboStatus UNKNOWN ERROR");
		PVR_ASSERT(0); break;
	}
	return false;
}

Result::Enum DepthStencilViewImpl::init(const pvr::api::DepthStencilViewCreateParam& createParam)
{
	texture = createParam.image;
	mipLevel = createParam.mipLevel;
	baseArraySlice = createParam.baseArraySlice;
	arraySize = createParam.arraySize;
	msaaResolveImage = createParam.msaaResolveImage;
	return Result::Success;
}

Result::Enum ColorAttachmentViewImpl::init(const pvr::api::ColorAttachmentViewCreateParam& createParam)
{
	texture = createParam.image;
	mipLevel = createParam.mipLevel;
	baseArraySlice = createParam.baseArraySlice;
	arraySize = createParam.arraySize;
	msaaResolveImage = createParam.msaaResolveImage;
	msaaResolveSubResRange = createParam.msaaResolveSubResRange;
	return Result::Success;
}
}// namespace impl
}// namespace api
}// namespace pvr
 //!\endcond