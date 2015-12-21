/*!*********************************************************************************************************************
\file         PVRApi\OGLES\Sampler.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES 2+ implementation of the Sampler class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/OGLES/ConvertToApiTypes.h"
#include "PVRApi/ApiErrors.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ContextGles.h"

namespace pvr {
namespace api {
static const GLenum glFilter[] = { GL_NEAREST,					GL_LINEAR,
									GL_NEAREST_MIPMAP_NEAREST,	GL_LINEAR_MIPMAP_NEAREST, 
									GL_NEAREST_MIPMAP_LINEAR,	GL_LINEAR_MIPMAP_LINEAR
                                 };
static GLenum glCmpFunc[] = { GL_NONE, GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_ALWAYS };

namespace impl {
void SamplerImpl::destroy()
{
	if (m_initialised && m_context.isValid() && m_context->hasApiCapability(ApiCapabilities::Sampler))
	{
		gl::DeleteSamplers(1, &m_sampler->handle);
	}
}
void SamplerImpl::init(const assets::SamplerCreateParam& samplerDesc) const
{
	//If samplers are not supported, no need to do anything - we will be using textures for it...
	if (!m_context->hasApiCapability(ApiCapabilities::Sampler)) { return; }


	if (m_initialised) { return; }
	using namespace assets;

	uint8 minFilter = 0;
	if (samplerDesc.mipMappingFilter != SamplerFilter::None)
	{
		minFilter = (samplerDesc.mipMappingFilter == SamplerFilter::Nearest ? 2 /*nearset*/ : 4/*linear*/);
	}
	if (samplerDesc.minificationFilter == SamplerFilter::Linear) { minFilter += 1; }
	if (!m_sampler.isValid()) { m_sampler.construct(0); }
#if BUILD_API_MAX>=30
	gl::GenSamplers(1, &m_sampler->handle);

	gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_MIN_FILTER, glFilter[minFilter]);
	debugLogApiError("SamplerImpl::init SetMinFilter");

	gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_MAG_FILTER, glFilter[samplerDesc.magnificationFilter]);
	debugLogApiError("SamplerImpl::init SetMagFilter");

	if (samplerDesc.compareMode == ComparisonMode::None)
	{
		gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
	else
	{
		gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_COMPARE_FUNC, api::ConvertToGles::comparisonMode(samplerDesc.compareMode));
	}
	debugLogApiError("SamplerImpl::init TextureCompareMode");

	gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_WRAP_S, api::ConvertToGles::samplerWrap(samplerDesc.wrapModeU));
	debugLogApiError("SamplerImpl::init WrapS");
	gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_WRAP_T, api::ConvertToGles::samplerWrap(samplerDesc.wrapModeV));
	debugLogApiError("SamplerImpl::init WrapT");
	gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_WRAP_R, api::ConvertToGles::samplerWrap(samplerDesc.wrapModeW));
	debugLogApiError("SamplerImpl::init WrapR");
	gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_MIN_LOD, static_cast<GLint>(samplerDesc.lodMinimum));
	debugLogApiError("SamplerImpl::init MinLod");
	gl::SamplerParameteri(m_sampler->handle, GL_TEXTURE_MAX_LOD, static_cast<GLint>(samplerDesc.lodMaximum));
	debugLogApiError("SamplerImpl::init MaxLod");
	if (m_context->hasApiCapability(ApiCapabilities::AnisotropicFiltering) && samplerDesc.anisotropyMaximum)
	{
		gl::SamplerParameterf(m_sampler->handle, GL_TEXTURE_MAX_ANISOTROPY_EXT, samplerDesc.anisotropyMaximum);
		debugLogApiError("SamplerImpl::init Anisotropy");
	}
#endif
	m_initialised = true;
}

void SamplerImpl::bind(IGraphicsContext& context, uint32 index) const
{
	pvr::platform::ContextGles& contextEs = static_cast<pvr::platform::ContextGles&>(context);
	if (contextEs.hasApiCapability(ApiCapabilities::Sampler)) //API supports separate sampler objects
	{
		if (!m_initialised)
		{
			// safe to initialize only once even binding to different context, since we only care about the extensions
			init(m_desc);
			m_initialised = true;
		}
		if (contextEs.getCurrentRenderStates().texSamplerBindings[index].lastBoundSampler == this){ return; }
		gl::BindSampler(index, m_sampler->handle);
		contextEs.onBind(*this, (uint16)index);
		debugLogApiError("SamplerImpl::bind exit");
	}
	else  //API has fused textures with sampler objects
	{
		
		platform::ContextGles::RenderStatesTracker& renderStates =  contextEs.getCurrentRenderStates();
		const TextureViewImpl* textureToBind = renderStates.texSamplerBindings[renderStates.lastBoundTexBindIndex].toBindTex;
		const GLenum texType = ConvertToGles::textureDimension(textureToBind->getTextureType());
		if (textureToBind->m_sampler == this){ return; }
		textureToBind->m_sampler = this;
		debugLogApiError("Begin SamplerImpl::bind\n");
		uint8 minFilter = 0;
		if (m_desc.mipMappingFilter != SamplerFilter::None && textureToBind->getResource()->getFormat().mipmapLevels > 1)
		{
			minFilter = (m_desc.mipMappingFilter == SamplerFilter::Nearest ? 2 /*nearset*/ : 4/*linear*/);
		}
		if (m_desc.minificationFilter == SamplerFilter::Linear) { minFilter += 1; }
		
		if (renderStates.lastBoundTexBindIndex != index) {
			gl::ActiveTexture(GL_TEXTURE0 + index);
		}
		debugLogApiError("calling glActiveTexture in SampelrImpl::bind\n");
		gl::TexParameteri(texType, GL_TEXTURE_MIN_FILTER, glFilter[minFilter]);
		debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
		gl::TexParameteri(texType, GL_TEXTURE_MAG_FILTER, glFilter[m_desc.magnificationFilter]);
		debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
		if (context.hasApiCapability(ApiCapabilities::ShadowSamplers))
		{
			if (m_desc.compareMode == ComparisonMode::None)
			{
				gl::TexParameteri(texType, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE);
				debugLogApiError("calling glTexParameteri in SamplerImpl::bind\n");
			}
			else
			{
				gl::TexParameteri(texType, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT);
				debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
				gl::TexParameteri(texType, GL_TEXTURE_COMPARE_FUNC_EXT, glCmpFunc[m_desc.compareMode]);
				debugLogApiError("calling glTexParameteri in SamplerImpl::bind\n");
			}
		}

		gl::TexParameteri(texType, GL_TEXTURE_WRAP_S, ConvertToGles::samplerWrap(m_desc.wrapModeU));
		debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
		gl::TexParameteri(texType, GL_TEXTURE_WRAP_T, ConvertToGles::samplerWrap(m_desc.wrapModeV));
		debugLogApiError("calling glTexParameteri in SamplerImpl::bind\n");
#ifdef GL_TEXTURE_WRAP_R_OES
        if (context.hasApiCapability(ApiCapabilities::Texture3D) && textureToBind->getTextureType() == TextureDimension::Texture3D)
		{
			gl::TexParameteri(texType, GL_TEXTURE_WRAP_R_OES, ConvertToGles::samplerWrap(m_desc.wrapModeW));
			debugLogApiError("calling glTexParameteri in SamplerImpl::bind\n");
		}
#endif
		/*gl::TexParameteri(gSamplerType[m_desc.samplerType], GL_TEXTURE_MIN_LOD, m_desc.lodMinimum);
		gl::TexParameteri(gSamplerType[m_desc.samplerType], GL_TEXTURE_MAX_LOD, m_desc.lodMaximum);*/
		if (context.hasApiCapability(ApiCapabilities::AnisotropicFiltering) && m_desc.anisotropyMaximum)
		{
			gl::TexParameterf(texType, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_desc.anisotropyMaximum);
			debugLogApiError("calling glTexParameteri in SamplerImpl::bind\n");
		}
		debugLogApiError("End SamplerImpl::bind\n");
	}
}
}

}// namespace api
}// namespace pvr
 //!\endcond