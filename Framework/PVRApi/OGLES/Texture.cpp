/*!*********************************************************************************************************************
\file         PVRApi\OGLES\Texture.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains definitions for the OpenGL ES texture implementation methods.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRApi/OGLES/TextureUtils.h"
#include "PVRApi/ApiErrors.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ContextGles.h"

namespace pvr {
namespace api {
namespace impl {

TextureStoreImpl::TextureStoreImpl(GraphicsContext& context, const native::HTexture_& texture)
	: context(context)
{
	m_texture.construct(texture);
}


TextureStoreImpl::TextureStoreImpl(GraphicsContext& context) : context(context)
{
	m_texture.construct();
}

TextureStoreImpl::~TextureStoreImpl()
{
	if (isAllocated())
	{
		if (context.isValid())
		{
			gl::DeleteTextures(1, &m_texture->handle);
			debugLogApiError("TextureGlesImpl::~TextureGlesImpl exit");
		}
		else
		{
			Log(Log.Warning, "Texture object was not released up before context destruction");
		}
	}
}

inline void TextureStoreImpl::setSwizzle(SwizzleMask::Enum red, SwizzleMask::Enum green, SwizzleMask::Enum blue,
    SwizzleMask::Enum alpha)
{
#if BUILD_API_MAX>=30
	static const GLenum toSwizzleMask[] = { GL_NONE, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_ZERO, GL_ONE };
	static const GLenum toTexGetBinding[] = { GL_NONE, GL_NONE, GL_TEXTURE_BINDING_2D, GL_TEXTURE_BINDING_3D, GL_TEXTURE_BINDING_CUBE_MAP, GL_NONE, GL_TEXTURE_BINDING_2D_ARRAY, GL_NONE,
#ifndef TARGET_OS_IPHONE
	                                          GL_TEXTURE_BINDING_EXTERNAL_OES
#else
	                                          GL_NONE
#endif
	                                        };
	if (!context->hasApiCapability(ApiCapabilities::TextureSwizzling))
	{
		Log(Log.Error, "Attempted to set Texture Swizzling, but swizzling is not supported by the actual API level");
		return;
	}
	if ((red | green | blue | alpha) != SwizzleMask::Unset)
	{
		GLint prevTex;
		gl::GetIntegerv(toTexGetBinding[getDimensions()], &prevTex);
		debugLogApiError("TextureStoreImpl::GetIntegerv");
		if (prevTex != (GLint)m_texture->handle) { gl::BindTexture(m_texture->target, m_texture->handle); }

		if (red != SwizzleMask::Unset)
		{
			gl::TexParameteri(m_texture->target, GL_TEXTURE_SWIZZLE_R, toSwizzleMask[red]);
			debugLogApiError("TextureStoreImpl::TexParameteri");
		}
		if (green != SwizzleMask::Unset)
		{
			gl::TexParameteri(m_texture->target, GL_TEXTURE_SWIZZLE_G, toSwizzleMask[green]);
			debugLogApiError("TextureStoreImpl::TexParameteri");
		}
		if (green != SwizzleMask::Unset)
		{
			gl::TexParameteri(m_texture->target, GL_TEXTURE_SWIZZLE_B, toSwizzleMask[blue]);
			debugLogApiError("TextureStoreImpl::TexParameteri");
		}
		if (green != SwizzleMask::Unset)
		{
			gl::TexParameteri(m_texture->target, GL_TEXTURE_SWIZZLE_A, toSwizzleMask[alpha]);
			debugLogApiError("TextureStoreImpl::TexParameteri");
		}
		if (prevTex != (GLint)m_texture->handle) { gl::BindTexture(m_texture->target, (GLuint)prevTex); }
		debugLogApiError("TextureStoreImpl::BindTexture");
	}
	debugLogApiError("TextureStoreImpl::setSwizzle exit");
#endif
}

TextureViewImpl::TextureViewImpl(GraphicsContext& context, const native::HTexture_& texture)
	:	m_sampler(0)
{
	TextureStore tex;
	tex.construct(context, texture);
	resource = tex;
}

TextureViewImpl::TextureViewImpl(GraphicsContext& context) : m_sampler(0)
{
	TextureStore tex;
	tex.construct(context);
	resource = tex;
}

void TextureViewImpl::bind(IGraphicsContext& context, uint16 bindIdx)const
{
	platform::ContextGles& contextEs = static_cast<platform::ContextGles&>(context);

	if (resource.isNull())
	{
		Log("TextureViewImpl::bind attempted to bind a texture with NULL native texture handle");
		return;
	}
	platform::ContextGles::RenderStatesTracker& renderStates = contextEs.getCurrentRenderStates();

	if (renderStates.texSamplerBindings[bindIdx].lastBoundTex == this){ return; }
	if (renderStates.lastBoundTexBindIndex != bindIdx){
		gl::ActiveTexture(GL_TEXTURE0 + bindIdx);
	}
	
	gl::BindTexture(native::useNativeHandleTarget(resource), native::useNativeHandle(resource));
	debugLogApiError(strings::createFormatted("TextureViewImpl::bind TARGET%x HANDLE%x", 
		native::useNativeHandleTarget(resource), native::useNativeHandle(resource)).c_str());
	contextEs.onBind(*this, bindIdx);
}

void TextureViewImpl::allocate2D(const ImageStorageFormat& format, uint32 width, uint32 height)
{
	if (!isAllocated())
	{
		GLenum target = GL_TEXTURE_2D;
		resource->format = format;
		gl::GenTextures(1, &native::useNativeHandle(resource));
		native::useNativeHandleTarget(resource) = target;
		GLenum internalFormat, imageFormat, dataType;
		uint32 typeSize; bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat,
		                       imageFormat, dataType, typeSize, isCompressed);

		gl::BindTexture(target, native::useNativeHandle(resource));
		debugLogApiError("Texture2DImpl::allocate bind");
#if BUILD_API_MAX>=30
		if (resource->context->hasApiCapability(ApiCapabilities::TexureStorage))
		{
			gl::TexStorage2D(target, format.mipmapLevels, internalFormat, width, height);
			debugLogApiError("Texture2DImpl::allocate texStorage");
		}
		else
#endif
		{
            if(getResource()->context->getApiType() == Api::OpenGLES2)
            {
                internalFormat = imageFormat;
            }
			for (int8 i = 0; i < format.mipmapLevels; i++)
			{
				gl::TexImage2D(target, i, internalFormat, width, height, 0, imageFormat,
				               dataType, NULL);
				int tmpw = (std::max)(1u, width / 2);
				int tmph = (std::max)(1u, width / 2);
				if (tmpw == (int)width || tmph == (int)height)
				{
					break; //Wooops
				}
			}
		}
	}
	else
	{
		Log(Log.Warning, "TextureViewImpl::allocate3D: Attempted double allocation. No effect in allocate call.");
	}
	debugLogApiError("Texture2DImpl::allocate exit");
}
void TextureViewImpl::allocate3D(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth)
{
#if BUILD_API_MAX<30
	Log(Log.Error, "Called Texture3DImpl::allocate, but Texture3D support was not present");
	PVR_ASSERT(0 && "TextureCube not supported on OpenGL ES 2");
#else
	if (!resource->context->hasApiCapability(ApiCapabilities::Texture3D))
	{
		Log(Log.Error, "Called TextureViewImpl::allocate3D, but Texture3D is not supported");
		PVR_ASSERT(0 && "Attempt to allocate unsupported Texture3D");
		return;
	}
	if (!isAllocated())
	{
		GLenum target = GL_TEXTURE_3D;
		resource->format = format;
		gl::GenTextures(1, &native::useNativeHandle(resource));
		native::useNativeHandleTarget(resource) = target;
		GLenum internalFormat, imageFormat, dataType;
		uint32 typeSize;
		bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat,
		                       imageFormat, dataType, typeSize, isCompressed);
		gl::BindTexture(target, native::useNativeHandle(resource));
		gl::TexStorage3D(target, format.mipmapLevels, internalFormat, width, height, depth);
		debugLogApiError("TextureViewImpl::allocate3D");
	}
	else
	{
		Log(Log.Warning, "TextureViewImpl::allocate3D: Attempted double allocation. No effect in allocate call.");
	}
#endif
}
void TextureViewImpl::allocate2DArray(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySlices)
{
#if BUILD_API_MAX<30
	Log(Log.Error, "Called TextureViewImpl::allocate2DArray, but Texture2DArray support was not present");
	PVR_ASSERT(0 && "TextureCube not supported on OpenGL ES 2");
#else
	if (!resource->context->hasApiCapability(ApiCapabilities::Texture3D))
	{
		Log(Log.Error, "Called TextureViewImpl::allocate2DArray, but Texture2DArray is not supported");
		PVR_ASSERT(0 && "Attempt to allocate unsupported Texture2DArray");
		return;
	}
	if (!isAllocated())
	{
		GLenum target = GL_TEXTURE_2D_ARRAY;
		resource->format = format;
		gl::GenTextures(1, &native::useNativeHandle(resource));
		native::useNativeHandleTarget(resource) = target;
		GLenum internalFormat, imageFormat, dataType;
		uint32 typeSize;
		bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat,
		                       imageFormat, dataType, typeSize, isCompressed);
		gl::BindTexture(target, native::useNativeHandle(resource));
		gl::TexStorage3D(target, format.mipmapLevels, internalFormat, width, height, arraySlices);
		debugLogApiError("TextureViewImpl::allocate2DArray");
	}
	else
	{
		Log(Log.Warning, "TextureViewImpl::allocate2DArray: Attempted double allocation. No effect in allocate call.");
	}
#endif
}

void TextureViewImpl::allocate2DCube(const ImageStorageFormat& format, uint32 width, uint32 height)
{
#if BUILD_API_MAX<30
	Log(Log.Error, "Called TextureImpl::bind, but Texture3D support was not present");
	PVR_ASSERT(0 && "TextureCube not supported on OpenGL ES 2");
#else
	if (!isAllocated())
	{
		GLenum target = GL_TEXTURE_CUBE_MAP;
		resource->format = format;
		gl::GenTextures(1, &native::useNativeHandle(resource));
		native::useNativeHandleTarget(resource) = target;
		GLenum internalFormat, imageFormat, dataType;
		uint32 typeSize;
		bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat,
		                       imageFormat, dataType, typeSize, isCompressed);
		gl::BindTexture(target, native::useNativeHandle(resource));
		debugLogApiError("TextureCubeImpl::allocate glBindTexture");
		gl::TexStorage2D(target, format.mipmapLevels, internalFormat, width, height);
		debugLogApiError("TextureCubeImpl::allocate glTexStorage2D");
	}
#endif
}

void TextureViewImpl::update(const void* data, const ImageDataFormat& format, const TextureArea& area)
{
	GLenum target = native::useNativeHandleTarget(resource);
	using namespace assets;
	if (isAllocated())
	{
		GLenum internalFormat, imageFormat, dataType, typeSize; bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat, imageFormat, dataType, typeSize,
		                       isCompressed);
		gl::BindTexture(target, native::useNativeHandle(resource));
		const char* compressString = isCompressed ? "CompressedTexSubImage" : "";
		const char* dimensionString = "UNKNOWN";
		debugLogApiError("Texture2DImpl::update bind");
		if (this->getTextureType() == TextureDimension::Texture2D ||
		    this->getTextureType() == TextureDimension::Texture2DCube)
		{
			dimensionString = "2D";
			if (this->getTextureType() == TextureDimension::Texture2DCube)
			{
				dimensionString = "2DCube";
				target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)area.cubeFace;
			}
			if (isCompressed)
			{
				gl::CompressedTexSubImage2D(target, area.mipLevel, area.offsetx, area.offsety, area.width, area.height,
				                            imageFormat, area.compressedSize, data);
			}
			else
			{
				gl::TexSubImage2D(target, area.mipLevel, area.offsetx, area.offsety, area.width, area.height,
				                  imageFormat, dataType, data);
			}
		}
		else if (this->getTextureType() == TextureDimension::Texture3D || this->getTextureType() == TextureDimension::Texture3DArray)
		{
			uint32 zoffset = (this->getTextureType() == TextureDimension::Texture3D) ? area.offsetz : area.arrayIndex;
			uint32 zsize = (this->getTextureType() == TextureDimension::Texture3D) ? area.depth : area.arraySize;
			dimensionString = "3D";
			if (isCompressed)
			{
				gl::CompressedTexSubImage3D(target, area.mipLevel, area.offsetx, area.offsety, zoffset,
				                            area.width, area.height, zsize, imageFormat, area.compressedSize, data);
			}
			else
			{
				gl::TexSubImage3D(target, area.mipLevel, area.offsetx, area.offsety, zoffset,
				                  area.width, area.height, zsize, imageFormat, dataType, data);
			}
		}
		debugLogApiError(strings::createFormatted("TextureViewImpl::update gl::%sTexSubImage%d", compressString,
		                 dimensionString).c_str());

	}
	else
	{
		Log(Log.Error, "TextureViewImpl::update called on unallocated texture object. Call allocate to set "
		    "texture characteristics.");
	}
}

inline static TextureDimension::Enum  glesTargetToDimension(GLenum target)
{
	switch (target)
	{
	case GL_TEXTURE_2D:
#ifndef TARGET_OS_IPHONE
	case GL_TEXTURE_EXTERNAL_OES:
#endif
		return TextureDimension::Texture2D;
	case GL_TEXTURE_3D:
		return TextureDimension::Texture3D;
	case GL_TEXTURE_2D_ARRAY:
		return TextureDimension::Texture2DArray;
	case GL_TEXTURE_CUBE_MAP:
		return TextureDimension::Texture2DCube;
	case 0:
		return TextureDimension::Unallocated;
	default:
		return TextureDimension::TextureUnknown;
	}
}

TextureDimension::Enum TextureStoreImpl::getDimensions() const
{
	return glesTargetToDimension(m_texture->target);
}
bool TextureStoreImpl::isAllocated() const
{
	return (m_texture.isValid() && m_texture->handle != 0);
}

}// namespace impl
}// namespace api
}// namespace pvr
 //!\endcond