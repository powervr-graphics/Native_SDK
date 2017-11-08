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

bool TextureWriterPVR::addAssetToWrite(const Texture& asset)
{
	if (_assetsToWrite.size() >= 1)
	{
		return false;
	}
	_assetsToWrite.push_back(&asset);
	return true;
}

bool TextureWriterPVR::writeAllAssets()
{
	// Get the file header to write.
	TextureHeader::Header textureHeader = _assetsToWrite[0]->getHeader();

	// Check the size of data written.
	size_t dataWritten = 0;
	uint32_t version = TextureHeader::Header::PVRv3;

	// Write the texture header version
	if (!_assetStream->write(sizeof(version), 1, &version, dataWritten) || dataWritten != 1) { return false; }

	// Write the flags
	if (!_assetStream->write(sizeof(textureHeader.flags), 1, &textureHeader.flags, dataWritten) || dataWritten != 1) { return false; }

	// Write the pixel format
	if (!_assetStream->write(sizeof(textureHeader.pixelFormat), 1, &textureHeader.pixelFormat, dataWritten) || dataWritten != 1) { return false; }

	// Write the color space
	if (!_assetStream->write(sizeof(textureHeader.colorSpace), 1, &textureHeader.colorSpace, dataWritten) || dataWritten != 1) { return false; }

	// Write the channel type
	if (!_assetStream->write(sizeof(textureHeader.channelType), 1, &textureHeader.channelType, dataWritten) || dataWritten != 1) { return false; }

	// Write the height
	if (!_assetStream->write(sizeof(textureHeader.height), 1, &textureHeader.height, dataWritten) || dataWritten != 1) { return false; }

	// Write the width
	if (!_assetStream->write(sizeof(textureHeader.width), 1, &textureHeader.width, dataWritten) || dataWritten != 1) { return false; }

	// Write the depth
	if (!_assetStream->write(sizeof(textureHeader.depth), 1, &textureHeader.depth, dataWritten) || dataWritten != 1) { return false; }

	// Write the number of surfaces
	if (!_assetStream->write(sizeof(textureHeader.numSurfaces), 1, &textureHeader.numSurfaces, dataWritten) || dataWritten != 1) { return false; }

	// Write the number of faces
	if (!_assetStream->write(sizeof(textureHeader.numFaces), 1, &textureHeader.numFaces, dataWritten) || dataWritten != 1) { return false; }

	// Write the number of MIP maps
	if (!_assetStream->write(sizeof(textureHeader.numMipMaps), 1, &textureHeader.numMipMaps, dataWritten) || dataWritten != 1) { return false; }

	// Write the meta data size
	if (!_assetStream->write(sizeof(textureHeader.metaDataSize), 1, &textureHeader.metaDataSize, dataWritten) || dataWritten != 1) { return false; }

	// Write the meta data
	const map<uint32_t, map<uint32_t, TextureMetaData>>* metaDataMap = _assetsToWrite[0]->getMetaDataMap();
	map<uint32_t, map<uint32_t, TextureMetaData>>::const_iterator walkMetaDataMap = metaDataMap->begin();
	for (; walkMetaDataMap != metaDataMap->end(); ++walkMetaDataMap)
	{
		const map<uint32_t, TextureMetaData>& currentDevMetaDataMap = walkMetaDataMap->second;
		map<uint32_t, TextureMetaData>::const_iterator walkCurDevMetaMap = currentDevMetaDataMap.begin();
		for (; walkCurDevMetaMap != currentDevMetaDataMap.end(); ++walkCurDevMetaMap)
		{
			if (!walkCurDevMetaMap->second.writeToStream(*_assetStream)) { return false; }
		}
	}

	// Write the texture data
	if (!_assetStream->write(1, _assetsToWrite[0]->getDataSize(), _assetsToWrite[0]->getDataPointer(), dataWritten)
	    || dataWritten != _assetsToWrite[0]->getDataSize()) { return false; }
	return true;
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
}
}
}
//!\endcond