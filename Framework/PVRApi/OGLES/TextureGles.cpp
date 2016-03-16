/*!*********************************************************************************************************************
\file         PVRApi\OGLES\TextureGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains definitions for the OpenGL ES texture implementation methods.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRNativeApi/OGLES/TextureUtilsGles.h"


namespace pvr {
namespace api {
inline static types::TextureDimension::Enum  glesTargetToDimension(GLenum target)
{
	switch (target)
	{
	case GL_TEXTURE_2D:
#ifndef TARGET_OS_IPHONE
	case GL_TEXTURE_EXTERNAL_OES:
#endif
		return types::TextureDimension::Texture2D;
	case GL_TEXTURE_3D:
		return types::TextureDimension::Texture3D;
	case GL_TEXTURE_2D_ARRAY:
		return types::TextureDimension::Texture2DArray;
	case GL_TEXTURE_CUBE_MAP:
		return types::TextureDimension::Texture2DCube;
	case 0:
		return types::TextureDimension::Unallocated;
	default:
		return types::TextureDimension::TextureUnknown;
	}
}

namespace impl {
native::HTexture_& TextureStore_::getNativeObject() { return static_cast<gles::TextureStoreGles_&>(*this); }
const native::HTexture_& TextureStore_::getNativeObject() const { return static_cast<const gles::TextureStoreGles_&>(*this); }


types::TextureDimension::Enum TextureStore_::getDimensions() const
{
	return static_cast<const gles::TextureStoreGles_&>(*this).getDimensions();
}

bool TextureStore_::isAllocated() const
{
	return static_cast<const gles::TextureStoreGles_&>(*this).isAllocated();
}

void TextureStore_::allocate2D(const ImageStorageFormat& format, uint32 width, uint32 height)
{
	if (!isAllocated())
	{
		GLenum target = GL_TEXTURE_2D;
		this->format = format;
		gl::GenTextures(1, &getNativeObject().handle);
		getNativeObject().target = target;
		GLenum internalFormat, imageFormat, dataType;
		uint32 typeSize; bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat,
		                       imageFormat, dataType, typeSize, isCompressed);

		gl::BindTexture(target, getNativeObject().handle);
		debugLogApiError("Texture2DImpl::allocate bind");
#if BUILD_API_MAX>=30
		if (context->hasApiCapability(ApiCapabilities::TexureStorage))
		{
			gl::TexStorage2D(target, format.mipmapLevels, internalFormat, width, height);
			debugLogApiError("Texture2DImpl::allocate texStorage");
		}
		else
#endif
		{
			if (context->getApiType() == Api::OpenGLES2)
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

	this->format = format;
}

void TextureStore_::allocate3D(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth)
{
#if BUILD_API_MAX<30
	Log(Log.Error, "Called Texture3DImpl::allocate, but Texture3D support was not present");
	assertion(0, "TextureCube not supported on OpenGL ES 2");
#else
	if (!context->hasApiCapability(ApiCapabilities::Texture3D))
	{
		Log(Log.Error, "Called TextureViewImpl::allocate3D, but Texture3D is not supported");
		assertion(0, "Attempt to allocate unsupported Texture3D");
		return;
	}
	if (!isAllocated())
	{
		GLenum target = GL_TEXTURE_3D;
		this->format = format;
		gl::GenTextures(1, &getNativeObject().handle);
		getNativeObject().target = target;
		GLenum internalFormat, imageFormat, dataType;
		uint32 typeSize; bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat,
			imageFormat, dataType, typeSize, isCompressed);
		gl::BindTexture(target, getNativeObject().handle);
		gl::TexStorage3D(target, format.mipmapLevels, internalFormat, width, height, depth);
		debugLogApiError("TextureViewImpl::allocate3D");
	}
	else
	{
		Log(Log.Warning, "TextureViewImpl::allocate3D: Attempted double allocation. No effect in allocate call.");
	}
#endif
}

void TextureStore_::allocate2DArray(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySlices)
{
#if BUILD_API_MAX<30
	Log(Log.Error, "Called TextureViewImpl::allocate2DArray, but Texture2DArray support was not present");
	assertion(0,  "TextureCube not supported on OpenGL ES 2");
#else
	if (!context->hasApiCapability(ApiCapabilities::Texture3D))
	{
		Log(Log.Error, "Called TextureViewImpl::allocate2DArray, but Texture2DArray is not supported");
		assertion(0,  "Attempt to allocate unsupported Texture2DArray");
		return;
	}
	if (!isAllocated())
	{
		GLenum target = GL_TEXTURE_2D_ARRAY;
		this->format = format;
		gl::GenTextures(1, &getNativeObject().handle);
		getNativeObject().target = target;
		GLenum internalFormat, imageFormat, dataType;
		uint32 typeSize; bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat,
			imageFormat, dataType, typeSize, isCompressed);
		gl::BindTexture(target, getNativeObject().handle);
		gl::TexStorage3D(target, format.mipmapLevels, internalFormat, width, height, arraySlices);
		debugLogApiError("TextureViewImpl::allocate2DArray");
	}
	else
	{
		Log(Log.Warning, "TextureViewImpl::allocate2DArray: Attempted double allocation. No effect in allocate call.");
	}
#endif
}

void TextureStore_::allocate2DCube(const ImageStorageFormat& format, uint32 width, uint32 height)
{
#if BUILD_API_MAX<30
	Log(Log.Error, "Called TextureImpl::bind, but Texture3D support was not present");
	assertion(0, "TextureCube not supported on OpenGL ES 2");
#else
	if (!isAllocated())
	{
		GLenum target = GL_TEXTURE_CUBE_MAP;
		this->format = format;
		gl::GenTextures(1, &getNativeObject().handle);
		getNativeObject().target = target;
		GLenum internalFormat, imageFormat, dataType;
		uint32 typeSize; bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat,
			imageFormat, dataType, typeSize, isCompressed);
		gl::BindTexture(target, getNativeObject().handle);
		debugLogApiError("TextureCubeImpl::allocate glBindTexture");
		gl::TexStorage2D(target, format.mipmapLevels, internalFormat, width, height);
		debugLogApiError("TextureCubeImpl::allocate glTexStorage2D");
	}
#endif
}

void TextureStore_::update(const void* data, const ImageDataFormat& format, const TextureArea& area)
{
	GLenum target = getNativeObject().target;

	if (isAllocated())
	{
		GLenum internalFormat, imageFormat, dataType, typeSize; bool isCompressed;
		utils::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat, imageFormat, dataType, typeSize,
			isCompressed);
		gl::BindTexture(target, getNativeObject().handle);
		const char* compressString = isCompressed ? "CompressedTexSubImage" : "";
		const char* dimensionString = "UNKNOWN";
		debugLogApiError("Texture2DImpl::update bind");
		if (this->getDimensions() == types::TextureDimension::Texture2D ||
			this->getDimensions() == types::TextureDimension::Texture2DCube)
		{
			dimensionString = "2D";
			if (this->getDimensions() == types::TextureDimension::Texture2DCube)
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
		else if (this->getDimensions() == types::TextureDimension::Texture3D || this->getDimensions() == types::TextureDimension::Texture3DArray)
		{
			uint32 zoffset = (this->getDimensions() == types::TextureDimension::Texture3D) ? area.offsetz : area.arrayIndex;
			uint32 zsize = (this->getDimensions() == types::TextureDimension::Texture3D) ? area.depth : area.arraySize;
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

TextureStore_::~TextureStore_() {}

TextureView_::TextureView_(const TextureStore& texture, const native::HImageView_& view) :
	resource(texture)
{
}

TextureView_::TextureView_(const TextureStore& texture) :
	resource(texture)
{
}
}//namespace impl
namespace gles {

inline types::TextureDimension::Enum gles::TextureStoreGles_::getDimensions() const
{
	return glesTargetToDimension(target);
}

TextureStoreGles_::~TextureStoreGles_()
{
	if (isAllocated())
	{
		if (context.isValid())
		{
			gl::DeleteTextures(1, &handle);
			debugLogApiError("TextureGles_::~TextureGles_ exit");
		}
		else
		{
			Log(Log.Warning, "Texture object was not released up before context destruction");
		}
	}
}

inline void TextureStoreGles_::setSwizzle(types::Swizzle::Enum red, types::Swizzle::Enum green, types::Swizzle::Enum blue, types::Swizzle::Enum alpha)
{
	static const GLenum toSwizzleMask[] = { GL_NONE, GL_ZERO, GL_ONE, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
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
	if ((red | green | blue | alpha))
	{
		GLint prevTex;
		gl::GetIntegerv(toTexGetBinding[getDimensions()], &prevTex);
		debugLogApiError("TextureStore_::GetIntegerv");
		if (prevTex != (GLint)handle) { gl::BindTexture(target, handle); }

		gl::TexParameteri(target, GL_TEXTURE_SWIZZLE_R, (red == types::Swizzle::Identity ? GL_RED : toSwizzleMask[red]));
		debugLogApiError("TextureStore_::TexParameteri");

		gl::TexParameteri(target, GL_TEXTURE_SWIZZLE_G, (green == types::Swizzle::Identity ? GL_GREEN : toSwizzleMask[green]));
		debugLogApiError("TextureStore_::TexParameteri");

		gl::TexParameteri(target, GL_TEXTURE_SWIZZLE_B, (blue == types::Swizzle::Identity ? GL_BLUE : toSwizzleMask[blue]));
		debugLogApiError("TextureStore_::TexParameteri");

		gl::TexParameteri(target, GL_TEXTURE_SWIZZLE_A, (alpha == types::Swizzle::Identity ? GL_ALPHA : toSwizzleMask[alpha]));
		debugLogApiError("TextureStore_::TexParameteri");

		if (prevTex != (GLint)handle) { gl::BindTexture(target, (GLuint)prevTex); }
		debugLogApiError("TextureStore_::BindTexture");
	}
	debugLogApiError("TextureStore_::setSwizzle exit");
}

}// namespace gles
}// namespace api
}// namespace pvr
//!\endcond
