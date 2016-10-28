/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\TextureUtils.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains function definitions for OpenGL ES Texture Utils.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/PVRCore.h"
#include "PVRCore/FileStream.h"
#include "PVRNativeApi/TextureUtils.h"
#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/FileIO/TextureReaderPVR.h"
#include "PVRAssets/FileIO/TextureWriterPVR.h"
#include "PVRAssets/FileIO/TextureReaderTGA.h"
#include "PVRAssets/FileIO/TextureReaderKTX.h"
#include "PVRAssets/FileIO/TextureReaderDDS.h"
#include "PVRAssets/FileIO/TextureReaderBMP.h"
#include "PVRAssets/FileIO/TextureWriterLegacyPVR.h"
#include "PVRAssets/Texture/PVRTDecompress.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include <algorithm>
namespace pvr {
using namespace types;
namespace utils {


bool isExtensionSupported(std::string& extensionStore, const char8* extension)
{
	if (extensionStore.empty())
	{
		const char* extensions = (const char*)gl::GetString(GL_EXTENSIONS);
		if (extensions)
		{
			extensionStore.assign(extensions);
		}
			else
			{
			extensionStore.assign("");
		}
			}
	return extensionStore.find(extension) != extensionStore.npos;
			}

Result textureUpload(
  IPlatformContext& context, const assets::Texture& texture, native::HTexture_& outTextureName,
                     types::ImageAreaSize& outTextureSize, PixelFormat& outFormat, bool& isDecompressed,
                     bool allowDecompress/*=true*/)
{
	std::string extensionString;

	static_cast<types::Extent3D&>(outTextureSize) = texture.getDimensions();
	static_cast<types::ImageLayersSize&>(outTextureSize) = texture.getLayersSize();

	using namespace assets;
	//Initial error checks
	// Check for any glError occurring prior to loading the texture, and warn the user.
	api::debugLogApiError("TextureUtils.h:textureUpload:: GL error was set prior to function call.\n");

	// Check that the texture is valid.
	if (!texture.getDataSize())
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs.\n");
		return Result::UnsupportedRequest;
	}

	// Setup code to get various state
	// Generic error strings for textures being unsupported.
	const char8* cszUnsupportedFormat =
	    "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation.\n";

	const char8* cszUnsupportedFormatDecompressionAvailable =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation. "
	  "Allowing software decompression (allowDecompress=true) will enable you to use this format.\n";

	// Get the texture format for the API.
	GLenum glInternalFormat = 0;
	GLenum glFormat = 0;
	GLenum glType = 0;
	GLenum glTypeSize = 0;
	bool unused;

	// Check that the format is a valid format for this API - Doesn't check specifically between OpenGL/ES,
	// it simply gets the values that would be set for a KTX file.
	if (!api::ConvertToGles::getOpenGLFormat(texture.getPixelFormat(), texture.getColorSpace(), texture.getChannelType(),
	    glInternalFormat, glFormat, glType, glTypeSize, unused))
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Texture's pixel type is not supported by this API.\n");
		return Result::UnsupportedRequest;
	}

	// Is the texture compressed? RGB9E5 is treated as an uncompressed texture in OpenGL/ES so is a special case.
	bool isCompressedFormat = (texture.getPixelFormat().getPart().High == 0)
	                          && (texture.getPixelFormat().getPixelTypeId() != (uint64)CompressedPixelFormat::SharedExponentR9G9B9E5);

	//Whether we should use TexStorage or not.
	bool isEs2 = context.getApiType() < Api::OpenGLES3;
	bool useTexStorage = !isEs2;
	bool needsSwizzling = false;
	GLenum swizzle_r = GL_RED, swizzle_g = GL_GREEN, swizzle_b = GL_BLUE, swizzle_a = GL_ALPHA;

	//Texture to use if we decompress in software.
	assets::Texture cDecompressedTexture;

	// Texture pointer which points at the texture we should use for the function. Allows switching to,
	// for example, a decompressed version of the texture.
	const assets::Texture* textureToUse = &texture;

	//Default texture target, modified as necessary as the texture type is determined.
	outTextureName.target = GL_TEXTURE_2D;


	// Check that extension support exists for formats supported in this way.
	{
		//Check for formats that cannot be supported by this context version
		switch (glFormat)
		{
		case GL_LUMINANCE:
			if (!isEs2)
			{
				Log(Log.Information,
				    "LUMINANCE texture format detected in OpenGL ES 3+ context. Remapping to RED texture "
				    "with swizzling (r,r,r,1) enabled.");
				needsSwizzling = true;
				glFormat = GL_RED;
				glInternalFormat = GL_R8;
				swizzle_r = GL_RED;
				swizzle_g = GL_RED;
				swizzle_b = GL_RED;
				swizzle_a = GL_ONE;
			}
			break;
		case GL_ALPHA:
			if (!isEs2)
			{
				Log(Log.Information,
				    "ALPHA format texture detected in OpenGL ES 3+ context. Remapping to RED texture with "
				    "swizzling (0,0,0,r) enabled in order to allow Texture Storage.");
				needsSwizzling = true;
				glFormat = GL_RED;
				glInternalFormat = GL_R8;
				swizzle_r = GL_ZERO;
				swizzle_g = GL_ZERO;
				swizzle_b = GL_ZERO;
				swizzle_a = GL_RED;
			}
			break;
		case GL_LUMINANCE_ALPHA:
			if (!isEs2)
			{
				Log(Log.Information,
				    "LUMINANCE/ALPHA format texture detected in OpenGL ES 3+ context. Remapping to RED "
				    "texture with swizzling (r,r,r,g) enabled in order to allow Texture Storage.");
				needsSwizzling = true;
				glFormat = GL_RG;
				glInternalFormat = GL_RG8;
				swizzle_r = GL_RED;
				swizzle_g = GL_RED;
				swizzle_b = GL_RED;
				swizzle_a = GL_GREEN;
			} break;
		case GL_RED:
			if (isEs2)
			{
				Log(Log.Warning,
				    "RED channel texture format texture detected in OpenGL ES 2+ context. Remapping to LUMINANCE"
				    " texture to avoid errors. Ensure shaders are compatible with a LUMINANCE swizzle (r,r,r,1)");
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
			} break;
		case GL_RG:
			if (isEs2)
			{
				Log(Log.Warning,
				    "RED/GREEN channel texture format texture detected in OpenGL ES 2+ context. Remapping to "
				    "LUMINANCE_ALPHA texture to avoid errors. Ensure shaders are compatible with a LUMINANCE/ALPHA swizzle (r,r,r,g)");
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
			} break;
		}

		// Check for formats only supported by extensions.
		switch (glInternalFormat)
		{
		case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(extensionString, "GL_IMG_texture_compression_pvrtc"))
			{
				if (allowDecompress)
				{
					//No longer compressed if this is the case.
					isCompressedFormat = false;

					//Set up the new texture and header.
					assets::TextureHeader cDecompressedHeader(texture);
					// robin: not sure what should happen here. The PVRTGENPIXELID4 macro is used in the old SDK.
					cDecompressedHeader.setPixelFormat(assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);

					cDecompressedHeader.setChannelType(VariableType::UnsignedByteNorm);
					cDecompressedTexture = assets::Texture(cDecompressedHeader);

					//Update the texture format.
					api::ConvertToGles::getOpenGLFormat(cDecompressedTexture.getPixelFormat(), cDecompressedTexture.getColorSpace(),
					                                    cDecompressedTexture.getChannelType(), glInternalFormat, glFormat, glType,
					                                    glTypeSize, unused);

					//Do decompression, one surface at a time.
					for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
					{
						for (uint32 uiArray = 0; uiArray < textureToUse->getNumberOfArrayMembers(); ++uiArray)
						{
							for (uint32 uiFace = 0; uiFace < textureToUse->getNumberOfFaces(); ++uiFace)
							{
								PVRTDecompressPVRTC(textureToUse->getDataPointer(uiMIPLevel, uiArray, uiFace),
								                    (textureToUse->getBitsPerPixel() == 2 ? 1 : 0),
								                    textureToUse->getWidth(uiMIPLevel), textureToUse->getHeight(uiMIPLevel),
								                    cDecompressedTexture.getDataPointer(uiMIPLevel, uiArray, uiFace));
							}
						}
					}
					//Make sure the function knows to use a decompressed texture instead.
					textureToUse = &cDecompressedTexture;
				}
				else
				{
					Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "PVRTC1");
					return Result::UnsupportedRequest;
				}
			}
			break;
		}
#if defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG) || defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG)
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(extensionString, "GL_IMG_texture_compression_pvrtc2"))
			{
				Log(Log.Error, cszUnsupportedFormat, "PVRTC2");
				return Result::UnsupportedRequest;
			}
			break;
		}
#endif
#ifdef GL_ETC1_RGB8_OES
		case GL_ETC1_RGB8_OES:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(extensionString, "GL_OES_compressed_ETC1_RGB8_texture"))
			{
				//if (false && allowDecompress)
				//{
				//  //No longer compressed if this is the case
				//  isCompressedFormat = false;

				//  //Set up the new texture and header.
				//  assets::TextureHeader cDecompressedHeader(texture);
				//  // robin: not sure what should happen here. The PVRTGENPIXELID4 macro is used in the old SDK.
				//  //cDecompressedHeader.setPixelFormat(PVRTGENPIXELID4('r','g','b','a',8,8,8,8));
				//  cDecompressedHeader.setChannelType(VariableType::UnsignedByteNorm);
				//  cDecompressedTexture = assets::Texture(cDecompressedHeader);

				//  //Update the texture format
				//  utils::getOpenGLFormat(cDecompressedTexture.getPixelFormat(), cDecompressedTexture.getColorSpace(),
				//                         cDecompressedTexture.getChannelType(), glInternalFormat, glFormat, glType,
				//                         glTypeSize, isCompressedFormat);

				//  //Do decompression, one surface at a time.
				//  for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				//  {
				//    for (uint32 uiArray = 0; uiArray < textureToUse->getNumberOfArrayMembers(); ++uiArray)
				//    {
				//      for (uint32 uiFace = 0; uiFace < textureToUse->getNumberOfFaces(); ++uiFace)
				//      {
				//        // Robin: This mode variable is currently unused inside PVRTDecompressETC()
				//        //int mode = 0;
				//        /*PVRTDecompressETC(textureToUse->getDataPointer(uiMIPLevel,uiArray,uiFace),
				//        textureToUse->getWidth(uiMIPLevel),
				//        textureToUse->getHeight(uiMIPLevel),
				//        (unsigned char*)cDecompressedTexture.getDataPointer(uiMIPLevel,uiArray,uiFace),
				//        mode);*/
				//        assertion(false , "Need to decouple sdk1 from sdk2");
				//      }
				//    }
				//  }

				//  //Make sure the function knows to use a decompressed texture instead.
				//  textureToUse = &cDecompressedTexture;
				//}
				//else
				{
					Log(Log.Error, cszUnsupportedFormat, "ETC1");
					return Result::UnsupportedRequest;
				}
			}
			break;
		}
#endif
#if !defined(TARGET_OS_IPHONE)
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(extensionString, "GL_EXT_texture_compression_dxt1"))
			{
				Log(Log.Error, cszUnsupportedFormat, "DXT1");
				return Result::UnsupportedRequest;
			}
			break;
		}
		case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(extensionString, "GL_ANGLE_texture_compression_dxt3"))
			{
				Log(Log.Error, cszUnsupportedFormat, "DXT3");
				return Result::UnsupportedRequest;
			}
			break;
		}
		case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(extensionString, "GL_ANGLE_texture_compression_dxt5"))
			{
				Log(Log.Error, cszUnsupportedFormat, "DXT5");
				return Result::UnsupportedRequest;
			}
			break;
		}
#endif
		case GL_BGRA_EXT:
		{
			//useTexStorage = true;
			if (!isExtensionSupported(extensionString, "GL_EXT_texture_format_BGRA8888"))
			{
				//Check if the APPLE extension is available instead of the EXT version.
				if (isExtensionSupported(extensionString, "GL_APPLE_texture_format_BGRA8888"))
				{
					//The APPLE extension differs from the EXT extension, and accepts GL_RGBA as the internal format instead.
					glInternalFormat =  GL_RGBA;
				}
				else
				{
					Log(Log.Error, cszUnsupportedFormat, "BGRA8888");
					return Result::UnsupportedRequest;
				}
			}
			break;
		}
		default:
		{}
		}
	}

	// Check the type of texture (e.g. 3D textures).
	{
		// Only 2D Arrays are supported in this API.
		if (textureToUse->getNumberOfArrayMembers() > 1)
		{
#if !defined(BUILD_API_MAX)||BUILD_API_MAX>=30
			//Make sure it's not also a cube map or 3D texture, as this is unsupported.
			if (textureToUse->getNumberOfFaces() > 1)
			{
				Log(Log.Error,
				    "TextureUtils.h:textureUpload:: Texture arrays with multiple faces are not supported by this implementation.\n");
				return Result::UnsupportedRequest;
			}
			else if (textureToUse->getDepth() > 1)
			{
				Log(Log.Error,
				    "TextureUtils.h:textureUpload:: 3D Texture arrays are not supported by this implementation.\n");
				return Result::UnsupportedRequest;
			}
			outTextureName.target = GL_TEXTURE_2D_ARRAY;
#else
			Log(Log.Error,
			    "TextureUtils.h:textureUpload:: Texture arrays are not supported by this implementation.\n");
			return Result::UnsupportedRequest;
#endif
		}

		// 3D Cubemaps aren't supported
		if (textureToUse->getDepth() > 1)
		{
#if !defined(BUILD_API_MAX)||BUILD_API_MAX>=30
			//Make sure it's not also a cube map, as this is unsupported. We've already checked for arrays so no need to check again.
			if (textureToUse->getNumberOfFaces() > 1)
			{
				Log(Log.Error,
				    "TextureUtils.h:textureUpload:: 3-Dimensional textures with multiple faces are not supported by this implementation.\n");
				return Result::UnsupportedRequest;
			}
			outTextureName.target = GL_TEXTURE_3D;
#else
			Log(Log.Error,
			    "TextureUtils.h:textureUpload:: 3-Dimensional textures are not supported by this implementation.\n");
			return Result::UnsupportedRequest;
#endif
		}

		//Check if it's a Cube Map.
		if (textureToUse->getNumberOfFaces() > 1)
		{
			//Make sure it's a complete cube, otherwise warn the user. We've already checked if it's a 3D texture or a texture array as well.
			if (textureToUse->getNumberOfFaces() < 6)
			{
				Log(Log.Warning,
				    "TextureUtils.h:textureUpload:: Textures with between 2 and 5 faces are unsupported. Faces up to 6 will be allocated in a cube map as undefined surfaces.\n");
			}
			else if (textureToUse->getNumberOfFaces() > 6)
			{
				Log(Log.Warning,
				    "TextureUtils.h:textureUpload:: Textures with more than 6 faces are unsupported. Only the first 6 faces will be loaded into the API.\n");
			}
			outTextureName.target = GL_TEXTURE_CUBE_MAP;
		}
	}

	// Setup the texture object.
	{
		// Check the error here, in case the extension loader or anything else raised any errors.
		api::debugLogApiError("TextureUtils.h:textureUpload:: GL has raised error from prior to uploading the texture.");

		//Generate a new texture name.
		gl::GenTextures(1, &outTextureName.handle);

		//Bind the texture to edit it.
		gl::BindTexture(outTextureName.target, outTextureName.handle);

		//Set the unpack alignment to 1 - PVR textures are not stored as padded.
		gl::PixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (needsSwizzling)
		{
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, swizzle_r);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, swizzle_g);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, swizzle_b);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, swizzle_a);
			api::logApiError("TextureUtils.h:textureUpload:: GL has raised error attempting to swizzle a texture.");
		}

		if (api::logApiError("TextureUtils.h:textureUpload:: GL has raised error attempting to bind the texture for first use."))
		{
			return Result::UnknownError;
		}
	}

	// Load the texture.
	{
		api::debugLogApiError("TextureUtils.h:textureUpload:: GL has a raised error before attempting to define texture storage.");
		// 2D textures.
		if (outTextureName.target == GL_TEXTURE_2D)
		{
			//Use tex storage if available, to generate an immutable texture.
#if !defined(BUILD_API_MAX)||BUILD_API_MAX>=30
			if (useTexStorage)
			{
				gl::TexStorage2D(outTextureName.target, textureToUse->getNumberOfMIPLevels(), glInternalFormat, textureToUse->getWidth(),
				                 textureToUse->getHeight());
				if (api::logApiError(pvr::strings::createFormatted("textureUpload::glTexStorage2D With InternalFormat : % x",
				                     glInternalFormat).c_str()))
				{
					return Result::UnsupportedRequest;
				}


				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexSubImage2D(outTextureName.target, uiMIPLevel, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                            textureToUse->getHeight(uiMIPLevel), glInternalFormat, textureToUse->getDataSize(uiMIPLevel, false,
						                                    false),
						                            textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError("TextureUtils::textureUpload:: glCompressedTexSubImage2D"))
						{
							return Result::UnsupportedRequest;
						}

					}
					else
					{
						gl::TexSubImage2D(outTextureName.target, uiMIPLevel, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                  textureToUse->getHeight(uiMIPLevel),
						                  glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glTexSubImage2D")))
						{
							return Result::UnsupportedRequest;
						}
					}
				}
			}
			else
#endif
			{
				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexImage2D(outTextureName.target, uiMIPLevel, glInternalFormat,
						                         textureToUse->getWidth(uiMIPLevel),
						                         textureToUse->getHeight(uiMIPLevel), 0,
						                         textureToUse->getDataSize(uiMIPLevel, false, false),
						                         textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glCompressedTexImage2D")))
						{
							return Result::UnsupportedRequest;
						}
					}
					else
					{
						if (isEs2) { glInternalFormat = glFormat;}
						gl::TexImage2D(outTextureName.target, uiMIPLevel, glInternalFormat, textureToUse->getWidth(uiMIPLevel),
						               textureToUse->getHeight(uiMIPLevel), 0, glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glTexImage2D")))
						{
							return Result::UnsupportedRequest;
						}
					}
				}
			}
		}
		// Cube maps.
		else if (outTextureName.target == GL_TEXTURE_CUBE_MAP)
		{
#if !defined(BUILD_API_MAX)||BUILD_API_MAX>=30
			if (useTexStorage)
			{
				//Use tex storage, to generate an immutable texture.
				gl::TexStorage2D(outTextureName.target, textureToUse->getNumberOfMIPLevels(), glInternalFormat,
				                 textureToUse->getWidth(), textureToUse->getHeight());
				if (api::logApiError(("TextureUtils::textureUpload::(cubemap) glTexStorage2D")))
				{
					return Result::UnsupportedRequest;
				}

				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					//Iterate through 6 faces regardless, as these should always be iterated through. We fill in the blanks with uninitialized data for uncompressed textures, or repeat faces for compressed data.
					for (uint32 uiFace = 0; uiFace < 6; ++uiFace)
					{
						GLenum eTexImageTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
						if (isCompressedFormat)
						{
							//Make sure to wrap texture faces around if there are fewer faces than 6 in a compressed texture.
							gl::CompressedTexSubImage2D(eTexImageTarget + uiFace, uiMIPLevel, 0, 0,
							                            textureToUse->getWidth(uiMIPLevel),
							                            textureToUse->getHeight(uiMIPLevel), glInternalFormat, textureToUse->getDataSize(uiMIPLevel, false,
							                                    false),
							                            textureToUse->getDataPointer(uiMIPLevel, 0, uiFace % textureToUse->getNumberOfFaces()));
							if (api::logApiError(
							            strings::createFormatted("TextureUtils::textureUpload::(cubemap face %d) glCompressedTexSubImage2D",
							                                     uiFace).c_str()))
							{
								return Result::UnsupportedRequest;
							}
						}
						else
						{
							//No need to wrap faces for uncompressed textures, as gl will handle a NULL pointer, which Texture::getDataPtr will do when requesting a non-existant face.
							gl::TexSubImage2D(eTexImageTarget + uiFace, uiMIPLevel, 0, 0, textureToUse->getWidth(uiMIPLevel),
							                  textureToUse->getHeight(uiMIPLevel), glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0,
							                          uiFace % textureToUse->getNumberOfFaces()));
							if (api::logApiError(
							            strings::createFormatted("TextureUtils::textureUpload::(cubemap face %d) glTexSubImage2D",
							                                     uiFace).c_str()))
							{
								return Result::UnsupportedRequest;
							}
						}
					}
				}
			}
			else
#endif
			{
				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					//Iterate through 6 faces regardless, as these should always be iterated through. We fill in the blanks with uninitialized data for uncompressed textures, or repeat faces for compressed data.
					for (uint32 uiFace = 0; uiFace < 6; ++uiFace)
					{
						GLenum eTexImageTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
						if (isCompressedFormat)
						{
							//Make sure to wrap texture faces around if there are fewer faces than 6 in a compressed texture.
							gl::CompressedTexImage2D(eTexImageTarget + uiFace, uiMIPLevel, glInternalFormat,
							                         textureToUse->getWidth(uiMIPLevel),
							                         textureToUse->getHeight(uiMIPLevel), 0, textureToUse->getDataSize(uiMIPLevel, false, false),
							                         textureToUse->getDataPointer(uiMIPLevel, 0, uiFace % textureToUse->getNumberOfFaces()));
							if (api::logApiError(
							            strings::createFormatted("TextureUtils::textureUpload::(cubemap face %d) glCompressedTexImage2D",
							                                     uiFace).c_str()))
							{
								return Result::UnsupportedRequest;
							}
						}
						else
						{
							//No need to wrap faces for uncompressed textures, as gl will handle a NULL pointer, which Texture::getDataPtr will do when requesting a non-existant face.
							gl::TexImage2D(eTexImageTarget + uiFace, uiMIPLevel, glInternalFormat,
							               textureToUse->getWidth(uiMIPLevel),
							               textureToUse->getHeight(uiMIPLevel), 0, glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0,
							                       uiFace % textureToUse->getNumberOfFaces()));
							if (api::logApiError(
							            strings::createFormatted("TextureUtils::textureUpload::(cubemap face %d) glTexImage2D",
							                                     uiFace).c_str()))
							{
								return Result::UnsupportedRequest;
							}
						}
					}
				}
			}
		}
#if !defined(BUILD_API_MAX)||BUILD_API_MAX>=30
		// 3D textures
		else if (outTextureName.target == GL_TEXTURE_3D)
		{
			if (useTexStorage)
			{
				//Use tex storage, to generate an immutable texture.
				gl::TexStorage3D(outTextureName.target, textureToUse->getNumberOfMIPLevels(), glInternalFormat,
				                 textureToUse->getWidth(),
				                 textureToUse->getHeight(), textureToUse->getDepth());
				if (api::logApiError(("TextureUtils::textureUpload:: glTexStorage3D")))
				{
					return Result::UnsupportedRequest;
				}
				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexSubImage3D(outTextureName.target, uiMIPLevel, 0, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                            textureToUse->getHeight(uiMIPLevel), textureToUse->getDepth(uiMIPLevel), glInternalFormat,
						                            textureToUse->getDataSize(uiMIPLevel, false, false), textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glCompressedTexSubImage3D")))
						{
							return Result::UnsupportedRequest;
						}
					}
					else
					{
						gl::TexSubImage3D(outTextureName.target, uiMIPLevel, 0, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                  textureToUse->getHeight(uiMIPLevel),
						                  textureToUse->getDepth(uiMIPLevel), glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0,
						                          0));
						if (api::logApiError(("TextureUtils::textureUpload:: glTexSubImage3D")))
						{
							return Result::UnsupportedRequest;
						}
					}
				}
			}
			else
			{
				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexImage3D(outTextureName.target, uiMIPLevel, glInternalFormat,
						                         textureToUse->getWidth(uiMIPLevel),
						                         textureToUse->getHeight(uiMIPLevel), textureToUse->getDepth(uiMIPLevel), 0,
						                         textureToUse->getDataSize(uiMIPLevel, false,
						                                 false), textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glCompressedTexImage3D")))
						{
							return Result::UnsupportedRequest;
						}
					}
					else
					{
						gl::TexImage3D(outTextureName.target, uiMIPLevel, glInternalFormat, textureToUse->getWidth(uiMIPLevel),
						               textureToUse->getHeight(uiMIPLevel), textureToUse->getDepth(uiMIPLevel), 0, glFormat, glType,
						               textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glTexImage3D")))
						{
							return Result::UnsupportedRequest;
						}
					}
				}
			}
		}
		// Texture arrays.
		else if (outTextureName.target == GL_TEXTURE_2D_ARRAY)
		{
			if (useTexStorage)
			{
				//Use tex storage, to generate an immutable texture.
				gl::TexStorage3D(outTextureName.target, textureToUse->getNumberOfMIPLevels(), glInternalFormat,
				                 textureToUse->getWidth(),
				                 textureToUse->getHeight(), textureToUse->getNumberOfArrayMembers());

				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexSubImage3D(outTextureName.target, uiMIPLevel, 0, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                            textureToUse->getHeight(uiMIPLevel), textureToUse->getNumberOfArrayMembers(), glInternalFormat,
						                            textureToUse->getDataSize(uiMIPLevel, false, false), textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glCompressedTexSubImage3D")))
						{
							return Result::UnsupportedRequest;
						}
					}
					else
					{
						gl::TexSubImage3D(outTextureName.target, uiMIPLevel, 0, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                  textureToUse->getHeight(uiMIPLevel),
						                  textureToUse->getNumberOfArrayMembers(), glFormat, glType, textureToUse->getDataPointer(uiMIPLevel,
						                          0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glTexSubImage3D")))
						{
							return Result::UnsupportedRequest;
						}
					}
				}
			}
			else
			{
				for (uint32 uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumberOfMIPLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexImage3D(outTextureName.target, uiMIPLevel, glInternalFormat,
						                         textureToUse->getWidth(uiMIPLevel),
						                         textureToUse->getHeight(uiMIPLevel), textureToUse->getNumberOfArrayMembers(), 0,
						                         textureToUse->getDataSize(uiMIPLevel,
						                                 false, false), textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glCompressedTexImage3D")))
						{
							return Result::UnsupportedRequest;
						}
					}
					else
					{
						gl::TexImage3D(outTextureName.target, uiMIPLevel, glInternalFormat, textureToUse->getWidth(uiMIPLevel),
						               textureToUse->getHeight(uiMIPLevel), textureToUse->getNumberOfArrayMembers(), 0, glFormat, glType,
						               textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (api::logApiError(("TextureUtils::textureUpload:: glTexImage3D")))
						{
							return Result::UnsupportedRequest;
						}
					}
				}
			}
		}
#endif
		else
		{
			Log(Log.Debug,
			    "TextureUtilsGLES3 : TextureUpload : File corrupted or suspected bug : unknown texture target type.");
		}
	}

	return Result::Success;
}

}// namespace utils
}// namespace pvr
//!\endcond
