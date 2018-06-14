/*!
\brief Implementation of methods of the TextureWriterKTX class.
\file PVRAssets/FileIO/TextureWriterKTX.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureWriterKTX.h"
#include "PVRCore/Texture/TextureDefines.h"

using std::vector;
namespace pvr {
namespace utils {
// CAUTION: This is a "hidden" dependency on PVRGlesUtils. If someone wants to use TextureWriterKTX without PVRUtils, he would need to implement
// and link in this function, otherwise there will be linker errors. The implementation can be found (and possibly copied from) PVRUtils/OpenGLES/TextureUtils.h
bool getOpenGLFormat(PixelFormat pixelFormat, ColorSpace colorSpace, VariableType dataType, uint32_t& glInternalFormat, uint32_t& glFormat, uint32_t& glType, uint32_t& glTypeSize,
	bool& isCompressedFormat);
} // namespace utils
namespace assets {
namespace assetWriters {

void TextureWriterKTX::addAssetToWrite(const Texture& asset)
{
	if (_assetsToWrite.size() >= 1)
	{
		throw InvalidOperationError("TextureWriterPVR::addAssetToWrite: Attempted to add another asset to write, but the KTX file writer only supports a single texture per file.");
	}
	_assetsToWrite.push_back(&asset);
}

void TextureWriterKTX::writeAllAssets()
{
	// Padding data zeroes that we can write later
	const char* paddingDataZeros[4] = { 0, 0, 0, 0 };

	// Create a KTX Texture header
	texture_ktx::FileHeader ktxFileHeader;

	// Set the identifier and endianness
	memcpy(ktxFileHeader.identifier, texture_ktx::c_identifier, sizeof(ktxFileHeader.identifier));
	ktxFileHeader.endianness = texture_ktx::c_endianReference;

	bool isCompressed;
	// Set the pixel format information
	utils::getOpenGLFormat(_assetsToWrite[0]->getPixelFormat(), _assetsToWrite[0]->getColorSpace(), _assetsToWrite[0]->getChannelType(), ktxFileHeader.glInternalFormat,
		ktxFileHeader.glFormat, ktxFileHeader.glType, ktxFileHeader.glTypeSize, isCompressed);

	// Set the dimensions
	ktxFileHeader.pixelWidth = _assetsToWrite[0]->getWidth();
	ktxFileHeader.pixelHeight = _assetsToWrite[0]->getHeight();
	ktxFileHeader.pixelDepth = _assetsToWrite[0]->getDepth();

	// Set the number of surfaces
	ktxFileHeader.numArrayElements = _assetsToWrite[0]->getNumArrayMembers();
	ktxFileHeader.numFaces = _assetsToWrite[0]->getNumFaces();
	ktxFileHeader.numMipmapLevels = _assetsToWrite[0]->getNumMipMapLevels();

	// Create the orientation meta data
	std::string orientationIdentifier(texture_ktx::c_orientationMetaDataKey);
	std::string orientationString;
	orientationString.append("S=");
	orientationString.push_back(_assetsToWrite[0]->getOrientation(TextureMetaData::AxisAxisX) == TextureMetaData::AxisOrientationLeft ? 'l' : 'r');
	orientationString.append(",T=");
	orientationString.push_back(_assetsToWrite[0]->getOrientation(TextureMetaData::AxisAxisY) == TextureMetaData::AxisOrientationUp ? 'u' : 'd');
	if (_assetsToWrite[0]->getDepth() > 1)
	{
		orientationString.append(",R=");
		orientationString.push_back(_assetsToWrite[0]->getOrientation(TextureMetaData::AxisAxisY) == TextureMetaData::AxisOrientationUp ? 'u' : 'd');
	}

	// Calculate the amount of orientation meta data (including 2 bytes for NULL characters and 4 bytes for the length of each meta data)
	uint32_t orientationMetaDataSize = static_cast<uint32_t>((orientationIdentifier.length() + 1) + (orientationString.length() + 1) + 4);

	// Make sure the length is including padded bytes
	uint32_t orientationPaddingSize = 0;
	if (orientationMetaDataSize % 4 != 0)
	{
		orientationPaddingSize = (4 - (orientationMetaDataSize % 4));
	}

	// Set the amount of meta data that's going to be added to this file
	ktxFileHeader.bytesOfKeyValueData = orientationMetaDataSize + orientationPaddingSize;

	_assetStream->writeExact(1, sizeof(ktxFileHeader.identifier), &ktxFileHeader.identifier);

	_assetStream->writeExact(sizeof(ktxFileHeader.endianness), 1, &ktxFileHeader.endianness);
	// Write the openGL type
	_assetStream->writeExact(sizeof(ktxFileHeader.glType), 1, &ktxFileHeader.glType);
	// Write the openGL type size
	_assetStream->writeExact(sizeof(ktxFileHeader.glTypeSize), 1, &ktxFileHeader.glTypeSize);
	// Write the openGL format

	_assetStream->writeExact(sizeof(ktxFileHeader.glFormat), 1, &ktxFileHeader.glFormat);
	// Write the openGL internal format
	_assetStream->writeExact(sizeof(ktxFileHeader.glInternalFormat), 1, &ktxFileHeader.glInternalFormat);
	// Write the openGL base (unsized) internal format
	_assetStream->writeExact(sizeof(ktxFileHeader.glBaseInternalFormat), 1, &ktxFileHeader.glBaseInternalFormat);
	// Write the width
	_assetStream->writeExact(sizeof(ktxFileHeader.pixelWidth), 1, &ktxFileHeader.pixelWidth);
	// Write the height
	_assetStream->writeExact(sizeof(ktxFileHeader.pixelHeight), 1, &ktxFileHeader.pixelHeight);
	// Write the depth
	_assetStream->writeExact(sizeof(ktxFileHeader.pixelDepth), 1, &ktxFileHeader.pixelDepth);
	// Write the number of array elements
	_assetStream->writeExact(sizeof(ktxFileHeader.numArrayElements), 1, &ktxFileHeader.numArrayElements);
	// Write the number of faces
	_assetStream->writeExact(sizeof(ktxFileHeader.numFaces), 1, &ktxFileHeader.numFaces);
	// Write the number of MIP Map levels
	_assetStream->writeExact(sizeof(ktxFileHeader.numMipmapLevels), 1, &ktxFileHeader.numMipmapLevels);
	// Write the meta data size
	_assetStream->writeExact(sizeof(ktxFileHeader.bytesOfKeyValueData), 1, &ktxFileHeader.bytesOfKeyValueData);
	// Write the size of the orientation data
	_assetStream->writeExact(sizeof(orientationMetaDataSize), 1, &orientationMetaDataSize);
	// Write the orientation data key
	_assetStream->writeExact(1, orientationIdentifier.length() + 1, orientationIdentifier.c_str());
	// Write the orientation data values
	_assetStream->writeExact(1, orientationString.length() + 1, orientationString.c_str());
	// Write in any padding data, use zeros for safety.
	_assetStream->writeExact(1, orientationPaddingSize, paddingDataZeros);

	// Write the texture data
	for (uint32_t mipMapLevel = 0; mipMapLevel < ktxFileHeader.numMipmapLevels; ++mipMapLevel)
	{
		// Calculate the MIP map size - regular cube maps are a slight exception
		uint32_t mipMapSize = 0;
		if (_assetsToWrite[0]->getNumFaces() == 6 && _assetsToWrite[0]->getNumArrayMembers() == 1)
		{
			mipMapSize = _assetsToWrite[0]->getDataSize(mipMapLevel, false, false);
		}
		else
		{
			mipMapSize = _assetsToWrite[0]->getDataSize(mipMapLevel);
		}

		// Write the stored size of the MIP Map.
		_assetStream->writeExact(sizeof(mipMapSize), 1, &mipMapSize);
		// Work out the Cube Map padding.
		uint32_t cubePadding = 0;
		if (_assetsToWrite[0]->getDataSize(mipMapLevel, false, false) % 4)
		{
			cubePadding = 4 - (_assetsToWrite[0]->getDataSize(mipMapLevel, false, false) % 4);
		}

		// Compressed images are written without scan line padding, because there aren't necessarily any scan lines.
		if (_assetsToWrite[0]->getPixelFormat().getPart().High == 0 &&
			_assetsToWrite[0]->getPixelFormat().getPixelTypeId() != static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5))
		{
			for (uint32_t iSurface = 0; iSurface < _assetsToWrite[0]->getNumArrayMembers(); ++iSurface)
			{
				for (uint32_t iFace = 0; iFace < _assetsToWrite[0]->getNumFaces(); ++iFace)
				{
					// Write in the texture data.
					_assetStream->writeExact(_assetsToWrite[0]->getDataSize(mipMapLevel, false, false), 1, _assetsToWrite[0]->getDataPointer(mipMapLevel, iSurface, iFace));

					// Advance past the cube face padding
					if (cubePadding && _assetsToWrite[0]->getNumFaces() == 6 && _assetsToWrite[0]->getNumArrayMembers() == 1)
					{
						_assetStream->writeExact(1, cubePadding, paddingDataZeros);
					}
				}
			}
		}
		// Uncompressed images have scan line padding.
		else
		{
			for (uint32_t iSurface = 0; iSurface < _assetsToWrite[0]->getNumArrayMembers(); ++iSurface)
			{
				for (uint32_t iFace = 0; iFace < _assetsToWrite[0]->getNumFaces(); ++iFace)
				{
					for (uint32_t texDepth = 0; texDepth < _assetsToWrite[0]->getDepth(); ++texDepth)
					{
						for (uint32_t texHeight = 0; texHeight < _assetsToWrite[0]->getHeight(); ++texHeight)
						{
							// Calculate the data offset for the relevant scan line
							uint64_t scanLineOffset =
								(textureOffset3D(0, texHeight, texDepth, _assetsToWrite[0]->getWidth(), _assetsToWrite[0]->getHeight()) * (_assetsToWrite[0]->getBitsPerPixel() / 8));
							// Write in the texture data for the current scan line.
							_assetStream->writeExact((_assetsToWrite[0]->getBitsPerPixel() / 8) * _assetsToWrite[0]->getWidth(mipMapLevel), 1,
								_assetsToWrite[0]->getDataPointer(mipMapLevel, iSurface, iFace) + scanLineOffset);

							// Work out the amount of scan line padding.
							uint32_t scanLinePadding = (static_cast<uint32_t>(-1) * ((_assetsToWrite[0]->getBitsPerPixel() / 8) * _assetsToWrite[0]->getWidth(mipMapLevel))) % 4;

							// Advance past the scan line padding
							if (scanLinePadding)
							{
								_assetStream->writeExact(1, scanLinePadding, paddingDataZeros);
							}
						}
					}

					// Advance past the cube face padding
					if (cubePadding && _assetsToWrite[0]->getNumFaces() == 6 && _assetsToWrite[0]->getNumArrayMembers() == 1)
					{
						_assetStream->writeExact(1, cubePadding, paddingDataZeros);
					}
				}
			}
		}

		// Calculate the amount MIP Map padding.
		uint32_t mipMapPadding = (3 - ((mipMapSize + 3) % 4));

		// Write MIP Map padding
		if (mipMapPadding)
		{
			_assetStream->writeExact(1, mipMapPadding, paddingDataZeros);
		}
	}
}

uint32_t TextureWriterKTX::assetsAddedSoFar()
{
	return static_cast<uint32_t>(_assetsToWrite.size());
}

bool TextureWriterKTX::supportsMultipleAssets()
{
	return false;
}

bool TextureWriterKTX::canWriteAsset(const Texture& asset)
{
	// Create a KTX Texture header to read the format into.
	texture_ktx::FileHeader ktxFileHeader;
	bool isCompressed;

	// Check if the pixel format is supported
	return utils::getOpenGLFormat(asset.getPixelFormat(), asset.getColorSpace(), asset.getChannelType(), ktxFileHeader.glInternalFormat, ktxFileHeader.glFormat,
		ktxFileHeader.glType, ktxFileHeader.glTypeSize, isCompressed);
}

vector<std::string> TextureWriterKTX::getSupportedFileExtensions()
{
	vector<std::string> extensions;
	extensions.push_back("ktx");
	return vector<std::string>(extensions);
}

std::string TextureWriterKTX::getWriterName()
{
	return "PowerVR Khronos Texture Writer";
}

std::string TextureWriterKTX::getWriterVersion()
{
	return "1.0.0";
}
} // namespace assetWriters
} // namespace assets
} // namespace pvr
//!\endcond
