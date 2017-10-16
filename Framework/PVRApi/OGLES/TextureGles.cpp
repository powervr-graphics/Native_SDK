/*!
\brief Contains definitions for the OpenGL ES texture implementation methods.
\file PVRApi/OGLES/TextureGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRNativeApi/OGLES/TextureUtilsGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"


namespace pvr {
namespace api {
namespace {
inline static types::ImageBaseType  glesTargetToDimension(GLenum target)
{
	switch (target)
	{
	case GL_TEXTURE_2D:
#ifndef TARGET_OS_IPHONE
	case GL_TEXTURE_EXTERNAL_OES:
#endif
		return types::ImageBaseType::Image2D;
	case GL_TEXTURE_3D:
		return types::ImageBaseType::Image3D;
	//  case GL_TEXTURE_2D_ARRAY:
	//  return types::TextureDimension::Texture2DArray;
	//  case GL_TEXTURE_CUBE_MAP:
	//    return types::TextureDimension::Texture2DCube;
	case 0:
		return types::ImageBaseType::Unallocated;
	default:
		return types::ImageBaseType::Unknown;
	}
}

inline static types::ImageViewType  glesTargetToImageViewType(GLenum target)
{
	switch (target)
	{
	case GL_TEXTURE_2D:
#ifndef TARGET_OS_IPHONE
	case GL_TEXTURE_EXTERNAL_OES:
#endif
		return types::ImageViewType::ImageView2D;
	case GL_TEXTURE_3D:
		return types::ImageViewType::ImageView3D;
	case GL_TEXTURE_2D_ARRAY:
		return types::ImageViewType::ImageView2DArray;
	case GL_TEXTURE_CUBE_MAP:
		return types::ImageViewType::ImageView2DCube;
	default: return types::ImageViewType::Unallocated;
	}
}

static inline types::ImageViewType getImageViewFromTexture(const TextureStore& texture)
{
	types::ImageViewType viewType = types::ImageViewType::ImageViewUnknown;
	bool isArray = texture->getDepth() > 1;
	bool isCube = texture->is2DCubeMap();

	//determine the image view type
	switch (texture->getImageBaseType())
	{
	case types::ImageBaseType::Image1D:
		viewType = isArray ? types::ImageViewType::ImageView1DArray : types::ImageViewType::ImageView1D;
		break;
	case types::ImageBaseType::Image2D:
		if (isArray)
		{
			viewType = isCube ? types::ImageViewType::ImageView2DCubeArray : types::ImageViewType::ImageView2DArray;
		}
		else
		{
			viewType = isCube ? types::ImageViewType::ImageView2DCube : types::ImageViewType::ImageView2D;
		}
		break;
	case types::ImageBaseType::Image3D:
		viewType = isArray ? types::ImageViewType::ImageView3DArray : types::ImageViewType::ImageView3D;
		break;
	}
	return viewType;
}
}

namespace impl {
TextureView_::TextureView_(const TextureStore& texture, const native::HImageView_& view) :
	resource(texture)
{
	this->viewtype = getImageViewFromTexture(texture);
}

TextureView_::TextureView_(const TextureStore& texture) :
	resource(texture)
{
	this->viewtype = getImageViewFromTexture(texture);
}
}
namespace gles {

void TextureStoreGles_::allocate_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth, uint32 arraySize, bool isCube,
                                  types::ImageUsageFlags usage, bool transient)
{
	if (transient) { return; }
	GLenum target = 0;
	if (!depth) { depth = 1; } bool is3D = (depth > 1);
	if (!arraySize) { arraySize = 1; } bool isArray = (arraySize > 1);
	bool multisampled = (format.numSamples > 1);
	types::ImageBaseType baseImage = types::ImageBaseType::Unallocated;

	if (depth > 1 && arraySize > 1)
	{
		assertion(0, "3D array texture not supported");
		return;
	}

	// check if Multisample texture is supported
	if (multisampled)
	{
		if (is3D) { Log("Musltisample Texture not supported for 3D textures"); return; }
		if (isCube) { Log("Musltisample Texture not supported for Cube textures"); return; }
		if (isArray)
		{
#ifdef GL_TEXTURE_2D_MULTISAMPLE
			target = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
			if (getContext().hasApiCapability(ApiCapabilities::Texture2DArrayMS))
			{
				target = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
			}
			else
#endif
			{
				assertion(0, "Musltisample Texture is not supported"); return;
			}
		}
		else
		{
#ifdef GL_TEXTURE_2D_MULTISAMPLE
			if (getContext().hasApiCapability(ApiCapabilities::Texture2DMS))
			{
				target = GL_TEXTURE_2D_MULTISAMPLE;
			}
			else
#endif
			{
				assertion(0, "Musltisample Texture is not supported"); return;
			}
		}
		baseImage = types::ImageBaseType::Image2D;
	}
	else if (is3D)
	{
#ifdef GL_TEXTURE_3D
		if (isCube) { assertion(0, "3D Cube Texture not supported"); return; }
		if (isArray) { assertion(0, "3D Array Texture not supported"); return; }
		if (getContext().hasApiCapability(ApiCapabilities::Texture3D))
		{
			target = GL_TEXTURE_3D;
			baseImage = types::ImageBaseType::Image3D;
		}
		else
#endif
		{
			assertion(0, "3D Texture not supported."); return;
		}
	}
	else if (isArray)
	{

		assertion(getContext().getApiCapabilities().supports(ApiCapabilities::Texture2DArray), "Texture Array not supported");
		baseImage = types::ImageBaseType::Image2D;
		if (isCube)
		{
#ifdef GL_TEXTURE_CUBE_MAP_ARRAY_OES
			target = GL_TEXTURE_CUBE_MAP_ARRAY_OES;
#else
			assertion(0, "Texture Cube Array not supported.");
#endif
		}
		else
		{
			target = GL_TEXTURE_2D_ARRAY;
		}
	}
	else if (isCube)
	{
		target = GL_TEXTURE_CUBE_MAP;
		baseImage = types::ImageBaseType::Image2D;
	}
	else
	{
		target = GL_TEXTURE_2D;
		baseImage = types::ImageBaseType::Image2D;
	}

	debugLogApiError("Texture2DImpl::allocate bind");
	gl::GenTextures(1, &handle);
	gl::BindTexture(target, handle);

	this->_format = format;
	this->target = target;
	GLenum internalFormat, imageFormat, dataType;
	uint32 typeSize; bool isCompressed;
	nativeGles::ConvertToGles::getOpenGLFormat(format.format, format.colorSpace, format.dataType,
	    internalFormat, imageFormat, dataType, typeSize, isCompressed);

	if (multisampled)
	{
		gl::TexStorage2DMultisample(target, nativeGles::ConvertToGles::samplesCount(types::SampleCount(format.numSamples)), internalFormat, width, height, false);
	}
	else if (isArray || is3D)
	{
		gl::TexStorage3D(target, format.mipmapLevels, internalFormat, width, height, std::max(depth, arraySize));
	}
	else if (_context->hasApiCapability(ApiCapabilities::TexureStorage))
	{
		gl::TexStorage2D(target, format.mipmapLevels, internalFormat, width, height);
		debugLogApiError("Texture2DImpl::allocate texStorage");
	}
	else
	{
		if (_context->getApiType() == Api::OpenGLES2)
		{
			internalFormat = imageFormat;
		}
		for (uint32 face = 0; face < (isCube ? 6 : 1); ++face)
		{
			if (isCube) { target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face; }
			for (int8 i = 0; i < format.mipmapLevels; i++)
			{
				gl::TexImage2D(target, i, internalFormat, width, height, 0, imageFormat, dataType, NULL);
				int tmpw = (std::max)(1u, width / 2);
				int tmph = (std::max)(1u, width / 2);
				if (tmpw == (int)width || tmph == (int)height)
				{
					break; //Wooops
				}
			}
		}
	}

	this->_extents.width = (uint16)width;
	this->_extents.height = (uint16)height;
	this->_extents.depth = (uint16)depth;
	this->_imageBaseType = baseImage;
	this->_layersSize.numMipLevels = format.mipmapLevels;
	this->_layersSize.numArrayLevels = arraySize;
	this->_samplesCount = types::SampleCount(format.numSamples);
	this->_isCubeMap = isCube;
}

void TextureStoreGles_::update_(const void* data, const ImageDataFormat& format, const TextureArea& area)
{
	if (isAllocated())
	{
		GLenum internalFormat, imageFormat, dataType, typeSize; bool isCompressed;
		nativeGles::ConvertToGles::getOpenGLFormat(format.format, format.colorSpace, format.dataType, internalFormat, imageFormat, dataType, typeSize,
		    isCompressed);
		gl::BindTexture(target, handle);
		const char* compressString = isCompressed ? "CompressedTexSubImage" : "";
		const char* dimensionString = "UNKNOWN";
		debugLogApiError("Texture2DImpl::update bind");
		gles::TextureStoreGles_& thisGles = static_cast<gles::TextureStoreGles_&>(*this);
		if (thisGles.getDimensions() == types::ImageBaseType::Image2D /*||
                                      this->getDimensions() == types::TextureDimension::Texture2DCube*/)
		{
			dimensionString = "2D";
			if (this->_isCubeMap)
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
		else if (thisGles.getDimensions() == types::ImageBaseType::Image3D /*|| this->getDimensions() == types::TextureDimension::Texture3DArray*/)
		{
			uint32 zoffset = (thisGles.getDimensions() == types::ImageBaseType::Image3D) ? area.offsetz : area.arrayIndex;
			uint32 zsize = (thisGles.getDimensions() == types::ImageBaseType::Image3D) ? area.depth : area.arraySize;
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

void TextureStoreGles_::allocateStorage_(const ImageStorageFormat& format, uint32 width, uint32 height)
{
	Log(Log.Critical, "Storage textures (a.k.a. Image Load Store) Not implemented for OpenGL ES");
	assertion(false, "Storage textures (a.k.a. Image Load Store) Not implemented for OpenGL ES");
}

inline types::ImageBaseType TextureStoreGles_::getDimensions() const
{
	return glesTargetToDimension(target);
}

TextureStoreGles_::~TextureStoreGles_()
{
	if (isAllocated())
	{
		if (_context.isValid())
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

inline void TextureStoreGles_::setSwizzle(types::Swizzle red, types::Swizzle green, types::Swizzle blue, types::Swizzle alpha)
{
	static const GLenum toSwizzleMask[] = { GL_NONE, GL_ZERO, GL_ONE, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
	static const GLenum toTexGetBinding[] = { GL_NONE, GL_NONE, GL_TEXTURE_BINDING_2D, GL_TEXTURE_BINDING_3D, GL_TEXTURE_BINDING_CUBE_MAP, GL_NONE, GL_TEXTURE_BINDING_2D_ARRAY, GL_NONE,
#ifndef TARGET_OS_IPHONE
	                                          GL_TEXTURE_BINDING_EXTERNAL_OES
#else
	                                          GL_NONE
#endif
	                                        };
	if (!_context->hasApiCapability(ApiCapabilities::TextureSwizzling))
	{
		Log(Log.Error, "Attempted to set Texture Swizzling, but swizzling is not supported by the actual API level");
		return;
	}

	{
		GLint prevTex;
		gl::GetIntegerv(toTexGetBinding[(uint32)getDimensions()], &prevTex);
		debugLogApiError("TextureStore_::GetIntegerv");
		if (prevTex != (GLint)handle) { gl::BindTexture(target, handle); }

		gl::TexParameteri(target, GL_TEXTURE_SWIZZLE_R, (red == types::Swizzle::Identity ? GL_RED : toSwizzleMask[(uint32)red]));
		debugLogApiError("TextureStore_::TexParameteri");

		gl::TexParameteri(target, GL_TEXTURE_SWIZZLE_G, (green == types::Swizzle::Identity ? GL_GREEN : toSwizzleMask[(uint32)green]));
		debugLogApiError("TextureStore_::TexParameteri");

		gl::TexParameteri(target, GL_TEXTURE_SWIZZLE_B, (blue == types::Swizzle::Identity ? GL_BLUE : toSwizzleMask[(uint32)blue]));
		debugLogApiError("TextureStore_::TexParameteri");

		gl::TexParameteri(target, GL_TEXTURE_SWIZZLE_A, (alpha == types::Swizzle::Identity ? GL_ALPHA : toSwizzleMask[(uint32)alpha]));
		debugLogApiError("TextureStore_::TexParameteri");

		if (prevTex != (GLint)handle) { gl::BindTexture(target, (GLuint)prevTex); }
		debugLogApiError("TextureStore_::BindTexture");
	}
	debugLogApiError("TextureStore_::setSwizzle exit");
}
}
}// namespace api
}// namespace pvr
