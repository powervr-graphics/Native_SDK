/*!
\brief Implementation of methods of the TextureReaderKTX class.
\file PVRAssets/FileIO/TextureReaderKTX.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureReaderKTX.h"
#include "PVRCore/Texture/FileDefinesKTX.h"
#include "PVRCore/Texture/TextureDefines.h"
#include "PVRCore/Log.h"

namespace {
bool setopenGLFormat(pvr::TextureHeader& hd, uint32_t glInternalFormat, uint32_t, uint32_t glType)
{
	/*  Try to determine the format. This code is naive, and only checks the data that matters (e.g. glInternalFormat first, then glType
	if it needs more information).
	*/
	switch (glInternalFormat)
	{
	//Unsized internal formats
	case pvr::texture_ktx::OpenGLFormats::GL_RED:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		switch (glType)
		{
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_BYTE:
		{
			hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_BYTE:
		{
			hd.setChannelType(pvr::VariableType::SignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_SHORT:
		{
			hd.setChannelType(pvr::VariableType::SignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_INT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_INT:
		{
			hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
			return true;
		}
		}
		break;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		switch (glType)
		{
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_BYTE:
		{
			hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_BYTE:
		{
			hd.setChannelType(pvr::VariableType::SignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_SHORT:
		{
			hd.setChannelType(pvr::VariableType::SignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_INT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 32, 32>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_INT:
		{
			hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 32, 32>::ID);
			return true;
		}
		}
		break;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		switch (glType)
		{
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_BYTE_3_3_2:
		{
			hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 3, 3, 2>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_BYTE:
		{
			hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_BYTE:
		{
			hd.setChannelType(pvr::VariableType::SignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_SHORT:
		{
			hd.setChannelType(pvr::VariableType::SignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_INT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_INT:
		{
			hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT_5_6_5:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID);
			return true;
		}
		}
		break;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		switch (glType)
		{
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_BYTE:
		{
			hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_BYTE:
		{
			hd.setChannelType(pvr::VariableType::SignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_SHORT:
		{
			hd.setChannelType(pvr::VariableType::SignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_INT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_INT:
		{
			hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT_5_5_5_1:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT_4_4_4_4:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID);
			return true;
		}
		}
		break;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_BGRA:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		if (glType == pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_BYTE)
		{
			hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID);
			return true;
		}
		break;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE_ALPHA:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		switch (glType)
		{
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_BYTE:
		{
			hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 8, 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_BYTE:
		{
			hd.setChannelType(pvr::VariableType::SignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 8, 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 16, 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_SHORT:
		{
			hd.setChannelType(pvr::VariableType::SignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 16, 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_INT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 32, 32>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_INT:
		{
			hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 32, 32>::ID);
			return true;
		}
		}
		break;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		switch (glType)
		{
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_BYTE:
		{
			hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'l', 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_BYTE:
		{
			hd.setChannelType(pvr::VariableType::SignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'l', 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'l', 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_SHORT:
		{
			hd.setChannelType(pvr::VariableType::SignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'l', 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_INT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'l', 32>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_INT:
		{
			hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'l', 32>::ID);
			return true;
		}
		}
		break;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_ALPHA:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		switch (glType)
		{
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_BYTE:
		{
			hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'a', 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_BYTE:
		{
			hd.setChannelType(pvr::VariableType::SignedByteNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'a', 8>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_SHORT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'a', 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_SHORT:
		{
			hd.setChannelType(pvr::VariableType::SignedShortNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'a', 16>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_UNSIGNED_INT:
		{
			hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'a', 32>::ID);
			return true;
		}
		case pvr::texture_ktx::OpenGLFormats::GL_INT:
		{
			hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
			hd.setPixelFormat(pvr::GeneratePixelType1<'a', 32>::ID);
			return true;
		}
		}
		break;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_ALPHA8:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'a', 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_ALPHA8_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'a', 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_ALPHA16:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'a', 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_ALPHA16_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'a', 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_ALPHA16F_ARB:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'a', 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_ALPHA32F_ARB:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'a', 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE8:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'l', 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE8_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'l', 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE16:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'l', 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE16_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'l', 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE16F_ARB:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'l', 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE32F_ARB:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'l', 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE8_ALPHA8:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE8_ALPHA8_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE_ALPHA16F_ARB:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_LUMINANCE_ALPHA32F_ARB:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'l', 'a', 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R8:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R8_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R16:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R16_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R16F:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R32F:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R8UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByte);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R8I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByte);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R16UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShort);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R16I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedShort);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R32UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R32I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedInteger);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG8:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG8_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG16:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG16_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG16F:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG32F:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG8UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByte);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG8I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByte);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG16UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShort);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG16I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedShort);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG32UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RG32I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedInteger);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R3_G3_B2:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 3, 3, 2>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB565:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB8:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB8_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_SRGB8:
	{
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB16:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB16_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB10:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'x', 10, 10, 10, 2>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_R11F_G11F_B10F:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB9_E5:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::SharedExponentR9G9B9E5);
		hd.setChannelType(pvr::VariableType::UnsignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB16F:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB32F:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB8UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByte);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB8I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByte);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB16UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShort);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB16I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedShort);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB32UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB32I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedInteger);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA8:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA8_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_SRGB8_ALPHA8:
	{
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA16:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA16_SNORM:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB5_A1:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA4:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB10_A2:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA16F:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA32F:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA8UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedByte);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA8I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setChannelType(pvr::VariableType::SignedByte);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGB10_A2UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA16UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedShort);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA16I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setChannelType(pvr::VariableType::SignedShort);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA32I:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_RGBA32UI:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID);
		hd.setChannelType(pvr::VariableType::SignedInteger);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::PVRTCI_2bpp_RGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::PVRTCI_2bpp_RGBA);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::PVRTCI_4bpp_RGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::PVRTCI_4bpp_RGBA);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::PVRTCII_2bpp);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::PVRTCII_4bpp);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_ETC1_RGB8_OES:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::ETC1);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT1);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT3);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT5);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_SRGB8_ETC2:
	{
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::ETC2_RGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGB8_ETC2:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::ETC2_RGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
	{
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::ETC2_RGBA);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGBA8_ETC2_EAC:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::ETC2_RGBA);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
	{
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::ETC2_RGB_A1);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
	{
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::ETC2_RGB_A1);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_SIGNED_R11_EAC:
	{
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::EAC_R11);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_R11_EAC:
	{
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::EAC_R11);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_SIGNED_RG11_EAC:
	{
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::EAC_RG11);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_ktx::OpenGLFormats::GL_COMPRESSED_RG11_EAC:
	{
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setPixelFormat(pvr::CompressedPixelFormat::EAC_RG11);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	}

	//Return false if format isn't found/valid.
	return false;
}
}

using std::vector;
namespace pvr {
namespace assets {
namespace assetReaders {
TextureReaderKTX::TextureReaderKTX() : _texturesToLoad(true)
{
}
TextureReaderKTX::TextureReaderKTX(Stream::ptr_type assetStream) : AssetReader<Texture>(std::move(assetStream)), _texturesToLoad(true)
{
}

bool TextureReaderKTX::readNextAsset(Texture& asset)
{
	if (_assetStream->getSize() < texture_ktx::c_expectedHeaderSize)
	{
		return false;
	}

	size_t dataRead;
	texture_ktx::FileHeader ktxFileHeader;

	// Acknowledge that once this function has returned the user won't be able load a texture from the file.
	_texturesToLoad = false;

	// Read the identifier
	if (!_assetStream->read(1, sizeof(ktxFileHeader.identifier), &ktxFileHeader.identifier, dataRead) || dataRead != sizeof(ktxFileHeader.identifier)) { return false; }

	// Check that the identifier matches
	if (memcmp(ktxFileHeader.identifier, texture_ktx::c_identifier, sizeof(ktxFileHeader.identifier)) != 0)
	{
		return false;
	}

	// Read the endianness
	if (!_assetStream->read(sizeof(ktxFileHeader.endianness), 1, &ktxFileHeader.endianness, dataRead) || dataRead != 1) { return false; }

	// Check the endianness of the file
	if (ktxFileHeader.endianness != texture_ktx::c_endianReference)
	{
		return false;
	}

	// Read the openGL type
	if (!_assetStream->read(sizeof(ktxFileHeader.glType), 1, &ktxFileHeader.glType, dataRead) || dataRead != 1) { return false; }

	// Read the openGL type size
	if (!_assetStream->read(sizeof(ktxFileHeader.glTypeSize), 1, &ktxFileHeader.glTypeSize, dataRead) || dataRead != 1) { return false; }

	// Read the openGL format
	if (!_assetStream->read(sizeof(ktxFileHeader.glFormat), 1, &ktxFileHeader.glFormat, dataRead) || dataRead != 1) { return false; }

	// Read the openGL internal format
	if (!_assetStream->read(sizeof(ktxFileHeader.glInternalFormat), 1, &ktxFileHeader.glInternalFormat, dataRead) || dataRead != 1) { return false; }

	// Read the openGL base (unsized) internal format
	if (!_assetStream->read(sizeof(ktxFileHeader.glBaseInternalFormat), 1, &ktxFileHeader.glBaseInternalFormat, dataRead) || dataRead != 1) { return false; }

	// Read the width
	if (!_assetStream->read(sizeof(ktxFileHeader.pixelWidth), 1, &ktxFileHeader.pixelWidth, dataRead) || dataRead != 1) { return false; }

	// Read the height
	if (!_assetStream->read(sizeof(ktxFileHeader.pixelHeight), 1, &ktxFileHeader.pixelHeight, dataRead) || dataRead != 1) { return false; }

	// Read the depth
	if (!_assetStream->read(sizeof(ktxFileHeader.pixelDepth), 1, &ktxFileHeader.pixelDepth, dataRead) || dataRead != 1) { return false; }

	// Read the number of array elements
	if (!_assetStream->read(sizeof(ktxFileHeader.numArrayElements), 1, &ktxFileHeader.numArrayElements, dataRead) || dataRead != 1) { return false; }

	// Read the number of faces
	if (!_assetStream->read(sizeof(ktxFileHeader.numFaces), 1, &ktxFileHeader.numFaces, dataRead) || dataRead != 1) { return false; }

	// Read the number of MIP Map levels
	if (!_assetStream->read(sizeof(ktxFileHeader.numMipmapLevels), 1, &ktxFileHeader.numMipmapLevels, dataRead) || dataRead != 1) { return false; }

	// Read the meta data size
	if (!_assetStream->read(sizeof(ktxFileHeader.bytesOfKeyValueData), 1, &ktxFileHeader.bytesOfKeyValueData, dataRead) || dataRead != 1) { return false; }

	// Read the meta data
	uint32_t metaDataRead = 0;

	// AxisOrientation if we find it.
	uint32_t orientation = 0;

	// Read MetaData
	if (ktxFileHeader.bytesOfKeyValueData > 0)
	{
		// Loop through all the meta data
		do
		{
			// Read the amount of meta data in this block.
			uint32_t keyAndValueSize = 0;
			if (!_assetStream->read(sizeof(keyAndValueSize), 1, &keyAndValueSize, dataRead) || dataRead != 1) { return false; }

			// Allocate enough memory to read in the meta data.
			UInt8Buffer keyAndData; keyAndData.resize(keyAndValueSize);

			// Read in the meta data.
			if (!_assetStream->read(1, keyAndValueSize, keyAndData.data(), dataRead) || dataRead != keyAndValueSize) { return false; }

			// Setup the key pointer
			std::string keyString(reinterpret_cast<char*>(keyAndData.data()));

			// Search for KTX orientation. This is the only meta data currently supported
			if (keyString == std::string(texture_ktx::c_orientationMetaDataKey))
			{
				// KTX AxisOrientation key/value found, offset to the data location.
				uint8_t* data = keyAndData.data() + (keyString.length() + 1);
				uint32_t dataSize = static_cast<uint32_t>(keyAndValueSize - (keyString.length() + 1));

				// Read the data as a char 8 std::string into a std::string to find the orientation.
				std::string orientationString(reinterpret_cast<char*>(data), dataSize);

				//Search for and set non-default orientations.
				if (orientationString.find("T=u") != std::string::npos)
				{
					orientation |= TextureMetaData::AxisOrientationUp;
				}
				if (orientationString.find("S=l") != std::string::npos)
				{
					orientation |= TextureMetaData::AxisOrientationLeft;
				}
				if (orientationString.find("R=o") != std::string::npos)
				{
					orientation |= TextureMetaData::AxisOrientationOut;
				}
			}

			// Work out the padding.
			uint32_t padding = 0;

			//If it needs padding
			if (keyAndValueSize % 4)
			{
				padding = 4 - (keyAndValueSize % 4);
			}

			// Skip to the next meta data.
			if (!_assetStream->seek(padding, Stream::SeekOriginFromCurrent)) { return false; }

			// Increase the meta data read value
			metaDataRead += keyAndValueSize + padding;
		}
		while (_assetStream->getPosition() < (ktxFileHeader.bytesOfKeyValueData + texture_ktx::c_expectedHeaderSize));

		// Make sure the meta data size wasn't completely wrong. If it was, there are no guarantees about the contents of the texture data.
		if (metaDataRead > ktxFileHeader.bytesOfKeyValueData)
		{
			return false;
		}
	}

	// Construct the texture asset's header
	TextureHeader textureHeader;
	setopenGLFormat(textureHeader, ktxFileHeader.glInternalFormat, ktxFileHeader.glFormat, ktxFileHeader.glType);
	textureHeader.setWidth(ktxFileHeader.pixelWidth);
	textureHeader.setHeight(ktxFileHeader.pixelHeight);
	textureHeader.setDepth(ktxFileHeader.pixelDepth);
	textureHeader.setNumArrayMembers(ktxFileHeader.numArrayElements == 0 ? 1 : ktxFileHeader.numArrayElements);
	textureHeader.setNumFaces(ktxFileHeader.numFaces);
	textureHeader.setNumMipMapLevels(ktxFileHeader.numMipmapLevels);
	textureHeader.setOrientation(static_cast<TextureMetaData::AxisOrientation>(orientation));

	// Initialize the texture to allocate data
	asset = Texture(textureHeader, NULL);

	// Seek to the start of the texture data, just in case.
	if (!_assetStream->seek(ktxFileHeader.bytesOfKeyValueData + texture_ktx::c_expectedHeaderSize, Stream::SeekOriginFromStart)) { return false; }

	// Read in the texture data
	for (uint32_t mipMapLevel = 0; mipMapLevel < ktxFileHeader.numMipmapLevels; ++mipMapLevel)
	{
		// Read the stored size of the MIP Map.
		uint32_t mipMapSize = 0;
		if (!_assetStream->read(sizeof(mipMapSize), 1, &mipMapSize, dataRead) || dataRead != 1) { return false; }

		// Sanity check the size - regular cube maps are a slight exception
		if (asset.getNumFaces() == 6 && asset.getNumArrayMembers() == 1)
		{
			if (mipMapSize != asset.getDataSize(mipMapLevel, false, false))
			{
				return false;
			}
		}
		else
		{
			if (mipMapSize != asset.getDataSize(mipMapLevel))
			{
				return false;
			}
		}

		// Work out the Cube Map padding.
		uint32_t cubePadding = 0;
		if (asset.getDataSize(mipMapLevel, false, false) % 4)
		{
			cubePadding = 4 - (asset.getDataSize(mipMapLevel, false, false) % 4);
		}

		// Compressed images are written without scan line padding.
		if (asset.getPixelFormat().getPart().High == 0
		    && asset.getPixelFormat().getPixelTypeId() != static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5))
		{
			for (uint32_t iSurface = 0; iSurface < asset.getNumArrayMembers(); ++iSurface)
			{
				for (uint32_t iFace = 0; iFace < asset.getNumFaces(); ++iFace)
				{
					// Read in the texture data.
					if (!_assetStream->read(asset.getDataSize(mipMapLevel, false, false), 1,
					                        asset.getDataPointer(mipMapLevel, iSurface, iFace), dataRead) || dataRead != 1) { return false; }

					// Advance past the cube face padding
					if (cubePadding && asset.getNumFaces() == 6 && asset.getNumArrayMembers() == 1)
					{
						if (_assetStream->seek(cubePadding, Stream::SeekOriginFromCurrent) != true) { return false; }
					}
				}
			}
		}
		// Uncompressed images have scan line padding.
		else
		{
			for (uint32_t iSurface = 0; iSurface < asset.getNumArrayMembers(); ++iSurface)
			{
				for (uint32_t iFace = 0; iFace < asset.getNumFaces(); ++iFace)
				{
					for (uint32_t texDepth = 0; texDepth < asset.getDepth(); ++texDepth)
					{
						for (uint32_t texHeight = 0; texHeight < asset.getHeight(); ++texHeight)
						{
							// Calculate the data offset for the relevant scan line
							uint64_t scanLineOffset = (textureOffset3D(0, texHeight, texDepth, asset.getWidth(), asset.getHeight()) *
							                           (asset.getBitsPerPixel() / 8));
							// Read in the texture data for the current scan line.
							if (!_assetStream->read((asset.getBitsPerPixel() / 8) * asset.getWidth(mipMapLevel), 1,
							                        asset.getDataPointer(mipMapLevel, iSurface, iFace) + scanLineOffset, dataRead) || dataRead != 1) { return false; }

							// Work out the amount of scan line padding.
							uint32_t scanLinePadding = (static_cast<uint32_t>(-1) * ((asset.getBitsPerPixel() / 8) *
							                            asset.getWidth(mipMapLevel))) % 4;

							// Advance past the scan line padding
							if (scanLinePadding)
							{
								if (!_assetStream->seek(scanLinePadding, Stream::SeekOriginFromCurrent)) { return false; }
							}
						}
					}

					// Advance past the cube face padding
					if (cubePadding && asset.getNumFaces() == 6 && asset.getNumArrayMembers() == 1)
					{
						if (!_assetStream->seek(cubePadding, Stream::SeekOriginFromCurrent)) { return false; }
					}
				}
			}
		}

		// Calculate the amount MIP Map padding.
		uint32_t mipMapPadding = (3 - ((mipMapSize + 3) % 4));

		// Advance past the MIP Map padding if appropriate
		if (mipMapPadding)
		{
			if (!_assetStream->seek(mipMapPadding, Stream::SeekOriginFromCurrent)) { return false; }
		}
	}

	// Return
	return true;
}

bool TextureReaderKTX::hasAssetsLeftToLoad()
{
	return _texturesToLoad;
}

bool TextureReaderKTX::canHaveMultipleAssets()
{
	return false;
}

bool TextureReaderKTX::isSupportedFile(Stream& assetStream)
{
	// Try to open the stream
	bool result = assetStream.open();
	if (!result)
	{
		return false;
	}

	// Read the magic identifier
	char   magic[12];
	size_t dataRead;
	result = assetStream.read(1, sizeof(magic), &magic, dataRead);

	// Make sure it read ok, if not it's probably not a usable stream.
	if (!result || dataRead != sizeof(magic))
	{
		assetStream.close();
		return false;
	}

	// Reset the file
	assetStream.close();

	// Check that the identifier matches
	if (memcmp(magic, texture_ktx::c_identifier, sizeof(magic)) != 0)
	{
		return false;
	}

	return true;
}

vector<std::string> TextureReaderKTX::getSupportedFileExtensions()
{
	vector<std::string> extensions;
	extensions.push_back("ktx");
	return vector<std::string>(extensions);
}

}
}
}
//!\endcond