/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\ConvertToApiTypes.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of the conversions from PVR Framework enums to OpenGL types. See ConvertToApiTypes.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRCore/Assert_.h"

namespace pvr {
using namespace types;
namespace api  {
namespace ConvertToGles {

bool getOpenGLFormat(PixelFormat pixelFormat, pvr::types::ColorSpace colorSpace,
                     pvr::VariableType dataType, uint32& glInternalFormat,
                     uint32& glFormat, uint32& glType,
                     uint32& glTypeSize, bool& isCompressedFormat)
{
	isCompressedFormat = (pixelFormat.getPart().High == 0)
	                     && (pixelFormat.getPixelTypeId() != (uint64)CompressedPixelFormat::SharedExponentR9G9B9E5);
	if (pixelFormat.getPart().High == 0)
	{
		//Format and type == 0 for compressed textures.
		glFormat = 0;
		glType = 0;
		glTypeSize = 1;
		switch (pixelFormat.getPixelTypeId())
		{
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGB:
		{
			glInternalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
			return true;
		}
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGBA:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
			return true;
		}
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGB:
		{
			glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			return true;
		}
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGBA:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
			return true;
		}
#ifdef GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG
		case (uint64)CompressedPixelFormat::PVRTCII_2bpp:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
			return true;
		}
#endif
#ifdef GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG
		case (uint64)CompressedPixelFormat::PVRTCII_4bpp:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
			return true;
		}
#endif
#ifdef GL_ETC1_RGB8_OES
		case (uint64)CompressedPixelFormat::ETC1:
		{
			glInternalFormat = GL_ETC1_RGB8_OES;
			return true;
		}
#endif
#ifdef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
		case (uint64)CompressedPixelFormat::DXT1:
		{
			glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			return true;
		}
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE
		case (uint64)CompressedPixelFormat::DXT2:
		case (uint64)CompressedPixelFormat::DXT3:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE;
			return true;
		}
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE
		case (uint64)CompressedPixelFormat::DXT4:
		case (uint64)CompressedPixelFormat::DXT5:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE;
			return true;
		}
#endif
		case (uint64)CompressedPixelFormat::SharedExponentR9G9B9E5:
		{
			//Not technically a compressed format by OpenGL ES standards.
			glType = GL_UNSIGNED_INT_5_9_9_9_REV;
			glTypeSize = 4;
			glFormat = GL_RGB;
			glInternalFormat = GL_RGB9_E5;
			return true;
		}
		case (uint64)CompressedPixelFormat::ETC2_RGB:
		{
			if (colorSpace == ColorSpace::sRGB)
			{
				glInternalFormat = GL_COMPRESSED_SRGB8_ETC2;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_ETC2;
			};
			return true;
		}
		case (uint64)CompressedPixelFormat::ETC2_RGBA:
		{
			if (colorSpace == ColorSpace::sRGB)
			{
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			}
			return true;
		}
		case (uint64)CompressedPixelFormat::ETC2_RGB_A1:
		{
			if (colorSpace == ColorSpace::sRGB)
			{
				glInternalFormat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			return true;
		}
		case (uint64)CompressedPixelFormat::EAC_R11:
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm ||
			    dataType == VariableType::SignedShort || dataType == VariableType::SignedShortNorm ||
			    dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm ||
			    dataType == VariableType::SignedFloat)
			{
				glInternalFormat = GL_COMPRESSED_SIGNED_R11_EAC;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_R11_EAC;
			}
			return true;
		}
		case (uint64)CompressedPixelFormat::EAC_RG11:
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm ||
			    dataType == VariableType::SignedShort || dataType == VariableType::SignedShortNorm ||
			    dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm ||
			    dataType == VariableType::SignedFloat)
			{
				glInternalFormat = GL_COMPRESSED_SIGNED_RG11_EAC;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RG11_EAC;
			}
			return true;
		}
		//Formats not supported by opengl/opengles
		case (uint64)CompressedPixelFormat::BC4:
		case (uint64)CompressedPixelFormat::BC5:
		case (uint64)CompressedPixelFormat::BC6:
		case (uint64)CompressedPixelFormat::BC7:
		case (uint64)CompressedPixelFormat::RGBG8888:
		case (uint64)CompressedPixelFormat::GRGB8888:
		case (uint64)CompressedPixelFormat::UYVY:
		case (uint64)CompressedPixelFormat::YUY2:
		case (uint64)CompressedPixelFormat::BW1bpp:
		case (uint64)CompressedPixelFormat::NumCompressedPFs:
			return false;
		}
	}
	else
	{
		switch (dataType)
		{
		case VariableType::UnsignedFloat:
			if (pixelFormat.getPixelTypeId() == assets::GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID)
			{
				glTypeSize = 4;
				glType = GL_UNSIGNED_INT_10F_11F_11F_REV;
				glFormat = GL_RGB;
				glInternalFormat = GL_R11F_G11F_B10F;
				return true;
			}
			break;
		case VariableType::SignedFloat:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			//HALF_FLOAT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA16F;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB16F;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RG;
				glInternalFormat = GL_RG16F;
				return true;
			}
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R16F;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA16F_EXT;
				return true;
			}
			case assets::GeneratePixelType1<'l', 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE16F_EXT;
				return true;
			}
			case assets::GeneratePixelType1<'a', 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R16F;
				return true;
			}
			//FLOAT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA32F;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB32F;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RG;
				glInternalFormat = GL_RG32F;
				return true;
			}
			case assets::GeneratePixelType1<'r', 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R32F;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA32F_EXT;
				return true;
			}
			case assets::GeneratePixelType1<'l', 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE32F_EXT;
				return true;
			}
			case assets::GeneratePixelType1<'a', 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R32F;
				return true;
			}
			case assets::GeneratePixelType1<'d', 16>::ID:
			{
				glType = GL_UNSIGNED_SHORT;
				glTypeSize = 2;
				glInternalFormat = GL_DEPTH_COMPONENT16;
				glFormat = GL_DEPTH_COMPONENT;
				return true;
			}
			case assets::GeneratePixelType1<'d', 24>::ID:
			{
				glType = GL_UNSIGNED_INT;
				glTypeSize = 3;
				glInternalFormat = GL_DEPTH_COMPONENT24;
				glFormat = GL_DEPTH_COMPONENT;
				return true;
			}
			case assets::GeneratePixelType2<'d', 's', 24, 8>::ID:
			{
				glType = GL_UNSIGNED_INT_24_8;
				glTypeSize = 4;
				glInternalFormat = GL_DEPTH24_STENCIL8;
				glFormat = GL_DEPTH_STENCIL;
				return true;
			}
			case assets::GeneratePixelType2<'d', 's', 32, 8>::ID:
			{
				glType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
				glTypeSize = 5;
				glInternalFormat = GL_DEPTH32F_STENCIL8;
				glFormat = GL_DEPTH_STENCIL;
				return true;
			}
			case assets::GeneratePixelType1<'d', 32>::ID:
			{
				glType = GL_FLOAT;
				glTypeSize = 4;
				glInternalFormat = GL_DEPTH_COMPONENT32F;
				glFormat = GL_DEPTH_COMPONENT;
				return true;
			}
			case assets::GeneratePixelType1<'s', 8>::ID:
			{
				glTypeSize = 4;
				glInternalFormat = GL_STENCIL_INDEX8;
				glFormat = GL_DEPTH_STENCIL;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedByteNorm:
		{
			glType = GL_UNSIGNED_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA;
				if (colorSpace == ColorSpace::sRGB)
				{
					glInternalFormat = GL_SRGB8_ALPHA8;
				}
				else
				{
					glInternalFormat = GL_RGBA8;
				}
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = glInternalFormat = GL_RGB;
#ifdef GL_SRGB8
				if (colorSpace == ColorSpace::sRGB)
				{
					glInternalFormat = GL_SRGB8;
				}
				else
				{
					glInternalFormat = GL_RGB8;
				}
#endif
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RG8;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R8;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 8>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			case assets::GeneratePixelType1<'a', 8>::ID:
			{
				glFormat = GL_ALPHA;
				glInternalFormat = GL_ALPHA;
				return true;
			}
			case assets::GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_BGRA_EXT;
				glInternalFormat = GL_BGRA_EXT;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedByteNorm:
		{
			glType = GL_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA8_SNORM;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB8_SNORM;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RG8_SNORM;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R8_SNORM;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 8>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			case assets::GeneratePixelType1<'a', 8>::ID:
			{
				glFormat = GL_ALPHA;
				glInternalFormat = GL_ALPHA;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedByte:
		{
			glType = GL_UNSIGNED_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				//TO INVESTIGATE - This should be GL_RGBA?
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA8UI;
				//glInternalFormat = GL_RGBA;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				//TO INVESTIGATE - This should be GL_RGBA?
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB8UI;
				//glInternalFormat = GL_RGB;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG8UI;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R8UI;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedByte:
		{
			glType = GL_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA8I;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB8I;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG8I;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R8I;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedShortNorm:
		{
			glType = GL_UNSIGNED_SHORT;
			glTypeSize = 2;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID:
			{
				glType = GL_UNSIGNED_SHORT_4_4_4_4;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA4;
				return true;
			}
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID:
			{
				glType = GL_UNSIGNED_SHORT_5_5_5_1;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGB5_A1;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
			{
				glType = GL_UNSIGNED_SHORT_5_6_5;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB565;
				return true;
			}
#ifdef GL_RGBA16_EXT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA16_EXT;
				return true;
			}
#endif
#ifdef GL_RGBA16_EXT
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB16_EXT;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RGB16_EXT;
				return true;
			}
#endif
#ifdef GL_R16_EXT
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R16_EXT;
				return true;
			}
#endif
			case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 16>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			case assets::GeneratePixelType1<'a', 16>::ID:
			{
				glFormat = GL_ALPHA;
				glInternalFormat = GL_ALPHA16F_EXT;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedShortNorm:
		{
			glTypeSize = 2;
			glType = GL_SHORT;
			switch (pixelFormat.getPixelTypeId())
			{
#ifdef GL_RGBA16_SNORM_EXT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA16_SNORM_EXT;
				return true;
			}
#endif
#ifdef GL_RGB16_SNORM_EXT
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB16_SNORM_EXT;
				return true;
			}
#endif
#ifdef GL_RG16_SNORM_EXT
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RG16_SNORM_EXT;
				return true;
			}
#endif
#ifdef GL_R16_SNORM_EXT
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R16_SNORM_EXT;
				return true;
			}
#endif
			case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 16>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
				//case assets::GeneratePixelType1<'a', 16>::ID:
				//{
				//  glFormat =  GL_ALPHA;
				//  glInternalFormat = GL_ALPHA16_SNORM;
				//  return true;
				//}
			}
			break;
		}
		case VariableType::UnsignedShort:
		{
			glType = GL_UNSIGNED_SHORT;
			glTypeSize = 2;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA16UI;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB16UI;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG16UI;
				return true;
			}
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R16UI;
				return true;
			}
			case assets::GeneratePixelType1<'d', 16>::ID:
			{
				glFormat = GL_DEPTH_COMPONENT;
				glInternalFormat = GL_DEPTH_COMPONENT16;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedShort:
		{
			glType = GL_SHORT;
			glTypeSize = 2;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA16I;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB16I;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG16I;
				return true;
			}
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R16I;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedIntegerNorm:
		{
			glTypeSize = 4;
			if (pixelFormat.getPixelTypeId() == assets::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID)
			{
				glType = GL_UNSIGNED_INT_2_10_10_10_REV;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGB10_A2;
				return true;
			}
#ifdef GL_RGB10_EXT
			if (pixelFormat.getPixelTypeId() == assets::GeneratePixelType4<'x', 'b', 'g', 'r', 2, 10, 10, 10>::ID)
			{
				glType = GL_UNSIGNED_INT_2_10_10_10_REV;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB10_EXT;
				return true;
			}
#endif
			break;
		}
		case VariableType::UnsignedInteger:
		{
			glType = GL_UNSIGNED_INT;
			glTypeSize = 4;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA32UI;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB32UI;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG32UI;
				return true;
			}
			case assets::GeneratePixelType1<'r', 32>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R32UI;
				return true;
			}
			case assets::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
			{
				glType = GL_UNSIGNED_INT_2_10_10_10_REV;
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGB10_A2UI;
				return true;
			}
			case assets::GeneratePixelType1<'d', 24>::ID:
			{
				glFormat = GL_DEPTH_COMPONENT;
#if defined(BUILD_API_MAX)&&BUILD_API_MAX<30
				glInternalFormat = GL_DEPTH_COMPONENT24_OES;
#else
				glInternalFormat = GL_DEPTH_COMPONENT24;
#endif
				return true;
			}
			case assets::GeneratePixelType2<'d', 's', 24, 8>::ID:
			{
#if defined(BUILD_API_MAX)&&BUILD_API_MAX<30
				glFormat = GL_DEPTH_STENCIL_OES;
				glInternalFormat = GL_DEPTH24_STENCIL8_OES;
#else
				glFormat = GL_DEPTH_STENCIL;
				glInternalFormat = GL_DEPTH24_STENCIL8;
#endif
				return true;
			}
			}
			break;
		}
		case VariableType::SignedInteger:
		{
			glType = GL_INT;
			glTypeSize = 4;
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA32I;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB32I;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG32I;
				return true;
			}
			case assets::GeneratePixelType1<'r', 32>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R32I;
				return true;
			}
			}
			break;
		}
		default: {}
		}
	}
	//Default (erroneous) return values.
	glTypeSize = glType = glFormat = glInternalFormat = 0;
	return false;
}

bool getOpenGLStorageFormat(PixelFormat pixelFormat, pvr::types::ColorSpace colorSpace,
                            VariableType dataType, GLenum& glInternalFormat)
{
	if (pixelFormat.getPart().High == 0)
	{
		//Format and type == 0 for compressed textures.
		switch (pixelFormat.getPixelTypeId())
		{
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGB:
		{
			glInternalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
			return true;
		}
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGBA:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
			return true;
		}
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGB:
		{
			glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			return true;
		}
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGBA:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
			return true;
		}
#ifdef GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG
		case (uint64)CompressedPixelFormat::PVRTCII_2bpp:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
			return true;
		}
#endif
#ifdef GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG
		case (uint64)CompressedPixelFormat::PVRTCII_4bpp:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
			return true;
		}
#endif
#ifdef GL_ETC1_RGB8_OES
		case (uint64)CompressedPixelFormat::ETC1:
		{
			glInternalFormat = GL_ETC1_RGB8_OES;
			return true;
		}
#endif
#ifdef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
		case (uint64)CompressedPixelFormat::DXT1:
		{
			glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			return true;
		}
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE
		case (uint64)CompressedPixelFormat::DXT2:
		case (uint64)CompressedPixelFormat::DXT3:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE;
			return true;
		}
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE
		case (uint64)CompressedPixelFormat::DXT4:
		case (uint64)CompressedPixelFormat::DXT5:
		{
			glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE;
			return true;
		}
#endif
		case (uint64)CompressedPixelFormat::SharedExponentR9G9B9E5:
		{
			//Not technically a compressed format by OpenGL ES standards.
			glInternalFormat = GL_RGB9_E5;
			return true;
		}
		case (uint64)CompressedPixelFormat::ETC2_RGB:
		{
			if (colorSpace == ColorSpace::sRGB)
			{
				glInternalFormat = GL_COMPRESSED_SRGB8_ETC2;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_ETC2;
			};
			return true;
		}
		case (uint64)CompressedPixelFormat::ETC2_RGBA:
		{
			if (colorSpace == ColorSpace::sRGB)
			{
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			}
			return true;
		}
		case (uint64)CompressedPixelFormat::ETC2_RGB_A1:
		{
			if (colorSpace == ColorSpace::sRGB)
			{
				glInternalFormat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			return true;
		}
		case (uint64)CompressedPixelFormat::EAC_R11:
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm ||
			    dataType == VariableType::SignedShort || dataType == VariableType::SignedShortNorm ||
			    dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm ||
			    dataType == VariableType::SignedFloat)
			{
				glInternalFormat = GL_COMPRESSED_SIGNED_R11_EAC;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_R11_EAC;
			}
			return true;
		}
		case (uint64)CompressedPixelFormat::EAC_RG11:
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm ||
			    dataType == VariableType::SignedShort || dataType == VariableType::SignedShortNorm ||
			    dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm ||
			    dataType == VariableType::SignedFloat)
			{
				glInternalFormat = GL_COMPRESSED_SIGNED_RG11_EAC;
			}
			else
			{
				glInternalFormat = GL_COMPRESSED_RG11_EAC;
			}
			return true;
		}
		//Formats not supported by opengl/opengles
		case (uint64)CompressedPixelFormat::BC4:
		case (uint64)CompressedPixelFormat::BC5:
		case (uint64)CompressedPixelFormat::BC6:
		case (uint64)CompressedPixelFormat::BC7:
		case (uint64)CompressedPixelFormat::RGBG8888:
		case (uint64)CompressedPixelFormat::GRGB8888:
		case (uint64)CompressedPixelFormat::UYVY:
		case (uint64)CompressedPixelFormat::YUY2:
		case (uint64)CompressedPixelFormat::BW1bpp:
		case (uint64)CompressedPixelFormat::NumCompressedPFs:
			return false;
		}
	}
	else
	{
		switch (dataType)
		{
		case VariableType::UnsignedFloat:
			if (pixelFormat.getPixelTypeId() == assets::GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID)
			{
				glInternalFormat = GL_R11F_G11F_B10F;
				return true;
			}
			break;
		case VariableType::SignedFloat:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			//HALF_FLOAT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16F;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16F;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16F;
				return true;
			}
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16F;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			case assets::GeneratePixelType1<'a', 16>::ID:
			{
				glInternalFormat = GL_ALPHA16F_EXT;
				return true;
			}
			//FLOAT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGBA32F;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGB32F;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glInternalFormat = GL_RG32F;
				return true;
			}
			case assets::GeneratePixelType1<'r', 32>::ID:
			{
				glInternalFormat = GL_R32F;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 32, 32>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 32>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			case assets::GeneratePixelType1<'a', 32>::ID:
			{
				glInternalFormat = GL_ALPHA32F_EXT;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedByteNorm:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::sRGB)
				{
					glInternalFormat = GL_SRGB8_ALPHA8;
				}
				else
				{
					glInternalFormat = GL_RGBA8;
				}
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::sRGB)
				{
					glInternalFormat = GL_SRGB8;
				}
				else
				{
					glInternalFormat = GL_RGB8;
				}
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glInternalFormat = GL_RG8;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glInternalFormat = GL_R8;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 8>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			case assets::GeneratePixelType1<'a', 8>::ID:
			{
				glInternalFormat = GL_ALPHA8_EXT;
				return true;
			}
			case assets::GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID:
			{
				glInternalFormat = GL_BGRA8_EXT;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedByteNorm:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGBA8_SNORM;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGB8_SNORM;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glInternalFormat = GL_RG8_SNORM;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glInternalFormat = GL_R8_SNORM;
				return true;
			}
			case assets::GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 8>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedByte:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGBA8UI;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGB8UI;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glInternalFormat = GL_RG8UI;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glInternalFormat = GL_R8UI;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedByte:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGBA8I;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGB8I;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glInternalFormat = GL_RG8I;
				return true;
			}
			case assets::GeneratePixelType1<'r', 8>::ID:
			{
				glInternalFormat = GL_R8I;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedShortNorm:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID:
			{
				glInternalFormat = GL_RGBA4;
				return true;
			}
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID:
			{
				glInternalFormat = GL_RGB5_A1;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
			{
				glInternalFormat = GL_RGB565;
				return true;
			}
#ifdef GL_RGBA16_EXT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16_EXT;
				return true;
			}
#endif
#ifdef GL_RGB16_EXT
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16_EXT;
				return true;
			}
#endif
#ifdef GL_RG16_EXT
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16_EXT;
				return true;
			}
#endif
#ifdef GL_R16_EXT
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16_EXT;
				return true;
			}
#endif
			case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
			case assets::GeneratePixelType1<'a', 16>::ID:
			{
				glInternalFormat = GL_ALPHA;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedShortNorm:
		{
			switch (pixelFormat.getPixelTypeId())
			{
#ifdef GL_RGBA16_SNORM_EXT
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16_SNORM_EXT;
				return true;
			}
#endif
#ifdef GL_RGB16_SNORM_EXT
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16_SNORM_EXT;
				return true;
			}
#endif
#ifdef GL_RG16_SNORM_EXT
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16_SNORM_EXT;
				return true;
			}
#endif
#ifdef GL_R16_SNORM_EXT
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16_SNORM_EXT;
				return true;
			}
#endif
			case assets::GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return true;
			}
			case assets::GeneratePixelType1<'l', 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return true;
			}
				//case assets::GeneratePixelType1<'a', 16>::ID:
				//{
				//  glInternalFormat = GL_ALPHA16_SNORM;
				//  return true;
				//}
			}
			break;
		}
		case VariableType::UnsignedShort:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16UI;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16UI;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16UI;
				return true;
			}
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16UI;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedShort:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16I;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16I;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16I;
				return true;
			}
			case assets::GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16I;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedIntegerNorm:
		{
			if (pixelFormat.getPixelTypeId() == assets::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID)
			{
				glInternalFormat = GL_RGB10_A2;
				return true;
			}
#ifdef GL_RGB10_EXT
			if (pixelFormat.getPixelTypeId() == assets::GeneratePixelType4<'x', 'b', 'g', 'r', 2, 10, 10, 10>::ID)
			{
				glInternalFormat = GL_RGB10_EXT;
				return true;
			}
			break;
#endif
		}
		case VariableType::UnsignedInteger:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGBA32UI;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGB32UI;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glInternalFormat = GL_RG32UI;
				return true;
			}
			case assets::GeneratePixelType1<'r', 32>::ID:
			{
				glInternalFormat = GL_R32UI;
				return true;
			}
			case assets::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
			{
				glInternalFormat = GL_RGB10_A2UI;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedInteger:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case assets::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGBA32I;
				return true;
			}
			case assets::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGB32I;
				return true;
			}
			case assets::GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glInternalFormat = GL_RG32I;
				return true;
			}
			case assets::GeneratePixelType1<'r', 32>::ID:
			{
				glInternalFormat = GL_R32I;
				return true;
			}
			}
			break;
		}
		default: {}
		}
	}

	//Default (erroneous) return values.
	glInternalFormat = 0;
	return false;
}

GLenum face(Face face)
{
	static GLenum glFace[] = { GL_NONE, GL_FRONT, GL_BACK, GL_FRONT_AND_BACK };
	return glFace[(uint32)face];
}

GLenum polygonWindingOrder(PolygonWindingOrder order)
{
	static GLenum glWindingOrder[] = { GL_CCW , GL_CW};
	return glWindingOrder[(uint32)order];
}

GLenum comparisonMode(ComparisonMode func)
{
	static GLenum glCompareMode[] = { GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS};
	return glCompareMode[(uint32)func];
}

GLenum imageAspect(ImageAspect type)
{
#if BUILD_API_MAX<30
	if (type == ImageAspect::DepthAndStencil)
	{
		Log(Log.Error, "DEPTH_STENCIL_ATTACHMENT not supported in OpenGL ES 2.0");
		return 0;
	}
#endif
	switch (type)
	{
	case ImageAspect::Color: return GL_COLOR_ATTACHMENT0;
	case ImageAspect::Depth: return GL_DEPTH_ATTACHMENT;
	case ImageAspect::Stencil: return GL_STENCIL_ATTACHMENT;
	case ImageAspect::DepthAndStencil: return GL_DEPTH_STENCIL_ATTACHMENT;
	default: pvr::assertion(0, "Invalid image aspect type"); return  GL_COLOR_ATTACHMENT0;
	}
}

GLenum fboTextureAttachmentTexType(FboTextureTarget type)
{
	static GLenum glTexType[] =
	{
		GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	return glTexType[(uint32)type];
}

GLenum textureViewType(ImageViewType texType)
{
#if BUILD_API_MAX<30
	static GLenum glTextureType[] = { GL_NONE, GL_NONE, GL_TEXTURE_2D, GL_TEXTURE_3D_OES, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE, GL_TEXTURE_EXTERNAL_OES };
#else
	static GLenum glTextureType[] = { GL_NONE, GL_NONE, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE,
#ifdef GL_TEXTURE_EXTERNAL_OES
        GL_TEXTURE_EXTERNAL_OES
#else
        GL_NONE
#endif
    };
#endif

	return glTextureType[(uint32)texType];
}

GLenum dataType(DataType dataType)
{
	static const GLenum map[] = { GL_NONE, GL_FLOAT, GL_INT, GL_UNSIGNED_SHORT, GL_RGBA,
	                              GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_FIXED,
	                              GL_UNSIGNED_BYTE, GL_SHORT, GL_SHORT,
	                              GL_BYTE, GL_BYTE,
	                              GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT,
	                              GL_UNSIGNED_INT, GL_NONE
	                            };
	return map[(int)dataType];
}

GLenum samplerWrap(SamplerWrap samplerWrap)
{
	if (samplerWrap > SamplerWrap::Clamp)
	{
		static const char* wrapNames[] = { "Border", "MirrorClamp" };
		pvr::Log("%s SamplerWrap '%s' not support falling back to default", wrapNames[(uint32)samplerWrap - (uint32)SamplerWrap::Border]);
		samplerWrap = SamplerWrap::Default;
	}
	const static GLenum glSampler[] = { GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE };
	return glSampler[(uint32)samplerWrap];
}

GLenum stencilOp(StencilOp stencilOp)
{
	static GLenum glStencilOpValue[] = { GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_DECR, GL_INVERT, GL_INCR_WRAP, GL_DECR_WRAP };
	return glStencilOpValue[(uint32)stencilOp];
}

GLenum blendEq(BlendOp blendOp)
{
	static GLenum glOp[] = {GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX };
	return glOp[(uint32)blendOp];
}

GLenum blendFactor(BlendFactor blendFactor)
{
	static GLenum glBlendFact[] = { GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
	                                GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
	                                GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_SRC_ALPHA_SATURATE
	                              };
	return glBlendFact[(uint32)blendFactor];
}


GLenum memBarrierFlagOut(uint32 mask)
{
#if defined(GL_FRAMEBUFFER_BARRIER_BIT)/*if on eof them is supported then the rest of the barrier bits must be supported*/
	uint32 result = 0;
	if ((mask & (uint32)(AccessFlags::InputAttachmentRead | AccessFlags::ColorAttachmentRead | AccessFlags::DepthStencilAttachmentRead)) != 0)
	{
		result |= GL_FRAMEBUFFER_BARRIER_BIT;
	}

	if ((mask & (uint32)AccessFlags::IndexRead) != 0) { result |= GL_ELEMENT_ARRAY_BARRIER_BIT; }
	if ((mask & (uint32)AccessFlags::IndirectCommandRead) != 0) { result |= GL_COMMAND_BARRIER_BIT; }
	if ((mask & (uint32)(AccessFlags::MemoryRead | AccessFlags::MemoryWrite | AccessFlags::HostRead)) != 0)
	{
		result |= (GL_BUFFER_UPDATE_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT
		           | GL_PIXEL_BUFFER_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		           | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT);
	}
	if ((mask & (uint32)(AccessFlags::ShaderRead | AccessFlags::ShaderWrite)) != 0)
	{
		result |= (GL_SHADER_STORAGE_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
		result |= (GL_BUFFER_UPDATE_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT
		           | GL_PIXEL_BUFFER_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	if ((mask & (uint32)AccessFlags::UniformRead) != 0)
	{
		result |= (GL_UNIFORM_BARRIER_BIT);
	}
	if ((mask & (uint32)AccessFlags::VertexAttributeRead) != 0)
	{
		result |= (GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	}
	return GLenum(result);
#else
	Log(Log.Error, "MemBarrierFlagout: MemBarrierFlagout not built into PVRApi (BUILD_API_MAX<=30)");
	return 0;
#endif
}

GLenum drawPrimitiveType(PrimitiveTopology primitiveType)
{
#if BUILD_API_MAX<31
	static GLenum glPrimtiveType[] = { GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN };
	if (primitiveType > PrimitiveTopology::TriangleFan)
	{
		Log(Log.Error,
		    "drawPrimitiveType: Primitive type not supported at this API level (BUILD_API_MAX is defined and BUILD_API_MAX<31)");
	}
#else

	static GLenum glPrimtiveType[] =
	{
		GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN,
		GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_PATCHES_EXT, GL_QUADS_EXT, GL_ISOLINES_EXT
	};
#endif

	return glPrimtiveType[(uint32)primitiveType];
}

GLenum gpuCapabilitiesTextureAndSamplers(gpuCapabilities::TextureAndSamplers capabilities)
{
	debug_assertion(capabilities < gpuCapabilities::TextureAndSamplers::Count , "Invalid GpuCapabilities TextureAndSamplers");
#if BUILD_API_MAX<30
	bool capsSupported = (capabilities != gpuCapabilities::TextureAndSamplers::MaxSamples &&
	                      capabilities != gpuCapabilities::TextureAndSamplers::Max3DTextureSize &&
	                      capabilities != gpuCapabilities::TextureAndSamplers::MaxArrayTextureLayer &&
	                      capabilities != gpuCapabilities::TextureAndSamplers::MaxTextureLodBias);
	if (!capsSupported) { Log(Log.Error, "GpuCapabilities: Specified capability queried not supported for OpenGL ES 2"); }
	assertion(capsSupported && capabilities < gpuCapabilities::TextureAndSamplers::Count, "Invalid GpuCapabilities");
	static const GLenum glCapabilities[] =
	{
		GL_MAX_TEXTURE_IMAGE_UNITS, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_MAX_TEXTURE_SIZE, GL_MAX_CUBE_MAP_TEXTURE_SIZE
	};
#else
	static const GLenum glCapabilities[] =
	{
		GL_MAX_TEXTURE_IMAGE_UNITS, GL_MAX_SAMPLES, GL_MAX_3D_TEXTURE_SIZE, GL_MAX_ARRAY_TEXTURE_LAYERS, GL_MAX_TEXTURE_LOD_BIAS, GL_MAX_TEXTURE_SIZE, GL_MAX_CUBE_MAP_TEXTURE_SIZE
	};
#endif
	return glCapabilities[(uint32)capabilities];
}

GLenum gpuCapabilitiesTransformFeedback(gpuCapabilities::TransformFeedback caps)
{
	debug_assertion(caps < gpuCapabilities::TransformFeedback::Count , "Invalid GpuCapabilities TransformFeedback");
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities::TransformFeedback: TransformFeedback not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] =
	{
		GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS,
		GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
		GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS
	};
	return glCaps[(uint32)caps];
#endif
}

GLenum gpuCapabilitiesFragment(gpuCapabilities::FragmentShader caps)
{
	debug_assertion(caps < gpuCapabilities::FragmentShader::Count, "Invalid FragmentShader");
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities::Fragment Shader capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = { GL_MAX_FRAGMENT_INPUT_COMPONENTS,
	                                 GL_MAX_FRAGMENT_UNIFORM_BLOCKS,
	                                 GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
	                               };
	return glCaps[(uint32)caps];
#endif
}

GLenum gpuCapabilitiesUniform(gpuCapabilities::Uniform caps)
{
	debug_assertion(caps < gpuCapabilities::Uniform::Count , "Invalid GpuCapabilities Uniform");
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities::Uniform capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = {GL_MAX_UNIFORM_BLOCK_SIZE,
	                                GL_MAX_UNIFORM_BUFFER_BINDINGS, GL_MAX_COMBINED_UNIFORM_BLOCKS,
	                                GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS
	                               };
	return glCaps[(uint32)caps];
#endif
}

GLenum gpuCapabilitiesElement(gpuCapabilities::Element caps)
{
	debug_assertion(caps < gpuCapabilities::Element::Count , "Invalid GpuCapabilities Element");
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities: Element capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = { GL_MAX_ELEMENT_INDEX, GL_MAX_ELEMENTS_VERTICES };
	return glCaps[(uint32)caps];
#endif
}

GLenum gpuCapabilitiesBuffers(gpuCapabilities::Buffers caps)
{
	debug_assertion(caps < gpuCapabilities::Buffers::Count , "Invalid GpuCapabilities Buffers");
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities: Buffers capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = {GL_MAX_DRAW_BUFFERS};
	return glCaps[(uint32)caps];
#endif
}

GLenum gpuCapabilitiesShaderAndPrograms(gpuCapabilities::ShaderAndProgram caps)
{
	debug_assertion(caps < gpuCapabilities::ShaderAndProgram::Count , "Invalid GpuCapabilities ShaderAndProgram");
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities: Shaders and Programs capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = { GL_MAX_PROGRAM_TEXEL_OFFSET, GL_MIN_PROGRAM_TEXEL_OFFSET, GL_NUM_COMPRESSED_TEXTURE_FORMATS,
	                                 GL_NUM_SHADER_BINARY_FORMATS, GL_NUM_PROGRAM_BINARY_FORMATS
	                               };
	return glCaps[(uint32)caps];
#endif
}
}
}
}
//!\endcond
