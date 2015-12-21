/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\TextureWriterPVR.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of methods of the TextureWriterPVR class.
***********************************************************************************************************************/
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
	if (m_assetsToWrite.size() >= 1)
	{
		return false;
	}
	m_assetsToWrite.push_back(&asset);
	return true;
}

bool TextureWriterPVR::writeAllAssets()
{
	// Get the file header to write.
	TextureHeader::Header textureHeader = m_assetsToWrite[0]->getFileHeader();

	// Check the size of data written.
	size_t dataWritten = 0;
	uint32 version = TextureHeader::Header::PVRv3;

	// Write the texture header version
	if (!m_assetStream->write(sizeof(version), 1, &version, dataWritten) || dataWritten != 1) { return false; }

	// Write the flags
	if (!m_assetStream->write(sizeof(textureHeader.flags), 1, &textureHeader.flags, dataWritten) || dataWritten != 1) { return false; }

	// Write the pixel format
	if (!m_assetStream->write(sizeof(textureHeader.pixelFormat), 1, &textureHeader.pixelFormat, dataWritten) || dataWritten != 1) { return false; }

	// Write the color space
	if (!m_assetStream->write(sizeof(textureHeader.colorSpace), 1, &textureHeader.colorSpace, dataWritten) || dataWritten != 1) { return false; }

	// Write the channel type
	if (!m_assetStream->write(sizeof(textureHeader.channelType), 1, &textureHeader.channelType, dataWritten) || dataWritten != 1) { return false; }

	// Write the height
	if (!m_assetStream->write(sizeof(textureHeader.height), 1, &textureHeader.height, dataWritten) || dataWritten != 1) { return false; }

	// Write the width
	if (!m_assetStream->write(sizeof(textureHeader.width), 1, &textureHeader.width, dataWritten) || dataWritten != 1) { return false; }

	// Write the depth
	if (!m_assetStream->write(sizeof(textureHeader.depth), 1, &textureHeader.depth, dataWritten) || dataWritten != 1) { return false; }

	// Write the number of surfaces
	if (!m_assetStream->write(sizeof(textureHeader.numberOfSurfaces), 1, &textureHeader.numberOfSurfaces, dataWritten) || dataWritten != 1) { return false; }

	// Write the number of faces
	if (!m_assetStream->write(sizeof(textureHeader.numberOfFaces), 1, &textureHeader.numberOfFaces, dataWritten) || dataWritten != 1) { return false; }

	// Write the number of MIP maps
	if (!m_assetStream->write(sizeof(textureHeader.mipMapCount), 1, &textureHeader.mipMapCount, dataWritten) || dataWritten != 1) { return false; }

	// Write the meta data size
	if (!m_assetStream->write(sizeof(textureHeader.metaDataSize), 1, &textureHeader.metaDataSize, dataWritten) || dataWritten != 1) { return false; }

	// Write the meta data
	const map<uint32, map<uint32, TextureMetaData> >* metaDataMap = m_assetsToWrite[0]->getMetaDataMap();
	map<uint32, map<uint32, TextureMetaData> >::const_iterator walkMetaDataMap = metaDataMap->begin();
	for (; walkMetaDataMap != metaDataMap->end(); ++walkMetaDataMap)
	{
		const map<uint32, TextureMetaData>& currentDevMetaDataMap = walkMetaDataMap->second;
		map<uint32, TextureMetaData>::const_iterator walkCurDevMetaMap = currentDevMetaDataMap.begin();
		for (; walkCurDevMetaMap != currentDevMetaDataMap.end(); ++walkCurDevMetaMap)
		{
			if (!walkCurDevMetaMap->second.writeToStream(*m_assetStream)) { return false; }
		}
	}

	// Write the texture data
	if (!m_assetStream->write(1, m_assetsToWrite[0]->getDataSize(), m_assetsToWrite[0]->getDataPointer(), dataWritten)
	    || dataWritten != m_assetsToWrite[0]->getDataSize()) { return false; }
	return true;
}

uint32 TextureWriterPVR::assetsAddedSoFar()
{
	return (uint32)m_assetsToWrite.size();
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

vector<string> TextureWriterPVR::getSupportedFileExtensions()
{
	vector<string> extensions;
	extensions.push_back("pvr");
	return vector<string>(extensions);
}

string TextureWriterPVR::getWriterName()
{
	return "PowerVR Texture Writer";
}

string TextureWriterPVR::getWriterVersion()
{
	return "1.0.0";
}
}
}
}
//!\endcond