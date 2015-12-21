/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ContextGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of the ContextGles class. See ContextGles.h, IGraphicsContext.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/BufferGles.h"
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/TextureFormats.h"
#include "PVRApi/OGLES/TextureUtils.h"
#include "PVRApi/OGLES/FboGles.h"
#include "PVRApi/ApiObjects/RenderPass.h"
#include "PVRApi/OGLES/DescriptorTableGles.h"
#include <algorithm>

using std::string;
namespace pvr {
//Creates an instance of a graphics context.
GraphicsContextStrongReference createGraphicsContext()
{
	RefCountedResource<platform::ContextGles> ctx;
	ctx.construct();
//DEFAULT CONTEXT PER PLATFORM. CAN(WILL) BE OVERRIDEN BY PVRShell
#ifndef BUILD_MAX_API
	ctx->m_apiType = Api::OpenGLESMaxVersion;
#else
#if (BUILD_MAX_API <= 2.0)
	ctx->m_apiType = Api::OpenGLES2;
#elif (BUILD_MAX_API <= 3.0)
	ctx->m_apiType = Api::OpenGLES3;
#elif (BUILD_MAX_API <= 3.1)
	ctx->m_apiType = Api::OpenGLES31;
#endif
#endif
	return ctx;
}

namespace platform {

ContextGles::ContextGles(size_t implementationID) : m_ContextImplementationID(implementationID)
{ }

api::Fbo ContextGles::createOnScreenFboWithRenderPass(const api::RenderPass& renderPass)
{
	pvr::api::FboCreateParam fboInfo;
	fboInfo.setRenderPass(renderPass);
	pvr::api::DefaultFboGles fbo;
	fbo.construct(m_this_shared);
	fbo->init(fboInfo);
	fbo->bind(*this, api::FboBindingTarget::ReadWrite);
	return fbo;
}

api::Fbo ContextGles::createOnScreenFboWithParams(LoadOp::Enum colorLoadOp, StoreOp::Enum colorStoreOp,
    LoadOp::Enum depthLoadOp, StoreOp::Enum depthStoreOp,
    LoadOp::Enum stencilLoadOp, StoreOp::Enum stencilStoreOp,
    uint32 numColorSamples, uint32 numDepthStencilSamples)
{
	// create the default fbo
	api::RenderPassColorInfo colorInfo;
	api::RenderPassDepthStencilInfo dsInfo;
	api::getDisplayFormat(getDisplayAttributes(), &colorInfo.format, &dsInfo.format);
	colorInfo.loadOpColor = colorLoadOp;
	colorInfo.storeOpColor = colorStoreOp;
	colorInfo.numSamples = numColorSamples;

	dsInfo.loadOpDepth = depthLoadOp;
	dsInfo.storeOpDepth = depthStoreOp;
	dsInfo.loadOpStencil = stencilLoadOp;
	dsInfo.storeOpStencil = stencilStoreOp;
	dsInfo.numSamples = numDepthStencilSamples;

	pvr::api::RenderPassCreateParam renderPassDesc;
	renderPassDesc.addColorInfo(0, colorInfo);
	renderPassDesc.setDepthStencilInfo(dsInfo);

	// Require at least one sub pass
	pvr::api::SubPass subPass;
	subPass.setColorAttachment(0);// use color attachment 0
	renderPassDesc.addSubPass(0, subPass);

	return createOnScreenFboWithRenderPass(createRenderPass(renderPassDesc));
}

Result::Enum ContextGles::screenCaptureRegion(uint32 x, uint32 y, uint32 w, uint32 h, byte* pBuffer,
    ImageFormat requestedImageFormat)
{
	if (!pBuffer) {	return Result::InvalidArgument;}

	gl::ReadPixels(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLint>(w),
	               static_cast<GLint>(h), GL_RGBA,
	               GL_UNSIGNED_BYTE, pBuffer);

	GLenum err = gl::GetError();

	switch (err)
	{
	case GL_NO_ERROR:
	{
		switch (requestedImageFormat)
		{
		case IGraphicsContext::ImageFormatBGRA:
		{
			uint32 size = (w - x) * (h - y) * 4;

			// Switch the red and blue channels to convert to BGRA
			for (uint32 i = 0; i < size; i += 4)
			{
				byte tmp = pBuffer[i];
				pBuffer[i] = pBuffer[i + 2];
				pBuffer[i + 2] = tmp;
			}
		}
		break;
		default:
			// Do nothing
			break;
		}

		return Result::Success;
	}
	case GL_INVALID_VALUE:
		return Result::InvalidArgument;
	default:
		return Result::UnknownError;
	}
}

string ContextGles::getInfo()const
{
	string out, tmp;
	out.reserve(2048);
	tmp.reserve(1024);
	out.append("\nGL:\n");

	tmp = strings::createFormatted("\tVendor:   %hs\n", (const char*) gl::GetString(GL_VENDOR));
	out.append(tmp);

	tmp = strings::createFormatted("\tRenderer: %hs\n", (const char*) gl::GetString(GL_RENDERER));
	out.append(tmp);

	tmp = strings::createFormatted("\tVersion:  %hs\n", (const char*) gl::GetString(GL_VERSION));
	out.append(tmp);

	tmp = strings::createFormatted("\tExtensions:  %hs\n",
	                               (const char*) gl::GetString(GL_EXTENSIONS));
	out.append(tmp);

	return "";
}

bool ContextGles::isExtensionSupported(const char8* extension) const
{
	if (m_extensions.empty())
	{
		const char* extensions = (const char*)gl::GetString(GL_EXTENSIONS);
		if (extensions)
		{
			m_extensions.assign(extensions);
		}
		else
		{
			m_extensions.assign("");
		}
	}
	return m_extensions.find(extension) != m_extensions.npos;
}

api::DescriptorSetLayout ContextGles::createDescriptorSetLayout(const api::DescriptorSetLayoutCreateParam& desc)
{
	api::DescriptorSetLayoutGles layout;
	layout.construct(m_this_shared, desc);
	return layout;
}

api::DescriptorSet ContextGles::allocateDescriptorSet(const api::DescriptorSetLayout& layout,
    const api::DescriptorPool& pool)
{
	api::DescriptorSetGles set;
	set.construct(layout, pool);
	if (set->init() != Result::Success) { set.release(); }
	return set;
}

}

api::Buffer IGraphicsContext::createBuffer(uint32 size, api::BufferBindingUse::Bits bufferUsage,
    api::BufferUse::Flags hint)
{
	api::BufferGles buffer;
	buffer.construct(m_this_shared, size, bufferUsage, hint);
	return buffer;
}


}
//!\endcond