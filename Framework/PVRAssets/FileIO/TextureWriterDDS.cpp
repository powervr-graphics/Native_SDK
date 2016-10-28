/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\TextureWriterDDS.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods of the TextureWriterDDS class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureWriterDDS.h"
#include <algorithm>
using std::string;
using std::vector;
namespace pvr {
namespace assets {
namespace assetWriters {
bool TextureWriterDDS::addAssetToWrite(const Texture& asset)
{
	if (m_assetsToWrite.size() < 1)
	{
		m_assetsToWrite.push_back(&asset);
		return true;
	}
	else
	{
		assertion(0 ,  "OUT OF BOUNDS");
		return false;
	}
}

bool TextureWriterDDS::writeAllAssets()
{
	//Boolean as to whether we should work in DDS10 compatibility mode.
	bool doDDS10 = false;

	//DirectX 9-style DDS headers can't handle texture arrays, so if we have one of these we should use a DX10-style DDS.
	if (m_assetsToWrite[0]->getNumberOfArrayMembers() > 1)
	{
		doDDS10 = true;
	}

	// Create a DDS texture header and a DX10 texture header if appropriate
	texture_dds::FileHeader ddsFileHeader;
	texture_dds::FileHeaderDX10 ddsFileHeaderDX10;

	// Set common data that doesn't depend on whether it's a DX10 header or not
	ddsFileHeader.size = texture_dds::c_expectedDDSSize;
	ddsFileHeader.flags = texture_dds::e_Capabilities | texture_dds::e_width | texture_dds::e_height | texture_dds::e_pixelFormat;
	ddsFileHeader.height = m_assetsToWrite[0]->getHeight();
	ddsFileHeader.width = m_assetsToWrite[0]->getWidth();
	ddsFileHeader.depth = m_assetsToWrite[0]->getDepth();
	ddsFileHeader.mipMapCount = m_assetsToWrite[0]->getNumberOfMIPLevels();
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
	if (ddsFileHeader.mipMapCount > 1)
	{
		ddsFileHeader.flags |= texture_dds::e_mipMapCount;
		ddsFileHeader.Capabilities1 |= texture_dds::e_mipMaps;
		ddsFileHeader.Capabilities1 |= texture_dds::e_complex;
	}

	// Set the pitch or linear size
	if (m_assetsToWrite[0]->getPixelFormat().getPart().High == 0)
	{
		ddsFileHeader.flags |= texture_dds::e_linearSize;
		ddsFileHeader.pitchOrLinearSize = m_assetsToWrite[0]->getDataSize(0);
	}
	else
	{
		ddsFileHeader.flags |= texture_dds::e_pitch;
		ddsFileHeader.pitchOrLinearSize = std::max<uint32>(1,
		                                  (m_assetsToWrite[0]->getWidth() * m_assetsToWrite[0]->getBitsPerPixel() + 7) / 8);
	}

	// Proper cube map handling is a little complicated, but doable.
	if (m_assetsToWrite[0]->getNumberOfFaces() > 1)
	{
		if (m_assetsToWrite[0]->getNumberOfFaces() > 6)
		{
			assertion(0 ,  "Invalid Argument");
			return false;
		}

		ddsFileHeader.Capabilities2 |= texture_dds::e_cubeMap;

		std::string cubeFaces = m_assetsToWrite[0]->getCubeMapOrder();

		// Handle the cube map's faces, setting flags for faces that are available.
		for (uint32 face = 0; face < m_assetsToWrite[0]->getNumberOfFaces(); ++face)
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
	uint32 d3dFormat;
	if (doDDS10 || !m_assetsToWrite[0]->getDirect3DFormat(d3dFormat))
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
		if (!m_assetsToWrite[0]->getDirectXGIFormat(ddsFileHeaderDX10.dxgiFormat, notAlpha))
		{
			assertion(0 ,  "INVALID ARGUMENT");
			return false;
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
		if (m_assetsToWrite[0]->getNumberOfFaces() == 6)
		{
			ddsFileHeaderDX10.miscFlags = texture_dds::e_textureCube;
		}
		else if (m_assetsToWrite[0]->getNumberOfFaces() != 1)
		{
			assertion(0 ,  "INVALID ARGUMENT");
			return false;
		}

		// Set the array size.
		ddsFileHeaderDX10.arraySize = m_assetsToWrite[0]->getNumberOfArrayMembers();

		if (notAlpha)
		{
			ddsFileHeaderDX10.miscFlags2 = texture_dds::e_custom;
		}
		else if (m_assetsToWrite[0]->isPreMultiplied())
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

		if (m_assetsToWrite[0]->getPixelFormat().getPixelTypeId() == (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGBA ||
		    m_assetsToWrite[0]->getPixelFormat().getPixelTypeId() == (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGBA)
		{
			ddsFileHeader.pixelFormat.flags |= texture_dds::e_alphaPixels;
		}
	}

	// Start writing data

	// Check the size of data written.
	size_t dataWritten = 0;

	// Write the identifier
	if (!m_assetStream->write(sizeof(texture_dds::c_magicIdentifier), 1, &texture_dds::c_magicIdentifier, dataWritten) || dataWritten != 1) { return false; }

	// Write the DDS Header
	if (!writeFileHeader(ddsFileHeader)) { return false; }

	// Write the DX10 Header if appropriate
	if (doDDS10)
	{
		if (!writeFileHeaderDX10(ddsFileHeaderDX10)) { return false; }
	}

	// Write the texture data
	for (uint32 surface = 0; surface < m_assetsToWrite[0]->getNumberOfArrayMembers(); ++surface)
	{
		for (uint32 face = 0; face < m_assetsToWrite[0]->getNumberOfFaces(); ++face)
		{
			for (uint32 mipMapLevel = 0; mipMapLevel < m_assetsToWrite[0]->getNumberOfMIPLevels(); ++mipMapLevel)
			{
				// Write out all the data - DDS files have a different order to PVR v3 files, but are not affected by padding.
				if (!m_assetStream->write(m_assetsToWrite[0]->getDataSize(mipMapLevel, false, false), 1,
				                          m_assetsToWrite[0]->getDataPointer(mipMapLevel, surface, face), dataWritten))
				{ return false; }
			}
		}
	}

	// Return
	return true;
}

uint32 TextureWriterDDS::assetsAddedSoFar()
{
	return static_cast<uint32>(m_assetsToWrite.size());
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

bool TextureWriterDDS::writeFileHeader(const texture_dds::FileHeader& ddsFileHeader)
{
	bool result = true;
	size_t dataWritten = 0;

	// Write the size
	result = m_assetStream->write(sizeof(ddsFileHeader.size), 1, &ddsFileHeader.size, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the flags
	result = m_assetStream->write(sizeof(ddsFileHeader.flags), 1, &ddsFileHeader.flags, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the height
	result = m_assetStream->write(sizeof(ddsFileHeader.height), 1, &ddsFileHeader.height, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the width
	result = m_assetStream->write(sizeof(ddsFileHeader.width), 1, &ddsFileHeader.width, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the pitchOrLinearSize
	result = m_assetStream->write(sizeof(ddsFileHeader.pitchOrLinearSize), 1, &ddsFileHeader.pitchOrLinearSize, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the depth
	result = m_assetStream->write(sizeof(ddsFileHeader.depth), 1, &ddsFileHeader.depth, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the mipMapCount
	result = m_assetStream->write(sizeof(ddsFileHeader.mipMapCount), 1, &ddsFileHeader.mipMapCount, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the reserved data
	result = m_assetStream->write(sizeof(ddsFileHeader.reserved[0]), 11, &ddsFileHeader.reserved, dataWritten);
	if (result != true || dataWritten != 11) { return result; }

	// Write the pixelFormat
	{
		// Write the size
		result = m_assetStream->write(sizeof(ddsFileHeader.pixelFormat.size), 1, &ddsFileHeader.pixelFormat.size, dataWritten);
		if (result != true || dataWritten != 1) { return result; }

		// Write the flags
		result = m_assetStream->write(sizeof(ddsFileHeader.pixelFormat.flags), 1, &ddsFileHeader.pixelFormat.flags, dataWritten);
		if (result != true || dataWritten != 1) { return result; }

		// Write the fourCC
		result = m_assetStream->write(sizeof(ddsFileHeader.pixelFormat.fourCC), 1, &ddsFileHeader.pixelFormat.fourCC, dataWritten);
		if (result != true || dataWritten != 1) { return result; }

		// Write the bitCount
		result = m_assetStream->write(sizeof(ddsFileHeader.pixelFormat.bitCount), 1, &ddsFileHeader.pixelFormat.bitCount, dataWritten);
		if (result != true || dataWritten != 1) { return result; }

		// Write the redMask
		result = m_assetStream->write(sizeof(ddsFileHeader.pixelFormat.redMask), 1, &ddsFileHeader.pixelFormat.redMask, dataWritten);
		if (result != true || dataWritten != 1) { return result; }

		// Write the greenMask
		result = m_assetStream->write(sizeof(ddsFileHeader.pixelFormat.greenMask), 1, &ddsFileHeader.pixelFormat.greenMask, dataWritten);
		if (result != true || dataWritten != 1) { return result; }

		// Write the blueMask
		result = m_assetStream->write(sizeof(ddsFileHeader.pixelFormat.blueMask), 1, &ddsFileHeader.pixelFormat.blueMask, dataWritten);
		if (result != true || dataWritten != 1) { return result; }

		// Write the alphaMask
		result = m_assetStream->write(sizeof(ddsFileHeader.pixelFormat.alphaMask), 1, &ddsFileHeader.pixelFormat.alphaMask, dataWritten);
		if (result != true || dataWritten != 1) { return result; }
	}

	// Write the Capabilities values
	result = m_assetStream->write(sizeof(ddsFileHeader.Capabilities1), 1, &ddsFileHeader.Capabilities1, dataWritten);
	if (result != true || dataWritten != 1) { return result; }
	result = m_assetStream->write(sizeof(ddsFileHeader.Capabilities2), 1, &ddsFileHeader.Capabilities2, dataWritten);
	if (result != true  || dataWritten != 1) { return result; }
	result = m_assetStream->write(sizeof(ddsFileHeader.Capabilities3), 1, &ddsFileHeader.Capabilities3, dataWritten);
	if (result != true || dataWritten != 1) { return result; }
	result = m_assetStream->write(sizeof(ddsFileHeader.Capabilities4), 1, &ddsFileHeader.Capabilities4, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the final reserved value
	result = m_assetStream->write(sizeof(ddsFileHeader.reserved2), 1, &ddsFileHeader.reserved2, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	return result;
}

bool TextureWriterDDS::writeFileHeaderDX10(const texture_dds::FileHeaderDX10& ddsFileHeaderDX10)
{
	bool result = true;
	size_t dataWritten = 0;

	// Write the DXGI format
	result = m_assetStream->write(sizeof(ddsFileHeaderDX10.dxgiFormat), 1, &ddsFileHeaderDX10.dxgiFormat, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the resource dimension
	result = m_assetStream->write(sizeof(ddsFileHeaderDX10.resourceDimension), 1, &ddsFileHeaderDX10.resourceDimension, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the first miscellaneous flags
	result = m_assetStream->write(sizeof(ddsFileHeaderDX10.miscFlags), 1, &ddsFileHeaderDX10.miscFlags, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the array size
	result = m_assetStream->write(sizeof(ddsFileHeaderDX10.arraySize), 1, &ddsFileHeaderDX10.arraySize, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	// Write the second miscellaneous flags
	result = m_assetStream->write(sizeof(ddsFileHeaderDX10.miscFlags2), 1, &ddsFileHeaderDX10.miscFlags2, dataWritten);
	if (result != true || dataWritten != 1) { return result; }

	return result;
}

bool TextureWriterDDS::canWriteAsset(const Texture& asset)
{
	uint32 d3dFormat, dxgiFormat;
	bool notAlpha;
	return (asset.getDirect3DFormat(d3dFormat) || asset.getDirectXGIFormat(dxgiFormat, notAlpha));
}

vector<string> TextureWriterDDS::getSupportedFileExtensions()
{
	vector<string> extensions;
	extensions.push_back("dds");
	return vector<string>(extensions);
}

string TextureWriterDDS::getWriterName()
{
	return "PowerVR Direct Draw Surface Writer";
}

string TextureWriterDDS::getWriterVersion()
{
	return "1.0.0";
}
}
}
}
//!\endcond
