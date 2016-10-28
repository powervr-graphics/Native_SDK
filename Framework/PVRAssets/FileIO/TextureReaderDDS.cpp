/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\TextureReaderDDS.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods of the TextureReaderDDS class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureReaderDDS.h"
#include "PVRCore/Log.h"
using std::string;
using std::vector;
namespace pvr {
namespace assets {
namespace assetReaders {
TextureReaderDDS::TextureReaderDDS() : m_texturesToLoad(true)
{
}
TextureReaderDDS::TextureReaderDDS(Stream::ptr_type assetStream) : AssetReader<Texture>(assetStream), m_texturesToLoad(true)
{
}

bool TextureReaderDDS::readNextAsset(Texture& asset)
{
	if (m_assetStream->getSize() < texture_dds::c_expectedDDSSize)
	{
		return false;
	}

	// Acknowledge that once this function has returned the user won't be able load a texture from the file.
	m_texturesToLoad = false;

	bool result = true;

	size_t dataRead;

	texture_dds::FileHeader ddsFileHeader;

	// Read the magic identifier
	uint32 magic;
	result = m_assetStream->read(sizeof(magic), 1, &magic, dataRead);
	if (!result || dataRead != 1) { return result; }

	if (magic != texture_dds::c_magicIdentifier)
	{
		return false;
	}

	// Read the header size
	result = m_assetStream->read(sizeof(ddsFileHeader.size), 1, &ddsFileHeader.size, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Check that the size matches what's expected
	if (ddsFileHeader.size != texture_dds::c_expectedDDSSize)
	{
		return false;
	}

	// Read the flags
	result = m_assetStream->read(sizeof(ddsFileHeader.flags), 1, &ddsFileHeader.flags, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the width
	result = m_assetStream->read(sizeof(ddsFileHeader.width), 1, &ddsFileHeader.width, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the height
	result = m_assetStream->read(sizeof(ddsFileHeader.height), 1, &ddsFileHeader.height, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the pitchOrLinearSize
	result = m_assetStream->read(sizeof(ddsFileHeader.pitchOrLinearSize), 1, &ddsFileHeader.pitchOrLinearSize, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the depth
	result = m_assetStream->read(sizeof(ddsFileHeader.depth), 1, &ddsFileHeader.depth, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the number of MIP Map levels
	result = m_assetStream->read(sizeof(ddsFileHeader.mipMapCount), 1, &ddsFileHeader.mipMapCount, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the first chunk of "reserved" data (11 * uint32)
	result = m_assetStream->read(sizeof(ddsFileHeader.reserved[0]), 11, &ddsFileHeader.reserved, dataRead);
	if (!result || dataRead != 11) { return result; }

	// Read the Pixel Format size
	result = m_assetStream->read(sizeof(ddsFileHeader.pixelFormat.size), 1, &ddsFileHeader.pixelFormat.size, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Check that the Pixel Format size is correct
	if (ddsFileHeader.pixelFormat.size != texture_dds::c_expectedPixelFormatSize)
	{
		return false;
	}

	// Read the rest of the pixel format structure
	result = m_assetStream->read(sizeof(ddsFileHeader.pixelFormat.flags), 1, &ddsFileHeader.pixelFormat.flags, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = m_assetStream->read(sizeof(ddsFileHeader.pixelFormat.fourCC), 1, &ddsFileHeader.pixelFormat.fourCC, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = m_assetStream->read(sizeof(ddsFileHeader.pixelFormat.bitCount), 1, &ddsFileHeader.pixelFormat.bitCount, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = m_assetStream->read(sizeof(ddsFileHeader.pixelFormat.redMask), 1, &ddsFileHeader.pixelFormat.redMask, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = m_assetStream->read(sizeof(ddsFileHeader.pixelFormat.greenMask), 1, &ddsFileHeader.pixelFormat.greenMask, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = m_assetStream->read(sizeof(ddsFileHeader.pixelFormat.blueMask), 1, &ddsFileHeader.pixelFormat.blueMask, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = m_assetStream->read(sizeof(ddsFileHeader.pixelFormat.alphaMask), 1, &ddsFileHeader.pixelFormat.alphaMask, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the Capabilities2 structure
	result = m_assetStream->read(sizeof(ddsFileHeader.Capabilities1), 1, &ddsFileHeader.Capabilities1, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = m_assetStream->read(sizeof(ddsFileHeader.Capabilities2), 1, &ddsFileHeader.Capabilities2, dataRead);
	if (!result || dataRead != 1) { return result; }
	result = m_assetStream->read(sizeof(ddsFileHeader.reserved[0]), 2, &ddsFileHeader.reserved, dataRead);
	if (!result || dataRead != 2) { return result; }

	// Read the final miscFlags2 value
	result = m_assetStream->read(sizeof(ddsFileHeader.reserved2), 1, &ddsFileHeader.reserved2, dataRead);
	if (!result || dataRead != 1) { return result; }

	bool hasDX10Header = (ddsFileHeader.pixelFormat.flags & texture_dds::e_fourCC) &&
	                     ddsFileHeader.pixelFormat.fourCC == texture_dds::MakeFourCC<'D', 'X', '1', '0'>::FourCC;

	// Get the data for the DX10 header if present
	texture_dds::FileHeaderDX10 dx10FileHeader;
	if (hasDX10Header)
	{

		// Read the DX10 header
		result = m_assetStream->read(sizeof(dx10FileHeader.dxgiFormat), 1, &dx10FileHeader.dxgiFormat, dataRead);
		if (!result || dataRead != 1) { return result; }
		result = m_assetStream->read(sizeof(dx10FileHeader.resourceDimension), 1, &dx10FileHeader.resourceDimension, dataRead);
		if (!result || dataRead != 1) { return result; }
		result = m_assetStream->read(sizeof(dx10FileHeader.miscFlags), 1, &dx10FileHeader.miscFlags, dataRead);
		if (!result || dataRead != 1) { return result; }
		result = m_assetStream->read(sizeof(dx10FileHeader.arraySize), 1, &dx10FileHeader.arraySize, dataRead);
		if (!result || dataRead != 1) { return result; }
		result = m_assetStream->read(sizeof(dx10FileHeader.miscFlags2), 1, &dx10FileHeader.miscFlags2, dataRead);
		if (!result || dataRead != 1) { return result; }
	}

	// Construct the texture asset's header
	TextureHeader textureHeader;

	// There is a lot of different behaviour based on whether the DX10 header is there or not.
	if (hasDX10Header)
	{
		textureHeader.setDirectXGIFormat(dx10FileHeader.dxgiFormat);

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

		if ((ddsFileHeader.flags & texture_dds::e_mipMapCount) || (ddsFileHeader.Capabilities1 & texture_dds::e_mipMaps))
		{
			textureHeader.setNumberOfMIPLevels(ddsFileHeader.mipMapCount);
		}
		if (dx10FileHeader.miscFlags & texture_dds::e_textureCube)
		{
			textureHeader.setNumberOfFaces(6);
		}
		textureHeader.setNumberOfArrayMembers((dx10FileHeader.arraySize == 0) ? 1 : dx10FileHeader.arraySize);

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
		uint32 d3dFormat = getDirect3DFormatFromDDSHeader(ddsFileHeader);
		textureHeader.setDirect3DFormat(d3dFormat);
		textureHeader.setWidth(ddsFileHeader.width);
		textureHeader.setHeight(ddsFileHeader.height);
		if ((ddsFileHeader.flags & texture_dds::e_depth) || (ddsFileHeader.Capabilities2 & texture_dds::e_volume))
		{
			textureHeader.setDepth(ddsFileHeader.depth);
		}
		if ((ddsFileHeader.flags & texture_dds::e_mipMapCount) || (ddsFileHeader.Capabilities1 & texture_dds::e_mipMaps))
		{
			textureHeader.setNumberOfMIPLevels(ddsFileHeader.mipMapCount);
		}
		if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMap)
		{
			uint32 numberOfFaces = 0;
			string faceOrder;

			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapPositiveX)
			{
				faceOrder += "X";
				++numberOfFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapNegativeX)
			{
				faceOrder += "x";
				++numberOfFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapPositiveY)
			{
				faceOrder += "Y";
				++numberOfFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapNegativeY)
			{
				faceOrder += "y";
				++numberOfFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapPositiveZ)
			{
				faceOrder += "Z";
				++numberOfFaces;
			}
			if (ddsFileHeader.Capabilities2 & texture_dds::e_cubeMapNegativeZ)
			{
				faceOrder += "z";
				++numberOfFaces;
			}

			textureHeader.setNumberOfFaces(numberOfFaces);
			textureHeader.setCubeMapOrder(faceOrder);
		}
	}

	// Initialize the texture to allocate data
	asset = Texture(textureHeader, NULL);

	// Read in the texture data
	for (uint32 surface = 0; surface < asset.getNumberOfArrayMembers(); ++surface)
	{
		for (uint32 face = 0; face < asset.getNumberOfFaces(); ++face)
		{
			for (uint32 mipMapLevel = 0; mipMapLevel < asset.getNumberOfMIPLevels(); ++mipMapLevel)
			{
				//Read in the texture data.
				result = m_assetStream->read(asset.getDataSize(mipMapLevel, false, false), 1, asset.getDataPointer(mipMapLevel, surface, face),
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
	return m_texturesToLoad;
}

bool TextureReaderDDS::canHaveMultipleAssets()
{
	return false;
}

uint32 TextureReaderDDS::getDirect3DFormatFromDDSHeader(texture_dds::FileHeader& textureFileHeader)
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
	uint32 magic;
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

vector<string> TextureReaderDDS::getSupportedFileExtensions()
{
	vector<string> extensions;
	extensions.push_back("dds");
	return vector<string>(extensions);
}

}
}
}
//!\endcond