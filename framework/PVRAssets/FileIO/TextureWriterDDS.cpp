/*!
\brief Implementation of methods of the TextureWriterDDS class.
\file PVRAssets/FileIO/TextureWriterDDS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureWriterDDS.h"
#include <algorithm>
using std::string;
using std::vector;
namespace pvr {
namespace assets {
namespace assetWriters {
void TextureWriterDDS::addAssetToWrite(const Texture& asset)
{
	if (_assetsToWrite.size() >= 1)
	{
		throw InvalidOperationError("[TextureWriterDDS::addAssetToWrite] Attempted to add asset but an asset was already added. DDS only supports one texture per file");
	}
	_assetsToWrite.push_back(&asset);
}

void TextureWriterDDS::writeAllAssets()
{
	// Boolean as to whether we should work in DDS10 compatibility mode.
	bool doDDS10 = false;

	// DirectX 9-style DDS headers can't handle texture arrays, so if we have one of these we should use a DX10-style DDS.
	if (_assetsToWrite[0]->getNumArrayMembers() > 1)
	{
		doDDS10 = true;
	}

	// Create a DDS texture header and a DX10 texture header if appropriate
	texture_dds::FileHeader ddsFileHeader;
	texture_dds::FileHeaderDX10 ddsFileHeaderDX10;

	// Set common data that doesn't depend on whether it's a DX10 header or not
	ddsFileHeader.size = texture_dds::c_expectedDDSSize;
	ddsFileHeader.flags = texture_dds::e_Capabilities | texture_dds::e_width | texture_dds::e_height | texture_dds::e_pixelFormat;
	ddsFileHeader.height = _assetsToWrite[0]->getHeight();
	ddsFileHeader.width = _assetsToWrite[0]->getWidth();
	ddsFileHeader.depth = _assetsToWrite[0]->getDepth();
	ddsFileHeader.numMipMaps = _assetsToWrite[0]->getNumMipMapLevels();
	ddsFileHeader.Capabilities1 = texture_dds::e_texture;
	ddsFileHeader.Capabilities2 = 0;
	ddsFileHeader.Capabilities3 = 0;
	ddsFileHeader.Capabilities4 = 0;

	// Initialize the reserved data.
	ddsFileHeader.reserved2 = 0;
	memset(ddsFileHeader.reserved, 0, sizeof(ddsFileHeader.reserved));

	if (ddsFileHeader.depth > 1)
	{
		ddsFileHeader.flags |= texture_dds::e_depth;
		ddsFileHeader.Capabilities2 |= texture_dds::e_volume;
	}
	if (ddsFileHeader.numMipMaps > 1)
	{
		ddsFileHeader.flags |= texture_dds::e_numMipMaps;
		ddsFileHeader.Capabilities1 |= texture_dds::e_mipMaps;
		ddsFileHeader.Capabilities1 |= texture_dds::e_complex;
	}

	// Set the pitch or linear size
	if (_assetsToWrite[0]->getPixelFormat().getPart().High == 0)
	{
		ddsFileHeader.flags |= texture_dds::e_linearSize;
		ddsFileHeader.pitchOrLinearSize = _assetsToWrite[0]->getDataSize(0);
	}
	else
	{
		ddsFileHeader.flags |= texture_dds::e_pitch;
		ddsFileHeader.pitchOrLinearSize = std::max<uint32_t>(1, (_assetsToWrite[0]->getWidth() * _assetsToWrite[0]->getBitsPerPixel() + 7) / 8);
	}

	// Proper cube map handling is a little complicated, but doable.
	if (_assetsToWrite[0]->getNumFaces() > 1)
	{
		if (_assetsToWrite[0]->getNumFaces() > 6)
		{
			throw InvalidDataError("[TextureWriterDDS::writeAllAssets]: Texture object had more than 6 cube faces.");
		}

		ddsFileHeader.Capabilities2 |= texture_dds::e_cubeMap;

		std::string cubeFaces = _assetsToWrite[0]->getCubeMapOrder();

		// Handle the cube map's faces, setting flags for faces that are available.
		for (uint32_t face = 0; face < _assetsToWrite[0]->getNumFaces(); ++face)
		{
			switch (cubeFaces[face])
			{
			case 'X':
			{
				ddsFileHeader.Capabilities2 |= texture_dds::e_cubeMapPositiveX;
				break;
			}
			case 'x':
			{
				ddsFileHeader.Capabilities2 |= texture_dds::e_cubeMapNegativeX;
				break;
			}
			case 'Y':
			{
				ddsFileHeader.Capabilities2 |= texture_dds::e_cubeMapPositiveY;
				break;
			}
			case 'y':
			{
				ddsFileHeader.Capabilities2 |= texture_dds::e_cubeMapNegativeY;
				break;
			}
			case 'Z':
			{
				ddsFileHeader.Capabilities2 |= texture_dds::e_cubeMapPositiveZ;
				break;
			}
			case 'z':
			{
				ddsFileHeader.Capabilities2 |= texture_dds::e_cubeMapNegativeZ;
				break;
			}
			}
		}
	}

	// Check if a DDS file can be written without the DX10 header first, as this maintains the highest compatibility.
	uint32_t d3dFormat;
	if (doDDS10 || !_assetsToWrite[0]->getDirect3DFormat(d3dFormat))
	{
		// If a D3D format does not exist, attempt to write it as a DXGI format in a DX10 style header
		doDDS10 = true;

		// Sort out the pixel format first
		ddsFileHeader.pixelFormat.size = sizeof(texture_dds::PixelFormat);
		ddsFileHeader.pixelFormat.flags = texture_dds::e_fourCC;
		ddsFileHeader.pixelFormat.fourCC = texture_dds::MakeFourCC<'D', 'X', '1', '0'>::FourCC;
		ddsFileHeader.pixelFormat.bitCount = 0;
		ddsFileHeader.pixelFormat.redMask = 0;
		ddsFileHeader.pixelFormat.greenMask = 0;
		ddsFileHeader.pixelFormat.blueMask = 0;
		ddsFileHeader.pixelFormat.alphaMask = 0;

		// Set the dxgi format, and whether or not it's actually a custom channel ('x') rather than actually alpha. Not confusing at all.
		bool notAlpha = false;
		if (!_assetsToWrite[0]->getDirectXGIFormat(ddsFileHeaderDX10.dxgiFormat, notAlpha))
		{
			throw InvalidDataError("[TextureWriterDDS::writeAllAssets]: Texture object had unrecognise DXGI format.");
		}

		// Set the resource dimension
		if (ddsFileHeader.depth > 1)
		{
			ddsFileHeaderDX10.resourceDimension = texture_dds::e_texture3D;
		}
		else if (ddsFileHeader.height > 1)
		{
			ddsFileHeaderDX10.resourceDimension = texture_dds::e_texture2D;
		}
		else
		{
			ddsFileHeaderDX10.resourceDimension = texture_dds::e_texture1D;
		}

		// Check for a cube map - only full cube maps are supported.
		if (_assetsToWrite[0]->getNumFaces() == 6)
		{
			ddsFileHeaderDX10.miscFlags = texture_dds::e_textureCube;
		}
		else if (_assetsToWrite[0]->getNumFaces() != 1)
		{
			throw InvalidDataError("[TextureWriterDDS::writeAllAssets]: Non-cube object had number of faces that was not (1).");
		}

		// Set the array size.
		ddsFileHeaderDX10.arraySize = _assetsToWrite[0]->getNumArrayMembers();

		if (notAlpha)
		{
			ddsFileHeaderDX10.miscFlags2 = texture_dds::e_custom;
		}
		else if (_assetsToWrite[0]->isPreMultiplied())
		{
			ddsFileHeaderDX10.miscFlags2 = texture_dds::e_premultiplied;
		}
		else
		{
			// Technically this should be "straight", but the legacy D3DX 10/11 libraries would fail to load it.
			ddsFileHeaderDX10.miscFlags2 = texture_dds::Unknown;
		}
	}
	else
	{
		ddsFileHeader.pixelFormat.size = sizeof(texture_dds::PixelFormat);
		ddsFileHeader.pixelFormat.flags = 0;
		ddsFileHeader.pixelFormat.fourCC = 0;
		ddsFileHeader.pixelFormat.bitCount = 0;
		ddsFileHeader.pixelFormat.redMask = 0;
		ddsFileHeader.pixelFormat.greenMask = 0;
		ddsFileHeader.pixelFormat.blueMask = 0;
		ddsFileHeader.pixelFormat.alphaMask = 0;

		setDirect3DFormatToDDSHeader(static_cast<texture_dds::D3DFormat>(d3dFormat), ddsFileHeader);

		if (_assetsToWrite[0]->getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA) ||
			_assetsToWrite[0]->getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA))
		{
			ddsFileHeader.pixelFormat.flags |= texture_dds::e_alphaPixels;
		}
	}

	// Write the identifier
	_assetStream->writeExact(sizeof(texture_dds::c_magicIdentifier), 1, &texture_dds::c_magicIdentifier);

	// Write the DDS Header
	writeFileHeader(ddsFileHeader);

	// Write the DX10 Header if appropriate
	if (doDDS10)
	{
		writeFileHeaderDX10(ddsFileHeaderDX10);
	}

	// Write the texture data
	for (uint32_t surface = 0; surface < _assetsToWrite[0]->getNumArrayMembers(); ++surface)
	{
		for (uint32_t face = 0; face < _assetsToWrite[0]->getNumFaces(); ++face)
		{
			for (uint32_t mipMapLevel = 0; mipMapLevel < _assetsToWrite[0]->getNumMipMapLevels(); ++mipMapLevel)
			{
				// Write out all the data - DDS files have a different order to PVR v3 files, but are not affected by padding.
				_assetStream->writeExact(_assetsToWrite[0]->getDataSize(mipMapLevel, false, false), 1, _assetsToWrite[0]->getDataPointer(mipMapLevel, surface, face));
			}
		}
	}
}

uint32_t TextureWriterDDS::assetsAddedSoFar()
{
	return static_cast<uint32_t>(_assetsToWrite.size());
}

bool TextureWriterDDS::supportsMultipleAssets()
{
	return false;
}

bool TextureWriterDDS::setDirect3DFormatToDDSHeader(texture_dds::D3DFormat d3dFormat, texture_dds::FileHeader& ddsFileHeader)
{
	switch (d3dFormat)
	{
	case texture_dds::D3DFMT_DXT1:
	case texture_dds::D3DFMT_DXT2:
	case texture_dds::D3DFMT_DXT3:
	case texture_dds::D3DFMT_DXT4:
	case texture_dds::D3DFMT_DXT5:
	case texture_dds::D3DFMT_PVRTC2:
	case texture_dds::D3DFMT_PVRTC4:
	case texture_dds::D3DFMT_A16B16G16R16:
	case texture_dds::D3DFMT_R16F:
	case texture_dds::D3DFMT_G16R16F:
	case texture_dds::D3DFMT_A16B16G16R16F:
	case texture_dds::D3DFMT_R32F:
	case texture_dds::D3DFMT_G32R32F:
	case texture_dds::D3DFMT_A32B32G32R32F:
	case texture_dds::D3DFMT_YUY2:
	case texture_dds::D3DFMT_UYVY:
	{
		ddsFileHeader.pixelFormat.fourCC = d3dFormat;
		break;
	}
	case texture_dds::D3DFMT_A4R4G4B4:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb | texture_dds::e_alphaPixels;
		ddsFileHeader.pixelFormat.alphaMask = 0x0000f000;
		ddsFileHeader.pixelFormat.redMask = 0x00000f00;
		ddsFileHeader.pixelFormat.greenMask = 0x000000f0;
		ddsFileHeader.pixelFormat.blueMask = 0x0000000f;
		ddsFileHeader.pixelFormat.bitCount = 16;
		break;
	}
	case texture_dds::D3DFMT_A1R5G5B5:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb | texture_dds::e_alphaPixels;
		ddsFileHeader.pixelFormat.alphaMask = 0x00008000;
		ddsFileHeader.pixelFormat.redMask = 0x00007C00;
		ddsFileHeader.pixelFormat.greenMask = 0x000003E0;
		ddsFileHeader.pixelFormat.blueMask = 0x0000001F;
		ddsFileHeader.pixelFormat.bitCount = 16;
		break;
	}
	case texture_dds::D3DFMT_R5G6B5:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb;
		ddsFileHeader.pixelFormat.redMask = 0x0000F800;
		ddsFileHeader.pixelFormat.greenMask = 0x000007E0;
		ddsFileHeader.pixelFormat.blueMask = 0x0000001F;
		ddsFileHeader.pixelFormat.bitCount = 16;
		break;
	}
	case texture_dds::D3DFMT_X1R5G5B5:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb;
		ddsFileHeader.pixelFormat.redMask = 0x00007C00;
		ddsFileHeader.pixelFormat.greenMask = 0x000003E0;
		ddsFileHeader.pixelFormat.blueMask = 0x0000001F;
		ddsFileHeader.pixelFormat.bitCount = 16;
		break;
	}
	case texture_dds::D3DFMT_R8G8B8:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb;
		ddsFileHeader.pixelFormat.redMask = 0x00ff0000;
		ddsFileHeader.pixelFormat.greenMask = 0x0000ff00;
		ddsFileHeader.pixelFormat.blueMask = 0x000000ff;
		ddsFileHeader.pixelFormat.bitCount = 24;
		break;
	}
	case texture_dds::D3DFMT_A8R8G8B8:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb | texture_dds::e_alphaPixels;
		ddsFileHeader.pixelFormat.alphaMask = 0xff000000;
		ddsFileHeader.pixelFormat.redMask = 0x00ff0000;
		ddsFileHeader.pixelFormat.greenMask = 0x0000ff00;
		ddsFileHeader.pixelFormat.blueMask = 0x000000ff;
		ddsFileHeader.pixelFormat.bitCount = 32;
		break;
	}
	case texture_dds::D3DFMT_A8R3G3B2:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb | texture_dds::e_alphaPixels;
		ddsFileHeader.pixelFormat.alphaMask = 0x0000ff00;
		ddsFileHeader.pixelFormat.redMask = 0x000000E0;
		ddsFileHeader.pixelFormat.greenMask = 0x0000001C;
		ddsFileHeader.pixelFormat.blueMask = 0x00000003;
		ddsFileHeader.pixelFormat.bitCount = 16;
		break;
	}
	case texture_dds::D3DFMT_R3G3B2:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb;
		ddsFileHeader.pixelFormat.redMask = 0x000000E0;
		ddsFileHeader.pixelFormat.greenMask = 0x0000001C;
		ddsFileHeader.pixelFormat.blueMask = 0x00000003;
		ddsFileHeader.pixelFormat.bitCount = 8;
		break;
	}
	case texture_dds::D3DFMT_L8:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_luminance;
		ddsFileHeader.pixelFormat.redMask = 0xff;
		ddsFileHeader.pixelFormat.bitCount = 8;
		break;
	}
	case texture_dds::D3DFMT_A8L8:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_luminance | texture_dds::e_alphaPixels;
		ddsFileHeader.pixelFormat.alphaMask = 0xff00;
		ddsFileHeader.pixelFormat.redMask = 0xff;
		ddsFileHeader.pixelFormat.bitCount = 16;
		break;
	}
	case texture_dds::D3DFMT_A4L4:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_luminance | texture_dds::e_alphaPixels;
		ddsFileHeader.pixelFormat.alphaMask = 0xf0;
		ddsFileHeader.pixelFormat.redMask = 0xf;
		ddsFileHeader.pixelFormat.bitCount = 8;
		break;
	}
	case texture_dds::D3DFMT_A2R10G10B10:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb | texture_dds::e_alphaPixels;
		ddsFileHeader.pixelFormat.alphaMask = 0xc0000000;
		ddsFileHeader.pixelFormat.redMask = 0x3ff;
		ddsFileHeader.pixelFormat.greenMask = 0xffc00;
		ddsFileHeader.pixelFormat.blueMask = 0x3ff00000;
		ddsFileHeader.pixelFormat.bitCount = 32;
		break;
	}
	case texture_dds::D3DFMT_A2B10G10R10:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb | texture_dds::e_alphaPixels;
		ddsFileHeader.pixelFormat.alphaMask = 0xc0000000;
		ddsFileHeader.pixelFormat.redMask = 0x3ff00000;
		ddsFileHeader.pixelFormat.greenMask = 0xffc00;
		ddsFileHeader.pixelFormat.blueMask = 0x3ff;
		ddsFileHeader.pixelFormat.bitCount = 32;
		break;
	}
	case texture_dds::D3DFMT_G16R16:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_rgb;
		ddsFileHeader.pixelFormat.redMask = 0xffff;
		ddsFileHeader.pixelFormat.greenMask = 0xffff0000;
		ddsFileHeader.pixelFormat.bitCount = 32;
		break;
	}
	case texture_dds::D3DFMT_A8:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_alpha;
		ddsFileHeader.pixelFormat.alphaMask = 0xff;
		ddsFileHeader.pixelFormat.bitCount = 8;
		break;
	}
	case texture_dds::D3DFMT_L16:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_luminance;
		ddsFileHeader.pixelFormat.redMask = 0xffff;
		ddsFileHeader.pixelFormat.bitCount = 16;
		break;
	}
	case texture_dds::D3DFMT_V8U8:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_unknownBump2;
		ddsFileHeader.pixelFormat.redMask = 0x000000ff;
		ddsFileHeader.pixelFormat.greenMask = 0x0000ff00;
		ddsFileHeader.pixelFormat.bitCount = 16;
		break;
	}
	case texture_dds::D3DFMT_Q8W8V8U8:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_unknownBump2;
		ddsFileHeader.pixelFormat.alphaMask = 0xff000000;
		ddsFileHeader.pixelFormat.redMask = 0x000000ff;
		ddsFileHeader.pixelFormat.greenMask = 0x0000ff00;
		ddsFileHeader.pixelFormat.blueMask = 0x00ff0000;
		ddsFileHeader.pixelFormat.bitCount = 32;
		break;
	}
	case texture_dds::D3DFMT_L6V5U5:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_unknownBump1;
		ddsFileHeader.pixelFormat.redMask = 0x0000001f;
		ddsFileHeader.pixelFormat.greenMask = 0x000003e0;
		ddsFileHeader.pixelFormat.blueMask = 0x0000fc00;
		ddsFileHeader.pixelFormat.bitCount = 16;
		break;
	}
	case texture_dds::D3DFMT_X8L8V8U8:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_unknownBump1;
		ddsFileHeader.pixelFormat.redMask = 0x000000ff;
		ddsFileHeader.pixelFormat.greenMask = 0x0000ff00;
		ddsFileHeader.pixelFormat.blueMask = 0x00ff0000;
		ddsFileHeader.pixelFormat.bitCount = 32;
		break;
	}
	case texture_dds::D3DFMT_A2W10V10U10:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_unknownBump1 | texture_dds::e_alphaPixels;
		ddsFileHeader.pixelFormat.alphaMask = 0xc0000000;
		ddsFileHeader.pixelFormat.redMask = 0x3ff00000;
		ddsFileHeader.pixelFormat.greenMask = 0xffc00;
		ddsFileHeader.pixelFormat.blueMask = 0x3ff;
		ddsFileHeader.pixelFormat.bitCount = 32;
		break;
	}
	case texture_dds::D3DFMT_V16U16:
	{
		ddsFileHeader.pixelFormat.flags = texture_dds::e_unknownBump2;
		ddsFileHeader.pixelFormat.redMask = 0x0000ffff;
		ddsFileHeader.pixelFormat.greenMask = 0xffff0000;
		ddsFileHeader.pixelFormat.bitCount = 32;
		break;
	}
	default:
	{
		ddsFileHeader.pixelFormat.fourCC = texture_dds::D3DFMT_UNKNOWN;
		return false;
	}
	}

	return true;
}

void TextureWriterDDS::writeFileHeader(const texture_dds::FileHeader& ddsFileHeader)
{
	// Write the size
	_assetStream->writeExact(sizeof(ddsFileHeader.size), 1, &ddsFileHeader.size);
	// Write the flags
	_assetStream->writeExact(sizeof(ddsFileHeader.flags), 1, &ddsFileHeader.flags);
	// Write the height
	_assetStream->writeExact(sizeof(ddsFileHeader.height), 1, &ddsFileHeader.height);
	// Write the width
	_assetStream->writeExact(sizeof(ddsFileHeader.width), 1, &ddsFileHeader.width);
	// Write the pitchOrLinearSize
	_assetStream->writeExact(sizeof(ddsFileHeader.pitchOrLinearSize), 1, &ddsFileHeader.pitchOrLinearSize);
	// Write the depth
	_assetStream->writeExact(sizeof(ddsFileHeader.depth), 1, &ddsFileHeader.depth);
	// Write the numMipMaps
	_assetStream->writeExact(sizeof(ddsFileHeader.numMipMaps), 1, &ddsFileHeader.numMipMaps);
	// Write the reserved data
	_assetStream->writeExact(sizeof(ddsFileHeader.reserved[0]), 11, &ddsFileHeader.reserved);
	// Write the pixelFormat
	{
		// Write the size
		_assetStream->writeExact(sizeof(ddsFileHeader.pixelFormat.size), 1, &ddsFileHeader.pixelFormat.size);
		// Write the flags
		_assetStream->writeExact(sizeof(ddsFileHeader.pixelFormat.flags), 1, &ddsFileHeader.pixelFormat.flags);
		// Write the fourCC
		_assetStream->writeExact(sizeof(ddsFileHeader.pixelFormat.fourCC), 1, &ddsFileHeader.pixelFormat.fourCC);
		// Write the bitCount
		_assetStream->writeExact(sizeof(ddsFileHeader.pixelFormat.bitCount), 1, &ddsFileHeader.pixelFormat.bitCount);
		// Write the redMask
		_assetStream->writeExact(sizeof(ddsFileHeader.pixelFormat.redMask), 1, &ddsFileHeader.pixelFormat.redMask);
		// Write the greenMask
		_assetStream->writeExact(sizeof(ddsFileHeader.pixelFormat.greenMask), 1, &ddsFileHeader.pixelFormat.greenMask);
		// Write the blueMask
		_assetStream->writeExact(sizeof(ddsFileHeader.pixelFormat.blueMask), 1, &ddsFileHeader.pixelFormat.blueMask);
		// Write the alphaMask
		_assetStream->writeExact(sizeof(ddsFileHeader.pixelFormat.alphaMask), 1, &ddsFileHeader.pixelFormat.alphaMask);
	}

	// Write the Capabilities values
	_assetStream->writeExact(sizeof(ddsFileHeader.Capabilities1), 1, &ddsFileHeader.Capabilities1);
	_assetStream->writeExact(sizeof(ddsFileHeader.Capabilities2), 1, &ddsFileHeader.Capabilities2);
	_assetStream->writeExact(sizeof(ddsFileHeader.Capabilities3), 1, &ddsFileHeader.Capabilities3);
	_assetStream->writeExact(sizeof(ddsFileHeader.Capabilities4), 1, &ddsFileHeader.Capabilities4);
	// Write the final reserved value
	_assetStream->writeExact(sizeof(ddsFileHeader.reserved2), 1, &ddsFileHeader.reserved2);
}

void TextureWriterDDS::writeFileHeaderDX10(const texture_dds::FileHeaderDX10& ddsFileHeaderDX10)
{
	// Write the DXGI format
	_assetStream->writeExact(sizeof(ddsFileHeaderDX10.dxgiFormat), 1, &ddsFileHeaderDX10.dxgiFormat);
	// Write the resource dimension
	_assetStream->writeExact(sizeof(ddsFileHeaderDX10.resourceDimension), 1, &ddsFileHeaderDX10.resourceDimension);
	// Write the first miscellaneous flags
	_assetStream->writeExact(sizeof(ddsFileHeaderDX10.miscFlags), 1, &ddsFileHeaderDX10.miscFlags);
	// Write the array size
	_assetStream->writeExact(sizeof(ddsFileHeaderDX10.arraySize), 1, &ddsFileHeaderDX10.arraySize);
	// Write the second miscellaneous flags
	_assetStream->writeExact(sizeof(ddsFileHeaderDX10.miscFlags2), 1, &ddsFileHeaderDX10.miscFlags2);
}

bool TextureWriterDDS::canWriteAsset(const Texture& asset)
{
	uint32_t d3dFormat, dxgiFormat;
	bool notAlpha;
	return (asset.getDirect3DFormat(d3dFormat) || asset.getDirectXGIFormat(dxgiFormat, notAlpha));
}

vector<std::string> TextureWriterDDS::getSupportedFileExtensions()
{
	vector<std::string> extensions;
	extensions.push_back("dds");
	return vector<std::string>(extensions);
}

std::string TextureWriterDDS::getWriterName()
{
	return "PowerVR Direct Draw Surface Writer";
}

std::string TextureWriterDDS::getWriterVersion()
{
	return "1.0.0";
}
} // namespace assetWriters
} // namespace assets
} // namespace pvr
//!\endcond