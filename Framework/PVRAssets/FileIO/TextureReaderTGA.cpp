/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\TextureReaderTGA.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods of the TextureReaderTGA class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/FilePath.h"
#include "PVRCore/Log.h"

#include "PVRAssets/FileIO/TextureReaderTGA.h"
#include "PVRAssets/FileIO/PaletteExpander.h"
#include <algorithm>
using std::vector;
namespace pvr {
namespace assets {
namespace assetReaders {
TextureReaderTGA::TextureReaderTGA() : m_texturesToLoad(true)
{ }

TextureReaderTGA::TextureReaderTGA(Stream::ptr_type assetStream) : AssetReader<Texture>(assetStream), m_texturesToLoad(true)
{ }

bool TextureReaderTGA::readNextAsset(Texture& asset)
{
	// Check the Result
	bool result = true;
	if (m_hasNewAssetStream)
	{
		result = initializeFile();

		if (result)
		{
			m_texturesToLoad = true;
		}
		m_hasNewAssetStream = false;
	}

	long streamPosition = (long)m_assetStream->getPosition();
	if (result)
	{
		result = loadImageFromFile(asset);
	}

	if (result)
	{
		// If it succeeded, let the user know that there are no more texture to load.
		m_texturesToLoad = false;
	}
	else
	{
		// Return to the beginning of the texture data if not loaded correctly.
		m_assetStream->seek(streamPosition, Stream::SeekOriginFromStart);
	}

	// Return the result
	return result;
}

bool TextureReaderTGA::hasAssetsLeftToLoad()
{
	return m_texturesToLoad;
}

bool TextureReaderTGA::canHaveMultipleAssets()
{
	return false;
}

bool TextureReaderTGA::isSupportedFile(Stream& assetStream)
{
	// Try to open the stream
	FilePath filePath(assetStream.getFileName());
	string fileExt;
	std::transform(filePath.getFileExtension().begin(), filePath.getFileExtension().end(), fileExt.begin(), ::tolower);
	return fileExt == "tga";
}

vector<string> TextureReaderTGA::getSupportedFileExtensions()
{
	vector<string> extensions;
	extensions.push_back("tga");
	return vector<string>(extensions);
}

bool TextureReaderTGA::initializeFile()
{
	// Read the file header
	bool result = readFileHeader(m_fileHeader);
	if (!result) { return result; }

	// Skip the identifier area
	result = m_assetStream->seek(m_fileHeader.identSize, Stream::SeekOriginFromCurrent);
	if (!result) { return result; }

	// Mark that the header has been loaded
	m_fileHeaderLoaded = true;

	return result;
}

bool TextureReaderTGA::readFileHeader(texture_tga::FileHeader& fileheader)
{
	// Data Size check
	size_t dataRead = 0;
	bool result = true;

	// Read the size of the identifier area
	result = m_assetStream->read(sizeof(fileheader.identSize), 1, &fileheader.identSize, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the color map type
	result = m_assetStream->read(sizeof(fileheader.colorMapType), 1, &fileheader.colorMapType, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the image type
	result = m_assetStream->read(sizeof(fileheader.imageType), 1, &fileheader.imageType, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the start position of the color map
	result = m_assetStream->read(sizeof(fileheader.colorMapStart), 1, &fileheader.colorMapStart, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the length of the color map
	result = m_assetStream->read(sizeof(fileheader.colorMapLength), 1, &fileheader.colorMapLength, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the number of bits per palette entry in the color map
	result = m_assetStream->read(sizeof(fileheader.colorMapBits), 1, &fileheader.colorMapBits, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the horizontal offset for the start of the image
	result = m_assetStream->read(sizeof(fileheader.xStart), 1, &fileheader.xStart, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the vertical offset for the start of the image
	result = m_assetStream->read(sizeof(fileheader.yStart), 1, &fileheader.yStart, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the width of the image
	result = m_assetStream->read(sizeof(fileheader.width), 1, &fileheader.width, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the height of the image
	result = m_assetStream->read(sizeof(fileheader.height), 1, &fileheader.height, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the bits per pixel in the image
	result = m_assetStream->read(sizeof(fileheader.bits), 1, &fileheader.bits, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the descriptor flags
	result = m_assetStream->read(sizeof(fileheader.descriptor), 1, &fileheader.descriptor, dataRead);
	if (!result || dataRead != 1) { return result; }

	return result;
}

bool TextureReaderTGA::loadImageFromFile(Texture& asset)
{
	bool result = true;

	// Make sure the file is ready to load
	if (!m_fileHeaderLoaded || !m_texturesToLoad)
	{
		assertion(0 ,  "[TextureReaderTGA::loadImageFromFile] Attempted to read empty TGA.");
		return false;
	}

	// Setup the texture header.
	TextureHeader textureHeader;

	// Set the width and height from the file header.
	textureHeader.setWidth(m_fileHeader.width);
	textureHeader.setHeight(m_fileHeader.height);

	// Check whether the alpha value is ignored or not.
	bool alphaIgnored = ((m_fileHeader.descriptor & texture_tga::DescriptorFlagAlpha) == 0);

	// Get the bytes per data entry
	uint32 bytesPerDataEntry = m_fileHeader.bits / 8;
	if (m_fileHeader.bits == 15)
	{
		bytesPerDataEntry = 2;
	}

	// Get the bytes per color map entry
	uint32 bytesPerPaletteEntry = m_fileHeader.colorMapBits / 8;
	if (m_fileHeader.colorMapBits == 15)
	{
		bytesPerPaletteEntry = 2;
	}

	// Work out the bits per pixel of the final pixel format
	uint32 bitsPerPixel = m_fileHeader.bits;
	if (m_fileHeader.colorMapType == texture_tga::ColorMap::Paletted)
	{
		bitsPerPixel = m_fileHeader.colorMapBits;
	}

	// Work out the pixel format - based on the number of bits in the final pixel format
	switch (bitsPerPixel)
	{
	case 8:
	{
		textureHeader.setPixelFormat(GeneratePixelType1<'l', 8>::ID);
		break;
	}
	case 15:
	{
		textureHeader.setPixelFormat(GeneratePixelType4<'x', 'b', 'g', 'r', 1, 5, 5, 5>::ID);
		textureHeader.setChannelType(VariableType::UnsignedShortNorm);
		break;
	}
	case 16:
	{
		if (alphaIgnored)
		{
			textureHeader.setPixelFormat(GeneratePixelType4<'x', 'b', 'g', 'r', 1, 5, 5, 5>::ID);
		}
		else
		{
			textureHeader.setPixelFormat(GeneratePixelType4<'a', 'b', 'g', 'r', 1, 5, 5, 5>::ID);
		}
		textureHeader.setChannelType(VariableType::UnsignedShortNorm);
		break;
	}
	case 24:
	{
		textureHeader.setPixelFormat(GeneratePixelType3<'b', 'g', 'r', 8, 8, 8>::ID);
		break;
	}
	case 32:
	{
		if (alphaIgnored)
		{
			textureHeader.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID);
		}
		else
		{
			textureHeader.setPixelFormat(GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID);
		}
		break;
	}
	default:
		// Invalid format
		Log(Log.Error, "Reading from \"%s\" - Invalid number of bits per pixel in TGA file: %d",
		    m_assetStream->getFileName().c_str(), m_fileHeader.bits);
		return false;
	}

	// Create the texture data
	asset = Texture(textureHeader);

	// Read the texture data according to how it's stored
	switch (m_fileHeader.imageType)
	{
	case texture_tga::ImageType::None:
	{
		memset(asset.getDataPointer(), 0, asset.getDataSize());
		break;
	}
	case texture_tga::ImageType::Indexed:
	{
		result = loadIndexed(asset, bytesPerPaletteEntry, bytesPerDataEntry);
		break;
	}
	case texture_tga::ImageType::RGB:
	case texture_tga::ImageType::GreyScale:
	{
		size_t dataRead = 0;
		m_assetStream->read(bytesPerDataEntry, asset.getTextureSize(), asset.getDataPointer(), dataRead);
		if (!result || dataRead != asset.getTextureSize()) { return result; }
		break;
	}
	case texture_tga::ImageType::RunLengthIndexed:
	{
		result = loadRunLengthIndexed(asset, bytesPerPaletteEntry, bytesPerDataEntry);
		break;
	}
	case texture_tga::ImageType::RunLengthRGB:
	case texture_tga::ImageType::RunLengthGreyScale:
	{
		result = loadRunLength(asset, bytesPerDataEntry);
		break;
	}
	case texture_tga::ImageType::RunLengthHuffmanDelta:
	case texture_tga::ImageType::RunLengthHuffmanDeltaFourPassQuadTree:
	default:
	{
		result = false;
		break;
	}
	}

	// Signify that the image has been loaded.
	m_texturesToLoad = false;

	return result;
}

bool TextureReaderTGA::loadIndexed(Texture& asset, uint32 bytesPerPaletteEntry, uint32 bytesPerDataEntry)
{
	bool result = true;
	size_t dataRead = 0;

	// Check that a palette is present.
	if (m_fileHeader.colorMapType != texture_tga::ColorMap::Paletted)
	{
		Log(Log.Error, "Reading from \"%s\" - Image Type specifies palette data, but no palette is supplied.",
		    m_assetStream->getFileName().c_str());
		return false;
	}

	// Work out the size of the palette data entries
	uint32 paletteEntries = (m_fileHeader.colorMapLength - m_fileHeader.colorMapStart);
	uint32 paletteSize = paletteEntries * bytesPerPaletteEntry;

	// Allocate data to read the palette into.
	UCharBuffer paletteData;
	paletteData.resize(paletteSize);

	// seek to the beginning of the palette
	result = m_assetStream->seek(m_fileHeader.colorMapStart * bytesPerPaletteEntry, Stream::SeekOriginFromCurrent);
	if (!result) { return result; }

	// Read the palette
	result = m_assetStream->read(bytesPerPaletteEntry, paletteEntries, paletteData.data(), dataRead);
	if (!result || dataRead != paletteEntries) { return result; }

	// Create the palette helper class
	PaletteExpander paletteLookup(paletteData.data(), paletteSize, bytesPerPaletteEntry);

	// Start reading data
	byte* outputPixel = asset.getDataPointer();
	uint32 currentIndex = 0;
	for (uint32 texturePosition = 0; texturePosition < (asset.getTextureSize()); ++texturePosition)
	{
		// Read the index
		result = m_assetStream->read(bytesPerDataEntry, 1, &currentIndex, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Get the color output
		if (!paletteLookup.getColorFromIndex(currentIndex, outputPixel)) { return false; }

		// Increment the pixel
		outputPixel += bytesPerPaletteEntry;
	}

	return result;
}

bool TextureReaderTGA::loadRunLength(Texture& asset, uint32 bytesPerDataEntry)
{
	bool result = true;
	size_t dataRead = 0;

	// Buffer for any repeated values come across
	vector<byte> repeatedValue;
	repeatedValue.resize(bytesPerDataEntry);

	// Read the run length encoded data, and decode it.
	byte* outputPixel = asset.getDataPointer();
	while (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
	{
		// Read the leading character for this block
		int8 leadingCharacter;
		result = m_assetStream->read(1, 1, &leadingCharacter, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Check if it's a run of differing values or a run of the same value multiple times
		if (leadingCharacter >= 0)
		{
			// Read each value in turn
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Read in the value
					result = m_assetStream->read(bytesPerDataEntry, 1, outputPixel, dataRead);
					if (!result || dataRead != 1) { return result; }

					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		else if (leadingCharacter > -128)
		{
			// Read a repeated value
			result = m_assetStream->read(bytesPerDataEntry, 1, repeatedValue.data(), dataRead);
			if (!result || dataRead != 1) { return result; }

			// Write the repeated value the appropriate number of times
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Copy the data across
					memcpy(outputPixel, repeatedValue.data(), bytesPerDataEntry);

					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		//Character -128 is a "no op", so there's nothing to do for it. It's used as padding basically.
	}

	return result;
}

bool TextureReaderTGA::loadRunLengthIndexed(Texture& asset, uint32 bytesPerPaletteEntry, uint32 bytesPerDataEntry)
{
	bool result = true;
	size_t dataRead = 0;

	// Check that a palette is present.
	if (m_fileHeader.colorMapType != texture_tga::ColorMap::Paletted)
	{
		Log(Log.Error, "Reading from \"%s\" - Image Type specifies palette data, but no palette is supplied.",
		    m_assetStream->getFileName().c_str());
		return false;
	}

	// Work out the size of the palette data entries
	uint32 paletteEntries = (m_fileHeader.colorMapLength - m_fileHeader.colorMapStart);
	uint32 paletteSize = paletteEntries * bytesPerPaletteEntry;

	// Allocate data to read the palette into.
	vector<byte> paletteData;
	paletteData.resize(paletteSize);

	// seek to the beginning of the palette
	result = m_assetStream->seek(m_fileHeader.colorMapStart, Stream::SeekOriginFromCurrent);
	if (!result) { return result; }

	// Read the palette
	result = m_assetStream->read(bytesPerPaletteEntry, paletteEntries, paletteData.data(), dataRead);
	if (!result || dataRead != paletteEntries) { return result; }

	// Create the palette helper class
	PaletteExpander paletteLookup(paletteData.data(), paletteSize, bytesPerPaletteEntry);

	// Index value into the palette
	uint32 currentIndex = 0;

	// Read the run length encoded data, and decode it.
	byte* outputPixel = asset.getDataPointer();
	while (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
	{
		// Read the leading character for this block
		int8 leadingCharacter;
		result = m_assetStream->read(1, 1, &leadingCharacter, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Check if it's a run of differing values or a run of the same value multiple times
		if (leadingCharacter >= 0)
		{
			// Read each index value in turn
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Read in the value
					result = m_assetStream->read(bytesPerDataEntry, 1, &currentIndex, dataRead);
					if (!result || dataRead != 1) { return result; }

					// Get the color output
					if (!paletteLookup.getColorFromIndex(currentIndex, outputPixel)) { return false; }

					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		else if (leadingCharacter > -128)
		{
			// Read in the repeated value
			result = m_assetStream->read(bytesPerDataEntry, 1, &currentIndex, dataRead);
			if (!result || dataRead != 1) { return result; }

			// Write the repeated value the appropriate number of times
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Get the color output
					if (!paletteLookup.getColorFromIndex(currentIndex, outputPixel)) { return false; }

					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		//Character -128 is a "no op", so there's nothing to do for it. It's used as padding basically.
	}

	return result;
}

}
}
}
//!\endcond