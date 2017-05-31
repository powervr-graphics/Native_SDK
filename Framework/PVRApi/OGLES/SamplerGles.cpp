/*!
\brief OpenGL ES 2+ implementation of the Sampler class.
\file PVRApi/OGLES/SamplerGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/SamplerGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"
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
<<<<<<< HEAD

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
=======
>>>>>>> 1776432f... 4.3

const char* samplerFilterToStr(types::SamplerFilter filter)
{
	const char* str[] = {"Nearest",
	                     "Linear",
	                     "None",
	                     "Cubic"
	                    };
	return str[(uint32)filter];
}

}



namespace gles {
void SamplerGles_::destroy()
{
	if (_initialized && _context.isValid() && _context->hasApiCapability(ApiCapabilities::Sampler))
	{
		gl::DeleteSamplers(1, &handle);
	}
}

<<<<<<< HEAD
GLenum getMinificationFilter(IGraphicsContext& context, uint32 texMipLevelCount, const assets::SamplerCreateParam& samplerDesc)
{
	pvr::platform::ContextGles& contextEs = static_cast<pvr::platform::ContextGles&>(context);
=======
GLenum getMinificationFilter(IGraphicsContext& context, uint32 texMipLevelCount, const SamplerCreateParam& samplerDesc)
{
	platform::ContextGles& contextEs = native_cast(context);
>>>>>>> 1776432f... 4.3

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

<<<<<<< HEAD
GLenum getMagnificationFilter(IGraphicsContext& context, const assets::SamplerCreateParam& samplerDesc)
{
	pvr::platform::ContextGles& contextEs = static_cast<pvr::platform::ContextGles&>(context);
=======
GLenum getMagnificationFilter(IGraphicsContext& context, const SamplerCreateParam& samplerDesc)
{
	platform::ContextGles& contextEs = native_cast(context);
>>>>>>> 1776432f... 4.3

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
	platform::ContextGles& contextEs = native_cast(context);
	if (contextEs.hasApiCapability(ApiCapabilities::Sampler)) //API supports separate sampler objects
	{
		if (contextEs.getCurrentRenderStates().texSamplerBindings[index].lastBoundSampler == this) { return; }
		gl::BindSampler(index, handle); contextEs.onBind(*this, (uint16)index);
		debugLogApiError("Sampler_::bind exit");
	}
	else  //API has fused textures with sampler objects
	{
<<<<<<< HEAD
		pvr::platform::ContextGles::RenderStatesTracker& renderStates =  contextEs.getCurrentRenderStates();
		const impl::TextureView_* textureToBind = renderStates.texSamplerBindings[renderStates.lastBoundTexBindIndex].toBindTex;
		const GLenum texType = ConvertToGles::textureViewType(textureToBind->getViewType());
=======
		platform::ContextGles::RenderStatesTracker& renderStates =  contextEs.getCurrentRenderStates();
		const TextureStoreGles_* textureToBind = renderStates.texSamplerBindings[renderStates.lastBoundTexBindIndex].lastBoundTex;
		const GLenum texType = textureToBind->target;
>>>>>>> 1776432f... 4.3

		if (static_cast<const TextureStoreGles_&>(*textureToBind)._sampler == this) { return; }
		static_cast<const TextureStoreGles_&>(*textureToBind)._sampler = this;
		debugLogApiError("Begin Sampler_::bind\n");

#ifdef  GL_TEXTURE_EXTERNAL_OES
		if (texType != GL_TEXTURE_EXTERNAL_OES && texType != GL_NONE)
#else
		if (texType != GL_NONE)
#endif
		{
<<<<<<< HEAD
			GLenum minFilter = (GLenum)getMinificationFilter(context, textureToBind->getResource()->getNumMipLevels(), m_desc);
			GLenum magFilter = (GLenum)getMagnificationFilter(context, m_desc);
=======
			GLenum minFilter = (GLenum)getMinificationFilter(context, textureToBind->getNumMipLevels(), _desc);
			GLenum magFilter = (GLenum)getMagnificationFilter(context, _desc);
>>>>>>> 1776432f... 4.3

			if (renderStates.lastBoundTexBindIndex != index)
			{
				gl::ActiveTexture(GL_TEXTURE0 + index);
<<<<<<< HEAD
=======
				renderStates.lastBoundTexBindIndex = index;
>>>>>>> 1776432f... 4.3
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
<<<<<<< HEAD
				if (m_desc.compareMode == types::ComparisonMode::None)
=======
				if (_desc.compareMode == types::ComparisonMode::None)
>>>>>>> 1776432f... 4.3
				{
					gl::TexParameteri(texType, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE);
					debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
				}
				else
				{
					gl::TexParameteri(texType, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT);
					debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
<<<<<<< HEAD
					gl::TexParameteri(texType, GL_TEXTURE_COMPARE_FUNC_EXT, glCmpFunc[(uint32)m_desc.compareMode]);
=======
					gl::TexParameteri(texType, GL_TEXTURE_COMPARE_FUNC_EXT, glCmpFunc[(uint32)_desc.compareMode]);
>>>>>>> 1776432f... 4.3
					debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
				}
			}

<<<<<<< HEAD
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
=======
			gl::TexParameteri(texType, GL_TEXTURE_WRAP_S, nativeGles::ConvertToGles::samplerWrap(_desc.wrapModeU));
			debugLogApiError("calling glTexParameteri in SampelrImpl::bind\n");
			gl::TexParameteri(texType, GL_TEXTURE_WRAP_T, nativeGles::ConvertToGles::samplerWrap(_desc.wrapModeV));
			debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
#ifdef GL_TEXTURE_WRAP_R_OES
			if (context.hasApiCapability(ApiCapabilities::Texture3D) && textureToBind->target == GL_TEXTURE_3D)
			{
				gl::TexParameteri(texType, GL_TEXTURE_WRAP_R_OES, nativeGles::ConvertToGles::samplerWrap(_desc.wrapModeW));
				debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
			}
#endif
			/*gl::TexParameteri(gSamplerType[_desc.samplerType], GL_TEXTURE_MIN_LOD, _desc.lodMinimum);
			gl::TexParameteri(gSamplerType[_desc.samplerType], GL_TEXTURE_MAX_LOD, _desc.lodMaximum);*/
			if (context.hasApiCapability(ApiCapabilities::AnisotropicFiltering) && _desc.anisotropyMaximum)
			{
				gl::TexParameterf(texType, GL_TEXTURE_MAX_ANISOTROPY_EXT, _desc.anisotropyMaximum);
>>>>>>> 1776432f... 4.3
				debugLogApiError("calling glTexParameteri in Sampler_::bind\n");
			}
		}
		debugLogApiError("End Sampler_::bind\n");
	}
}

bool SamplerGles_::init(const SamplerCreateParam& samplerDesc)
{
	//If samplers are not supported, no need to do anything - we will be using textures for it...
	if (!_context->hasApiCapability(ApiCapabilities::Sampler)) { return true; }
	if (_initialized) { return true; }

<<<<<<< HEAD
	GLenum minFilter = getMinificationFilter(*m_context, uint32(-1), samplerDesc);
	GLenum magFilter = getMagnificationFilter(*m_context, samplerDesc);
=======
	GLenum minFilter = getMinificationFilter(*_context, uint32(-1), samplerDesc);
	GLenum magFilter = getMagnificationFilter(*_context, samplerDesc);
>>>>>>> 1776432f... 4.3

#if BUILD_API_MAX>=30
	gl::GenSamplers(1, &handle);

	gl::SamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, glFilter[minFilter]);
	debugLogApiError("Sampler_::init SetMinFilter");

<<<<<<< HEAD
	gl::SamplerParameteri(getNativeObject().handle, GL_TEXTURE_MAG_FILTER, glFilter[magFilter]);
=======
	gl::SamplerParameteri(handle, GL_TEXTURE_MAG_FILTER, glFilter[magFilter]);
>>>>>>> 1776432f... 4.3
	debugLogApiError("Sampler_::init SetMagFilter");

	if (samplerDesc.compareMode == types::ComparisonMode::None)
	{
		gl::SamplerParameteri(handle, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
	else
	{
		gl::SamplerParameteri(handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		gl::SamplerParameteri(handle, GL_TEXTURE_COMPARE_FUNC, nativeGles::ConvertToGles::comparisonMode(samplerDesc.compareMode));
	}
	debugLogApiError("Sampler_::init TextureCompareMode");

	gl::SamplerParameteri(handle, GL_TEXTURE_WRAP_S, nativeGles::ConvertToGles::samplerWrap(samplerDesc.wrapModeU));
	debugLogApiError("Sampler_::init WrapS");
	gl::SamplerParameteri(handle, GL_TEXTURE_WRAP_T, nativeGles::ConvertToGles::samplerWrap(samplerDesc.wrapModeV));
	debugLogApiError("Sampler_::init WrapT");
	gl::SamplerParameteri(handle, GL_TEXTURE_WRAP_R, nativeGles::ConvertToGles::samplerWrap(samplerDesc.wrapModeW));
	debugLogApiError("Sampler_::init WrapR");
	gl::SamplerParameteri(handle, GL_TEXTURE_MIN_LOD, static_cast<GLint>(samplerDesc.lodMinimum));
	debugLogApiError("Sampler_::init MinLod");
	gl::SamplerParameteri(handle, GL_TEXTURE_MAX_LOD, static_cast<GLint>(samplerDesc.lodMaximum));
	debugLogApiError("Sampler_::init MaxLod");
	if (_context->hasApiCapability(ApiCapabilities::AnisotropicFiltering) && samplerDesc.anisotropyMaximum)
	{
		gl::SamplerParameterf(handle, GL_TEXTURE_MAX_ANISOTROPY_EXT, samplerDesc.anisotropyMaximum);
		debugLogApiError("Sampler_::init Anisotropy");
	}
#endif
	_initialized = true;
	return true;
}
}
}// namespace api
}// namespace pvr
