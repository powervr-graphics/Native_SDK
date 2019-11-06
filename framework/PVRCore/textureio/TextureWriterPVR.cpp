/*!
\brief Implementation of methods of the TextureWriterPVR class.
\file PVRCore/textureio/TextureWriterPVR.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/textureio/TextureWriterPVR.h"
#include "PVRCore/textureio/FileDefinesPVR.h"
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

inline static void writeTextureMetaDataToStream(Stream&& stream, const TextureMetaData& metadata)
{
	Stream& str = stream;
	writeTextureMetaDataToStream(str, metadata);
}

void writePVR(const Texture& asset, Stream& stream)
{
	// Get the file header to write.
	TextureHeader::Header textureHeader = asset.getHeader();

	// Check the size of data written.
	uint32_t version = TextureHeader::Header::PVRv3;

	stream.writeExact(sizeof(version), 1, &version); // Write the texture header version
	stream.writeExact(sizeof(textureHeader.flags), 1, &textureHeader.flags); // Write the flags
	stream.writeExact(sizeof(textureHeader.pixelFormat), 1, &textureHeader.pixelFormat); // Write the pixel format
	stream.writeExact(sizeof(textureHeader.colorSpace), 1, &textureHeader.colorSpace); // Write the color space
	stream.writeExact(sizeof(textureHeader.channelType), 1, &textureHeader.channelType); // Write the channel type
	stream.writeExact(sizeof(textureHeader.height), 1, &textureHeader.height); // Write the height
	stream.writeExact(sizeof(textureHeader.width), 1, &textureHeader.width); // Write the width
	stream.writeExact(sizeof(textureHeader.depth), 1, &textureHeader.depth); // Write the depth
	stream.writeExact(sizeof(textureHeader.numSurfaces), 1, &textureHeader.numSurfaces); // Write the number of surfaces
	stream.writeExact(sizeof(textureHeader.numFaces), 1, &textureHeader.numFaces); // Write the number of faces
	stream.writeExact(sizeof(textureHeader.numMipMaps), 1, &textureHeader.numMipMaps); // Write the number of MIP maps
	stream.writeExact(sizeof(textureHeader.metaDataSize), 1, &textureHeader.metaDataSize); // Write the meta data size

	// Write the meta data
	const map<uint32_t, map<uint32_t, TextureMetaData>>* metaDataMap = asset.getMetaDataMap();
	map<uint32_t, map<uint32_t, TextureMetaData>>::const_iterator walkMetaDataMap = metaDataMap->begin();
	for(; walkMetaDataMap != metaDataMap->end(); ++walkMetaDataMap)
	{
		const map<uint32_t, TextureMetaData>& currentDevMetaDataMap = walkMetaDataMap->second;
		map<uint32_t, TextureMetaData>::const_iterator walkCurDevMetaMap = currentDevMetaDataMap.begin();
		for(; walkCurDevMetaMap != currentDevMetaDataMap.end(); ++walkCurDevMetaMap) { writeTextureMetaDataToStream(stream, walkCurDevMetaMap->second); }
	}

	// Write the texture data
	stream.writeExact(1, asset.getDataSize(), asset.getDataPointer());
}

void writePVR(const Texture& asset, Stream&& stream)
{
	Stream& str = stream;
	writePVR(asset, str);
}

} // namespace assetWriters
} // namespace pvr
//!\endcond
