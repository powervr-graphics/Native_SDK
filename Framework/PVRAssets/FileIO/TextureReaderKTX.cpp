/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\TextureReaderKTX.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Implementation of methods of the TextureReaderKTX class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureReaderKTX.h"
#include "PVRCore/Log.h"
using std::vector;
namespace pvr {
namespace assets {
namespace assetReaders {
TextureReaderKTX::TextureReaderKTX() : m_texturesToLoad(true)
{
}
TextureReaderKTX::TextureReaderKTX(Stream::ptr_type assetStream) : AssetReader<Texture>(assetStream), m_texturesToLoad(true)
{
}

bool TextureReaderKTX::readNextAsset(Texture& asset)
{
	if (m_assetStream->getSize() < texture_ktx::c_expectedHeaderSize)
	{
		return false;
	}

	size_t dataRead;
	texture_ktx::FileHeader ktxFileHeader;

	// Acknowledge that once this function has returned the user won't be able load a texture from the file.
	m_texturesToLoad = false;

	// Read the identifier
	if (!m_assetStream->read(1, sizeof(ktxFileHeader.identifier), &ktxFileHeader.identifier, dataRead) || dataRead != sizeof(ktxFileHeader.identifier)) { return false; }

	// Check that the identifier matches
	if (memcmp(ktxFileHeader.identifier, texture_ktx::c_identifier, sizeof(ktxFileHeader.identifier)) != 0)
	{
		return false;
	}

	// Read the endianness
	if (!m_assetStream->read(sizeof(ktxFileHeader.endianness), 1, &ktxFileHeader.endianness, dataRead) || dataRead != 1) { return false; }

	// Check the endianness of the file
	if (ktxFileHeader.endianness != texture_ktx::c_endianReference)
	{
		return false;
	}

	// Read the openGL type
	if (!m_assetStream->read(sizeof(ktxFileHeader.glType), 1, &ktxFileHeader.glType, dataRead) || dataRead != 1) { return false; }

	// Read the openGL type size
	if (!m_assetStream->read(sizeof(ktxFileHeader.glTypeSize), 1, &ktxFileHeader.glTypeSize, dataRead) || dataRead != 1) { return false; }

	// Read the openGL format
	if (!m_assetStream->read(sizeof(ktxFileHeader.glFormat), 1, &ktxFileHeader.glFormat, dataRead) || dataRead != 1) { return false; }

	// Read the openGL internal format
	if (!m_assetStream->read(sizeof(ktxFileHeader.glInternalFormat), 1, &ktxFileHeader.glInternalFormat, dataRead) || dataRead != 1) { return false; }

	// Read the openGL base (unsized) internal format
	if (!m_assetStream->read(sizeof(ktxFileHeader.glBaseInternalFormat), 1, &ktxFileHeader.glBaseInternalFormat, dataRead) || dataRead != 1) { return false; }

	// Read the width
	if (!m_assetStream->read(sizeof(ktxFileHeader.pixelWidth), 1, &ktxFileHeader.pixelWidth, dataRead) || dataRead != 1) { return false; }

	// Read the height
	if (!m_assetStream->read(sizeof(ktxFileHeader.pixelHeight), 1, &ktxFileHeader.pixelHeight, dataRead) || dataRead != 1) { return false; }

	// Read the depth
	if (!m_assetStream->read(sizeof(ktxFileHeader.pixelDepth), 1, &ktxFileHeader.pixelDepth, dataRead) || dataRead != 1) { return false; }

	// Read the number of array elements
	if (!m_assetStream->read(sizeof(ktxFileHeader.numberOfArrayElements), 1, &ktxFileHeader.numberOfArrayElements, dataRead) || dataRead != 1) { return false; }

	// Read the number of faces
	if (!m_assetStream->read(sizeof(ktxFileHeader.numberOfFaces), 1, &ktxFileHeader.numberOfFaces, dataRead) || dataRead != 1) { return false; }

	// Read the number of MIP Map levels
	if (!m_assetStream->read(sizeof(ktxFileHeader.numberOfMipmapLevels), 1, &ktxFileHeader.numberOfMipmapLevels, dataRead) || dataRead != 1) { return false; }

	// Read the meta data size
	if (!m_assetStream->read(sizeof(ktxFileHeader.bytesOfKeyValueData), 1, &ktxFileHeader.bytesOfKeyValueData, dataRead) || dataRead != 1) { return false; }

	// Read the meta data
	uint32 metaDataRead = 0;

	// AxisOrientation if we find it.
	uint32 orientation = 0;

	// Read MetaData
	if (ktxFileHeader.bytesOfKeyValueData > 0)
	{
		// Loop through all the meta data
		do
		{
			// Read the amount of meta data in this block.
			uint32 keyAndValueSize = 0;
			if (!m_assetStream->read(sizeof(keyAndValueSize), 1, &keyAndValueSize, dataRead) || dataRead != 1) { return false; }

			// Allocate enough memory to read in the meta data.
			UCharBuffer keyAndData; keyAndData.resize(keyAndValueSize);

			// Read in the meta data.
			if (!m_assetStream->read(1, keyAndValueSize, keyAndData.data(), dataRead) || dataRead != keyAndValueSize) { return false; }

			// Setup the key pointer
			string keyString(reinterpret_cast<char8*>(keyAndData.data()));

			// Search for KTX orientation. This is the only meta data currently supported
			if (keyString == string(texture_ktx::c_orientationMetaDataKey))
			{
				// KTX AxisOrientation key/value found, offset to the data location.
				byte* data = keyAndData.data() + (keyString.length() + 1);
				uint32 dataSize = (uint32)(keyAndValueSize - (keyString.length() + 1));

				// Read the data as a char 8 string into a string to find the orientation.
				string orientationString(reinterpret_cast<char8*>(data), dataSize);

				//Search for and set non-default orientations.
				if (orientationString.find("T=u") != string::npos)
				{
					orientation |= TextureMetaData::AxisOrientationUp;
				}
				if (orientationString.find("S=l") != string::npos)
				{
					orientation |= TextureMetaData::AxisOrientationLeft;
				}
				if (orientationString.find("R=o") != string::npos)
				{
					orientation |= TextureMetaData::AxisOrientationOut;
				}
			}

			// Work out the padding.
			uint32 padding = 0;

			//If it needs padding
			if (keyAndValueSize % 4)
			{
				padding = 4 - (keyAndValueSize % 4);
			}

			// Skip to the next meta data.
			if (!m_assetStream->seek(padding, Stream::SeekOriginFromCurrent)) { return false; }

			// Increase the meta data read value
			metaDataRead += keyAndValueSize + padding;
		}
		while (m_assetStream->getPosition() < (ktxFileHeader.bytesOfKeyValueData + texture_ktx::c_expectedHeaderSize));

		// Make sure the meta data size wasn't completely wrong. If it was, there are no guarantees about the contents of the texture data.
		if (metaDataRead > ktxFileHeader.bytesOfKeyValueData)
		{
			return false;
		}
	}

	// Construct the texture asset's header
	TextureHeader textureHeader;
	textureHeader.setopenGLFormat(ktxFileHeader.glInternalFormat, ktxFileHeader.glFormat, ktxFileHeader.glType);
	textureHeader.setWidth(ktxFileHeader.pixelWidth);
	textureHeader.setHeight(ktxFileHeader.pixelHeight);
	textureHeader.setDepth(ktxFileHeader.pixelDepth);
	textureHeader.setNumberOfArrayMembers(ktxFileHeader.numberOfArrayElements == 0 ? 1 : ktxFileHeader.numberOfArrayElements);
	textureHeader.setNumberOfFaces(ktxFileHeader.numberOfFaces);
	textureHeader.setNumberOfMIPLevels(ktxFileHeader.numberOfMipmapLevels);
	textureHeader.setOrientation(static_cast<TextureMetaData::AxisOrientation>(orientation));

	// Initialize the texture to allocate data
	asset = Texture(textureHeader, NULL);

	// Seek to the start of the texture data, just in case.
	if (!m_assetStream->seek(ktxFileHeader.bytesOfKeyValueData + texture_ktx::c_expectedHeaderSize, Stream::SeekOriginFromStart)) { return false; }

	// Read in the texture data
	for (uint32 mipMapLevel = 0; mipMapLevel < ktxFileHeader.numberOfMipmapLevels; ++mipMapLevel)
	{
		// Read the stored size of the MIP Map.
		uint32 mipMapSize = 0;
		if (!m_assetStream->read(sizeof(mipMapSize), 1, &mipMapSize, dataRead) || dataRead != 1) { return false; }

		// Sanity check the size - regular cube maps are a slight exception
		if (asset.getNumberOfFaces() == 6 && asset.getNumberOfArrayMembers() == 1)
		{
			if (mipMapSize != asset.getDataSize(mipMapLevel, false, false))
			{
				return false;
			}
		}
		else
		{
			if (mipMapSize != asset.getDataSize(mipMapLevel))
			{
				return false;
			}
		}

		// Work out the Cube Map padding.
		uint32 cubePadding = 0;
		if (asset.getDataSize(mipMapLevel, false, false) % 4)
		{
			cubePadding = 4 - (asset.getDataSize(mipMapLevel, false, false) % 4);
		}

		// Compressed images are written without scan line padding.
		if (asset.getPixelFormat().getPart().High == 0
		    && asset.getPixelFormat().getPixelTypeId() != (uint64)CompressedPixelFormat::SharedExponentR9G9B9E5)
		{
			for (uint32 iSurface = 0; iSurface < asset.getNumberOfArrayMembers(); ++iSurface)
			{
				for (uint32 iFace = 0; iFace < asset.getNumberOfFaces(); ++iFace)
				{
					// Read in the texture data.
					if (!m_assetStream->read(asset.getDataSize(mipMapLevel, false, false), 1,
					                         asset.getDataPointer(mipMapLevel, iSurface, iFace), dataRead) || dataRead != 1) { return false; }

					// Advance past the cube face padding
					if (cubePadding && asset.getNumberOfFaces() == 6 && asset.getNumberOfArrayMembers() == 1)
					{
						if (m_assetStream->seek(cubePadding, Stream::SeekOriginFromCurrent) != true) { return false; }
					}
				}
			}
		}
		// Uncompressed images have scan line padding.
		else
		{
			for (uint32 iSurface = 0; iSurface < asset.getNumberOfArrayMembers(); ++iSurface)
			{
				for (uint32 iFace = 0; iFace < asset.getNumberOfFaces(); ++iFace)
				{
					for (uint32 texDepth = 0; texDepth < asset.getDepth(); ++texDepth)
					{
						for (uint32 texHeight = 0; texHeight < asset.getHeight(); ++texHeight)
						{
							// Calculate the data offset for the relevant scan line
							uint64 scanLineOffset = (textureOffset3D(0, texHeight, texDepth, asset.getWidth(), asset.getHeight()) *
							                         (asset.getBitsPerPixel() / 8));
							// Read in the texture data for the current scan line.
							if (!m_assetStream->read((asset.getBitsPerPixel() / 8) * asset.getWidth(mipMapLevel), 1,
							                         asset.getDataPointer(mipMapLevel, iSurface, iFace) + scanLineOffset, dataRead) || dataRead != 1) { return false; }

							// Work out the amount of scan line padding.
							uint32 scanLinePadding = (static_cast<uint32>(-1) * ((asset.getBitsPerPixel() / 8) *
							                          asset.getWidth(mipMapLevel))) % 4;

							// Advance past the scan line padding
							if (scanLinePadding)
							{
								if (!m_assetStream->seek(scanLinePadding, Stream::SeekOriginFromCurrent)) { return false; }
							}
						}
					}

					// Advance past the cube face padding
					if (cubePadding && asset.getNumberOfFaces() == 6 && asset.getNumberOfArrayMembers() == 1)
					{
						if (!m_assetStream->seek(cubePadding, Stream::SeekOriginFromCurrent)) { return false; }
					}
				}
			}
		}

		// Calculate the amount MIP Map padding.
		uint32 mipMapPadding = (3 - ((mipMapSize + 3) % 4));

		// Advance past the MIP Map padding if appropriate
		if (mipMapPadding)
		{
			if (!m_assetStream->seek(mipMapPadding, Stream::SeekOriginFromCurrent)) { return false; }
		}
	}

	// Return
	return true;
}

bool TextureReaderKTX::hasAssetsLeftToLoad()
{
	return m_texturesToLoad;
}

bool TextureReaderKTX::canHaveMultipleAssets()
{
	return false;
}

bool TextureReaderKTX::isSupportedFile(Stream& assetStream)
{
	// Try to open the stream
	bool result = assetStream.open();
	if (!result)
	{
		return false;
	}

	// Read the magic identifier
	byte   magic[12];
	size_t dataRead;
	result = assetStream.read(1, sizeof(magic), &magic, dataRead);

	// Make sure it read ok, if not it's probably not a usable stream.
	if (!result || dataRead != sizeof(magic))
	{
		assetStream.close();
		return false;
	}

	// Reset the file
	assetStream.close();

	// Check that the identifier matches
	if (memcmp(magic, texture_ktx::c_identifier, sizeof(magic)) != 0)
	{
		return false;
	}

	return true;
}

vector<string> TextureReaderKTX::getSupportedFileExtensions()
{
	vector<string> extensions;
	extensions.push_back("ktx");
	return vector<string>(extensions);
}

}
}
}
//!\endcond
