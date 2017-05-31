/*!
\brief Implementation of methods of the TextureHeader class.
\file PVRCore/Texture/TextureHeader.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRCore/Maths.h"
#include "PVRCore/Texture/TextureHeader.h"
#include "PVRCore/Texture/FileDefinesDDS.h"
#include "PVRCore/PixelFormat.h"
#include "PVRCore/Log.h"
#include <algorithm>
using std::string;
using std::map;
namespace pvr {
using namespace types;

TextureHeader::TextureHeader()
{
	_header.flags            = 0;
	_header.pixelFormat      = CompressedPixelFormat::NumCompressedPFs;
	_header.colorSpace       = ColorSpace::lRGB;
	_header.channelType      = VariableType::UnsignedByteNorm;
	_header.height           = 1;
	_header.width            = 1;
	_header.depth            = 1;
	_header.numberOfSurfaces = 1;
	_header.numberOfFaces    = 1;
	_header.mipMapCount      = 1;
	_header.metaDataSize     = 0;
}



TextureHeader::TextureHeader(TextureHeader::Header& header) : _header(header) {}

TextureHeader::TextureHeader(Header fileHeader, uint32 metaDataCount, TextureMetaData* metaData)
	: _header(fileHeader)
{
	if (metaData)
	{
		for (uint32 i = 0; i < metaDataCount; ++i)
		{
			addMetaData(metaData[i]);
		}
	}
}

TextureHeader::TextureHeader(PixelFormat pixelFormat, uint32 width, uint32 height, uint32 depth, uint32 mipMapCount,
                             types::ColorSpace colorSpace, VariableType channelType, uint32 numberOfSurfaces, uint32 numberOfFaces,
                             uint32 flags, TextureMetaData* metaData, uint32 metaDataSize)
{
	_header.pixelFormat = pixelFormat;
	_header.width = width, _header.height = height, _header.depth = depth;
	_header.mipMapCount = mipMapCount;
	_header.colorSpace = colorSpace;
	_header.channelType = channelType;
	_header.numberOfSurfaces = numberOfSurfaces;
	_header.numberOfFaces = numberOfFaces;
	_header.flags = flags;
	if (metaData)
	{
		for (uint32 i = 0; i < metaDataSize; ++i)
		{
			addMetaData(metaData[i]);
		}
	}
}


const string TextureHeader::getCubeMapOrder() const
{
	//Make sure the meta block exists
	//uint32 fourCCIndex = _metaDataMap.find(Header::PVRv3);
	map<uint32, map<uint32, TextureMetaData> >::const_iterator foundFourCC = _metaDataMap.find(Header::PVRv3);
	if (getNumberOfFaces() > 1)
	{
		if (foundFourCC != _metaDataMap.end())
		{

			map<uint32, TextureMetaData>::const_iterator foundMetaData = (foundFourCC->second).find(
			      TextureMetaData::IdentifierCubeMapOrder);
			if (foundMetaData != (foundFourCC->second).end())
			{
				char8 cubeMapOrder[7];
				cubeMapOrder[6] = 0;
				memcpy(cubeMapOrder, (foundMetaData->second).getData(), 6);
				return string(cubeMapOrder);
			}
		}

		string defaultOrder("XxYyZz");

		// Remove characters for faces that don't exist
		defaultOrder.resize(defaultOrder.size() - 6 - getNumberOfFaces());

		return defaultOrder;
	}

	return string("");
}




uint32 TextureHeader::getBitsPerPixel() const
{
	if (getPixelFormat().getPart().High != 0)
	{
		return getPixelFormat().getPixelTypeChar()[4] + getPixelFormat().getPixelTypeChar()[5] + getPixelFormat().getPixelTypeChar()[6] +
		       getPixelFormat().getPixelTypeChar()[7];
	}
	else
	{
		switch (getPixelFormat().getPixelTypeId())
		{
		case (uint64)CompressedPixelFormat::BW1bpp:
			return 1;
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGB:
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGBA:
		case (uint64)CompressedPixelFormat::PVRTCII_2bpp:
			return 2;
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGB:
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGBA:
		case (uint64)CompressedPixelFormat::PVRTCII_4bpp:
		case (uint64)CompressedPixelFormat::ETC1:
		case (uint64)CompressedPixelFormat::EAC_R11:
		case (uint64)CompressedPixelFormat::ETC2_RGB:
		case (uint64)CompressedPixelFormat::ETC2_RGB_A1:
		case (uint64)CompressedPixelFormat::DXT1:
		case (uint64)CompressedPixelFormat::BC4:
			return 4;
		case (uint64)CompressedPixelFormat::DXT2:
		case (uint64)CompressedPixelFormat::DXT3:
		case (uint64)CompressedPixelFormat::DXT4:
		case (uint64)CompressedPixelFormat::DXT5:
		case (uint64)CompressedPixelFormat::BC5:
		case (uint64)CompressedPixelFormat::EAC_RG11:
		case (uint64)CompressedPixelFormat::ETC2_RGBA:
			return 8;
		case (uint64)CompressedPixelFormat::YUY2:
		case (uint64)CompressedPixelFormat::UYVY:
		case (uint64)CompressedPixelFormat::RGBG8888:
		case (uint64)CompressedPixelFormat::GRGB8888:
			return 16;
		case (uint64)CompressedPixelFormat::SharedExponentR9G9B9E5:
			return 32;
		default:
			return 0;
		}
	}
	return 0;
}

void TextureHeader::getMinDimensionsForFormat(uint32& minX, uint32& minY, uint32& minZ) const
{
	if (getPixelFormat().getPart().High != 0)
	{
		// Non-compressed formats all return 1.
		minX = minY = minZ = 1;
	}
	else
	{
		// Default
		minX = minY = minZ = 1;

		switch (getPixelFormat().getPixelTypeId())
		{
		case (uint64)CompressedPixelFormat::DXT1:
		case (uint64)CompressedPixelFormat::DXT2:
		case (uint64)CompressedPixelFormat::DXT3:
		case (uint64)CompressedPixelFormat::DXT4:
		case (uint64)CompressedPixelFormat::DXT5:
		case (uint64)CompressedPixelFormat::BC4:
		case (uint64)CompressedPixelFormat::BC5:
		case (uint64)CompressedPixelFormat::ETC1:
		case (uint64)CompressedPixelFormat::ETC2_RGB:
		case (uint64)CompressedPixelFormat::ETC2_RGBA:
		case (uint64)CompressedPixelFormat::ETC2_RGB_A1:
		case (uint64)CompressedPixelFormat::EAC_R11:
		case (uint64)CompressedPixelFormat::EAC_RG11:
			minX = 4;
			minY = 4;
			minZ = 1;
			break;
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGB:
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGBA:
			minX = 8;
			minY = 8;
			minZ = 1;
			break;
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGB:
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGBA:
			minX = 16;
			minY = 8;
			minZ = 1;
			break;
		case (uint64)CompressedPixelFormat::PVRTCII_4bpp:
			minX = 4;
			minY = 4;
			minZ = 1;
			break;
		case (uint64)CompressedPixelFormat::PVRTCII_2bpp:
			minX = 8;
			minY = 4;
			minZ = 1;
			break;
		case (uint64)CompressedPixelFormat::UYVY:
		case (uint64)CompressedPixelFormat::YUY2:
		case (uint64)CompressedPixelFormat::RGBG8888:
		case (uint64)CompressedPixelFormat::GRGB8888:
			minX = 2;
			minY = 1;
			minZ = 1;
			break;
		case (uint64)CompressedPixelFormat::BW1bpp:
			minX = 8;
			minY = 1;
			minZ = 1;
			break;
		//Error
		case (uint64)CompressedPixelFormat::NumCompressedPFs:
			break;
		}
	}
}

void TextureHeader::setBumpMap(float bumpScale, string bumpOrder)
{
	if (bumpOrder.find_first_not_of("xyzh") != std::string::npos)
	{
		assertion(false ,  "Invalid Bumpmap order string");
		pvr::Log("Invalid Bumpmap order string");
		return;
	}
	//Get a reference to the meta data block.
	TextureMetaData& bumpMetaData = _metaDataMap[Header::PVRv3][TextureMetaData::IdentifierBumpData];

	//Check if it's already been set or not.
	if (bumpMetaData.getData())
	{
		_header.metaDataSize -= bumpMetaData.getTotalSizeInMemory();
	}

	// Initialize and clear the bump map data
	byte bumpData[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	//Copy the floating point scale and character order into the bumpmap data
	memcpy(bumpData, &bumpScale, 4);
	memcpy(bumpData + 4, bumpOrder.c_str(), (std::min)(bumpOrder.length(), size_t(4)));

	bumpMetaData = TextureMetaData(Header::PVRv3, TextureMetaData::IdentifierBumpData, 8, bumpData);

	//Increment the meta data size.
	_header.metaDataSize += bumpMetaData.getTotalSizeInMemory();
}

TextureMetaData::AxisOrientation TextureHeader::getOrientation(TextureMetaData::Axis axis) const
{
	//Make sure the meta block exists

	std::map<uint32, std::map<uint32, TextureMetaData> >::const_iterator foundIdentifer = _metaDataMap.find(Header::PVRv3);
	if (foundIdentifer != _metaDataMap.end())
	{
		std::map<uint32, TextureMetaData>::const_iterator foundTexMetaData = foundIdentifer->second.find(
		      TextureMetaData::IdentifierTextureOrientation);
		if (foundTexMetaData != foundIdentifer->second.end())
		{
			return (TextureMetaData::AxisOrientation)
			       (foundTexMetaData->second.getData()[axis]);
		}
	}


	return (TextureMetaData::AxisOrientation)0; //Default is the flag values.
}

bool TextureHeader::getDirectXGIFormat(uint32& dxgiFormat, bool& notAlpha) const
{
	// Default value in case of errors
	dxgiFormat = texture_dds::DXGI_FORMAT_UNKNOWN;
	notAlpha = false;
	if (getPixelFormat().getPart().High == 0)
	{
		if (getPixelFormat().getPixelTypeId() == (uint64)CompressedPixelFormat::RGBG8888)
		{
			dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_B8G8_UNORM;
			return true;
		}
		if (getPixelFormat().getPixelTypeId() == (uint64)CompressedPixelFormat::GRGB8888)
		{
			dxgiFormat = texture_dds::DXGI_FORMAT_G8R8_G8B8_UNORM;
			return true;
		}
		if (getPixelFormat().getPixelTypeId() == (uint64)CompressedPixelFormat::BW1bpp)
		{
			dxgiFormat = texture_dds::DXGI_FORMAT_R1_UNORM;
			return true;
		}
		if (getChannelType() == VariableType::UnsignedIntegerNorm || getChannelType() == VariableType::UnsignedShortNorm ||
		    getChannelType() == VariableType::UnsignedByteNorm)
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case (uint64)CompressedPixelFormat::BC1:
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC1_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC1_UNORM;
					return true;
				}
			case (uint64)CompressedPixelFormat::BC2:
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC2_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC2_UNORM;
					return true;
				}
			case (uint64)CompressedPixelFormat::BC3:
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC3_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_BC3_UNORM;
					return true;
				}
			case (uint64)CompressedPixelFormat::BC4:
				dxgiFormat = texture_dds::DXGI_FORMAT_BC4_UNORM;
				return true;
			case (uint64)CompressedPixelFormat::BC5:
				dxgiFormat = texture_dds::DXGI_FORMAT_BC5_UNORM;
				return true;
			}
		}
		else if (getChannelType() == VariableType::SignedIntegerNorm || getChannelType() == VariableType::SignedShortNorm ||
		         getChannelType() == VariableType::SignedByteNorm)
		{
			if (getPixelFormat().getPixelTypeId() == (uint64)CompressedPixelFormat::BC4)
			{
				dxgiFormat = texture_dds::DXGI_FORMAT_BC4_SNORM;
				return true;
			}
			if (getPixelFormat().getPixelTypeId() == (uint64)CompressedPixelFormat::BC5)
			{
				dxgiFormat = texture_dds::DXGI_FORMAT_BC5_SNORM;
				return true;
			}
		}
	}
	else
	{
		switch (getChannelType())
		{
		case VariableType::SignedFloat:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 32, 32, 32, 32>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32A32_FLOAT;
				return true;
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32_FLOAT;
				return true;
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32G32_FLOAT;
				return true;
			case GeneratePixelType1<'r', 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32_FLOAT;
				return true;
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_FLOAT;
				return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_FLOAT;
				return true;
			case GeneratePixelType1<'r', 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16_FLOAT;
				return true;
			case GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R11G11B10_FLOAT;
				return true;

			}
			break;
		}
		case VariableType::UnsignedByte:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_UINT;
				return true;
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_UINT;
				return true;
			case GeneratePixelType1<'r', 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8_UINT;
				return true;
			}
			break;
		}
		case VariableType::UnsignedByteNorm:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_UNORM;
					return true;
				}
			}
			case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID:
			{
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_B8G8R8A8_UNORM;
					return true;
				}
			}
			case GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID:
			{
				notAlpha = true;
				if (getColorSpace() == ColorSpace::sRGB)
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
					return true;
				}
				else
				{
					dxgiFormat = texture_dds::DXGI_FORMAT_B8G8R8X8_UNORM;
					return true;
				}
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_UNORM;
				return true;
			case GeneratePixelType1<'r', 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8_UNORM;
				return true;
			case GeneratePixelType1<'x', 8>::ID:
				notAlpha = true;
			case GeneratePixelType1<'a', 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_A8_UNORM;
				return true;
			}
			break;
		}
		case VariableType::SignedByte:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_SINT;
				return true;
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_SINT;
				return true;
			case GeneratePixelType1<'r', 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8_SINT;
				return true;
			}
			break;
		}
		case VariableType::SignedByteNorm:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8G8B8A8_SNORM;
				return true;
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8G8_SNORM;
				return true;
			case GeneratePixelType1<'r', 8>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R8_SNORM;
				return true;
			}
			break;
		}
		case VariableType::UnsignedShort:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_UINT;
				return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_UINT;
				return true;
			case GeneratePixelType1<'r', 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16_UINT;
				return true;
			}
			break;
		}
		case VariableType::UnsignedShortNorm:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_UNORM;
				return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_UNORM;
				return true;
			case GeneratePixelType1<'r', 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16_UNORM;
				return true;
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_B5G6R5_UNORM;
				return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 5, 5, 5, 1>::ID:
				notAlpha = true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 5, 5, 5, 1>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_B5G5R5A1_UNORM;
				return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 4, 4, 4, 4>::ID:
				notAlpha = true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_B4G4R4A4_UNORM;
				return true;
			}
			break;
		}
		case VariableType::SignedShort:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_SINT;
				return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_SINT;
				return true;
			case GeneratePixelType1<'r', 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16_SINT;
				return true;
			}
			break;
		}
		case VariableType::SignedShortNorm:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 16, 16, 16, 16>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16B16A16_SNORM;
				return true;
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16G16_SNORM;
				return true;
			case GeneratePixelType1<'r', 16>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R16_SNORM;
				return true;
			}
			break;
		}
		case VariableType::UnsignedInteger:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 32, 32, 32, 32>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32A32_UINT;
				return true;
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32_UINT;
				return true;
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32G32_UINT;
				return true;
			case GeneratePixelType1<'r', 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32_UINT;
				return true;
			case GeneratePixelType4<'r', 'g', 'b', 'x', 10, 10, 10, 2>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R10G10B10A2_UINT;
				return true;

			}
			break;
		}
		case VariableType::UnsignedIntegerNorm:
		{
			if (getPixelFormat().getPixelTypeId() == GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID)
			{
				dxgiFormat = texture_dds::DXGI_FORMAT_R10G10B10A2_UNORM;
				return true;
			}
			if (getPixelFormat().getPixelTypeId() == GeneratePixelType4<'r', 'g', 'b', 'x', 10, 10, 10, 2>::ID)
			{
				notAlpha = true;
				dxgiFormat = texture_dds::DXGI_FORMAT_R10G10B10A2_UNORM;
				return true;
			}
			break;
		}
		case VariableType::SignedInteger:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'x', 32, 32, 32, 32>::ID:
				notAlpha = true;
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32A32_SINT;
				return true;
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32G32B32_SINT;
				return true;
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32G32_SINT;
				return true;
			case GeneratePixelType1<'r', 32>::ID:
				dxgiFormat = texture_dds::DXGI_FORMAT_R32_SINT;
				return true;
			}
			break;
		}
		default:
		{
		}
		}
	}

	// Default return value if no errors.
	return false;
}

bool TextureHeader::getDirect3DFormat(uint32& d3dFormat) const
{
	//Default return value if unrecognised
	d3dFormat = texture_dds::D3DFMT_UNKNOWN;

	if (getPixelFormat().getPart().High == 0)
	{
		switch (getPixelFormat().getPixelTypeId())
		{
		case (uint64)CompressedPixelFormat::DXT1:
			d3dFormat = texture_dds::D3DFMT_DXT1;
			return true;
		case (uint64)CompressedPixelFormat::DXT2:
			d3dFormat = texture_dds::D3DFMT_DXT2;
			return true;
		case (uint64)CompressedPixelFormat::DXT3:
			d3dFormat = texture_dds::D3DFMT_DXT3;
			return true;
		case (uint64)CompressedPixelFormat::DXT4:
			d3dFormat = texture_dds::D3DFMT_DXT4;
			return true;
		case (uint64)CompressedPixelFormat::DXT5:
			d3dFormat = texture_dds::D3DFMT_DXT5;
			return true;
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGB:
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGBA:
			d3dFormat = texture_dds::D3DFMT_PVRTC2;
			return true;
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGB:
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGBA:
			d3dFormat = texture_dds::D3DFMT_PVRTC4;
			return true;
		case (uint64)CompressedPixelFormat::YUY2:
			d3dFormat = texture_dds::D3DFMT_YUY2;
			return true;
		case (uint64)CompressedPixelFormat::UYVY:
			d3dFormat = texture_dds::D3DFMT_UYVY;
			return true;
		case (uint64)CompressedPixelFormat::RGBG8888:
			d3dFormat = texture_dds::D3DFMT_R8G8_B8G8;
			return true;
		case (uint64)CompressedPixelFormat::GRGB8888:
			d3dFormat = texture_dds::D3DFMT_G8R8_G8B8;
			return true;
		}
	}
	else
	{
		switch (getChannelType())
		{
		case VariableType::SignedFloat:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType1<'r', 16>::ID:
				d3dFormat = texture_dds::D3DFMT_R16F;
				return true;
			case GeneratePixelType2<'g', 'r', 16, 16>::ID:
				d3dFormat = texture_dds::D3DFMT_G16R16F;
				return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID:
				d3dFormat = texture_dds::D3DFMT_A16B16G16R16F;
				return true;
			case GeneratePixelType1<'r', 32>::ID:
				d3dFormat = texture_dds::D3DFMT_R32F;
				return true;
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
				d3dFormat = texture_dds::D3DFMT_G32R32F;
				return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 32, 32, 32, 32>::ID:
				d3dFormat = texture_dds::D3DFMT_A32B32G32R32F;
				return true;
			}
			break;
		}
		case VariableType::UnsignedIntegerNorm:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_R8G8B8;
				return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_A8R8G8B8;
				return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 8, 8, 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_X8R8G8B8;
				return true;
			case GeneratePixelType2<'a', 'l', 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_A8L8;
				return true;
			case GeneratePixelType1<'a', 8>::ID:
				d3dFormat = texture_dds::D3DFMT_A8;
				return true;
			case GeneratePixelType1<'l', 8>::ID:
				d3dFormat = texture_dds::D3DFMT_L8;
				return true;
			case GeneratePixelType2<'a', 'l', 4, 4>::ID:
				d3dFormat = texture_dds::D3DFMT_A4L4;
				return true;
			case GeneratePixelType3<'r', 'g', 'b', 3, 3, 2>::ID:
				d3dFormat = texture_dds::D3DFMT_R3G3B2;
				return true;
			case GeneratePixelType1<'l', 16>::ID:
				d3dFormat = texture_dds::D3DFMT_L16;
				return true;
			case GeneratePixelType2<'g', 'r', 16, 16>::ID:
				d3dFormat = texture_dds::D3DFMT_G16R16;
				return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID:
				d3dFormat = texture_dds::D3DFMT_A16B16G16R16;
				return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID:
				d3dFormat = texture_dds::D3DFMT_A4R4G4B4;
				return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 1, 5, 5, 5>::ID:
				d3dFormat = texture_dds::D3DFMT_A1R5G5B5;
				return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 1, 5, 5, 5>::ID:
				d3dFormat = texture_dds::D3DFMT_X1R5G5B5;
				return true;
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
				d3dFormat = texture_dds::D3DFMT_R5G6B5;
				return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 3, 3, 2>::ID:
				d3dFormat = texture_dds::D3DFMT_A8R3G3B2;
				return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
				d3dFormat = texture_dds::D3DFMT_A2B10G10R10;
				return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 2, 10, 10, 10>::ID:
				d3dFormat = texture_dds::D3DFMT_A2R10G10B10;
				return true;
			}
			break;
		}
		case VariableType::UnsignedByteNorm:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_R8G8B8;
				return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_A8R8G8B8;
				return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 8, 8, 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_X8R8G8B8;
				return true;
			case GeneratePixelType2<'a', 'l', 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_A8L8;
				return true;
			case GeneratePixelType1<'a', 8>::ID:
				d3dFormat = texture_dds::D3DFMT_A8;
				return true;
			case GeneratePixelType1<'l', 8>::ID:
				d3dFormat = texture_dds::D3DFMT_L8;
				return true;
			case GeneratePixelType2<'a', 'l', 4, 4>::ID:
				d3dFormat = texture_dds::D3DFMT_A4L4;
				return true;
			case GeneratePixelType3<'r', 'g', 'b', 3, 3, 2>::ID:
				d3dFormat = texture_dds::D3DFMT_R3G3B2;
				return true;
			}
			break;
		}
		case VariableType::UnsignedShortNorm:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType1<'l', 16>::ID:
				d3dFormat = texture_dds::D3DFMT_L16;
				return true;
			case GeneratePixelType2<'g', 'r', 16, 16>::ID:
				d3dFormat = texture_dds::D3DFMT_G16R16;
				return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID:
				d3dFormat = texture_dds::D3DFMT_A16B16G16R16;
				return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID:
				d3dFormat = texture_dds::D3DFMT_A4R4G4B4;
				return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 1, 5, 5, 5>::ID:
				d3dFormat = texture_dds::D3DFMT_A1R5G5B5;
				return true;
			case GeneratePixelType4<'x', 'r', 'g', 'b', 1, 5, 5, 5>::ID:
				d3dFormat = texture_dds::D3DFMT_X1R5G5B5;
				return true;
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
				d3dFormat = texture_dds::D3DFMT_R5G6B5;
				return true;
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 3, 3, 2>::ID:
				d3dFormat = texture_dds::D3DFMT_A8R3G3B2;
				return true;
			}
			break;
		}
		case VariableType::SignedIntegerNorm:
		{
			switch (getPixelFormat().getPixelTypeId())
			{
			case GeneratePixelType2<'g', 'r', 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_V8U8;
				return true;
			case GeneratePixelType4<'x', 'l', 'g', 'r', 8, 8, 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_X8L8V8U8;
				return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID:
				d3dFormat = texture_dds::D3DFMT_Q8W8V8U8;
				return true;
			case GeneratePixelType3<'l', 'g', 'r', 6, 5, 5>::ID:
				d3dFormat = texture_dds::D3DFMT_L6V5U5;
				return true;
			case GeneratePixelType2<'g', 'r', 16, 16>::ID:
				d3dFormat = texture_dds::D3DFMT_V16U16;
				return true;
			case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
				d3dFormat = texture_dds::D3DFMT_A2W10V10U10;
				return true;
			}
			break;
		}
		default:
			break;
		}
	}

	//Return false if format recognised
	return false;
}

uint32 TextureHeader::getDataSize(int32 iMipLevel, bool bAllSurfaces, bool bAllFaces) const
{
	//The smallest divisible sizes for a pixel format
	uint32 uiSmallestWidth = 1;
	uint32 uiSmallestHeight = 1;
	uint32 uiSmallestDepth = 1;

	//Get the pixel format's minimum dimensions.
	getMinDimensionsForFormat(uiSmallestWidth, uiSmallestHeight, uiSmallestDepth);

	//Needs to be 64-bit integer to support 16kx16k and higher sizes.
	uint64 uiDataSize = 0;
	if (iMipLevel == -1)
	{
		for (uint32 uiCurrentMIP = 0; uiCurrentMIP < getNumberOfMIPLevels(); ++uiCurrentMIP)
		{
			//Get the dimensions of the current MIP Map level.
			uint32 uiWidth = getWidth(uiCurrentMIP);
			uint32 uiHeight = getHeight(uiCurrentMIP);
			uint32 uiDepth = getDepth(uiCurrentMIP);
			uint32  bpp = getBitsPerPixel();


			//If pixel format is compressed, the dimensions need to be padded.
			if (getPixelFormat().getPart().High == 0)
			{
				uiWidth = uiWidth + ((-1 * uiWidth) % uiSmallestWidth);
				uiHeight = uiHeight + ((-1 * uiHeight) % uiSmallestHeight);
				uiDepth = uiDepth + ((-1 * uiDepth) % uiSmallestDepth);
			}

			//Add the current MIP Map's data size to the total.

			uiDataSize += bpp * (uint64)uiWidth * (uint64)uiHeight * (uint64)uiDepth;
		}
	}
	else
	{
		//Get the dimensions of the specified MIP Map level.
		uint32 uiWidth = getWidth(iMipLevel);
		uint32 uiHeight = getHeight(iMipLevel);
		uint32 uiDepth = getDepth(iMipLevel);

		//If pixel format is compressed, the dimensions need to be padded.
		if (getPixelFormat().getPart().High == 0)
		{
			uiWidth = uiWidth + ((-1 * uiWidth) % uiSmallestWidth);
			uiHeight = uiHeight + ((-1 * uiHeight) % uiSmallestHeight);
			uiDepth = uiDepth + ((-1 * uiDepth) % uiSmallestDepth);
		}

		//Work out the specified MIP Map's data size
		uiDataSize = (uint64)getBitsPerPixel() * (uint64)uiWidth * (uint64)uiHeight * (uint64)uiDepth;
	}

	//The number of faces/surfaces to register the size of.
	uint32 numfaces = (bAllFaces ? getNumberOfFaces() : 1);
	uint32 numsurfs = (bAllSurfaces ? getNumberOfArrayMembers() : 1);

	//Multiply the data size by number of faces and surfaces specified, and return.
	return (uint32)(uiDataSize / 8) * numsurfs * numfaces;
}

ptrdiff_t TextureHeader::getDataOffset(uint32 mipMapLevel/*= 0*/, uint32 arrayMember/*= 0*/, uint32 face/*= 0*/) const
{
	//Initialize the offSet value.
	uint32 uiOffSet = 0;

	//Error checking
	if ((int32)mipMapLevel == pvrTextureAllMIPMaps)
	{
		return 0;
	}

	if (mipMapLevel >= getNumberOfMIPLevels() || arrayMember >= getNumberOfArrayMembers() || face >= getNumberOfFaces())
	{
		return 0;
	}

	//File is organised by MIP Map levels, then surfaces, then faces.

	//Get the start of the MIP level.
	if (mipMapLevel != 0)
	{
		//Get the size for all MIP Map levels up to this one.
		for (uint32 uiCurrentMIPMap = 0; uiCurrentMIPMap < mipMapLevel; ++uiCurrentMIPMap)
		{
			uiOffSet += getDataSize(uiCurrentMIPMap, true, true);
		}
	}

	//Get the start of the array.
	if (arrayMember != 0)
	{
		uiOffSet += arrayMember * getDataSize(mipMapLevel, false, true);
	}

	//Get the start of the face.
	if (face != 0)
	{
		uiOffSet += face * getDataSize(mipMapLevel, false, false);
	}

	//Return the data pointer plus whatever offSet has been specified.
	return uiOffSet;
}

void TextureHeader::setOrientation(TextureMetaData::AxisOrientation eAxisOrientation)
{
	//Get a reference to the meta data block.
	TextureMetaData& orientationMetaData = _metaDataMap[Header::PVRv3][TextureMetaData::IdentifierTextureOrientation];

	//Check if it's already been set or not.
	if (orientationMetaData.getData())
	{
		_header.metaDataSize -= orientationMetaData.getTotalSizeInMemory();
	}

	// Set the orientation data
	byte orientationData[3];

	//Check for left/right (x-axis) orientation
	if ((eAxisOrientation & TextureMetaData::AxisOrientationLeft) > 0)
	{
		orientationData[TextureMetaData::AxisAxisX] = TextureMetaData::AxisOrientationLeft;
	}
	else
	{
		orientationData[TextureMetaData::AxisAxisX] = TextureMetaData::AxisOrientationRight;
	}

	//Check for up/down (y-axis) orientation
	if ((eAxisOrientation & TextureMetaData::AxisOrientationUp) > 0)
	{
		orientationData[TextureMetaData::AxisAxisY] = TextureMetaData::AxisOrientationUp;
	}
	else
	{
		orientationData[TextureMetaData::AxisAxisY] = TextureMetaData::AxisOrientationDown;
	}

	//Check for in/out (z-axis) orientation
	if ((eAxisOrientation & TextureMetaData::AxisOrientationOut) > 0)
	{
		orientationData[TextureMetaData::AxisAxisZ] = TextureMetaData::AxisOrientationOut;
	}
	else
	{
		orientationData[TextureMetaData::AxisAxisZ] = TextureMetaData::AxisOrientationIn;
	}

	// Update the meta data block
	orientationMetaData = TextureMetaData(Header::PVRv3, TextureMetaData::IdentifierTextureOrientation, 3, orientationData);

	// Check that the meta data was created successfully.
	if (orientationMetaData.getDataSize() != 0)
	{
		// Increment the meta data size.
		_header.metaDataSize += orientationMetaData.getTotalSizeInMemory();
	}
	else
	{
		// Otherwise remove it.
		_metaDataMap.erase(TextureMetaData::IdentifierTextureOrientation);
	}
}

void TextureHeader::setCubeMapOrder(string cubeMapOrder)
{
	if (cubeMapOrder.find_first_not_of("xXyYzZ") != std::string::npos)
	{
		assertion(false , "Invalid cubemap order string");
		pvr::Log("Invalid cubemap order string");
		return;
	}

	//Get a reference to the meta data block.
	TextureMetaData& cubeOrderMetaData = _metaDataMap[Header::PVRv3][TextureMetaData::IdentifierCubeMapOrder];

	//Check if it's already been set or not.
	if (cubeOrderMetaData.getData())
	{
		_header.metaDataSize -= cubeOrderMetaData.getTotalSizeInMemory();
	}

	cubeOrderMetaData = TextureMetaData(Header::PVRv3, TextureMetaData::IdentifierCubeMapOrder,
	                                    (std::min)((uint32)cubeMapOrder.length(), 6u), reinterpret_cast<const byte*>(cubeMapOrder.data()));

	//Increment the meta data size.
	_header.metaDataSize += cubeOrderMetaData.getTotalSizeInMemory();
}

void TextureHeader::addMetaData(const TextureMetaData& metaData)
{
	// Get a reference to the meta data block.
	TextureMetaData& currentMetaData = _metaDataMap[metaData.getFourCC()][metaData.getKey()];

	// Check if it's already been set or not.
	if (currentMetaData.getData())
	{
		_header.metaDataSize -= currentMetaData.getTotalSizeInMemory();
	}

	// Set the meta data block
	currentMetaData = metaData;

	// Increment the meta data size.
	_header.metaDataSize += currentMetaData.getTotalSizeInMemory();
}

bool TextureHeader::isBumpMap()const
{
	const std::map<uint32, TextureMetaData>& dataMap = _metaDataMap.at(Header::PVRv3);
	return (dataMap.find(TextureMetaData::IdentifierBumpData) != dataMap.end());
}

}
//!\endcond