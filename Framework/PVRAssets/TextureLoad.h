#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/FileIO/TextureReaderPVR.h"
#include "PVRAssets/FileIO/TextureReaderBMP.h"
#include "PVRAssets/FileIO/TextureReaderKTX.h"
#include "PVRAssets/FileIO/TextureReaderDDS.h"
#include "PVRAssets/FileIO/TextureReaderXNB.h"
#include "PVRAssets/FileIO/TextureReaderTGA.h"

namespace pvr {
namespace assets {
/// <summary>Load a texture from binary data. Synchronous.</summary>
/// <param name="textureStream">A stream from which to load the binary data</param>
/// <param name="type">The type of the texture. Several supported formats.</param>
/// <param name="outTex">Output parameter - the texture will be loaded here.</param>
/// <returns>True if successful, otherwise false</returns>
inline bool textureLoad(Stream::ptr_type& textureStream, TextureFileFormat type, Texture& outTex)
{
	if (!textureStream.get() || !textureStream->open())
	{
		return false;
	}
	std::unique_ptr<AssetReader<Texture>> assetRd;
	switch (type)
	{
	case TextureFileFormat::KTX: assetRd.reset(new assetReaders::TextureReaderKTX(std::move(textureStream))); break;
	case TextureFileFormat::PVR: assetRd.reset(new assetReaders::TextureReaderPVR(std::move(textureStream))); break;
	case TextureFileFormat::TGA: assetRd.reset(new assetReaders::TextureReaderTGA(std::move(textureStream))); break;
	case TextureFileFormat::BMP: assetRd.reset(new assetReaders::TextureReaderBMP(std::move(textureStream))); break;
	case TextureFileFormat::DDS: assetRd.reset(new assetReaders::TextureReaderDDS(std::move(textureStream))); break;
	default: assertion(0); return false;
	}

	bool rslt = assetRd->readAsset(outTex);
	assetRd->closeAssetStream();
	return rslt;
}// namespace helper
}// namespace assets
}// namespace pvr
