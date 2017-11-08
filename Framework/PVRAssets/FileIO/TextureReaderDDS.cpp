/*!
\brief Implementation of methods of the TextureReaderDDS class.
\file PVRAssets/FileIO/TextureReaderDDS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureReaderDDS.h"
#include "PVRCore/Log.h"
using std::string;
using std::vector;



bool setDirect3DFormat(pvr::TextureHeader& hd, uint32_t d3dFormat)
{
	switch (d3dFormat)
	{
	case pvr::texture_dds::D3DFMT_R8G8B8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A8R8G8B8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_X8R8G8B8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'x', 'r', 'g', 'b', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_R5G6B5:
	{
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A1R5G5B5:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'r', 'g', 'b', 1, 5, 5, 5>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_X1R5G5B5:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'x', 'r', 'g', 'b', 1, 5, 5, 5>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A4R4G4B4:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_R3G3B2:
	{
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 3, 3, 2>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'a', 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A8R3G3B2:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'r', 'g', 'b', 8, 3, 3, 2>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_X4R4G4B4:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A2B10G10R10:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case  pvr::texture_dds::D3DFMT_A8B8G8R8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case  pvr::texture_dds::D3DFMT_X8B8G8R8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'x', 'b', 'g', 'r', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A2R10G10B10:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'r', 'g', 'b', 2, 10, 10, 10>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A16B16G16R16:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_L8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'l', 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A8L8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'a', 'l', 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A4L4:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'a', 'l', 4, 4>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_V8U8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'g', 'r', 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_L6V5U5:
	{
		hd.setPixelFormat(pvr::GeneratePixelType3<'l', 'g', 'r', 6, 5, 5>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_X8L8V8U8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'x', 'l', 'g', 'r', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_Q8W8V8U8:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_V16U16:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'g', 'r', 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_A2W10V10U10:
	{
		//Mixed format...
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_UYVY:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::UYVY);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_R8G8_B8G8:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::RGBG8888);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_YUY2:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::YUY2);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_G8R8_G8B8:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::GRGB8888);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_DXT1:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT1);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}

	case pvr::texture_dds::D3DFMT_DXT2:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT2);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(true);
		return true;
	}

	case pvr::texture_dds::D3DFMT_DXT3:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT3);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}

	case pvr::texture_dds::D3DFMT_DXT4:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT4);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(true);
		return true;
	}

	case pvr::texture_dds::D3DFMT_DXT5:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT5);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_L16:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'l', 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_G16R16:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'g', 'r', 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_Q16W16V16U16:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}

	case pvr::texture_dds::D3DFMT_R16F:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		hd.setIsPreMultiplied(false);
		return true;
	}

	case pvr::texture_dds::D3DFMT_G16R16F:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'g', 'r', 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		hd.setIsPreMultiplied(false);
		return true;
	}

	case pvr::texture_dds::D3DFMT_A16B16G16R16F:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		hd.setIsPreMultiplied(false);
		return true;
	}

	case pvr::texture_dds::D3DFMT_R32F:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		hd.setIsPreMultiplied(false);
		return true;
	}

	case pvr::texture_dds::D3DFMT_G32R32F:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'g', 'r', 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		hd.setIsPreMultiplied(false);
		return true;
	}

	case pvr::texture_dds::D3DFMT_A32B32G32R32F:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'b', 'g', 'r', 32, 32, 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_PVRTC2:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::PVRTCI_2bpp_RGBA);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	case pvr::texture_dds::D3DFMT_PVRTC4:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::PVRTCI_4bpp_RGBA);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		hd.setIsPreMultiplied(false);
		return true;
	}
	}
	return false;
}

bool setDirectXGIFormat(pvr::TextureHeader& hd, uint32_t dxgiFormat)
{
	switch (dxgiFormat)
	{
	case pvr::texture_dds::DXGI_FORMAT_R32G32B32A32_FLOAT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R32G32B32A32_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R32G32B32A32_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedInteger);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R32G32B32_FLOAT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R32G32B32_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R32G32B32_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedInteger);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R16G16B16A16_FLOAT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R16G16B16A16_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R16G16B16A16_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedShort);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R16G16B16A16_SNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedShortNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R16G16B16A16_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedShort);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R32G32_FLOAT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R32G32_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R32G32_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 32, 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedInteger);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R10G10B10A2_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R10G10B10A2_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R11G11B10_FLOAT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R8G8B8A8_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R8G8B8A8_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByte);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R8G8B8A8_SNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R8G8B8A8_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedByte);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R16G16_FLOAT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R16G16_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_R16G16_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedShort);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R16G16_SNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedShortNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R16G16_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 16, 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedShort);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R32_FLOAT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R32_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedInteger);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R32_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 32>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedInteger);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R8G8_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R8G8_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByte);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R8G8_SNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R8G8_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'r', 'g', 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedByte);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R16_FLOAT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R16_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R16_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedShort);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R16_SNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedShortNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R16_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 16>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedShort);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R8_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R8_UINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByte);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R8_SNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R8_SINT:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedByte);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_A8_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType1<'r', 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R1_UNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::BW1bpp);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::SharedExponentR9G9B9E5);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_R8G8_B8G8_UNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::RGBG8888);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_G8R8_G8B8_UNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::GRGB8888);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_BC1_UNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT1);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_BC1_UNORM_SRGB:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT1);
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_BC2_UNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT3);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_BC2_UNORM_SRGB:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT3);
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_BC3_UNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT5);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_BC3_UNORM_SRGB:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::DXT5);
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_BC4_UNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::BC4);
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_BC4_SNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::BC4);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_BC5_UNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::BC5);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}

	case pvr::texture_dds::DXGI_FORMAT_BC5_SNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::BC5);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::SignedIntegerNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_B5G6R5_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_B5G5R5A1_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'r', 'g', 'b', 5, 5, 5, 1>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_B8G8R8A8_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_B8G8R8X8_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID);
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_BC6H_SF16:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::BC7);
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setChannelType(pvr::VariableType::SignedFloat);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_BC7_UNORM:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::BC7);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_BC7_UNORM_SRGB:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::BC7);
		hd.setColorSpace(pvr::ColorSpace::sRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_YUY2:
	{
		hd.setPixelFormat(pvr::CompressedPixelFormat::YUY2);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedIntegerNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_AI44:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'a', 'i', 4, 4>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_IA44:
	{
		hd.setPixelFormat(pvr::GeneratePixelType2<'i', 'a', 4, 4>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedByteNorm);
		return true;
	}
	case pvr::texture_dds::DXGI_FORMAT_B4G4R4A4_UNORM:
	{
		hd.setPixelFormat(pvr::GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID);
		hd.setColorSpace(pvr::ColorSpace::lRGB);
		hd.setChannelType(pvr::VariableType::UnsignedShortNorm);
		return true;
	}
	}

	return false;
}










namespace pvr {
namespace assets {
namespace assetReaders {
TextureReaderDDS::TextureReaderDDS() : _texturesToLoad(true)
{
}
TextureReaderDDS::TextureReaderDDS(Stream::ptr_type assetStream) : AssetReader<Texture>(std::move(assetStream)), _texturesToLoad(true)
{
}

bool TextureReaderDDS::readNextAsset(Texture& asset)
{
	if (_assetStream->getSize() < texture_dds::c_expectedDDSSize)
	{
		return false;
	}

	// Acknowledge that once this function has returned the user won't be able load a texture from the file.
	_texturesToLoad = false;

	bool result = true;

	size_t dataRead;

	texture_dds::FileHeader ddsFileHeader;

	// Read the magic identifier
	uint32_t magic;
	result = _assetStream->read(sizeof(magic), 1, &magic, dataRead);
	if (!result || dataRead != 1) { return result; }

	if (magic != texture_dds::c_magicIdentifier)
	{
		return false;
	}

	// Read the header size
	result = _assetStream->read(sizeof(ddsFileHeader.size), 1, &ddsFileHeader.size, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Check that the size matches what's expected
	if (ddsFileHeader.size != texture_dds::c_expectedDDSSize)
	{
		return false;
	}

	// Read the flags
	result = _assetStream->read(sizeof(ddsFileHeader.flags), 1, &ddsFileHeader.flags, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the width
	result = _assetStream->read(sizeof(ddsFileHeader.width), 1, &ddsFileHeader.width, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the height
	result = _assetStream->read(sizeof(ddsFileHeader.height), 1, &ddsFileHeader.height, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the pitchOrLinearSize
	result = _assetStream->read(sizeof(ddsFileHeader.pitchOrLinearSize), 1, &ddsFileHeader.pitchOrLinearSize, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the depth
	result = _assetStream->read(sizeof(ddsFileHeader.depth), 1, &ddsFileHeader.depth, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the number of MIP Map levels
	result = _assetStream->read(sizeof(ddsFileHeader.numMipMaps), 1, &ddsFileHeader.numMipMaps, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the first chunk of "reserved" data (11 * uint32_t)
	result = _assetStream->read(sizeof(ddsFileHeader.reserved[0]), 11, &ddsFileHeader.reserved, dataRead);
	if (!result || dataRead != 11) { return result; }

	// Read the Pixel Format size
	result = _assetStream->read(sizeof(ddsFileHeader.pixelFormat.size), 1, &ddsFileHeader.pixelFormat.size, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Check that the Pixel Format size is correct
	if (ddsFileHeader.pixelFormat.size != texture_dds::c_expectedPixelFormatSize)
	{
		return false;
	}

	// Read the rest of the pixel format structure
	result = _assetStream->read(sizeof(ddsFileHeader.pixelFormat.flags), 1, &ddsFileHeader.pixelFormat.flags, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = _assetStream->read(sizeof(ddsFileHeader.pixelFormat.fourCC), 1, &ddsFileHeader.pixelFormat.fourCC, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = _assetStream->read(sizeof(ddsFileHeader.pixelFormat.bitCount), 1, &ddsFileHeader.pixelFormat.bitCount, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = _assetStream->read(sizeof(ddsFileHeader.pixelFormat.redMask), 1, &ddsFileHeader.pixelFormat.redMask, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = _assetStream->read(sizeof(ddsFileHeader.pixelFormat.greenMask), 1, &ddsFileHeader.pixelFormat.greenMask, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = _assetStream->read(sizeof(ddsFileHeader.pixelFormat.blueMask), 1, &ddsFileHeader.pixelFormat.blueMask, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = _assetStream->read(sizeof(ddsFileHeader.pixelFormat.alphaMask), 1, &ddsFileHeader.pixelFormat.alphaMask, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the Capabilities2 structure
	result = _assetStream->read(sizeof(ddsFileHeader.Capabilities1), 1, &ddsFileHeader.Capabilities1, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = _assetStream->read(sizeof(ddsFileHeader.Capabilities2), 1, &ddsFileHeader.Capabilities2, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = _assetStream->read(sizeof(ddsFileHeader.reserved[0]), 2, &ddsFileHeader.reserved, dataRead);
	if (!result || dataRead != 2) { return result; }

	// Read the final miscFlags2 value
	result = _assetStream->read(sizeof(ddsFileHeader.reserved2), 1, &ddsFileHeader.reserved2, dataRead);
	if (!result || dataRead != 1) { return result; }

	bool hasDX10Header = (ddsFileHeader.pixelFormat.flags & texture_dds::e_fourCC) &&
	                     ddsFileHeader.pixelFormat.fourCC == texture_dds::MakeFourCC<'D', 'X', '1', '0'>::FourCC;

	// Get the data for the DX10 header if present
	texture_dds::FileHeaderDX10 dx10FileHeader = {};
	if (hasDX10Header)
	{

		// Read the DX10 header
		result = _assetStream->read(sizeof(dx10FileHeader.dxgiFormat), 1, &dx10FileHeader.dxgiFormat, dataRead);
		if (!result || dataRead != 1) { return result; }
		result = _assetStream->read(sizeof(dx10FileHeader.resourceDimension), 1, &dx10FileHeader.resourceDimension, dataRead);
		if (!result || dataRead != 1) { return result; }
		result = _assetStream->read(sizeof(dx10FileHeader.miscFlags), 1, &dx10FileHeader.miscFlags, dataRead);
		if (!result || dataRead != 1) { return result; }
		result = _assetStream->read(sizeof(dx10FileHeader.arraySize), 1, &dx10FileHeader.arraySize, dataRead);
		if (!result || dataRead != 1) { return result; }
		result = _assetStream->read(sizeof(dx10FileHeader.miscFlags2), 1, &dx10FileHeader.miscFlags2, dataRead);
		if (!result || dataRead != 1) { return result; }
	}

	// Construct the texture asset's header
	TextureHeader textureHeader;

	// There is a lot of different behaviour based on whether the DX10 header is there or not.
	if (hasDX10Header)
	{
		setDirectXGIFormat(textureHeader, dx10FileHeader.dxgiFormat);

		// Set the dimensions
		switch (dx10FileHeader.resourceDimension)
		{
		case texture_dds::e_texture3D:
			textureHeader.setDepth(ddsFileHeader.depth);
		case texture_dds::e_texture2D:
			textureHeader.setHeight(ddsFileHeader.height);
		case texture_dds::e_texture1D:
			textureHeader.setWidth(ddsFileHeader.width);
		}

		if ((ddsFileHeader.flags & texture_dds::e_numMipMaps) || (ddsFileHeader.Capabilities1 & texture_dds::e_mipMaps))
		{
			textureHeader.setNumMipMapLevels(ddsFileHeader.numMipMaps);
		}
		if (dx10FileHeader.miscFlags & texture_dds::e_textureCube)
		{
			textureHeader.setNumFaces(6);
		}
		textureHeader.setNumArrayMembers((dx10FileHeader.arraySize == 0) ? 1 : dx10FileHeader.arraySize);

		if (dx10FileHeader.miscFlags2 == texture_dds::e_premultiplied)
		{
			textureHeader.setIsPreMultiplied(true);
		}
		else if (dx10FileHeader.miscFlags2 == texture_dds::e_custom)
		{
			PixelFormat pixelType = textureHeader.getPixelFormat();
			if (pixelType.getPixelTypeChar()[0] == 'a')
			{
				pixelType.getPixelTypeChar()[0] = 'x';
			}
			if (pixelType.getPixelTypeChar()[1] == 'a')
			{
				pixelType.getPixelTypeChar()[1] = 'x';
			}
			if (pixelType.getPixelTypeChar()[2] == 'a')
			{
				pixelType.getPixelTypeChar()[2] = 'x';
			}
			if (pixelType.getPixelTypeChar()[3] == 'a')
			{
				pixelType.getPixelTypeChar()[3] = 'x';
			}
		}
	}
	else
	{
		uint32_t d3dFormat = getDirect3DFormatFromDDSHeader(ddsFileHeader);
		setDirect3DFormat(textureHeader, d3dFormat);
		textureHeader.setWidth(ddsFileHeader.width);
		textureHeader.setHeight(ddsFileHeader.height);
		if ((ddsFileHeader.flags & texture_dds::e_depth) || (ddsFileHeader.Capabilities2 & texture_dds::e_volume))
		{
			textureHeader.setDepth(ddsFileHeader.depth);
		}
		if ((ddsFileHeader.flags & texture_dds::e_numMipMaps) || (ddsFileHeader.Capabilities1 & texture_dds::e_mipMaps))
		{
			textureHeader.setNumMipMapLevels(ddsFileHeader.numMipMaps);
		}
		if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMap)
		{
			uint32_t numFaces = 0;
			std::string faceOrder;

			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapPositiveX)
			{
				faceOrder += "X";
				++numFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapNegativeX)
			{
				faceOrder += "x";
				++numFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapPositiveY)
			{
				faceOrder += "Y";
				++numFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapNegativeY)
			{
				faceOrder += "y";
				++numFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapPositiveZ)
			{
				faceOrder += "Z";
				++numFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapNegativeZ)
			{
				faceOrder += "z";
				++numFaces;
			}

			textureHeader.setNumFaces(numFaces);
			textureHeader.setCubeMapOrder(faceOrder);
		}
	}

	// Initialize the texture to allocate data
	asset = Texture(textureHeader, NULL);

	// Read in the texture data
	for (uint32_t surface = 0; surface < asset.getNumArrayMembers(); ++surface)
	{
		for (uint32_t face = 0; face < asset.getNumFaces(); ++face)
		{
			for (uint32_t mipMapLevel = 0; mipMapLevel < asset.getNumMipMapLevels(); ++mipMapLevel)
			{
				//Read in the texture data.
				result = _assetStream->read(asset.getDataSize(mipMapLevel, false, false), 1, asset.getDataPointer(mipMapLevel, surface, face),
				                             dataRead);
				if (!result || dataRead != 1) { return result; }
			}
		}
	}

	// Return
	return result;
}

bool TextureReaderDDS::hasAssetsLeftToLoad()
{
	return _texturesToLoad;
}

bool TextureReaderDDS::canHaveMultipleAssets()
{
	return false;
}

uint32_t TextureReaderDDS::getDirect3DFormatFromDDSHeader(texture_dds::FileHeader& textureFileHeader)
{
	// First check for FourCC formats as these are easy to handle
	if (textureFileHeader.pixelFormat.flags & texture_dds::e_fourCC)
	{
		return textureFileHeader.pixelFormat.fourCC;
	}

	// Otherwise it's an uncompressed format using the rather awkward bit masks...
	if (textureFileHeader.pixelFormat.flags & texture_dds::e_rgb)
	{
		switch (textureFileHeader.pixelFormat.bitCount)
		{
		case 32:
			if (textureFileHeader.pixelFormat.flags & texture_dds::e_alphaPixels)
			{
				if (textureFileHeader.pixelFormat.alphaMask == 0xff000000 &&
				    textureFileHeader.pixelFormat.redMask == 0x00ff0000 &&
				    textureFileHeader.pixelFormat.greenMask == 0x0000ff00 &&
				    textureFileHeader.pixelFormat.blueMask == 0x000000ff)
				{
					return texture_dds::D3DFMT_A8R8G8B8;
				}
				if (textureFileHeader.pixelFormat.alphaMask == 0xc0000000 &&
				    textureFileHeader.pixelFormat.redMask == 0x3ff00000 &&
				    textureFileHeader.pixelFormat.greenMask == 0x000ffc00 &&
				    textureFileHeader.pixelFormat.blueMask == 0x000003ff)
				{
					return texture_dds::D3DFMT_A2B10G10R10;
				}
				if (textureFileHeader.pixelFormat.alphaMask == 0xc0000000 &&
				    textureFileHeader.pixelFormat.blueMask == 0x3ff00000 &&
				    textureFileHeader.pixelFormat.greenMask == 0x000ffc00 &&
				    textureFileHeader.pixelFormat.redMask == 0x000003ff)
				{
					return (texture_dds::D3DFMT_A2R10G10B10);
				}
			}
			else
			{
				if (textureFileHeader.pixelFormat.greenMask == 0xffff0000 &&
				    textureFileHeader.pixelFormat.redMask == 0x0000ffff)
				{
					return (texture_dds::D3DFMT_G16R16);
				}
			}
			break;
		case 24:
			if (textureFileHeader.pixelFormat.redMask == 0x00ff0000 &&
			    textureFileHeader.pixelFormat.greenMask == 0x0000ff00 &&
			    textureFileHeader.pixelFormat.blueMask == 0x000000ff)
			{
				return (texture_dds::D3DFMT_R8G8B8);
			}
			break;
		case 16:
			if (textureFileHeader.pixelFormat.flags & texture_dds::e_alphaPixels)
			{
				if (textureFileHeader.pixelFormat.alphaMask == 0x0000F000 &&
				    textureFileHeader.pixelFormat.redMask == 0x00000F00 &&
				    textureFileHeader.pixelFormat.greenMask == 0x000000F0 &&
				    textureFileHeader.pixelFormat.blueMask == 0x0000000F)
				{
					return (texture_dds::D3DFMT_A4R4G4B4);
				}
				if (textureFileHeader.pixelFormat.alphaMask == 0x0000FF00 &&
				    textureFileHeader.pixelFormat.redMask == 0x000000E0 &&
				    textureFileHeader.pixelFormat.greenMask == 0x0000001C &&
				    textureFileHeader.pixelFormat.blueMask == 0x00000003)
				{
					return (texture_dds::D3DFMT_A8R3G3B2);
				}
				if (textureFileHeader.pixelFormat.alphaMask == 0x00008000 &&
				    textureFileHeader.pixelFormat.redMask == 0x00007C00 &&
				    textureFileHeader.pixelFormat.greenMask == 0x000003E0 &&
				    textureFileHeader.pixelFormat.blueMask == 0x0000001F)
				{
					return (texture_dds::D3DFMT_A1R5G5B5);
				}
			}
			else
			{
				if (textureFileHeader.pixelFormat.redMask == 0x0000F800 &&
				    textureFileHeader.pixelFormat.greenMask == 0x000007E0 &&
				    textureFileHeader.pixelFormat.blueMask == 0x0000001F)
				{
					return (texture_dds::D3DFMT_R5G6B5);
				}
				if (textureFileHeader.pixelFormat.redMask == 0x00007C00 &&
				    textureFileHeader.pixelFormat.greenMask == 0x000003E0 &&
				    textureFileHeader.pixelFormat.blueMask == 0x0000001F)
				{
					return (texture_dds::D3DFMT_X1R5G5B5);
				}
			}
			break;
		case 8:
			if (textureFileHeader.pixelFormat.redMask == 0x000000E0 &&
			    textureFileHeader.pixelFormat.greenMask == 0x0000001C &&
			    textureFileHeader.pixelFormat.blueMask == 0x00000003)
			{
				return (texture_dds::D3DFMT_R3G3B2);
			}
			break;
		}
	}
	else if (textureFileHeader.pixelFormat.flags & texture_dds::e_unknownBump1)
	{
		if (textureFileHeader.pixelFormat.bitCount == 32 &&
		    textureFileHeader.pixelFormat.redMask == 0x000000ff &&
		    textureFileHeader.pixelFormat.greenMask == 0x0000ff00 &&
		    textureFileHeader.pixelFormat.blueMask == 0x00ff0000)
		{
			return (texture_dds::D3DFMT_X8L8V8U8);
		}
		if (textureFileHeader.pixelFormat.bitCount == 16 &&
		    textureFileHeader.pixelFormat.redMask == 0x0000001f &&
		    textureFileHeader.pixelFormat.greenMask == 0x000003e0 &&
		    textureFileHeader.pixelFormat.blueMask == 0x0000fc00)
		{
			return (texture_dds::D3DFMT_L6V5U5);
		}
	}
	else if (textureFileHeader.pixelFormat.flags & texture_dds::e_unknownBump2)
	{
		if (textureFileHeader.pixelFormat.bitCount == 32)
		{
			if (textureFileHeader.pixelFormat.alphaMask == 0xff000000 &&
			    textureFileHeader.pixelFormat.redMask == 0x000000ff &&
			    textureFileHeader.pixelFormat.greenMask == 0x0000ff00 &&
			    textureFileHeader.pixelFormat.blueMask == 0x00ff0000)
			{
				return (texture_dds::D3DFMT_Q8W8V8U8);
			}
			if (textureFileHeader.pixelFormat.alphaMask == 0xc0000000 &&
			    textureFileHeader.pixelFormat.redMask == 0x3ff00000 &&
			    textureFileHeader.pixelFormat.greenMask == 0x000ffc00 &&
			    textureFileHeader.pixelFormat.blueMask == 0x000003ff)
			{
				return (texture_dds::D3DFMT_A2W10V10U10);
			}
			if (textureFileHeader.pixelFormat.redMask == 0x0000ffff &&
			    textureFileHeader.pixelFormat.greenMask == 0xffff0000)
			{
				return (texture_dds::D3DFMT_V16U16);
			}
		}
		else if (textureFileHeader.pixelFormat.bitCount == 16)
		{
			if (textureFileHeader.pixelFormat.redMask == 0x000000ff &&
			    textureFileHeader.pixelFormat.greenMask == 0x0000ff00)
			{
				return (texture_dds::D3DFMT_V8U8);
			}
		}

	}
	else if (textureFileHeader.pixelFormat.flags & texture_dds::e_luminance)
	{
		if (textureFileHeader.pixelFormat.bitCount == 8 && textureFileHeader.pixelFormat.redMask == 0xff)
		{
			return (texture_dds::D3DFMT_L8);
		}
		if ((textureFileHeader.pixelFormat.flags & texture_dds::e_alphaPixels) &&
		    textureFileHeader.pixelFormat.bitCount == 16 &&
		    textureFileHeader.pixelFormat.redMask == 0x00ff &&
		    textureFileHeader.pixelFormat.alphaMask == 0xff00)
		{
			return (texture_dds::D3DFMT_A8L8);
		}
		if ((textureFileHeader.pixelFormat.flags & texture_dds::e_alphaPixels) &&
		    textureFileHeader.pixelFormat.bitCount == 8 &&
		    textureFileHeader.pixelFormat.redMask == 0x0f &&
		    textureFileHeader.pixelFormat.alphaMask == 0xf0)
		{
			return (texture_dds::D3DFMT_A4L4);
		}
		if (textureFileHeader.pixelFormat.bitCount == 16 && textureFileHeader.pixelFormat.redMask == 0xffff)
		{
			return (texture_dds::D3DFMT_L16);
		}
	}
	else if (textureFileHeader.pixelFormat.flags & texture_dds::e_alpha)
	{
		if (textureFileHeader.pixelFormat.bitCount == 8 && textureFileHeader.pixelFormat.alphaMask == 0xff)
		{
			return (texture_dds::D3DFMT_A8);
		}
	}

	return texture_dds::D3DFMT_UNKNOWN;
}

bool TextureReaderDDS::isSupportedFile(Stream& assetStream)
{
	// Try to open the stream
	bool result = assetStream.open();
	if (!result)
	{
		return false;
	}

	// Read the magic identifier
	uint32_t magic;
	size_t dataRead;
	result = assetStream.read(sizeof(magic), 1, &magic, dataRead);

	// Make sure it read ok, if not it's probably not a usable stream.
	if (!result || dataRead != 1)
	{
		assetStream.close();
		return false;
	}

	// Reset the file
	assetStream.close();

	// Finally, check the magic value is correct for a DDS file.
	if (magic != texture_dds::c_magicIdentifier)
	{
		return false;
	}

	return true;
}

vector<std::string> TextureReaderDDS::getSupportedFileExtensions()
{
	vector<std::string> extensions;
	extensions.push_back("dds");
	return vector<std::string>(extensions);
}

}
}
}
//!\endcond