/*!
\brief Implementation of methods of the TextureWriterPVR class.
\file PVRCore/textureio/TextureWriterPVR.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/textureio/TextureWriterPVR.h"
#include "PVRCore/Log.h"
using std::map;
using std::vector;
namespace pvr {
namespace assetWriters {
inline static void writeTextureMetaDataToStream(Stream& stream, const TextureMetaData& metadata)
{
	auto fourCC = metadata.getFourCC();
	auto key = metadata.getKey();
	auto dataSize = metadata.getDataSize();
	stream.writeExact(sizeof(fourCC), 1, &fourCC);
	stream.writeExact(sizeof(key), 1, &key);
	stream.writeExact(sizeof(dataSize), 1, &dataSize);

	stream.writeExact(1, dataSize, metadata.getData());
}
void TextureWriterPVR::writeAsset(const Texture& asset)
{
	// Get the file header to write.
	TextureHeader::Header textureHeader = asset.getHeader();

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
	const map<uint32_t, map<uint32_t, TextureMetaData> >* metaDataMap = asset.getMetaDataMap();
	map<uint32_t, map<uint32_t, TextureMetaData> >::const_iterator walkMetaDataMap = metaDataMap->begin();
	for (; walkMetaDataMap != metaDataMap->end(); ++walkMetaDataMap)
	{
		const map<uint32_t, TextureMetaData>& currentDevMetaDataMap = walkMetaDataMap->second;
		map<uint32_t, TextureMetaData>::const_iterator walkCurDevMetaMap = currentDevMetaDataMap.begin();
		for (; walkCurDevMetaMap != currentDevMetaDataMap.end(); ++walkCurDevMetaMap)
		{
			writeTextureMetaDataToStream(*_assetStream, walkCurDevMetaMap->second);
		}
	}

	// Write the texture data
	_assetStream->writeExact(1, asset.getDataSize(), asset.getDataPointer());
}

bool TextureWriterPVR::canWriteAsset(const Texture&)
{
	// PVR Files support anything that a Texture does, so always return true
	return true;
}

vector<std::string> TextureWriterPVR::getSupportedFileExtensions()
{
	vector<std::string> extensions;
	extensions.emplace_back("pvr");
	return vector<std::string>(extensions);
}

} // namespace assetWriters
} // namespace pvr
//!\endcond
