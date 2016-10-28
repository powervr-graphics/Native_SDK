/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\TextureReaderXNB.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Implementation of methods of the TextureWriterXNB class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureReaderXNB.h"
#include "PVRCore/Log.h"
using std::vector;
namespace pvr {
namespace assets {
namespace assetReaders {
TextureReaderXNB::TextureReaderXNB() : m_nextAssetToLoad(0), m_fileHeaderLoaded(false)
{
}

bool  TextureReaderXNB::readNextAsset(Texture& asset)
{
	bool result;
	if (m_hasNewAssetStream)
	{
		result = initializeFile();
		m_hasNewAssetStream = false;
		if (result)
		{
			// Check that the file is loaded properly.
			if (!m_fileHeaderLoaded)
			{
				return false;
			}
		}
	}
	// Make sure that the next data is a texture.
	if (m_objectsStrings[m_nextAssetToLoad] == "Texture2DReader")
	{
		texture_xnb::Texture2DHeader assetHeader;

		result = read2DTexture(assetHeader, asset);
	}
	else if (m_objectsStrings[m_nextAssetToLoad] == "Texture3DReader")
	{
		texture_xnb::Texture3DHeader assetHeader;

		result = read3DTexture(assetHeader, asset);
	}
	else if (m_objectsStrings[m_nextAssetToLoad] == "TextureCubeReader")
	{
		texture_xnb::TextureCubeHeader assetHeader;

		result = readCubeTexture(assetHeader, asset);
	}
	else
	{
		// Don't know how to handle it.
		return false;
	}

// Increment the assetIndex
	++m_nextAssetToLoad;

// Return
	return result;
}

bool TextureReaderXNB::hasAssetsLeftToLoad()
{
	return (m_nextAssetToLoad != m_objectsStrings.size());
}

bool TextureReaderXNB::canHaveMultipleAssets()
{
	return true;
}

bool TextureReaderXNB::isSupportedFile(Stream& assetStream)
{
	// Try to open the stream
	bool result = assetStream.open();
	if (!result)
	{
		return false;
	}

	// Read the identifier
	byte   identifier[3];
	size_t dataRead;
	result = assetStream.read(1, 3, identifier, dataRead);
	if (!result || dataRead != 3)
	{
		return false;
	}

	// Make sure it read ok, if not it's probably not a usable stream.
	if (!result || dataRead != 1)
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

vector<string> TextureReaderXNB::getSupportedFileExtensions()
{
	vector<string> extensions;
	extensions.push_back("xnb");
	return vector<string>(extensions);
}

bool TextureReaderXNB::initializeFile()
{
	const uint32 c_objectNotFound = 0xffffffffu;

	// Data Size check
	size_t dataRead = 0;

	// Read the file header
	bool result = readFileHeader(m_xnbFileHeader);
	if (!result) { return result; }

	// Check if the file is compressed, if it is it's currently unsupported
	if ((m_xnbFileHeader.flags & texture_xnb::e_fileCompressed) != 0)
	{
		return false;
	}

	// Check the file size makes sense
	if (m_xnbFileHeader.fileSize != m_assetStream->getSize())
	{
		return false;
	}

	// Read the number of primary objects in the file
	int32 numberOfAssets = 0;
	result = read7BitEncodedInt(numberOfAssets);
	if (!result) { return result; }

	// Resize the string array to hold string identifiers for all the assets
	m_objectsStrings.resize(numberOfAssets);

	// Loop through and get all the object names
	for (int32 assetIndex = 0; assetIndex < numberOfAssets; ++assetIndex)
	{
		// Get the asset information
		string typeReaderInformation;
		result = readString(typeReaderInformation);
		if (!result) { return result; }

		// Make sure the version is 4. something, and not incorrectly thrown in by something else.
		if (typeReaderInformation.find("Version=4") == string::npos)
		{
			return false;
		}

		// Extract the object name
		if (typeReaderInformation.find("Microsoft.Xna.framework.content.") == string::npos)
		{
			return false;
		}

		// Extract the name of the content reader type
		size_t contentLocation = typeReaderInformation.find("Content.", 0);
		size_t typeStart = typeReaderInformation.find('.', contentLocation) + 1;
		size_t typeEnd = typeReaderInformation.find(',', typeStart);
		if (contentLocation != c_objectNotFound || typeStart != c_objectNotFound || typeEnd != c_objectNotFound)
		{
			m_objectsStrings[assetIndex] = typeReaderInformation;
			m_objectsStrings[assetIndex].erase(0, typeStart);
			m_objectsStrings[assetIndex].erase(typeReaderInformation.length() - typeEnd, std::string::npos);
		}

		// Get the asset version
		int32 readerVersion = 0;
		m_assetStream->read(sizeof(readerVersion), 1, &readerVersion, dataRead);
		if (!result || dataRead != 1) { return result; }

		// If it's not version 0, it's not supported
		if (readerVersion != 0) { return false; }
	}

	// Read the number of shared objects in the file
	int32 numberOfSharedAssets = 0;
	result = read7BitEncodedInt(numberOfSharedAssets);
	if (!result) { return result; }

	// Mark that the header has been loaded
	m_fileHeaderLoaded = true;

	// Return
	return result;
}

uint64 TextureReaderXNB::getPVRFormatFromXNBFormat(uint32 xnbFormat)
{
	const uint64 mappedFormats[] =
	{
		GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID, //???
		GeneratePixelType3<'b', 'g', 'r', 5, 6, 5>::ID,
		GeneratePixelType4<'b', 'g', 'r', 'a', 5, 5, 5, 1>::ID,
		GeneratePixelType4<'b', 'g', 'r', 'a', 4, 4, 4, 4>::ID,
		(uint64)CompressedPixelFormat::DXT1,
		(uint64)CompressedPixelFormat::DXT3,
		(uint64)CompressedPixelFormat::DXT5,
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
	PVR_STATIC_ASSERT(sizeof(mappedFormats) / sizeof(mappedFormats[0]) == texture_xnb::NumberOfXNBFormats,
	                  xnb_mapped_formats_count_error);

	return mappedFormats[xnbFormat];
}

VariableType TextureReaderXNB::getPVRTypeFromXNBFormat(uint32 xnbFormat)
{
	const VariableType mappedTypes[] =
	{
		VariableType::UnsignedByteNorm,
		VariableType::UnsignedShortNorm,
		VariableType::UnsignedShortNorm,
		VariableType::UnsignedShortNorm,
		VariableType::UnsignedByteNorm,
		VariableType::UnsignedByteNorm,
		VariableType::UnsignedByteNorm,
		VariableType::UnsignedByteNorm,
		VariableType::UnsignedByteNorm,
		VariableType::UnsignedIntegerNorm,
		VariableType::UnsignedShortNorm,
		VariableType::UnsignedShortNorm,
		VariableType::UnsignedByteNorm,
		VariableType::SignedFloat,
		VariableType::SignedFloat,
		VariableType::SignedFloat,
		VariableType::SignedFloat,
		VariableType::SignedFloat,
		VariableType::SignedFloat,
		VariableType::SignedFloat
	};

	// Ensure that the number of elements in mapped types matches the number of texture formats that XNB supports.
	PVR_STATIC_ASSERT(sizeof(mappedTypes) / sizeof(mappedTypes[0]) == texture_xnb::NumberOfXNBFormats,
	                  xnb_mapped_types_count_error);

	return mappedTypes[xnbFormat];
}

bool TextureReaderXNB::read7BitEncodedInt(int32& decodedInteger)
{
	// Check that the read operations succeed
	bool result = true;
	size_t dataRead = 0;

	// Values used to decode the integer
	int32 bitsRead = 0;
	int32 value = 0;

	// Initialize the decoded integer
	decodedInteger = 0;

	// Loop through and read all the appropriate data to decode the integer
	do
	{
		// Read the first 7 bit value
		result = m_assetStream->read(1, 1, &value, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Add the bits to the decoded integer and increase the bit counter
		decodedInteger |= (value & 0x7f) << bitsRead;
		bitsRead += 7;
	}
	while (value & 0x80);

	// Return
	return result;
}

bool TextureReaderXNB::readFileHeader(texture_xnb::FileHeader& xnbFileHeader)
{
	// Check that the read operations succeed
	bool result = true;
	size_t dataRead = 0;

	// Read the identifier
	result = m_assetStream->read(1, 3, xnbFileHeader.identifier, dataRead);
	if (!result || dataRead != 3) { return result; }

	// Verify that it's an XNB header before doing anything else.
	if ((xnbFileHeader.identifier[0] != 'X') || (xnbFileHeader.identifier[1] != 'N') || (xnbFileHeader.identifier[2] != 'B'))
	{
		result = false;
		return result;
	}

	// Read the platform
	result = m_assetStream->read(1, 1, &xnbFileHeader.platform, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the version
	result = m_assetStream->read(1, 1, &xnbFileHeader.version, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Check that the version is '5' to ensure it's a supported version
	if (xnbFileHeader.version != 5)
	{
		result = false;
		return result;
	}

	// Read the flags
	result = m_assetStream->read(1, 1, &xnbFileHeader.flags, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the file size
	result = m_assetStream->read(4, 1, &xnbFileHeader.fileSize, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Return
	return result;
}

bool TextureReaderXNB::readString(string& stringToRead)
{
	bool result = true;
	size_t dataRead = 0;

	// Read the string length
	int32 stringLength = 0;
	result = read7BitEncodedInt(stringLength);
	if (!result) { return result; }

	// Allocate a buffer to read in the string, don't forget to add a byte for the NULL character.
	stringToRead.resize(stringLength + 1);

	// Read in the string data
	result = m_assetStream->read(1, stringLength + 1, &stringToRead, dataRead);
	if (!result || dataRead != static_cast<uint32>(stringLength) + 1) { return result; }

	// Return
	return result;
}

bool TextureReaderXNB::read2DTexture(texture_xnb::Texture2DHeader& assetHeader, Texture& asset)
{
	bool result = true;
	size_t dataRead = 0;

	// Read the surface format
	result = m_assetStream->read(sizeof(assetHeader.format), 1, &assetHeader.format, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the width
	result = m_assetStream->read(sizeof(assetHeader.width), 1, &assetHeader.width, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the height
	result = m_assetStream->read(sizeof(assetHeader.height), 1, &assetHeader.height, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the mip map count
	result = m_assetStream->read(sizeof(assetHeader.mipMapCount), 1, &assetHeader.mipMapCount, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Setup the texture header
	TextureHeader textureHeader;
	textureHeader.setPixelFormat(getPVRFormatFromXNBFormat(assetHeader.format));
	textureHeader.setChannelType(getPVRTypeFromXNBFormat(assetHeader.format));
	textureHeader.setWidth(assetHeader.width);
	textureHeader.setHeight(assetHeader.height);
	textureHeader.setNumberOfMIPLevels(assetHeader.mipMapCount);

	// Create the texture
	asset = Texture(textureHeader, NULL);

	// Read the texture data
	for (uint32 mipMapLevel = 0; mipMapLevel < asset.getNumberOfMIPLevels(); ++mipMapLevel)
	{
		// Read in the size of the next surface
		uint32 surfaceSize = 0;
		result = m_assetStream->read(sizeof(surfaceSize), 1, &surfaceSize, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Make sure the surface size matches...
		if (surfaceSize != asset.getDataSize(mipMapLevel))
		{
			return false;
		}

		// Read in the texture data.
		result = m_assetStream->read(1, surfaceSize, asset.getDataPointer(mipMapLevel), dataRead);
		if (!result || dataRead != 1) { return result; }
	}

	return true;
}

bool TextureReaderXNB::read3DTexture(texture_xnb::Texture3DHeader& assetHeader, Texture& asset)
{
	bool result = true;
	size_t dataRead = 0;

	// Read the surface format
	result = m_assetStream->read(sizeof(assetHeader.format), 1, &assetHeader.format, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the width
	result = m_assetStream->read(sizeof(assetHeader.width), 1, &assetHeader.width, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the height
	result = m_assetStream->read(sizeof(assetHeader.height), 1, &assetHeader.height, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the depth
	result = m_assetStream->read(sizeof(assetHeader.depth), 1, &assetHeader.depth, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the mip map count
	result = m_assetStream->read(sizeof(assetHeader.mipMapCount), 1, &assetHeader.mipMapCount, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Setup the texture header
	TextureHeader textureHeader;
	textureHeader.setPixelFormat(getPVRFormatFromXNBFormat(assetHeader.format));
	textureHeader.setChannelType(getPVRTypeFromXNBFormat(assetHeader.format));
	textureHeader.setWidth(assetHeader.width);
	textureHeader.setHeight(assetHeader.height);
	textureHeader.setDepth(assetHeader.depth);
	textureHeader.setNumberOfMIPLevels(assetHeader.mipMapCount);

	// Create the texture
	asset = Texture(textureHeader, NULL);

	// Read the texture data
	for (uint32 mipMapLevel = 0; mipMapLevel < asset.getNumberOfMIPLevels(); ++mipMapLevel)
	{
		// Read in the size of the next surface
		uint32 surfaceSize = 0;
		result = m_assetStream->read(sizeof(surfaceSize), 1, &surfaceSize, dataRead);
		if (!result || dataRead != 1) { return result; }

		// Make sure the surface size matches...
		if (surfaceSize != asset.getDataSize(mipMapLevel))
		{
			return false;
		}

		// Read in the texture data.
		result = m_assetStream->read(1, surfaceSize, asset.getDataPointer(mipMapLevel), dataRead);
		if (!result || dataRead != 1) { return result; }
	}

	return true;
}

bool TextureReaderXNB::readCubeTexture(texture_xnb::TextureCubeHeader& assetHeader, Texture& asset)
{
	bool result = true;
	size_t dataRead = 0;

	// Read the surface format
	result = m_assetStream->read(sizeof(assetHeader.format), 1, &assetHeader.format, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the width
	result = m_assetStream->read(sizeof(assetHeader.size), 1, &assetHeader.size, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Read the mip map count
	result = m_assetStream->read(sizeof(assetHeader.mipMapCount), 1, &assetHeader.mipMapCount, dataRead);
	if (!result || dataRead != 1) { return result; }

	// Setup the texture header
	TextureHeader textureHeader;
	textureHeader.setPixelFormat(getPVRFormatFromXNBFormat(assetHeader.format));
	textureHeader.setChannelType(getPVRTypeFromXNBFormat(assetHeader.format));
	textureHeader.setWidth(assetHeader.size);
	textureHeader.setHeight(assetHeader.size);
	textureHeader.setNumberOfFaces(6);
	textureHeader.setNumberOfMIPLevels(assetHeader.mipMapCount);

	// Create the texture
	asset = Texture(textureHeader, NULL);

	// Read the texture data
	for (uint32 face = 0; face < asset.getNumberOfFaces(); ++face)
	{
		for (uint32 mipMapLevel = 0; mipMapLevel < asset.getNumberOfMIPLevels(); ++mipMapLevel)
		{
			// Read in the size of the next surface
			uint32 surfaceSize = 0;
			result = m_assetStream->read(sizeof(surfaceSize), 1, &surfaceSize, dataRead);
			if (!result || dataRead != 1) { return result; }

			// Make sure the surface size matches...
			if (surfaceSize != asset.getDataSize(mipMapLevel, false, false))
			{
				return false;
			}

			// Read in the texture data.
			result = m_assetStream->read(1, surfaceSize, asset.getDataPointer(mipMapLevel, 0, face), dataRead);
			if (!result || dataRead != 1) { return result; }
		}
	}

	return true;
}
}
}
}
//!\endcond
