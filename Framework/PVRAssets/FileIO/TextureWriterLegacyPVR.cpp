/*!
\brief Implementation of methods of the TextureWriterLegacyPVR class.
\file PVRAssets/FileIO/TextureWriterLegacyPVR.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureWriterLegacyPVR.h"
using std::vector;
namespace pvr {
namespace assets {
namespace assetWriters {
TextureWriterLegacyPVR::TextureWriterLegacyPVR() : _targetAPI(texture_legacy::ApiOGL)
{
}

bool TextureWriterLegacyPVR::addAssetToWrite(const Texture& asset)
{
	bool result = true;
	if (_assetsToWrite.size() < 1)
	{
		_assetsToWrite.push_back(&asset);
	}
	else
	{
		result = false;
	}

	return result;
}

bool TextureWriterLegacyPVR::writeAllAssets()
{
	// Check the Result
	bool result = true;

	// Get the header to be written.
	const TextureHeader& currentTextureHeader = *_assetsToWrite[0];

	// Create a V2 legacy header from that (if possible!)
	texture_legacy::HeaderV2 textureHeader;
	if (!convertTextureHeader3To2(textureHeader, currentTextureHeader))
	{
		return false;
	}

	// Check the size of data written.
	size_t dataWritten = 0;

	// Write the header size
	result = _assetStream->write(sizeof(textureHeader.headerSize), 1, &textureHeader.headerSize, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the height
	result = _assetStream->write(sizeof(textureHeader.height), 1, &textureHeader.height, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the width
	result = _assetStream->write(sizeof(textureHeader.width), 1, &textureHeader.width, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the MIP map count
	result = _assetStream->write(sizeof(textureHeader.numMipMaps), 1, &textureHeader.numMipMaps, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the texture flags
	result = _assetStream->write(sizeof(textureHeader.pixelFormatAndFlags), 1, &textureHeader.pixelFormatAndFlags, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the texture data size
	result = _assetStream->write(sizeof(textureHeader.dataSize), 1, &textureHeader.dataSize, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the bit count of the texture format
	result = _assetStream->write(sizeof(textureHeader.bitCount), 1, &textureHeader.bitCount, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the red mask
	result = _assetStream->write(sizeof(textureHeader.redBitMask), 1, &textureHeader.redBitMask, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the green mask
	result = _assetStream->write(sizeof(textureHeader.greenBitMask), 1, &textureHeader.greenBitMask, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the blue mask
	result = _assetStream->write(sizeof(textureHeader.blueBitMask), 1, &textureHeader.blueBitMask, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the alpha mask
	result = _assetStream->write(sizeof(textureHeader.alphaBitMask), 1, &textureHeader.alphaBitMask, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the magic number
	result = _assetStream->write(sizeof(textureHeader.pvrMagic), 1, &textureHeader.pvrMagic, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the number of surfaces
	result = _assetStream->write(sizeof(textureHeader.numSurfaces), 1, &textureHeader.numSurfaces, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the texture data
	for (uint32_t surface = 0; surface < currentTextureHeader.getNumArrayMembers(); ++surface)
	{
		for (uint32_t depth = 0; depth < currentTextureHeader.getDepth(); ++depth)
		{
			for (uint32_t face = 0; face < currentTextureHeader.getNumFaces(); ++face)
			{
				for (uint32_t mipMap = 0; mipMap < currentTextureHeader.getNumMipMapLevels(); ++mipMap)
				{
					uint32_t surfaceSize = _assetsToWrite[0]->getDataSize(mipMap, false, false) / _assetsToWrite[0]->getDepth();
					const uint8_t* surfacePointer = _assetsToWrite[0]->getDataPointer(mipMap, surface, face) + depth * surfaceSize;

					// Write each surface, one at a time
					result = _assetStream->write(1, surfaceSize, surfacePointer, dataWritten);
					if (!result || dataWritten != surfaceSize) { return result; }
				}
			}
		}
	}
	return result;
}

uint32_t TextureWriterLegacyPVR::assetsAddedSoFar()
{
	return static_cast<uint32_t>(_assetsToWrite.size());
}

bool TextureWriterLegacyPVR::supportsMultipleAssets()
{
	return false;
}

bool TextureWriterLegacyPVR::canWriteAsset(const Texture& asset)
{
	// Check the pixel format is ok
	texture_legacy::HeaderV2 legacyHeader;
	return convertTextureHeader3To2(legacyHeader, asset);
}

vector<std::string> TextureWriterLegacyPVR::getSupportedFileExtensions()
{
	vector<std::string> extensions;
	extensions.push_back("pvr");
	return vector<std::string>(extensions);
}

std::string TextureWriterLegacyPVR::getWriterName()
{
	return "PowerVR Legacy Texture Writer";
}

std::string TextureWriterLegacyPVR::getWriterVersion()
{
	return "1.0.0";
}

void TextureWriterLegacyPVR::setTargetAPI(texture_legacy::API api)
{
	_targetAPI = api;
}

texture_legacy::API TextureWriterLegacyPVR::getTargetAPI()
{
	return _targetAPI;
}

bool TextureWriterLegacyPVR::convertTextureHeader3To2(texture_legacy::HeaderV2& legacyHeader, const TextureHeader& newHeader)
{
	// Get the legacy enumeration format from the available information - this may fail.
	texture_legacy::PixelFormat legacyPixelType;
	if (!mapNewFormatToLegacyEnum(legacyPixelType, newHeader.getPixelFormat(), newHeader.getColorSpace(), newHeader.getChannelType(),
	                              newHeader.isPreMultiplied()))
	{
		return false;
	}

	// Setup the simple parts of the legacy header based on the data provided.
	legacyHeader.headerSize = sizeof(texture_legacy::HeaderV2);
	legacyHeader.pixelFormatAndFlags = static_cast<uint32_t>(legacyPixelType);
	legacyHeader.height = newHeader.getHeight();
	legacyHeader.width = newHeader.getWidth();
	legacyHeader.numMipMaps = newHeader.getNumMipMapLevels() - 1;
	legacyHeader.dataSize = newHeader.getDataSize();
	legacyHeader.bitCount = newHeader.getBitsPerPixel();
	legacyHeader.redBitMask = 0;
	legacyHeader.greenBitMask = 0;
	legacyHeader.blueBitMask = 0;
	legacyHeader.alphaBitMask = 0;
	legacyHeader.pvrMagic = texture_legacy::c_identifierV2;

	// Set the number of surfaces
	legacyHeader.numSurfaces = newHeader.getDepth() * newHeader.getNumArrayMembers() * newHeader.getNumFaces();

	// Set the MIP Map flag.
	if (newHeader.getNumMipMapLevels() > 1)
	{
		legacyHeader.pixelFormatAndFlags |= texture_legacy::c_flagMipMap;
	}

	// Set the volume texture flag. Arrays of 3D textures will effectively become just 3D textures.
	if (newHeader.getDepth() > 1)
	{
		legacyHeader.pixelFormatAndFlags |= texture_legacy::c_flagVolumeTexture;
	}

	// Set the alpha flag for PVRTC1 data if appropriate
	if (newHeader.getPixelFormat().getPart().High == 0 &&
	    (newHeader.getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA) ||
	     newHeader.getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA)))
	{
		legacyHeader.pixelFormatAndFlags |= texture_legacy::c_flagHasAlpha;
		legacyHeader.alphaBitMask = 1;
	}

	// Set the cube map flag if appropriate.
	if (newHeader.getNumFaces() == 6)
	{
		legacyHeader.pixelFormatAndFlags |= texture_legacy::c_flagCubeMap;
	}

	// Check for bump map data
	if (newHeader.isBumpMap())
	{
		legacyHeader.pixelFormatAndFlags |= texture_legacy::c_flagBumpMap;
	}

	// Check if the texture is vertically flipped
	if (newHeader.getOrientation(TextureMetaData::AxisAxisY) == TextureMetaData::AxisOrientationUp)
	{
		legacyHeader.pixelFormatAndFlags |= texture_legacy::c_flagVerticalFlip;
	}

	return true;
}

bool TextureWriterLegacyPVR::mapNewFormatToLegacyEnum(texture_legacy::PixelFormat& legacyPixelType, const PixelFormat pixelType,
    const ColorSpace colorSpace, const VariableType channelType, const bool isPremultiplied)
{
	//Default error value
	legacyPixelType = texture_legacy::InvalidType;

	if (pixelType.getPart().High == 0)
	{
		switch (pixelType.getPart().Low)
		{
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA):
		{
			switch (_targetAPI)
			{
			case texture_legacy::ApiMGL:
			case texture_legacy::ApiD3DM:
			case texture_legacy::ApiDX10:
			case texture_legacy::ApiDX9:
				legacyPixelType = texture_legacy::MGL_PVRTC2;
				break;
			default:
				legacyPixelType = texture_legacy::GL_PVRTC2;
				break;
			}
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB):
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA):
		{
			switch (_targetAPI)
			{
			case texture_legacy::ApiMGL:
			case texture_legacy::ApiD3DM:
			case texture_legacy::ApiDX10:
			case texture_legacy::ApiDX9:
				legacyPixelType = texture_legacy::MGL_PVRTC4;
				break;
			default:
				legacyPixelType = texture_legacy::GL_PVRTC4;
				break;
			}
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp):
		{
			legacyPixelType = texture_legacy::GL_PVRTCII2;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp):
		{
			legacyPixelType = texture_legacy::GL_PVRTCII4;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ETC1):
		{
			legacyPixelType = texture_legacy::e_etc_RGB_4BPP;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::BW1bpp):
		{
			legacyPixelType = texture_legacy::VG_BW_1;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::YUY2):
		{
			legacyPixelType = texture_legacy::D3D_YUY2;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::UYVY):
		{
			legacyPixelType = texture_legacy::D3D_UYVY;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::RGBG8888):
		{
			legacyPixelType = texture_legacy::DXGI_R8G8_B8G8_UNORM;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::GRGB8888):
		{
			legacyPixelType = texture_legacy::DXGI_G8R8_G8B8_UNORM;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::DXT1):
		{
			if (_targetAPI == texture_legacy::ApiDX10)
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					legacyPixelType = texture_legacy::DXGI_BC1_UNORM;
					return true;
				}
				else
				{
					legacyPixelType = texture_legacy::DXGI_BC1_UNORM_SRGB;
					return true;
				}
			}
			else
			{
				legacyPixelType = texture_legacy::D3D_DXT1;
				return true;
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::DXT2):
		{
			legacyPixelType = texture_legacy::D3D_DXT2;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::DXT3):
		{
			if (_targetAPI == texture_legacy::ApiDX10)
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					legacyPixelType = texture_legacy::DXGI_BC2_UNORM;
					return true;
				}
				else
				{
					legacyPixelType = texture_legacy::DXGI_BC2_UNORM_SRGB;
					return true;
				}
			}
			else
			{
				legacyPixelType = texture_legacy::D3D_DXT3;
				return true;
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::DXT4):
		{
			legacyPixelType = texture_legacy::D3D_DXT4;
			return true;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::DXT5):
		{
			if (_targetAPI == texture_legacy::ApiDX10)
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					legacyPixelType = texture_legacy::DXGI_BC3_UNORM;
					return true;
				}
				else
				{
					legacyPixelType = texture_legacy::DXGI_BC3_UNORM_SRGB;
					return true;
				}
			}
			else
			{
				legacyPixelType = texture_legacy::D3D_DXT5;
				return true;
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::BC4):
		{
			switch (channelType)
			{
			case VariableType::UnsignedByteNorm:
			case VariableType::UnsignedShortNorm:
			case VariableType::UnsignedIntegerNorm:
				legacyPixelType = texture_legacy::DXGI_BC4_UNORM;
				return true;
			case VariableType::SignedByteNorm:
			case VariableType::SignedShortNorm:
			case VariableType::SignedIntegerNorm:
				legacyPixelType = texture_legacy::DXGI_BC4_SNORM;
				return true;
			default: return false;
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::BC5):
		{
			switch (channelType)
			{
			case VariableType::UnsignedByteNorm:
			case VariableType::UnsignedShortNorm:
			case VariableType::UnsignedIntegerNorm:
				legacyPixelType = texture_legacy::DXGI_BC5_UNORM;
				return true;
			case VariableType::SignedByteNorm:
			case VariableType::SignedShortNorm:
			case VariableType::SignedIntegerNorm:
				legacyPixelType = texture_legacy::DXGI_BC5_SNORM;
				return true;
			default: return false;

			}
			break;
		}
		break;
		default: return false;
		}
	}
	else
	{
		switch (channelType)
		{
		case VariableType::UnsignedByteNorm:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				if (_targetAPI == texture_legacy::ApiOVG)
				{
					if (colorSpace == ColorSpace::lRGB)
					{
						if (isPremultiplied)
						{
							legacyPixelType = texture_legacy::VG_lRGBA_8888_PRE;
						}
						else
						{
							legacyPixelType = texture_legacy::VG_lRGBA_8888;
						}
					}
					else
					{
						if (isPremultiplied)
						{
							legacyPixelType = texture_legacy::VG_sRGBA_8888_PRE;
						}
						else
						{
							legacyPixelType = texture_legacy::VG_sRGBA_8888;
						}
					}
				}
				else if (_targetAPI == texture_legacy::ApiDX10)
				{
					if (colorSpace == ColorSpace::lRGB)
					{
						legacyPixelType = texture_legacy::DXGI_R8G8B8A8_UNORM;
					}
					else
					{
						legacyPixelType = texture_legacy::DXGI_R8G8B8A8_UNORM_SRGB;
					}
				}
				else
				{
					legacyPixelType = texture_legacy::GL_RGBA_8888;
				}
				return true;
			}
			case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID:
			{
				if (_targetAPI == texture_legacy::ApiOVG)
				{
					if (colorSpace == ColorSpace::lRGB)
					{
						if (isPremultiplied)
						{
							legacyPixelType = texture_legacy::VG_lBGRA_8888_PRE;
						}
						else
						{
							legacyPixelType = texture_legacy::VG_lBGRA_8888;
						}
					}
					else
					{
						if (isPremultiplied)
						{
							legacyPixelType = texture_legacy::VG_sBGRA_8888_PRE;
						}
						else
						{
							legacyPixelType = texture_legacy::VG_sBGRA_8888;
						}
					}
				}
				else
				{
					legacyPixelType = texture_legacy::GL_BGRA_8888;
				}
				return true;
			}
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					if (isPremultiplied)
					{
						legacyPixelType = texture_legacy::VG_lARGB_8888_PRE;
					}
					else
					{
						legacyPixelType = texture_legacy::VG_lARGB_8888;
					}
				}
				else
				{
					if (isPremultiplied)
					{
						legacyPixelType = texture_legacy::VG_sARGB_8888_PRE;
					}
					else
					{
						legacyPixelType = texture_legacy::VG_sARGB_8888;
					}
				}
				return true;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					if (isPremultiplied)
					{
						legacyPixelType = texture_legacy::VG_lABGR_8888_PRE;
					}
					else
					{
						legacyPixelType = texture_legacy::VG_lABGR_8888;
					}
				}
				else
				{
					if (isPremultiplied)
					{
						legacyPixelType = texture_legacy::VG_sABGR_8888_PRE;
					}
					else
					{
						legacyPixelType = texture_legacy::VG_sABGR_8888;
					}
				}
				return true;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					legacyPixelType = texture_legacy::VG_lRGBX_8888;
				}
				else
				{
					legacyPixelType = texture_legacy::VG_sRGBX_8888;
				}
				return true;
			}
			case GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					legacyPixelType = texture_legacy::VG_lBGRX_8888;
				}
				else
				{
					legacyPixelType = texture_legacy::VG_sBGRX_8888;
				}
				return true;
			}
			case GeneratePixelType4<'x', 'r', 'g', 'b', 8, 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					legacyPixelType = texture_legacy::VG_lXRGB_8888;
				}
				else
				{
					legacyPixelType = texture_legacy::VG_sXRGB_8888;
				}
				return true;
			}
			case GeneratePixelType4<'x', 'b', 'g', 'r', 8, 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					legacyPixelType = texture_legacy::VG_lXBGR_8888;
				}
				else
				{
					legacyPixelType = texture_legacy::VG_sXBGR_8888;
				}
				return true;
			}
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::GL_RGB_888;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8G8_UNORM;
				return true;
			}
			case GeneratePixelType2<'a', 'i', 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::GL_AI_88;
				return true;
			}
			case GeneratePixelType1<'a', 8>::ID:
			{
				if (_targetAPI == texture_legacy::ApiOVG)
				{
					legacyPixelType = texture_legacy::VG_A_8;
				}
				else
				{
					legacyPixelType = texture_legacy::GL_A_8;
				}
				return true;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8_UNORM;
				return true;
			}
			case GeneratePixelType1<'i', 8>::ID:
			{
				legacyPixelType = texture_legacy::GL_I_8;
				return true;
			}
			case GeneratePixelType1<'l', 8>::ID:
			{
				if (colorSpace == ColorSpace::lRGB)
				{
					legacyPixelType = texture_legacy::VG_lL_8;
				}
				else
				{
					legacyPixelType = texture_legacy::VG_sL_8;
				}
				return true;
			}
			default: break;
			}
			break;
		}
		case VariableType::SignedByteNorm:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8G8B8A8_SNORM;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8G8_SNORM;
				return true;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8_SNORM;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedByte:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8G8B8A8_UINT;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8G8_UINT;
				return true;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8_UINT;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedByte:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8G8B8A8_SINT;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8G8_SINT;
				return true;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R8_SINT;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedShortNorm:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID:
			{
				if (_targetAPI == texture_legacy::ApiOVG && colorSpace == ColorSpace::sRGB)
				{
					legacyPixelType = texture_legacy::VG_sRGBA_4444;
				}
				else
				{
					legacyPixelType = texture_legacy::GL_RGBA_4444;
				}
				return true;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID:
			{
				if (_targetAPI == texture_legacy::ApiOVG && colorSpace == ColorSpace::sRGB)
				{
					legacyPixelType = texture_legacy::VG_sRGBA_5551;
				}
				else
				{
					legacyPixelType = texture_legacy::GL_RGBA_5551;
				}
				return true;
			}
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
			{
				if (_targetAPI == texture_legacy::ApiOVG && colorSpace == ColorSpace::sRGB)
				{
					legacyPixelType = texture_legacy::VG_sRGB_565;
				}
				else
				{
					legacyPixelType = texture_legacy::GL_RGB_565;
				}
				return true;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'x', 5, 5, 5, 1>::ID:
			{
				legacyPixelType = texture_legacy::GL_RGB_555;
				return true;
			}
			case GeneratePixelType4<'b', 'g', 'r', 'a', 4, 4, 4, 4>::ID:
			{
				legacyPixelType = texture_legacy::VG_sBGRA_4444;
				return true;
			}
			case GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID:
			{
				legacyPixelType = texture_legacy::VG_sARGB_4444;
				return true;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 4, 4, 4, 4>::ID:
			{
				legacyPixelType = texture_legacy::VG_sABGR_4444;
				return true;
			}
			case GeneratePixelType4<'b', 'g', 'r', 'a', 5, 5, 5, 1>::ID:
			{
				legacyPixelType = texture_legacy::VG_sBGRA_5551;
				return true;
			}
			case GeneratePixelType4<'a', 'r', 'g', 'b', 1, 5, 5, 5>::ID:
			{
				legacyPixelType = texture_legacy::VG_sARGB_1555;
				return true;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 1, 5, 5, 5>::ID:
			{
				legacyPixelType = texture_legacy::VG_sABGR_1555;
				return true;
			}
			case GeneratePixelType3<'b', 'g', 'r', 5, 6, 5>::ID:
			{
				legacyPixelType = texture_legacy::VG_sBGR_565;
				return true;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16B16A16_UNORM;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16_UNORM;
				return true;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16_UNORM;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedShortNorm:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16B16A16_SNORM;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16_SNORM;
				return true;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16_SNORM;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedShort:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16B16A16_UINT;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16_UINT;
				return true;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16_UINT;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedShort:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16B16A16_SINT;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16_SINT;
				return true;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16_SINT;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedIntegerNorm:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::MGL_RGB_888;
				return true;
			}
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::MGL_ARGB_8888;
				return true;
			}
			case GeneratePixelType2<'a', 'l', 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::D3D_AL_88;
				return true;
			}
			case GeneratePixelType1<'a', 8>::ID:
			{
				legacyPixelType = texture_legacy::D3D_A8;
				return true;
			}
			case GeneratePixelType1<'l', 8>::ID:
			{
				legacyPixelType = texture_legacy::D3D_L8;
				return true;
			}
			case GeneratePixelType2<'a', 'l', 4, 4>::ID:
			{
				legacyPixelType = texture_legacy::D3D_AL_44;
				return true;
			}
			case GeneratePixelType3<'r', 'g', 'b', 3, 3, 2>::ID:
			{
				legacyPixelType = texture_legacy::D3D_RGB_332;
				return true;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
			{
				legacyPixelType = texture_legacy::D3D_ABGR_2101010;
				return true;
			}
			case GeneratePixelType4<'a', 'r', 'g', 'b', 2, 10, 10, 10>::ID:
			{
				legacyPixelType = texture_legacy::D3D_ARGB_2101010;
				return true;
			}
			case GeneratePixelType1<'l', 16>::ID:
			{
				legacyPixelType = texture_legacy::D3D_L16;
				return true;
			}
			case GeneratePixelType2<'g', 'r', 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::D3D_GR_1616;
				return true;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::D3D_ABGR_16161616;
				return true;
			}
			case GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID:
			{
				legacyPixelType = texture_legacy::MGL_ARGB_4444;
				return true;
			}
			case GeneratePixelType4<'a', 'r', 'g', 'b', 1, 5, 5, 5>::ID:
			{
				legacyPixelType = texture_legacy::MGL_ARGB_1555;
				return true;
			}
			case GeneratePixelType4<'x', 'r', 'g', 'b', 1, 5, 5, 5>::ID:
			{
				legacyPixelType = texture_legacy::MGL_RGB_555;
				return true;
			}
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
			{
				legacyPixelType = texture_legacy::MGL_RGB_565;
				return true;
			}
			case GeneratePixelType4<'a', 'r', 'g', 'b', 8, 3, 3, 2>::ID:
			{
				legacyPixelType = texture_legacy::MGL_ARGB_8332;
				return true;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R10G10B10A2_UNORM;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedIntegerNorm:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType2<'g', 'r', 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::D3D_V8U8;
				return true;
			}
			case GeneratePixelType4<'x', 'l', 'g', 'r', 8, 8, 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::D3D_XLVU_8888;
				return true;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID:
			{
				legacyPixelType = texture_legacy::D3D_QWVU_8888;
				return true;
			}
			case GeneratePixelType3<'l', 'g', 'r', 6, 5, 5>::ID:
			{
				legacyPixelType = texture_legacy::D3D_LVU_655;
				return true;
			}
			case GeneratePixelType2<'g', 'r', 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::D3D_VU_1616;
				return true;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
			{
				legacyPixelType = texture_legacy::D3D_AWVU_2101010;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedInteger:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32G32B32A32_UINT;
				return true;
			}
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32G32B32_UINT;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32G32_UINT;
				return true;
			}
			case GeneratePixelType1<'r', 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32_UINT;
				return true;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R10G10B10A2_UINT;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedInteger:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32G32B32A32_SINT;
				return true;
			}
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32G32B32_SINT;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32G32_SINT;
				return true;
			}
			case GeneratePixelType1<'r', 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32_SINT;
				return true;
			}
			}
			break;
		}
		case VariableType::SignedFloat:
		{
			switch (pixelType.getPixelTypeId())
			{
			case GeneratePixelType1<'r', 16>::ID:
			{
				if (_targetAPI == texture_legacy::ApiD3DM)
				{
					legacyPixelType = texture_legacy::D3D_R16F;
				}
				else
				{
					legacyPixelType = texture_legacy::DXGI_R16_FLOAT;
				}
				return true;
			}
			case GeneratePixelType2<'g', 'r', 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::D3D_GR_1616F;
				return true;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::D3D_ABGR_16161616F;
				return true;
			}
			case GeneratePixelType1<'r', 32>::ID:
			{
				if (_targetAPI == texture_legacy::ApiD3DM)
				{
					legacyPixelType = texture_legacy::D3D_R32F;
				}
				else
				{
					legacyPixelType = texture_legacy::DXGI_R32_FLOAT;
				}
				return true;
			}
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				if (_targetAPI == texture_legacy::ApiD3DM)
				{
					legacyPixelType = texture_legacy::D3D_GR_3232F;
				}
				else
				{
					legacyPixelType = texture_legacy::DXGI_R32G32_FLOAT;
				}
				return true;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 32, 32, 32, 32>::ID:
			{
				legacyPixelType = texture_legacy::D3D_ABGR_32323232F;
				return true;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32G32B32A32_FLOAT;
				return true;
			}
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R32G32B32_FLOAT;
				return true;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16B16A16_FLOAT;
				return true;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R16G16_FLOAT;
				return true;
			}
			case GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID:
			{
				legacyPixelType = texture_legacy::DXGI_R11G11B10_FLOAT;
				return true;
			}
			}
			break;
		}
		case VariableType::UnsignedFloat: return false;
		}
	}

	return false;
}
}
}
}
//!\endcond