/*!
\brief Contains function definitions for OpenGL ES Texture Utils.
\file PVRUtils/OGLES/TextureUtilsGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/IO/FileStream.h"
#include "PVRUtils/OGLES/TextureUtilsGles.h"
#include "PVRCore/Texture.h"
#include "PVRCore/Texture/PVRTDecompress.h"
#include "PVRUtils/OGLES/ErrorsGles.h"
#include "PVRUtils/OGLES/BindingsGles.h"
#include "PVRUtils/OGLES/ConvertToGlesTypes.h"
#include <algorithm>

namespace pvr {
namespace utils {
TextureUploadResults textureUpload(const Texture& texture, bool isEs2, bool allowDecompress)
{
	TextureUploadResults retval;
	// Check that the texture is valid.
	if (!texture.getDataSize())
	{
		Log(LogLevel::Error, "TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs.\n");
		retval.successful = false;
		return retval;
	}

	std::string extensionString;

	//Initial error checks
	// Check for any glError occurring prior to loading the texture, and warn the user.
	debugLogApiError("TextureUtils.h:textureUpload:: GL error was set prior to function call.\n");

	// Setup code to get various state
	// Generic error strings for textures being unsupported.
	const char* cszUnsupportedFormat =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation.\n";

	const char* cszUnsupportedFormatDecompressionAvailable =
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
	if (!utils::getOpenGLFormat(texture.getPixelFormat(), texture.getColorSpace(), texture.getChannelType(),
	                            glInternalFormat, glFormat, glType, glTypeSize, unused))
	{
		Log(LogLevel::Error, "TextureUtils.h:textureUpload:: Texture's pixel type is not supported by this API.\n");
		retval.successful = false;
		return retval;
	}

	// Is the texture compressed? RGB9E5 is treated as an uncompressed texture in OpenGL/ES so is a special case.
	bool isCompressedFormat = (texture.getPixelFormat().getPart().High == 0)
	                          && (texture.getPixelFormat().getPixelTypeId() != static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5));

	//Whether we should use TexStorage or not.
	bool useTexStorage = !isEs2;
	bool needsSwizzling = false;
	GLenum swizzle_r = GL_RED, swizzle_g = GL_GREEN, swizzle_b = GL_BLUE, swizzle_a = GL_ALPHA;

	//Texture to use if we decompress in software.
	Texture cDecompressedTexture;

	// Texture pointer which points at the texture we should use for the function. Allows switching to,
	// for example, a decompressed version of the texture.
	const Texture* textureToUse = &texture;
	retval.target = GL_TEXTURE_2D;
	// Check that extension support exists for formats supported in this way.
	{
		//Check for formats that cannot be supported by this context version
		switch (glFormat)
		{
		case GL_LUMINANCE:
			if (!isEs2)
			{
				Log(LogLevel::Information,
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
				Log(LogLevel::Information,
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
				Log(LogLevel::Information,
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
				Log(LogLevel::Warning,
				    "RED channel texture format texture detected in OpenGL ES 2+ context. Remapping to LUMINANCE"
				    " texture to avoid errors. Ensure shaders are compatible with a LUMINANCE swizzle (r,r,r,1)");
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
			} break;
		case GL_RG:
			if (isEs2)
			{
				Log(LogLevel::Warning,
				    "RED/GREEN channel texture format texture detected in OpenGL ES 2+ context. Remapping to "
				    "LUMINANCE_ALPHA texture to avoid errors. Ensure shaders are compatible with a LUMINANCE/ALPHA swizzle (r,r,r,g)");
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
			} break;
		}

		// Check for formats only supported by extensions.
		switch (glInternalFormat)
		{
#ifdef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
		case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
#endif
#ifdef GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
#endif
#ifdef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
#endif
#ifdef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
#endif
#if defined(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG) || defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG) ||  defined(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG) || defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
		{
			if (!gl::isGlExtensionSupported("GL_IMG_texture_compression_pvrtc"))
			{
				if (allowDecompress)
				{
					//No longer compressed if this is the case.
					isCompressedFormat = false;

					//Set up the new texture and header.
					TextureHeader cDecompressedHeader(texture);
					cDecompressedHeader.setPixelFormat(GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);

					cDecompressedHeader.setChannelType(VariableType::UnsignedByteNorm);
					cDecompressedTexture = Texture(cDecompressedHeader);

					//Update the texture format.
					utils::getOpenGLFormat(cDecompressedTexture.getPixelFormat(), cDecompressedTexture.getColorSpace(),
					                       cDecompressedTexture.getChannelType(), glInternalFormat, glFormat, glType,
					                       glTypeSize, unused);

					//Do decompression, one surface at a time.
					for (uint32_t uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumMipMapLevels(); ++uiMIPLevel)
					{
						for (uint32_t uiArray = 0; uiArray < textureToUse->getNumArrayMembers(); ++uiArray)
						{
							for (uint32_t uiFace = 0; uiFace < textureToUse->getNumFaces(); ++uiFace)
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

					retval.isDecompressed = true;
				}
				else
				{
					Log(LogLevel::Error, cszUnsupportedFormatDecompressionAvailable, "PVRTC1");
					retval.successful = false;
					return retval;
				}
			}
			break;
		}
#endif
#if defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG) || defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG)
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:
		{
			if (!gl::isGlExtensionSupported("GL_IMG_texture_compression_pvrtc2"))
			{
				Log(LogLevel::Error, cszUnsupportedFormat, "PVRTC2");
				retval.successful = false;
				return retval;
			}
			break;
		}
#endif
#ifdef GL_ETC1_RGB8_OES
		case GL_ETC1_RGB8_OES:
		{
			if (!gl::isGlExtensionSupported("GL_OES_compressed_ETC1_RGB8_texture"))
			{
				{
					Log(LogLevel::Error, cszUnsupportedFormat, "ETC1");
					retval.successful = false;
					return retval;
				}
			}
			break;
		}
#endif
#if !defined(TARGET_OS_IPHONE)
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		{
			if (!gl::isGlExtensionSupported("GL_EXT_texture_compression_dxt1"))
			{
				Log(LogLevel::Error, cszUnsupportedFormat, "DXT1");
				retval.successful = false;
				return retval;
			}
			break;
		}
		case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
		{
			if (!gl::isGlExtensionSupported("GL_ANGLE_texture_compression_dxt3"))
			{
				Log(LogLevel::Error, cszUnsupportedFormat, "DXT3");
				retval.successful = false;
				return retval;
			}
			break;
		}
		case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
		{
			if (!gl::isGlExtensionSupported("GL_ANGLE_texture_compression_dxt5"))
			{
				Log(LogLevel::Error, cszUnsupportedFormat, "DXT5");
				retval.successful = false;
				return retval;
			}
			break;
		}
#endif
#ifdef GL_BGRA_EXT
		case GL_BGRA_EXT:
		{
			if (!gl::isGlExtensionSupported("GL_EXT_texture_format_BGRA8888"))
			{
				//Check if the APPLE extension is available instead of the EXT version.
				if (gl::isGlExtensionSupported("GL_APPLE_texture_format_BGRA8888"))
				{
					//The APPLE extension differs from the EXT extension, and accepts GL_RGBA as the internal format instead.
					glInternalFormat = GL_RGBA;
				}
				else
				{
					Log(LogLevel::Error, cszUnsupportedFormat, "BGRA8888");
					retval.successful = false;
					return retval;
				}
			}
			break;
		}
#endif
		default:
		{
#if defined(GL_COMPRESSED_RGBA_ASTC_3x3x3_OES) || defined(GL_COMPRESSED_RGBA_ASTC_6x6x6_OES)
			//Check ASTC all together for brevity
			if ((glInternalFormat >= GL_COMPRESSED_RGBA_ASTC_3x3x3_OES && glInternalFormat <= GL_COMPRESSED_RGBA_ASTC_6x6x6_OES) ||
			    (glInternalFormat >= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES && glInternalFormat <= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES))
			{
				if (!gl::isGlExtensionSupported("GL_OES_texture_compression_astc"))
				{
					Log(LogLevel::Error, cszUnsupportedFormat, "DXT5");
					retval.successful = false;
					return retval;
				}
			}
#endif
#if defined(GL_COMPRESSED_RGBA_ASTC_4x4_KHR) || defined(GL_COMPRESSED_RGBA_ASTC_12x12_KHR) || defined(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR)  || defined(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR)
			if ((glInternalFormat >= GL_COMPRESSED_RGBA_ASTC_4x4_KHR && glInternalFormat <= GL_COMPRESSED_RGBA_ASTC_12x12_KHR) ||
			    (glInternalFormat >= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR && glInternalFormat <= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR))
			{
				if (!gl::isGlExtensionSupported("GL_KHR_texture_compression_astc_hdr"))
				{
					Log(LogLevel::Error, cszUnsupportedFormat, "DXT5");
					retval.successful = false;
					return retval;
				}
			}
#endif
		}
		}
	}

	// Check the type of texture (e.g. 3D textures).
	{
		// Only 2D Arrays are supported in this API.
		if (textureToUse->getNumArrayMembers() > 1)
		{
#if defined(GL_TEXTURE_2D_ARRAY)
			//Make sure it's not also a cube map or 3D texture, as this is unsupported.
			if (textureToUse->getNumFaces() > 1)
			{
				Log(LogLevel::Error,
				    "TextureUtils.h:textureUpload:: Texture arrays with multiple faces are not supported by this implementation.\n");
				retval.successful = false;
				return retval;
			}
			else if (textureToUse->getDepth() > 1)
			{
				Log(LogLevel::Error,
				    "TextureUtils.h:textureUpload:: 3D Texture arrays are not supported by this implementation.\n");
				retval.successful = false;
				return retval;
			}
			retval.target = GL_TEXTURE_2D_ARRAY;
#else
			Log(LogLevel::Error,
			    "TextureUtils.h:textureUpload:: Texture arrays are not supported by this implementation.\n");
			retval.successful = false;
			return retval;
#endif
		}

		// 3D Cubemaps aren't supported
		if (textureToUse->getDepth() > 1)
		{
#if defined(GL_TEXTURE_3D)
			//Make sure it's not also a cube map, as this is unsupported. We've already checked for arrays so no need to check again.
			if (textureToUse->getNumFaces() > 1)
			{
				Log(LogLevel::Error,
				    "TextureUtils.h:textureUpload:: 3-Dimensional textures with multiple faces are not supported by this implementation.\n");
				retval.successful = false;
				return retval;
			}
			retval.target = GL_TEXTURE_3D;
#else
			Log(LogLevel::Error,
			    "TextureUtils.h:textureUpload:: 3-Dimensional textures are not supported by this implementation.\n");
			retval.successful = false;
			return retval;
#endif
		}

		//Check if it's a Cube Map.
		if (textureToUse->getNumFaces() > 1)
		{
			//Make sure it's a complete cube, otherwise warn the user. We've already checked if it's a 3D texture or a texture array as well.
			if (textureToUse->getNumFaces() < 6)
			{
				Log(LogLevel::Warning,
				    "TextureUtils.h:textureUpload:: Textures with between 2 and 5 faces are unsupported. Faces up to 6 will be allocated in a cube map as undefined surfaces.\n");
			}
			else if (textureToUse->getNumFaces() > 6)
			{
				Log(LogLevel::Warning,
				    "TextureUtils.h:textureUpload:: Textures with more than 6 faces are unsupported. Only the first 6 faces will be loaded into the API.\n");
			}
			retval.target = GL_TEXTURE_CUBE_MAP;
		}
	}

	// Setup the texture object.
	{
		// Check the error here, in case the extension loader or anything else raised any errors.
		debugLogApiError("TextureUtils.h:textureUpload:: GL has raised error from prior to uploading the texture.");

		//Generate a new texture name.
		gl::GenTextures(1, &retval.image);

		//Bind the texture to edit it.
		gl::BindTexture(retval.target, retval.image);

		//Set the unpack alignment to 1 - PVR textures are not stored as padded.
		gl::PixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (needsSwizzling)
		{
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, swizzle_r);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, swizzle_g);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, swizzle_b);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, swizzle_a);
			utils::logApiError("TextureUtils.h:textureUpload:: GL has raised error attempting to swizzle a texture.");
		}


		if (utils::logApiError("TextureUtils.h:textureUpload:: GL has raised error attempting to bind the texture for first use."))
		{
			retval.successful = false;
			gl::BindTexture(retval.target, 0);
			return retval;
		}
	}

	// Load the texture.
	{
		debugLogApiError("TextureUtils.h:textureUpload:: GL has a raised error before attempting to define texture storage.");
		// 2D textures.
		if (retval.target == GL_TEXTURE_2D)
		{
			//Use tex storage if available, to generate an immutable texture.
			if (useTexStorage)
			{
				gl::TexStorage2D(retval.target, textureToUse->getNumMipMapLevels(), glInternalFormat, textureToUse->getWidth(),
				                 textureToUse->getHeight());
				if (utils::logApiError(strings::createFormatted("textureUpload::glTexStorage2D With InternalFormat : % x",
				                       glInternalFormat).c_str()))
				{
					retval.successful = false;
					gl::BindTexture(retval.target, 0);
					return retval;
				}

				for (uint32_t uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumMipMapLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexSubImage2D(retval.target, uiMIPLevel, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                            textureToUse->getHeight(uiMIPLevel), glInternalFormat, textureToUse->getDataSize(uiMIPLevel, false,
						                                false),
						                            textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError("TextureUtils::textureUpload:: glCompressedTexSubImage2D"))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}

					}
					else
					{
						gl::TexSubImage2D(retval.target, uiMIPLevel, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                  textureToUse->getHeight(uiMIPLevel),
						                  glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glTexSubImage2D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
				}
			}
			else
			{
				for (uint32_t uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumMipMapLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexImage2D(retval.target, uiMIPLevel, glInternalFormat,
						                         textureToUse->getWidth(uiMIPLevel),
						                         textureToUse->getHeight(uiMIPLevel), 0,
						                         textureToUse->getDataSize(uiMIPLevel, false, false),
						                         textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glCompressedTexImage2D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
					else
					{
						if (isEs2) { glInternalFormat = glFormat; }
						gl::TexImage2D(retval.target, uiMIPLevel, glInternalFormat, textureToUse->getWidth(uiMIPLevel),
						               textureToUse->getHeight(uiMIPLevel), 0, glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glTexImage2D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
				}
			}
		}
		// Cube maps.
		else if (retval.target == GL_TEXTURE_CUBE_MAP)
		{
			if (useTexStorage)
			{
				//Use tex storage, to generate an immutable texture.
				gl::TexStorage2D(retval.target, textureToUse->getNumMipMapLevels(), glInternalFormat,
				                 textureToUse->getWidth(), textureToUse->getHeight());
				if (utils::logApiError(("TextureUtils::textureUpload::(cubemap) glTexStorage2D")))
				{
					retval.successful = false;
					gl::BindTexture(retval.target, 0);
					return retval;
				}

				for (uint32_t uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumMipMapLevels(); ++uiMIPLevel)
				{
					//Iterate through 6 faces regardless, as these should always be iterated through. We fill in the blanks with uninitialized data for uncompressed textures, or repeat faces for compressed data.
					for (uint32_t uiFace = 0; uiFace < 6; ++uiFace)
					{
						GLenum eTexImageTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
						if (isCompressedFormat)
						{
							//Make sure to wrap texture faces around if there are fewer faces than 6 in a compressed texture.
							gl::CompressedTexSubImage2D(eTexImageTarget + uiFace, uiMIPLevel, 0, 0,
							                            textureToUse->getWidth(uiMIPLevel),
							                            textureToUse->getHeight(uiMIPLevel), glInternalFormat, textureToUse->getDataSize(uiMIPLevel, false,
							                                false),
							                            textureToUse->getDataPointer(uiMIPLevel, 0, uiFace % textureToUse->getNumFaces()));
							if (utils::logApiError(
							      strings::createFormatted("TextureUtils::textureUpload::(cubemap face %d) glCompressedTexSubImage2D",
							                               uiFace).c_str()))
							{
								retval.successful = false;
								gl::BindTexture(retval.target, 0);
								return retval;
							}
						}
						else
						{
							//No need to wrap faces for uncompressed textures, as gl will handle a NULL pointer, which Texture::getDataPtr will do when requesting a non-existant face.
							gl::TexSubImage2D(eTexImageTarget + uiFace, uiMIPLevel, 0, 0, textureToUse->getWidth(uiMIPLevel),
							                  textureToUse->getHeight(uiMIPLevel), glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0,
							                      uiFace % textureToUse->getNumFaces()));
							if (utils::logApiError(
							      strings::createFormatted("TextureUtils::textureUpload::(cubemap face %d) glTexSubImage2D",
							                               uiFace).c_str()))
							{
								retval.successful = false;
								gl::BindTexture(retval.target, 0);
								return retval;
							}
						}
					}
				}
			}
			else
			{
				for (uint32_t uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumMipMapLevels(); ++uiMIPLevel)
				{
					//Iterate through 6 faces regardless, as these should always be iterated through. We fill in the blanks with uninitialized data for uncompressed textures, or repeat faces for compressed data.
					for (uint32_t uiFace = 0; uiFace < 6; ++uiFace)
					{
						GLenum eTexImageTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
						if (isCompressedFormat)
						{
							//Make sure to wrap texture faces around if there are fewer faces than 6 in a compressed texture.
							gl::CompressedTexImage2D(eTexImageTarget + uiFace, uiMIPLevel, glInternalFormat,
							                         textureToUse->getWidth(uiMIPLevel),
							                         textureToUse->getHeight(uiMIPLevel), 0, textureToUse->getDataSize(uiMIPLevel, false, false),
							                         textureToUse->getDataPointer(uiMIPLevel, 0, uiFace % textureToUse->getNumFaces()));
							if (utils::logApiError(
							      strings::createFormatted("TextureUtils::textureUpload::(cubemap face %d) glCompressedTexImage2D",
							                               uiFace).c_str()))
							{
								retval.successful = false;
								gl::BindTexture(retval.target, 0);
								return retval;
							}
						}
						else
						{
							//No need to wrap faces for uncompressed textures, as gl will handle a NULL pointer, which Texture::getDataPtr will do when requesting a non-existant face.
							gl::TexImage2D(eTexImageTarget + uiFace, uiMIPLevel, glInternalFormat,
							               textureToUse->getWidth(uiMIPLevel),
							               textureToUse->getHeight(uiMIPLevel), 0, glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0,
							                   uiFace % textureToUse->getNumFaces()));
							if (utils::logApiError(
							      strings::createFormatted("TextureUtils::textureUpload::(cubemap face %d) glTexImage2D",
							                               uiFace).c_str()))
							{
								retval.successful = false;
								gl::BindTexture(retval.target, 0);
								return retval;
							}
						}
					}
				}
			}
		}
#if defined(GL_TEXTURE_3D)
		// 3D textures
		else if (retval.target == GL_TEXTURE_3D)
		{
			if (useTexStorage)
			{
				//Use tex storage, to generate an immutable texture.
				gl::TexStorage3D(retval.target, textureToUse->getNumMipMapLevels(), glInternalFormat,
				                 textureToUse->getWidth(),
				                 textureToUse->getHeight(), textureToUse->getDepth());
				if (utils::logApiError(("TextureUtils::textureUpload:: glTexStorage3D")))
				{
					retval.successful = false;
					gl::BindTexture(retval.target, 0);
					return retval;
				}
				for (uint32_t uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumMipMapLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexSubImage3D(retval.target, uiMIPLevel, 0, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                            textureToUse->getHeight(uiMIPLevel), textureToUse->getDepth(uiMIPLevel), glInternalFormat,
						                            textureToUse->getDataSize(uiMIPLevel, false, false), textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glCompressedTexSubImage3D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
					else
					{
						gl::TexSubImage3D(retval.target, uiMIPLevel, 0, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                  textureToUse->getHeight(uiMIPLevel),
						                  textureToUse->getDepth(uiMIPLevel), glFormat, glType, textureToUse->getDataPointer(uiMIPLevel, 0,
						                      0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glTexSubImage3D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
				}
			}
			else
			{
				for (uint32_t uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumMipMapLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexImage3D(retval.target, uiMIPLevel, glInternalFormat,
						                         textureToUse->getWidth(uiMIPLevel),
						                         textureToUse->getHeight(uiMIPLevel), textureToUse->getDepth(uiMIPLevel), 0,
						                         textureToUse->getDataSize(uiMIPLevel, false,
						                             false), textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glCompressedTexImage3D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
					else
					{
						gl::TexImage3D(retval.target, uiMIPLevel, glInternalFormat, textureToUse->getWidth(uiMIPLevel),
						               textureToUse->getHeight(uiMIPLevel), textureToUse->getDepth(uiMIPLevel), 0, glFormat, glType,
						               textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glTexImage3D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
				}
			}
		}
#endif
#if defined(GL_TEXTURE_2D_ARRAY)
		// Texture arrays.
		else if (retval.target == GL_TEXTURE_2D_ARRAY)
		{
			if (useTexStorage)
			{
				//Use tex storage, to generate an immutable texture.
				gl::TexStorage3D(retval.target, textureToUse->getNumMipMapLevels(), glInternalFormat,
				                 textureToUse->getWidth(),
				                 textureToUse->getHeight(), textureToUse->getNumArrayMembers());

				for (uint32_t uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumMipMapLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexSubImage3D(retval.target, uiMIPLevel, 0, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                            textureToUse->getHeight(uiMIPLevel), textureToUse->getNumArrayMembers(), glInternalFormat,
						                            textureToUse->getDataSize(uiMIPLevel, false, false), textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glCompressedTexSubImage3D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
					else
					{
						gl::TexSubImage3D(retval.target, uiMIPLevel, 0, 0, 0, textureToUse->getWidth(uiMIPLevel),
						                  textureToUse->getHeight(uiMIPLevel),
						                  textureToUse->getNumArrayMembers(), glFormat, glType, textureToUse->getDataPointer(uiMIPLevel,
						                      0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glTexSubImage3D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
				}
			}
			else
			{
				for (uint32_t uiMIPLevel = 0; uiMIPLevel < textureToUse->getNumMipMapLevels(); ++uiMIPLevel)
				{
					if (isCompressedFormat)
					{
						gl::CompressedTexImage3D(retval.target, uiMIPLevel, glInternalFormat,
						                         textureToUse->getWidth(uiMIPLevel),
						                         textureToUse->getHeight(uiMIPLevel), textureToUse->getNumArrayMembers(), 0,
						                         textureToUse->getDataSize(uiMIPLevel,
						                             false, false), textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glCompressedTexImage3D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
					else
					{
						gl::TexImage3D(retval.target, uiMIPLevel, glInternalFormat, textureToUse->getWidth(uiMIPLevel),
						               textureToUse->getHeight(uiMIPLevel), textureToUse->getNumArrayMembers(), 0, glFormat, glType,
						               textureToUse->getDataPointer(uiMIPLevel, 0, 0));
						if (utils::logApiError(("TextureUtils::textureUpload:: glTexImage3D")))
						{
							retval.successful = false;
							gl::BindTexture(retval.target, 0);
							return retval;
						}
					}
				}
			}
		}
#endif
		else
		{
			Log(LogLevel::Debug,
			    "TextureUtilsGLES3 : TextureUpload : File corrupted or suspected bug : unknown texture target type.");
		}
	}

	gl::BindTexture(retval.target, 0);

	retval.successful = true;
	return retval;
}
}//namespace utils
}// namespace pvr
//!\endcond
