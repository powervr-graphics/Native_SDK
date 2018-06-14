/*!
\brief Implementation of methods of the TextureWriterPVR class.
\file PVRAssets/FileIO/TextureWriterPVR.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureWriterPVR.h"
#include "PVRCore/Log.h"
using std::map;
using std::vector;
namespace pvr {
namespace assets {
namespace assetWriters {

void TextureWriterPVR::addAssetToWrite(const Texture& asset)
{
	if (_assetsToWrite.size() >= 1)
	{
		throw InvalidOperationError("TextureWriterPVR::addAssetToWrite: Attempted to add another asset to write, but the PVR file format only supports a single texture per file.");
	}
	_assetsToWrite.push_back(&asset);
}

void TextureWriterPVR::writeAllAssets()
{
	// Get the file header to write.
	TextureHeader::Header textureHeader = _assetsToWrite[0]->getHeader();

	// Check the size of data written.
	uint32_t version = TextureHeader::Header::PVRv3;

	_assetStream->writeExact(sizeof(version), 1, &version); // Write the texture header version
	_assetStream->writeExact(sizeof(textureHeader.flags), 1, &textureHeader.flags); // Write the flags
	_assetStream->writeExact(sizeof(textureHeader.pixelFormat), 1, &textureHeader.pixelFormat); // Write the pixel format
	_assetStream->writeExact(sizeof(textureHeader.colorSpace), 1, &textureHeader.colorSpace); // Write the color space
	_assetStream->writeExact(sizeof(textureHeader.channelType), 1, &textureHeader.channelType); // Write the channel type
	_assetStream->writeExact(sizeof(textureHeader.height), 1, &textureHeader.height); // Write the height
	_assetStream->writeExact(sizeof(textureHeader.width), 1, &textureHeader.width); // Write the width
	_assetStream->writeExact(sizeof(textureHeader.depth), 1, &textureHeader.depth); // Write the depth
	_assetStream->writeExact(sizeof(textureHeader.numSurfaces), 1, &textureHeader.numSurfaces); // Write the number of surfaces
	_assetStream->writeExact(sizeof(textureHeader.numFaces), 1, &textureHeader.numFaces); // Write the number of faces
	_assetStream->writeExact(sizeof(textureHeader.numMipMaps), 1, &textureHeader.numMipMaps); // Write the number of MIP maps
	_assetStream->writeExact(sizeof(textureHeader.metaDataSize), 1, &textureHeader.metaDataSize); // Write the meta data size

	// Write the meta data
	const map<uint32_t, map<uint32_t, TextureMetaData> >* metaDataMap = _assetsToWrite[0]->getMetaDataMap();
	map<uint32_t, map<uint32_t, TextureMetaData> >::const_iterator walkMetaDataMap = metaDataMap->begin();
	for (; walkMetaDataMap != metaDataMap->end(); ++walkMetaDataMap)
	{
		const map<uint32_t, TextureMetaData>& currentDevMetaDataMap = walkMetaDataMap->second;
		map<uint32_t, TextureMetaData>::const_iterator walkCurDevMetaMap = currentDevMetaDataMap.begin();
		for (; walkCurDevMetaMap != currentDevMetaDataMap.end(); ++walkCurDevMetaMap)
		{
			walkCurDevMetaMap->second.writeToStream(*_assetStream);
		}
	}

	// Write the texture data
	_assetStream->writeExact(1, _assetsToWrite[0]->getDataSize(), _assetsToWrite[0]->getDataPointer());
}

uint32_t TextureWriterPVR::assetsAddedSoFar()
{
	return static_cast<uint32_t>(_assetsToWrite.size());
}

bool TextureWriterPVR::supportsMultipleAssets()
{
	return false;
}

bool TextureWriterPVR::canWriteAsset(const Texture&)
{
	// PVR Files support anything that a Texture does, so always return true
	return true;
}

vector<std::string> TextureWriterPVR::getSupportedFileExtensions()
{
	vector<std::string> extensions;
	extensions.push_back("pvr");
	return vector<std::string>(extensions);
}

std::string TextureWriterPVR::getWriterName()
{
	return "PowerVR Texture Writer";
}

std::string TextureWriterPVR::getWriterVersion()
{
	return "1.0.0";
}
} // namespace assetWriters
} // namespace assets
} // namespace pvr
//!\endcond