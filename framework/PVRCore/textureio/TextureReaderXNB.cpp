/*!
\brief Implementation of methods of the TextureWriterXNB class.
\file PVRCore/textureReaders/TextureReaderXNB.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/textureio/TextureReaderXNB.h"
#include "PVRCore/Log.h"
using std::vector;
namespace pvr {
namespace assetReaders {
TextureReaderXNB::TextureReaderXNB() : _nextAssetToLoad(0), _fileHeaderLoaded(false) {}

void TextureReaderXNB::readAsset_(Texture& asset)
{
	if (_hasNewAssetStream)
	{
		initializeFile();
		_hasNewAssetStream = false;
		// Check that the file is loaded properly
		if (!_fileHeaderLoaded)
		{
			throw InvalidOperationError("[TextureReaderXNB::readAsset_]: Header was not loaded correctly from the Stream");
		}
	}
	// Make sure that the next data is a texture
	if (_objectsStrings[_nextAssetToLoad] == "Texture2DReader")
	{
		texture_xnb::Texture2DHeader assetHeader;

		read2DTexture(assetHeader, asset);
	}
	else if (_objectsStrings[_nextAssetToLoad] == "Texture3DReader")
	{
		texture_xnb::Texture3DHeader assetHeader;

		read3DTexture(assetHeader, asset);
	}
	else if (_objectsStrings[_nextAssetToLoad] == "TextureCubeReader")
	{
		texture_xnb::TextureCubeHeader assetHeader;

		readCubeTexture(assetHeader, asset);
	}
	else
	{
		// Don't know how to handle it.
		throw InvalidDataError("[TextureReaderXNB::readAsset_]: Could not determine the texture type - was none of 2D, 3D or Cube");
	}

	// Increment the assetIndex
	++_nextAssetToLoad;
}

bool TextureReaderXNB::isSupportedFile(Stream& assetStream)
{
	// Try to open the stream
	assetStream.open();

	// Read the identifier
	char identifier[3];
	try
	{
		assetStream.readExact(1, 3, identifier);
	}
	catch (...)
	{
		assetStream.close();
		return false;
	}
	// Reset the file
	assetStream.close();

	// Check that the identifier matches
	if ((identifier[0] != 'X') || (identifier[1] != 'N') || (identifier[2] != 'B'))
	{
		return false;
	}

	return true;
}

void TextureReaderXNB::initializeFile()
{
	const uint32_t c_objectNotFound = 0xffffffffu;

	// Read the file header
	readFileHeader(_xnbFileHeader);

	// Check if the file is compressed, if it is it's currently unsupported
	if ((_xnbFileHeader.flags & texture_xnb::e_fileCompressed) != 0)
	{
		throw InvalidOperationError("[TextureReaderXNB::getSupportedFileExtensions][" + _assetStream->getFileName() + "]: Cannot load compressed XNB files - not supported.");
	}

	// Check the file size makes sense
	if (_xnbFileHeader.fileSize != _assetStream->getSize())
	{
		throw InvalidDataError("[TextureReaderXNB::getSupportedFileExtensions][" + _assetStream->getFileName() + "]: Data error: File size does not match stream size");
	}

	// Read the number of primary objects in the file
	int32_t numAssets = 0;
	read7BitEncodedInt(numAssets);

	// Resize the std::string array to hold std::string identifiers for all the assets
	_objectsStrings.resize(numAssets);

	// Loop through and get all the object names
	for (int32_t assetIndex = 0; assetIndex < numAssets; ++assetIndex)
	{
		// Get the asset information
		std::string typeReaderInformation;
		readString(typeReaderInformation);

		// Make sure the version is 4. something, and not incorrectly thrown in by something else.
		if (typeReaderInformation.find("Version=4") == std::string::npos)
		{
			throw InvalidDataError("[TextureReaderXNB::getSupportedFileExtensions][" + _assetStream->getFileName() + "]: Data error: Version should be 4");
		}

		// Extract the object name
		if (typeReaderInformation.find("Microsoft.Xna.framework.content.") == std::string::npos)
		{
			throw InvalidDataError("[TextureReaderXNB::getSupportedFileExtensions][" + _assetStream->getFileName() + "]: Could not get the object name");
		}

		// Extract the name of the content reader type
		size_t contentLocation = typeReaderInformation.find("Content.", 0);
		size_t typeStart = typeReaderInformation.find('.', contentLocation) + 1;
		size_t typeEnd = typeReaderInformation.find(',', typeStart);
		if (contentLocation != c_objectNotFound || typeStart != c_objectNotFound || typeEnd != c_objectNotFound)
		{
			_objectsStrings[assetIndex] = typeReaderInformation;
			_objectsStrings[assetIndex].erase(0, typeStart);
			_objectsStrings[assetIndex].erase(typeReaderInformation.length() - typeEnd, std::string::npos);
		}

		// Get the asset version
		int32_t readerVersion = 0;
		_assetStream->readExact(sizeof(readerVersion), 1, &readerVersion);

		// If it's not version 0, it's not supported
		if (readerVersion != 0)
		{
			throw InvalidDataError("[TextureReaderXNB::getSupportedFileExtensions][" + _assetStream->getFileName() + "]: Reader version should be 0");
		}
	}

	// Read the number of shared objects in the file
	int32_t numSharedAssets = 0;
	read7BitEncodedInt(numSharedAssets);

	// Mark that the header has been loaded
	_fileHeaderLoaded = true;
}

uint64_t TextureReaderXNB::getPVRFormatFromXNBFormat(uint32_t xnbFormat)
{
	const uint64_t mappedFormats[] = {
		GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID, //???
		GeneratePixelType3<'b', 'g', 'r', 5, 6, 5>::ID,
		GeneratePixelType4<'b', 'g', 'r', 'a', 5, 5, 5, 1>::ID,
		GeneratePixelType4<'b', 'g', 'r', 'a', 4, 4, 4, 4>::ID,
		static_cast<uint64_t>(CompressedPixelFormat::DXT1),
		static_cast<uint64_t>(CompressedPixelFormat::DXT3),
		static_cast<uint64_t>(CompressedPixelFormat::DXT5),
		GeneratePixelType2<'r', 'g', 8, 8>::ID, //???
		GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID, //???
		GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID,
		GeneratePixelType2<'r', 'g', 16, 16>::ID,
		GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID,
		GeneratePixelType1<'a', 8>::ID,
		GeneratePixelType1<'r', 32>::ID,
		GeneratePixelType2<'r', 'g', 32, 32>::ID,
		GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID,
		GeneratePixelType1<'r', 16>::ID,
		GeneratePixelType2<'r', 'g', 16, 16>::ID,
		GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID,
		GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID,
	};

	// Ensure that the number of elements in mapped formats matches the number of texture formats that XNB supports.
	debug_assertion(sizeof(mappedFormats) / sizeof(mappedFormats[0]) == texture_xnb::NumXNBFormats,
		"The number of elements in mapped formats must match the number of texture formats that XNB supports");

	return mappedFormats[xnbFormat];
}

VariableType TextureReaderXNB::getPVRTypeFromXNBFormat(uint32_t xnbFormat)
{
	const VariableType mappedTypes[] = { VariableType::UnsignedByteNorm, VariableType::UnsignedShortNorm, VariableType::UnsignedShortNorm, VariableType::UnsignedShortNorm,
		VariableType::UnsignedByteNorm, VariableType::UnsignedByteNorm, VariableType::UnsignedByteNorm, VariableType::UnsignedByteNorm, VariableType::UnsignedByteNorm,
		VariableType::UnsignedIntegerNorm, VariableType::UnsignedShortNorm, VariableType::UnsignedShortNorm, VariableType::UnsignedByteNorm, VariableType::SignedFloat,
		VariableType::SignedFloat, VariableType::SignedFloat, VariableType::SignedFloat, VariableType::SignedFloat, VariableType::SignedFloat, VariableType::SignedFloat };

	// Ensure that the number of elements in mapped types matches the number of texture formats that XNB supports.
	debug_assertion(sizeof(mappedTypes) / sizeof(mappedTypes[0]) == texture_xnb::NumXNBFormats,
		"The number of elements in mapped types must match the number of texture formats that XNB supports");

	return mappedTypes[xnbFormat];
}

void TextureReaderXNB::read7BitEncodedInt(int32_t& decodedInteger)
{
	// Check that the read operations succeed

	// Values used to decode the integer
	int32_t bitsRead = 0;
	int32_t value = 0;

	// Initialize the decoded integer
	decodedInteger = 0;

	// Loop through and read all the appropriate data to decode the integer
	do
	{
		// Read the first 7 bit value
		_assetStream->readExact(1, 1, &value);

		// Add the bits to the decoded integer and increase the bit counter
		decodedInteger |= (value & 0x7f) << bitsRead;
		bitsRead += 7;
	} while (value & 0x80);
}

void TextureReaderXNB::readFileHeader(texture_xnb::FileHeader& xnbFileHeader)
{
	// Read the identifier
	_assetStream->readExact(1, 3, xnbFileHeader.identifier);

	// Verify that it's an XNB header before doing anything else.
	if ((xnbFileHeader.identifier[0] != 'X') || (xnbFileHeader.identifier[1] != 'N') || (xnbFileHeader.identifier[2] != 'B'))
	{
		throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + _assetStream->getFileName() + "]: Stream was not a valid XNB");
	}

	// Read the platform
	_assetStream->readExact(1, 1, &xnbFileHeader.platform);
	// Read the version
	_assetStream->readExact(1, 1, &xnbFileHeader.version);
	// Check that the version is '5' to ensure it's a supported version
	if (xnbFileHeader.version != 5)
	{
		throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + _assetStream->getFileName() + "]: XNB Version must be 5");
	}
	// Read the flags
	_assetStream->readExact(1, 1, &xnbFileHeader.flags);
	// Read the file size
	_assetStream->readExact(4, 1, &xnbFileHeader.fileSize);
}

void TextureReaderXNB::readString(std::string& stringToRead)
{
	// Read the std::string length
	int32_t stringLength = 0;
	read7BitEncodedInt(stringLength);

	// Allocate a buffer to read in the std::string, don't forget to add a char for the NULL character.
	stringToRead.resize(stringLength + 1);

	// Read in the std::string data
	_assetStream->readExact(1, stringLength + 1, &stringToRead);
}

void TextureReaderXNB::read2DTexture(texture_xnb::Texture2DHeader& assetHeader, Texture& asset)
{
	// Read the surface format
	_assetStream->readExact(sizeof(assetHeader.format), 1, &assetHeader.format);
	// Read the width
	_assetStream->readExact(sizeof(assetHeader.width), 1, &assetHeader.width);
	// Read the height
	_assetStream->readExact(sizeof(assetHeader.height), 1, &assetHeader.height);
	// Read the mip map count
	_assetStream->readExact(sizeof(assetHeader.numMipMaps), 1, &assetHeader.numMipMaps);
	// Setup the texture header
	TextureHeader textureHeader;
	textureHeader.setPixelFormat(getPVRFormatFromXNBFormat(assetHeader.format));
	textureHeader.setChannelType(getPVRTypeFromXNBFormat(assetHeader.format));
	textureHeader.setWidth(assetHeader.width);
	textureHeader.setHeight(assetHeader.height);
	textureHeader.setNumMipMapLevels(assetHeader.numMipMaps);

	// Create the texture
	asset = Texture(textureHeader, NULL);

	// Read the texture data
	for (uint32_t mipMapLevel = 0; mipMapLevel < asset.getNumMipMapLevels(); ++mipMapLevel)
	{
		// Read in the size of the next surface
		uint32_t surfaceSize = 0;
		_assetStream->readExact(sizeof(surfaceSize), 1, &surfaceSize);
		// Make sure the surface size matches...
		if (surfaceSize != asset.getDataSize(mipMapLevel))
		{
			throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + _assetStream->getFileName() + "]: Expected data size did not match actual size");
		}

		// Read in the texture data.
		_assetStream->readExact(1, surfaceSize, asset.getDataPointer(mipMapLevel));
	}
}

void TextureReaderXNB::read3DTexture(texture_xnb::Texture3DHeader& assetHeader, Texture& asset)
{
	// Read the surface format
	_assetStream->readExact(sizeof(assetHeader.format), 1, &assetHeader.format);
	// Read the width
	_assetStream->readExact(sizeof(assetHeader.width), 1, &assetHeader.width);
	// Read the height
	_assetStream->readExact(sizeof(assetHeader.height), 1, &assetHeader.height);
	// Read the depth
	_assetStream->readExact(sizeof(assetHeader.depth), 1, &assetHeader.depth);
	// Read the mip map count
	_assetStream->readExact(sizeof(assetHeader.numMipMaps), 1, &assetHeader.numMipMaps);
	// Setup the texture header
	TextureHeader textureHeader;
	textureHeader.setPixelFormat(getPVRFormatFromXNBFormat(assetHeader.format));
	textureHeader.setChannelType(getPVRTypeFromXNBFormat(assetHeader.format));
	textureHeader.setWidth(assetHeader.width);
	textureHeader.setHeight(assetHeader.height);
	textureHeader.setDepth(assetHeader.depth);
	textureHeader.setNumMipMapLevels(assetHeader.numMipMaps);

	// Create the texture
	asset = Texture(textureHeader, NULL);

	// Read the texture data
	for (uint32_t mipMapLevel = 0; mipMapLevel < asset.getNumMipMapLevels(); ++mipMapLevel)
	{
		// Read in the size of the next surface
		uint32_t surfaceSize = 0;
		_assetStream->readExact(sizeof(surfaceSize), 1, &surfaceSize);
		// Make sure the surface size matches...
		if (surfaceSize != asset.getDataSize(mipMapLevel))
		{
			throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + _assetStream->getFileName() + "]: Expected data size did not match actual size");
		}

		// Read in the texture data.
		_assetStream->readExact(1, surfaceSize, asset.getDataPointer(mipMapLevel));
	}
}

void TextureReaderXNB::readCubeTexture(texture_xnb::TextureCubeHeader& assetHeader, Texture& asset)
{
	// Read the surface format
	_assetStream->readExact(sizeof(assetHeader.format), 1, &assetHeader.format);
	// Read the width
	_assetStream->readExact(sizeof(assetHeader.size), 1, &assetHeader.size);
	// Read the mip map count
	_assetStream->readExact(sizeof(assetHeader.numMipMaps), 1, &assetHeader.numMipMaps);
	// Setup the texture header
	TextureHeader textureHeader;
	textureHeader.setPixelFormat(getPVRFormatFromXNBFormat(assetHeader.format));
	textureHeader.setChannelType(getPVRTypeFromXNBFormat(assetHeader.format));
	textureHeader.setWidth(assetHeader.size);
	textureHeader.setHeight(assetHeader.size);
	textureHeader.setNumFaces(6);
	textureHeader.setNumMipMapLevels(assetHeader.numMipMaps);

	// Create the texture
	asset = Texture(textureHeader, NULL);

	// Read the texture data
	for (uint32_t face = 0; face < asset.getNumFaces(); ++face)
	{
		for (uint32_t mipMapLevel = 0; mipMapLevel < asset.getNumMipMapLevels(); ++mipMapLevel)
		{
			// Read in the size of the next surface
			uint32_t surfaceSize = 0;
			_assetStream->readExact(sizeof(surfaceSize), 1, &surfaceSize);

			// Make sure the surface size matches...
			if (surfaceSize != asset.getDataSize(mipMapLevel, false, false))
			{
				throw InvalidDataError("[TextureReaderXNB::readFileHeader][" + _assetStream->getFileName() + "]: Expected data size did not match actual size");
			}

			// Read in the texture data.
			_assetStream->readExact(1, surfaceSize, asset.getDataPointer(mipMapLevel, 0, face));
		}
	}
}
} // namespace assetReaders
} // namespace pvr
//!\endcond
