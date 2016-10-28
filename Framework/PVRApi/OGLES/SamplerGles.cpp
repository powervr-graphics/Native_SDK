/*!*********************************************************************************************************************
\file         PVRApi\OGLES\SamplerGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES 2+ implementation of the Sampler class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/SamplerGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/TextureGles.h"
namespace pvr {
namespace api {
#if defined TARGET_OS_IPHONE
static const GLenum glFilter[] = { GL_NEAREST,          GL_LINEAR,          GL_NONE,
                                   GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST,   GL_NONE,
                                   GL_NEAREST_MIPMAP_LINEAR,  GL_LINEAR_MIPMAP_LINEAR,    GL_NONE
                                 };
#else
static const GLenum glFilter[] = { GL_NEAREST,          GL_LINEAR,          GL_CUBIC_IMG,
                                   GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST,   GL_CUBIC_MIPMAP_NEAREST_IMG,
                                   GL_NEAREST_MIPMAP_LINEAR,  GL_LINEAR_MIPMAP_LINEAR,    GL_CUBIC_MIPMAP_LINEAR_IMG
                                 };
#endif

static GLenum glCmpFunc[] = { GL_NONE, GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_ALWAYS };

namespace {

const char* samplerFilterToStr(pvr::types::SamplerFilter filter)
{
	const char* str[] = {"Nearest",
	                     "Linear",
	                     "None",
	                     "Cubic"
	                    };
	return str[(uint32)filter];
}

}




namespace impl {
const native::HSampler_& Sampler_::getNativeObject()const
{
	return static_cast<const gles::SamplerGles_&>(*this);
}

native::HSampler_& Sampler_::getNativeObject()
{
	return static_cast<gles::SamplerGles_&>(*this);
}
}

namespace gles {
void SamplerGles_::destroy()
{
	if (m_initialized && m_context.isValid() && m_context->hasApiCapability(ApiCapabilities::Sampler))
	{
		gl::DeleteSamplers(1, &getNativeObject().handle);
	}
}

GLenum getMinificationFilter(IGraphicsContext& context, uint32 texMipLevelCount, const assets::SamplerCreateParam& samplerDesc)
{
	pvr::platform::ContextGles& contextEs = static_cast<pvr::platform::ContextGles&>(context);

	uint8 minFilter = 0;
	if (samplerDesc.mipMappingFilter != types::SamplerFilter::None &&
	    texMipLevelCount > 1)
	{
		minFilter = (samplerDesc.mipMappingFilter == types::SamplerFilter::Nearest ? 3 /*nearset*/ : 6/*linear*/);

		if (contextEs.hasApiCapability(ApiCapabilities::BicubicFiltering))
		{
			if (samplerDesc.minificationFilter == types::SamplerFilter::Cubic)
			{
				minFilter += 2;
			}
		}
	}
	else if (contextEs.hasApiCapability(ApiCapabilities::BicubicFiltering) && samplerDesc.minificationFilter == types::SamplerFilter::Cubic)
	{
		minFilter = 2;
	}

	if (samplerDesc.minificationFilter == types::SamplerFilter::Linear) { minFilter += 1; }

	return minFilter;
}

GLenum getMagnificationFilter(IGraphicsContext& context, const assets::SamplerCreateParam& samplerDesc)
{
	pvr::platform::ContextGles& contextEs = static_cast<pvr::platform::ContextGles&>(context);

	uint8 magFilter = 0;

	magFilter = (samplerDesc.magnificationFilter == types::SamplerFilter::Nearest ? 0 /*nearset*/ : 1/*linear*/);

	if (contextEs.hasApiCapability(ApiCapabilities::BicubicFiltering))
	{
		if (samplerDesc.magnificationFilter == types::SamplerFilter::Cubic)
		{
			magFilter = 2;
		}
	}

	return magFilter;
}

void SamplerGles_::bind(IGraphicsContext& context, uint32 index) const
{
	pvr::platform::ContextGles& contextEs = static_cast<pvr::platform::ContextGles&>(context);
	if (contextEs.hasApiCapability(ApiCapabilities::Sampler)) //API supports separate sampler objects
	{
		if (contextEs.getCurrentRenderStates().texSamplerBindings[index].lastBoundSampler == this) { return; }
		gl::BindSampler(index, getNativeObject().handle); contextEs.onBind(*this, (uint16)index);
		debugLogApiError("Sampler_::bind exit");
	}
	else  //API has fused textures with sampler objects
	{
		pvr::platform::ContextGles::RenderStatesTracker& renderStates =  contextEs.getCurrentRenderStates();
		const impl::TextureView_* textureToBind = renderStates.texSamplerBindings[renderStates.lastBoundTexBindIndex].toBindTex;
		const GLenum texType = ConvertToGles::textureViewType(textureToBind->getViewType());

		if (static_cast<const TextureStoreGles_&>(*textureToBind->getResource()).m_sampler == this) { return; }
		static_cast<const TextureStoreGles_&>(*textureToBind->getResource()).m_sampler = this;
		debugLogApiError("Begin Sampler_::bind\n");

#ifdef  GL_TEXTURE_EXTERNAL_OES
		if (texType != GL_TEXTURE_EXTERNAL_OES && texType != GL_NONE)
#else
		if (texType != GL_NONE)
#endif
		{
			GLenum minFilter = (GLenum)getMinificationFilter(context, textureToBind->getResource()->getNumMipLevels(), m_desc);
			GLenum magFilter = (GLenum)getMagnificationFilter(context, m_desc);

			if (renderStates.lastBoundTexBindIndex != index)
			{
				gl::ActiveTexture(GL_TEXTURE0 + index);
			}
			debugLogApiError("calling glActiveTexture in SampelrImpl::bind\n");
			if (glFilter[minFilter] == GL_NONE)
			{
				Log("Minification filter is not supported");
			}
			if (glFilter[magFilter] == GL_NONE)
			{
				Log("Magnification filter is not supported");
			}
			gl::TexParameteri(texType, GL_TEXTURE_MIN_FILTER, glFilter[minFilter]);
			debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
			gl::TexParameteri(texType, GL_TEXTURE_MAG_FILTER, glFilter[magFilter]);
			debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
			if (context.hasApiCapability(ApiCapabilities::ShadowSamplers))
			{
				if (m_desc.compareMode == types::ComparisonMode::None)
				{
					gl::TexParameteri(texType, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE);
					debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
				}
				else
				{
					gl::TexParameteri(texType, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT);
					debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
					gl::TexParameteri(texType, GL_TEXTURE_COMPARE_FUNC_EXT, glCmpFunc[(uint32)m_desc.compareMode]);
					debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
				}
			}

			gl::TexParameteri(texType, GL_TEXTURE_WRAP_S, ConvertToGles::samplerWrap(m_desc.wrapModeU));
			debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
			gl::TexParameteri(texType, GL_TEXTURE_WRAP_T, ConvertToGles::samplerWrap(m_desc.wrapModeV));
			debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
#ifdef GL_TEXTURE_WRAP_R_OES
			if (context.hasApiCapability(ApiCapabilities::Texture3D) && textureToBind->getViewType() == types::ImageViewType::ImageView3D)
			{
				gl::TexParameteri(texType, GL_TEXTURE_WRAP_R_OES, ConvertToGles::samplerWrap(m_desc.wrapModeW));
				debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
			}
#endif
			/*gl::TexParameteri(gSamplerType[m_desc.samplerType], GL_TEXTURE_MIN_LOD, m_desc.lodMinimum);
			gl::TexParameteri(gSamplerType[m_desc.samplerType], GL_TEXTURE_MAX_LOD, m_desc.lodMaximum);*/
			if (context.hasApiCapability(ApiCapabilities::AnisotropicFiltering) && m_desc.anisotropyMaximum)
			{
				gl::TexParameterf(texType, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_desc.anisotropyMaximum);
				debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
			}
		}
		debugLogApiError("End Sampler_::bind\n");
	}
}

bool SamplerGles_::init(const assets::SamplerCreateParam& samplerDesc)
{
	//If samplers are not supported, no need to do anything - we will be using textures for it...
	if (!m_context->hasApiCapability(ApiCapabilities::Sampler)) { return true; }
	if (m_initialized) { return true; }
	using namespace assets;

	GLenum minFilter = getMinificationFilter(*m_context, uint32(-1), samplerDesc);
	GLenum magFilter = getMagnificationFilter(*m_context, samplerDesc);

#if BUILD_API_MAX>=30
	gl::GenSamplers(1, &getNativeObject().handle);

	gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_MIN_FILTER, glFilter[minFilter]);
	debugLogApiError("Sampler_::init SetMinFilter");

	gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_MAG_FILTER, glFilter[magFilter]);
	debugLogApiError("Sampler_::init SetMagFilter");

	if (samplerDesc.compareMode == types::ComparisonMode::None)
	{
		gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
	else
	{
		gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_COMPARE_FUNC, api::ConvertToGles::comparisonMode(samplerDesc.compareMode));
	}
	debugLogApiError("Sampler_::init TextureCompareMode");

	gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_WRAP_S, api::ConvertToGles::samplerWrap(samplerDesc.wrapModeU));
	debugLogApiError("Sampler_::init WrapS");
	gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_WRAP_T, api::ConvertToGles::samplerWrap(samplerDesc.wrapModeV));
	debugLogApiError("Sampler_::init WrapT");
	gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_WRAP_R, api::ConvertToGles::samplerWrap(samplerDesc.wrapModeW));
	debugLogApiError("Sampler_::init WrapR");
	gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_MIN_LOD, static_cast<GLint>(samplerDesc.lodMinimum));
	debugLogApiError("Sampler_::init MinLod");
	gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_MAX_LOD, static_cast<GLint>(samplerDesc.lodMaximum));
	debugLogApiError("Sampler_::init MaxLod");
	if (m_context->hasApiCapability(ApiCapabilities::AnisotropicFiltering) && samplerDesc.anisotropyMaximum)
	{
		gl::SamplerParameterf(getNativeObject().handle, GL_TEXTURE_MAX_ANISOTROPY_EXT, samplerDesc.anisotropyMaximum);
		debugLogApiError("Sampler_::init Anisotropy");
	}
#endif
	m_initialized = true;
	return true;
}
}
}// namespace api
}// namespace pvr
//!\endcond
