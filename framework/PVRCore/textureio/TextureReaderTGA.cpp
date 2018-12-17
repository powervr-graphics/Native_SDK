/*!
\brief Implementation of methods of the TextureReaderTGA class.
\file PVRCore/textureReaders/TextureReaderTGA.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/stream/FilePath.h"
#include "PVRCore/Log.h"

#include "PVRCore/textureio/TextureReaderTGA.h"
#include "PVRCore/textureio/PaletteExpander.h"
#include <algorithm>
using std::vector;
namespace pvr {
namespace assetReaders {
TextureReaderTGA::TextureReaderTGA() : _texturesToLoad(true) {}

TextureReaderTGA::TextureReaderTGA(Stream::ptr_type assetStream) : AssetReader<Texture>(std::move(assetStream)), _texturesToLoad(true) {}

void TextureReaderTGA::readAsset_(Texture& asset)
{
	// Check the Result
	if (_hasNewAssetStream)
	{
		initializeFile();

		_texturesToLoad = true;
		_hasNewAssetStream = false;
	}

	long streamPosition = static_cast<long>(_assetStream->getPosition());
	try
	{
		loadImageFromFile(asset);
		// If it succeeded, let the user know that there are no more texture to load.
		_texturesToLoad = false;
	}
	catch (...)
	{
		// Return to the beginning of the texture data if not loaded correctly.
		_assetStream->seek(streamPosition, Stream::SeekOriginFromStart);
		throw;
	}
}

bool TextureReaderTGA::isSupportedFile(Stream& assetStream)
{
	// Try to open the stream
	FilePath filePath(assetStream.getFileName());
	std::string fileExt;
	std::transform(filePath.getFileExtension().begin(), filePath.getFileExtension().end(), fileExt.begin(), ::tolower);
	return fileExt == "tga";
}

void TextureReaderTGA::initializeFile()
{
	// Read the file header
	readFileHeader(_fileHeader);

	// Skip the identifier area
	_assetStream->seek(_fileHeader.identSize, Stream::SeekOriginFromCurrent);

	// Mark that the header has been loaded
	_fileHeaderLoaded = true;
}

void TextureReaderTGA::readFileHeader(texture_tga::FileHeader& fileheader)
{
	// Read the size of the identifier area
	_assetStream->readExact(sizeof(fileheader.identSize), 1, &fileheader.identSize);
	// Read the color map type
	_assetStream->readExact(sizeof(fileheader.colorMapType), 1, &fileheader.colorMapType);
	// Read the image type
	_assetStream->readExact(sizeof(fileheader.imageType), 1, &fileheader.imageType);
	// Read the start position of the color map
	_assetStream->readExact(sizeof(fileheader.colorMapStart), 1, &fileheader.colorMapStart);
	// Read the length of the color map
	_assetStream->readExact(sizeof(fileheader.colorMapLength), 1, &fileheader.colorMapLength);
	// Read the number of bits per palette entry in the color map
	_assetStream->readExact(sizeof(fileheader.colorMapBits), 1, &fileheader.colorMapBits);
	// Read the horizontal offset for the start of the image
	_assetStream->readExact(sizeof(fileheader.xStart), 1, &fileheader.xStart);
	// Read the vertical offset for the start of the image
	_assetStream->readExact(sizeof(fileheader.yStart), 1, &fileheader.yStart);
	// Read the width of the image
	_assetStream->readExact(sizeof(fileheader.width), 1, &fileheader.width);
	// Read the height of the image
	_assetStream->readExact(sizeof(fileheader.height), 1, &fileheader.height);
	// Read the bits per pixel in the image
	_assetStream->readExact(sizeof(fileheader.bits), 1, &fileheader.bits);
	// Read the descriptor flags
	_assetStream->readExact(sizeof(fileheader.descriptor), 1, &fileheader.descriptor);
}

void TextureReaderTGA::loadImageFromFile(Texture& asset)
{
	// Make sure the file is ready to load
	if (!_fileHeaderLoaded || !_texturesToLoad)
	{
		throw InvalidOperationError("[TextureReaderTGA::loadImageFromFile] Attempted to read empty TGA.");
	}

	// Setup the texture header.
	TextureHeader textureHeader;

	// Set the width and height from the file header.
	textureHeader.setWidth(_fileHeader.width);
	textureHeader.setHeight(_fileHeader.height);

	// Check whether the alpha value is ignored or not.
	bool alphaIgnored = ((_fileHeader.descriptor & texture_tga::DescriptorFlagAlpha) == 0);

	// Get the bytes per data entry
	uint32_t bytesPerDataEntry = _fileHeader.bits / 8;
	if (_fileHeader.bits == 15)
	{
		bytesPerDataEntry = 2;
	}

	// Get the bytes per color map entry
	uint32_t bytesPerPaletteEntry = _fileHeader.colorMapBits / 8;
	if (_fileHeader.colorMapBits == 15)
	{
		bytesPerPaletteEntry = 2;
	}

	// Work out the bits per pixel of the final pixel format
	uint32_t bitsPerPixel = _fileHeader.bits;
	if (_fileHeader.colorMapType == texture_tga::ColorMap::Paletted)
	{
		bitsPerPixel = _fileHeader.colorMapBits;
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
		throw InvalidOperationError("[TextureReaderTGA::loadImageFromFile]: Reading from [" + _assetStream->getFileName() + "] - Invalid number of bits per pixel in TGA file");
	}

	// Create the texture data
	asset = Texture(textureHeader);

	// Read the texture data according to how it's stored
	switch (_fileHeader.imageType)
	{
	case texture_tga::ImageType::None:
	{
		memset(asset.getDataPointer(), 0, asset.getDataSize());
		break;
	}
	case texture_tga::ImageType::Indexed:
	{
		loadIndexed(asset, bytesPerPaletteEntry, bytesPerDataEntry);
		break;
	}
	case texture_tga::ImageType::RGB:
	case texture_tga::ImageType::GreyScale:
	{
		_assetStream->readExact(bytesPerDataEntry, asset.getTextureSize(), asset.getDataPointer());
		break;
	}
	case texture_tga::ImageType::RunLengthIndexed:
	{
		loadRunLengthIndexed(asset, bytesPerPaletteEntry, bytesPerDataEntry);
		break;
	}
	case texture_tga::ImageType::RunLengthRGB:
	case texture_tga::ImageType::RunLengthGreyScale:
	{
		loadRunLength(asset, bytesPerDataEntry);
		break;
	}
	case texture_tga::ImageType::RunLengthHuffmanDelta:
	case texture_tga::ImageType::RunLengthHuffmanDeltaFourPassQuadTree:
	default:
	{
		throw InvalidOperationError("[TextureReaderTGA::loadImageFromFile]: Invalid image type");
		break;
	}
	}

	// Signify that the image has been loaded.
	_texturesToLoad = false;
}

void TextureReaderTGA::loadIndexed(Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bytesPerDataEntry)
{
	// Check that a palette is present.
	if (_fileHeader.colorMapType != texture_tga::ColorMap::Paletted)
	{
		throw InvalidOperationError(
			"[TextureReaderTGA::loadIndexed]: Reading from [" + _assetStream->getFileName() + "] - Image Type specifies palette data, but no palette is supplied.");
	}

	// Work out the size of the palette data entries
	uint32_t paletteEntries = (_fileHeader.colorMapLength - _fileHeader.colorMapStart);
	uint32_t paletteSize = paletteEntries * bytesPerPaletteEntry;

	// Allocate data to read the palette into.
	std::vector<unsigned char> paletteData;
	paletteData.resize(paletteSize);

	// seek to the beginning of the palette
	_assetStream->seek(_fileHeader.colorMapStart * bytesPerPaletteEntry, Stream::SeekOriginFromCurrent);

	// Read the palette
	_assetStream->readExact(bytesPerPaletteEntry, paletteEntries, paletteData.data());

	// Create the palette helper class
	PaletteExpander paletteLookup(paletteData.data(), paletteSize, bytesPerPaletteEntry);

	// Start reading data
	unsigned char* outputPixel = asset.getDataPointer();
	uint32_t currentIndex = 0;
	for (uint32_t texturePosition = 0; texturePosition < (asset.getTextureSize()); ++texturePosition)
	{
		// Read the index
		_assetStream->readExact(bytesPerDataEntry, 1, &currentIndex);
		// Get the color output
		paletteLookup.getColorFromIndex(currentIndex, outputPixel);

		// Increment the pixel
		outputPixel += bytesPerPaletteEntry;
	}
}

void TextureReaderTGA::loadRunLength(Texture& asset, uint32_t bytesPerDataEntry)
{
	// Buffer for any repeated values come across
	vector<char> repeatedValue;
	repeatedValue.resize(bytesPerDataEntry);

	// Read the run length encoded data, and decode it.
	unsigned char* outputPixel = asset.getDataPointer();
	while (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
	{
		// Read the leading character for this block
		int8_t leadingCharacter;
		_assetStream->readExact(1, 1, &leadingCharacter);
		// Check if it's a run of differing values or a run of the same value multiple times
		if (leadingCharacter >= 0)
		{
			// Read each value in turn
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Read in the value
					_assetStream->readExact(bytesPerDataEntry, 1, outputPixel);
					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		else if (leadingCharacter > -128)
		{
			// Read a repeated value
			_assetStream->readExact(bytesPerDataEntry, 1, repeatedValue.data());
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
		// Character -128 is a "no op", so there's nothing to do for it. It's used as padding basically.
	}
}

void TextureReaderTGA::loadRunLengthIndexed(Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bytesPerDataEntry)
{
	// Check that a palette is present.
	if (_fileHeader.colorMapType != texture_tga::ColorMap::Paletted)
	{
		throw InvalidDataError("[" + _assetStream->getFileName() + "]: Image Type specifies palette data, but no palette is supplied.");
	}

	// Work out the size of the palette data entries
	uint32_t paletteEntries = (_fileHeader.colorMapLength - _fileHeader.colorMapStart);
	uint32_t paletteSize = paletteEntries * bytesPerPaletteEntry;

	// Allocate data to read the palette into.
	vector<uint8_t> paletteData;
	paletteData.resize(paletteSize);

	// seek to the beginning of the palette
	_assetStream->seek(_fileHeader.colorMapStart, Stream::SeekOriginFromCurrent);
	// Read the palette
	_assetStream->readExact(bytesPerPaletteEntry, paletteEntries, paletteData.data());

	// Create the palette helper class
	PaletteExpander paletteLookup(paletteData.data(), paletteSize, bytesPerPaletteEntry);

	// Index value into the palette
	uint32_t currentIndex = 0;

	// Read the run length encoded data, and decode it.
	uint8_t* outputPixel = asset.getDataPointer();
	while (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
	{
		// Read the leading character for this block
		int8_t leadingCharacter;
		_assetStream->readExact(1, 1, &leadingCharacter);
		// Check if it's a run of differing values or a run of the same value multiple times
		if (leadingCharacter >= 0)
		{
			// Read each index value in turn
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Read in the value
					_assetStream->readExact(bytesPerDataEntry, 1, &currentIndex);
					// Get the color output
					paletteLookup.getColorFromIndex(currentIndex, outputPixel);

					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		else if (leadingCharacter > -128)
		{
			// Read in the repeated value
			_assetStream->readExact(bytesPerDataEntry, 1, &currentIndex);
			// Write the repeated value the appropriate number of times
			for (int runIndex = 0; runIndex < (1 + (leadingCharacter & 0x7f)); ++runIndex)
			{
				if (outputPixel < (asset.getDataPointer() + asset.getDataSize()))
				{
					// Get the color output
					paletteLookup.getColorFromIndex(currentIndex, outputPixel);

					// Increment the output location
					outputPixel += bytesPerDataEntry;
				}
			}
		}
		// Character -128 is a "no op", so there's nothing to do for it. It's used as padding basically.
	}
}

} // namespace assetReaders
} // namespace pvr
//!\endcond
