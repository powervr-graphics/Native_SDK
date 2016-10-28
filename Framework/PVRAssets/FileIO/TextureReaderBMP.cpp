/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\TextureReaderBMP.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods of the TextureReaderBMP class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/FilePath.h"
#include "PVRCore/Log.h"
#include "PVRAssets/FileIO/TextureReaderBMP.h"
#include "PVRAssets/FileIO/PaletteExpander.h"
#include "PVRAssets/Texture/MetaData.h"
using std::string;
using std::vector;
namespace pvr {
namespace assets {
namespace assetReaders {
TextureReaderBMP::TextureReaderBMP() : m_texturesToLoad(true)
{
}
TextureReaderBMP::TextureReaderBMP(Stream::ptr_type assetStream) : AssetReader<Texture>(assetStream), m_texturesToLoad(true)
{ }

bool TextureReaderBMP::readNextAsset(Texture& asset)
{
	bool result = true;
	if (m_hasNewAssetStream)
	{
		result = initializeFile();
		if (result)
		{
			m_texturesToLoad = true;
		}
		else
		{
			return result;
		}
		m_hasNewAssetStream = false;
	}
	// Check the Result

	long streamPosition = (long)m_assetStream->getPosition();
	if (result)
	{
		result = loadImageFromFile(asset);
	}

	if (!result)
	{
		// Return to the beginning of the texture data if not loaded correctly.
		m_assetStream->seek(streamPosition, Stream::SeekOriginFromStart);
	}
	else
	{
		// If it succeeded, let the user know that there are no more texture to load.
		m_texturesToLoad = false;
	}

	// Return the result
	return result;
}

bool TextureReaderBMP::hasAssetsLeftToLoad()
{
	return m_texturesToLoad;
}

bool TextureReaderBMP::canHaveMultipleAssets()
{
	return false;
}

bool TextureReaderBMP::isSupportedFile(Stream& assetStream)
{
	// Try to open the stream
	bool result = assetStream.open();
	if (!result)
	{
		return false;
	}

	// Read the magic identifier
	uint16 magic;
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

	// Check that the identifier matches
	if (magic != texture_bmp::Identifier)
	{
		return false;
	}

	return true;
}

vector<string> TextureReaderBMP::getSupportedFileExtensions()
{
	vector<string> extensions;
	extensions.push_back("bmp");
	return vector<string>(extensions);
}

bool TextureReaderBMP::initializeFile()
{
	// Read the file header
	bool result = readFileHeader(m_fileHeader);
	if (!result) { return result; }

	// Mark that the header has been loaded
	m_fileHeaderLoaded = true;

	return result;
}

bool TextureReaderBMP::loadImageFromFile(Texture& asset)
{
	bool result   = true;
	size_t      dataRead = 0;

	// Make sure the file is ready to load
	if (!m_fileHeaderLoaded || !m_texturesToLoad)
	{
		return false;
	}

	// Read the size of the description header, then the rest of the header.
	uint32 headerSize = 0;
	m_assetStream->read(sizeof(headerSize), 1, &headerSize, dataRead);

	switch (headerSize)
	{
	case texture_bmp::HeaderSize::Core:
	{
		// Read the core header
		texture_bmp::CoreHeader coreHeader;
		result = readCoreHeader(headerSize, coreHeader);
		if (!result) { return result; }

		// Translate it into a Texture
		result = readImageCoreHeader(coreHeader, asset);
		if (!result) { return result; }

		break;
	}
	case texture_bmp::HeaderSize::Core2:
	{
		Log(Log.Error, "Reading from \"%s\" - Version 2 Core Headers are not supported.",
		    m_assetStream->getFileName().c_str());
		// break;
		return false;
	}
	case texture_bmp::HeaderSize::Info1:
	case texture_bmp::HeaderSize::Info2:
	case texture_bmp::HeaderSize::Info3:
	case texture_bmp::HeaderSize::Info4:
	case texture_bmp::HeaderSize::Info5:
	{
		// Read the info header
		texture_bmp::InfoHeader5 infoHeader;
		result = readInfoHeader(headerSize, infoHeader);
		if (!result) { return result; }

		// Translate it into a TextureHeader
		result = readImageInfoHeader(infoHeader, asset);
		if (!result) { return result; }

		break;
	}
	default:
	{
		Log(Log.Error, "Reading from \"%s\" - Undefined image header size.", m_assetStream->getFileName().c_str());
		// Invalid header size, unrecognised format!
		return false;
	}
	}

	// Signify that the image has been loaded.
	m_texturesToLoad = false;

	return result;
}

bool TextureReaderBMP::readFileHeader(texture_bmp::FileHeader& fileheader)
{
	// Data Size check
	size_t dataRead = 0;
	bool result = true;

	// Read the magic identifier
	result = m_assetStream->read(sizeof(fileheader.signature), 1, &fileheader.signature, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Make sure the signature is correct
	if (m_fileHeader.signature != texture_bmp::Identifier)
	{
		return false;
	}

	// Read the recorded file size
	result = m_assetStream->read(sizeof(fileheader.fileSize), 1, &fileheader.fileSize, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the first reserved data
	result = m_assetStream->read(sizeof(fileheader.reserved1), 1, &fileheader.reserved1, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the second reserved data
	result = m_assetStream->read(sizeof(fileheader.reserved2), 1, &fileheader.reserved2, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the pixel data offset from the beginning of the file
	result = m_assetStream->read(sizeof(fileheader.pixelOffset), 1, &fileheader.pixelOffset, dataRead);
	if (!result || dataRead != 1) { return result; }

	return result;
}

bool TextureReaderBMP::readCoreHeader(uint32 headerSize, texture_bmp::CoreHeader& coreHeader)
{
	// Data Size check
	size_t dataRead = 0;
	bool result = true;

	// Set the header size
	coreHeader.headerSize = headerSize;

	// Read the width
	result = m_assetStream->read(sizeof(coreHeader.width), 1, &coreHeader.width, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the height
	result = m_assetStream->read(sizeof(coreHeader.height), 1, &coreHeader.height, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the number of planes
	result = m_assetStream->read(sizeof(coreHeader.numberOfPlanes), 1, &coreHeader.numberOfPlanes, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Make sure the number of planes is one
	if (coreHeader.numberOfPlanes != 1)
	{
		return false;
	}

	// Read the bits per pixel
	result = m_assetStream->read(sizeof(coreHeader.bitsPerPixel), 1, &coreHeader.bitsPerPixel, dataRead);
	if (!result || dataRead != 1) { return result; }

	return result;
}

bool TextureReaderBMP::readInfoHeader(uint32 headerSize, texture_bmp::InfoHeader5& infoHeader)
{
	// Data Size check
	size_t dataRead = 0;
	bool result = true;

	// Set the header size
	infoHeader.headerSize = headerSize;

	// Read the width
	result = m_assetStream->read(sizeof(infoHeader.width), 1, &infoHeader.width, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the height
	result = m_assetStream->read(sizeof(infoHeader.height), 1, &infoHeader.height, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the number of planes
	result = m_assetStream->read(sizeof(infoHeader.numberOfPlanes), 1, &infoHeader.numberOfPlanes, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Make sure the number of planes is one
	if (infoHeader.numberOfPlanes != 1)
	{
		return false;
	}

	// Read the bits per pixel
	result = m_assetStream->read(sizeof(infoHeader.bitsPerPixel), 1, &infoHeader.bitsPerPixel, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the compression type
	result = m_assetStream->read(sizeof(infoHeader.compressionType), 1, &infoHeader.compressionType, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the image size
	result = m_assetStream->read(sizeof(infoHeader.imageSize), 1, &infoHeader.imageSize, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the horizontal pixels per meter
	result = m_assetStream->read(sizeof(infoHeader.horizontalPixelsPerMeter), 1, &infoHeader.horizontalPixelsPerMeter, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the vertical pixels per meter
	result = m_assetStream->read(sizeof(infoHeader.verticalPixelsPerMeter), 1, &infoHeader.verticalPixelsPerMeter, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the number of colors in the table
	result = m_assetStream->read(sizeof(infoHeader.numberOfColorsInTable), 1, &infoHeader.numberOfColorsInTable, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the number of important colors in the table
	result = m_assetStream->read(sizeof(infoHeader.numberOfImportantColors), 1, &infoHeader.numberOfImportantColors, dataRead);
	if (!result || dataRead != 1) { return result; }

	if (headerSize >= texture_bmp::HeaderSize::Info1)
	{
		// Read the red mask
		result = m_assetStream->read(sizeof(infoHeader.redMask), 1, &infoHeader.redMask, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Read the green mask
		result = m_assetStream->read(sizeof(infoHeader.greenMask), 1, &infoHeader.greenMask, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Read the blue mask
		result = m_assetStream->read(sizeof(infoHeader.blueMask), 1, &infoHeader.blueMask, dataRead);
		if (!result || dataRead != 1) { return result; }
	}

	if (headerSize >= texture_bmp::HeaderSize::Info3)
	{
		// Read the alpha mask
		result = m_assetStream->read(sizeof(infoHeader.alphaMask), 1, &infoHeader.alphaMask, dataRead);
		if (!result || dataRead != 1) { return result; }
	}

	if (headerSize >= texture_bmp::HeaderSize::Info4)
	{
		// Read the color space
		result = m_assetStream->read(sizeof(infoHeader.alphaMask), 1, &infoHeader.alphaMask, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Read the XYZ endpoints
		result = m_assetStream->read(sizeof(infoHeader.xyzEndPoints[0]), 3, infoHeader.xyzEndPoints, dataRead);
		if (!result || dataRead != 3) { return result; }

		// Read the red gamma correction
		result = m_assetStream->read(sizeof(infoHeader.gammaRed), 1, &infoHeader.gammaRed, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Read the green gamma correction
		result = m_assetStream->read(sizeof(infoHeader.gammaGreen), 1, &infoHeader.gammaRed, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Read the blue gamma correction
		result = m_assetStream->read(sizeof(infoHeader.gammaBlue), 1, &infoHeader.gammaRed, dataRead);
		if (!result || dataRead != 1) { return result; }
	}

	if (headerSize >= texture_bmp::HeaderSize::Info5)
	{
		// Read the intent
		result = m_assetStream->read(sizeof(infoHeader.intent), 1, &infoHeader.intent, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Read the profile data offset
		result = m_assetStream->read(sizeof(infoHeader.profileData), 1, &infoHeader.profileData, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Read the profile size
		result = m_assetStream->read(sizeof(infoHeader.profileSize), 1, &infoHeader.profileSize, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Read the reserved bit
		result = m_assetStream->read(sizeof(infoHeader.reserved), 1, &infoHeader.reserved, dataRead);
		if (!result || dataRead != 1) { return result; }
	}

	return result;
}

bool TextureReaderBMP::translateInfoHeader(const texture_bmp::InfoHeader5& infoHeader, TextureHeader& header)
{
	uint32 orientation = 0;

	// Set the width and height
	if (infoHeader.width < 0)
	{
		header.setWidth(-1 * infoHeader.width);
		orientation |= TextureMetaData::AxisOrientationLeft;
	}
	else
	{
		header.setWidth(infoHeader.width);
		orientation |= TextureMetaData::AxisOrientationRight;
	}

	if (infoHeader.height < 0)
	{
		header.setHeight(-1 * infoHeader.height);
		orientation |= TextureMetaData::AxisOrientationDown;
	}
	else
	{
		header.setHeight(infoHeader.height);
		orientation |= TextureMetaData::AxisOrientationUp;
	}

	header.setOrientation(static_cast<TextureMetaData::AxisOrientation>(orientation));

	// The pixel format to be chosen is a little painful, but workable:
	if (infoHeader.compressionType == texture_bmp::CompressionMethod::Bitfields &&
	    infoHeader.headerSize >= texture_bmp::HeaderSize::Info2)
	{
		assertion(false ,  "Check for gaps in the bitfields, these are invalid. A single gap at the end is ok - shove in an X channel.");
	}
	else if (infoHeader.compressionType == texture_bmp::CompressionMethod::AlphaBitfields &&
	         infoHeader.headerSize >= texture_bmp::HeaderSize::Info3)
	{
		assertion(false ,  " Check for gaps in the bitfields, and that the scheme can be represented by PVRTexTool. An X channel can't be put in at the end as above if there's already 4 channels.");
	}
	else
	{
		switch (infoHeader.bitsPerPixel)
		{
		case 1:
		case 2:
		case 4:
		case 8:
		{
			if (infoHeader.headerSize >= texture_bmp::HeaderSize::Info3 && infoHeader.alphaMask != 0)
			{
				header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID);
			}
			else
			{
				header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID);
			}
			break;
		}
		case 16:
		{
			if (infoHeader.headerSize >= texture_bmp::HeaderSize::Info3 && infoHeader.alphaMask != 0)
			{
				header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'a', 5, 5, 5, 1>::ID);
			}
			else
			{
				header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'x', 5, 5, 5, 1>::ID);
			}
			break;
		}
		case 24:
		{
			header.setPixelFormat(GeneratePixelType3<'b', 'g', 'r', 8, 8, 8>::ID);
			break;
		}
		case 32:
		{
			if (infoHeader.headerSize >= texture_bmp::HeaderSize::Info3 && infoHeader.alphaMask != 0)
			{
				header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID);
			}
			else
			{
				header.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID);
			}
			break;
		}
		}
	}

	if (infoHeader.headerSize >= texture_bmp::HeaderSize::Info5)
	{
		switch (infoHeader.colorSpace)
		{
		case texture_bmp::ColorSpace::CalibratedRGB:
		{
			// Currently, gamma correction is ignored
			break;
		}
		case texture_bmp::ColorSpace::sRGB:
		case texture_bmp::ColorSpace::Windows:
		{
			header.setColorSpace(types::ColorSpace::lRGB);
			break;
		}
		case texture_bmp::ColorSpace::ProfileLinked:
		case texture_bmp::ColorSpace::ProfileEmbedded:
		{
			// Currently Unsupported
			return false;
		}
		}
	}

	return true;
}

bool TextureReaderBMP::translateCoreHeader(const texture_bmp::CoreHeader& coreHeader, TextureHeader& header)
{
	// Set the width and height
	header.setWidth(coreHeader.width);
	header.setHeight(coreHeader.height);
	header.setPixelFormat(GeneratePixelType3<'b', 'g', 'r', 8, 8, 8>::ID);
	header.setOrientation(TextureMetaData::AxisOrientationUp);

	return true;
}

bool TextureReaderBMP::readImageCoreHeader(const texture_bmp::CoreHeader& coreHeader, Texture& texture)
{
	bool result = true;

	// Create the texture header
	TextureHeader header;
	result = translateCoreHeader(coreHeader, header);
	if (!result) {return result;}
	// Create the texture from the header
	texture = Texture(header);

	// Load the image data as appropriate
	switch (coreHeader.bitsPerPixel)
	{
	case 1:
	case 4:
	case 8:
	{
		// Try to load the data
		result = loadIndexed(texture, texture.getBitsPerPixel() / 8, coreHeader.bitsPerPixel, (1 << coreHeader.bitsPerPixel), 4);
		if (!result) { return result; }

		break;
	}
	case 24:
	{
		// Straightforward row reading
		result = loadRowAligned(texture, (coreHeader.bitsPerPixel / 8), 4);
		if (!result) { return result; }

		break;
	}
	default:
	{
		return false;
	}
	}

	return result;
}

bool TextureReaderBMP::readImageInfoHeader(const texture_bmp::InfoHeader5& infoHeader, Texture& texture)
{
	bool result = true;

	// Create the texture header
	TextureHeader header;
	result = translateInfoHeader(infoHeader, header);
	if (!result) {return result;}
	// Create the texture from the header
	texture = Texture(header);

	// Check the allocation was successful
	if (texture.getDataSize() == 0)
	{
		return false;
	}

	if (infoHeader.compressionType == texture_bmp::CompressionMethod::RunLength4 ||
	    infoHeader.compressionType == texture_bmp::CompressionMethod::RunLength8)
	{
		assertion(false);
		return false;
	}
	else if (infoHeader.compressionType == texture_bmp::CompressionMethod::None ||
	         infoHeader.compressionType == texture_bmp::CompressionMethod::Bitfields ||
	         infoHeader.compressionType == texture_bmp::CompressionMethod::AlphaBitfields)
	{
		// Load the image data as appropriate
		switch (infoHeader.bitsPerPixel)
		{
		case 1:
		case 2:
		case 4:
		case 8:
		{
			// Work out the number of colors in the palette
			uint32 numberOfPaletteEntries = infoHeader.numberOfColorsInTable;
			if (numberOfPaletteEntries == 0)
			{
				numberOfPaletteEntries = 1 << infoHeader.bitsPerPixel;
			}

			// Try to load the data
			result = loadIndexed(texture, texture.getBitsPerPixel() / 8, infoHeader.bitsPerPixel, numberOfPaletteEntries, 4);
			if (!result) { return result; }

			break;
		}
		case 16:
		case 24:
		case 32:
		{
			// Straightforward row reading
			result = loadRowAligned(texture, (infoHeader.bitsPerPixel / 8), 4);
			if (!result) { return result; }

			break;
		}
		default:
		{
			return false;
		}
		}
	}
	else
	{
		return false;
	}

	return result;
}

bool TextureReaderBMP::loadRowAligned(Texture& asset, uint32 bytesPerDataEntry, uint32 rowAlignment)
{
	bool result = true;
	size_t dataRead = 0;

	// Calculate the number of bytes to skip in each row
	uint32 bytesPerScanline = asset.getWidth() * bytesPerDataEntry;
	uint32 scanlinePadding = ((-1 * bytesPerScanline) % rowAlignment);

	// Start reading data
	byte* outputPixel = asset.getDataPointer();

	for (uint32 y = 0; y < (asset.getHeight()); ++y)
	{
		// Read the next scan line
		result = m_assetStream->read(bytesPerDataEntry, asset.getWidth(), outputPixel, dataRead);
		if (!result || dataRead != asset.getWidth()) { return result; }

		// Increment the pixel
		outputPixel += bytesPerScanline;

		// seek past the scan line padding
		result = m_assetStream->seek(scanlinePadding, Stream::SeekOriginFromCurrent);
		if (!result) { return result; }
	}

	return result;
}

bool TextureReaderBMP::loadIndexed(Texture& asset, uint32 bytesPerPaletteEntry, uint32 bitsPerDataEntry,
                                   uint32 numberOfPaletteEntries, uint32 rowAlignment)
{
	bool result = true;
	size_t dataRead = 0;

	// Work out the size of the palette data entries
	uint32 paletteSize = numberOfPaletteEntries * bytesPerPaletteEntry;

	// Allocate data to read the palette into.
	vector<byte> paletteData;
	paletteData.resize(paletteSize);

	// Read the palette
	result = m_assetStream->read(bytesPerPaletteEntry, numberOfPaletteEntries, &paletteData[0], dataRead);
	if (!result || dataRead != numberOfPaletteEntries) { return result; }

	// Create the palette helper class
	PaletteExpander paletteLookup(&paletteData[0], paletteSize, bytesPerPaletteEntry);

	// seek to the pixel data
	result = m_assetStream->seek(m_fileHeader.pixelOffset, Stream::SeekOriginFromStart);
	if (!result) { return result; }

	// Make sure that there are a POT number of bits.
	if (!((bitsPerDataEntry != 0) && (!(bitsPerDataEntry & (bitsPerDataEntry - 1)))))
	{
		Log(Log.Error, "Reading from \"%s\" - Non-Power of two number of bits specified, unable to load.",
		    m_assetStream->getFileName().c_str());
		return false;
	}

	// Work out the number of indices per byte, and bytes per index
	uint32 indicesPerByte = 8 / bitsPerDataEntry;

	// Calculate the number of bytes to skip in each row
	uint32 bytesPerScanline = (asset.getWidth() + (indicesPerByte - 1)) / indicesPerByte;
	uint32 scanlinePadding = (((-1 * bytesPerScanline) % rowAlignment) * 8) / 8;

	// Work out the bit mask of the index value
	uint8 indexMask = 0xffu >> (8 - bitsPerDataEntry);

	// Start reading data
	byte* outputPixel = asset.getDataPointer();
	uint8 currentIndexData = 0;
	for (uint32 y = 0; y < (asset.getHeight()); ++y)
	{
		for (uint32 x = 0; x < (asset.getWidth()); x += indicesPerByte)
		{
			// Read the next byte of indices
			result = m_assetStream->read(1, 1, &currentIndexData, dataRead);
			if (!result || dataRead != 1) { return result; }

			// Loop through all the indices in the byte
			for (uint32 indexPosition = 0; indexPosition < indicesPerByte; ++indexPosition)
			{
				if ((x + indexPosition) < asset.getWidth())
				{
					// Mask out the actual index from the current index data
					uint8 bitShift = (8 - (bitsPerDataEntry * (indexPosition + 1)));

					uint8 actualIndex = (currentIndexData & (indexMask << (bitShift))) >> (bitShift);

					// Get the color output - don't need to check for errors, as the out of bounds condition is impossible due to the mask.
					paletteLookup.getColorFromIndex(actualIndex, outputPixel);

					// Increment the pixel
					outputPixel += bytesPerPaletteEntry;
				}
			}
		}

		// seek past the scan line padding
		result = m_assetStream->seek(scanlinePadding, Stream::SeekOriginFromCurrent);
		if (!result) { return result; }
	}

	return result;
}
}
}
}
//!\endcond
