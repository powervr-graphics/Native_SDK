/*!
\brief Functionality for loading a texture from disk or other sources
\file PVRAssets/TextureLoad.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

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
/// <returns>True if successful, otherwise false</returns>
inline Texture textureLoad(Stream::ptr_type&& textureStream, TextureFileFormat type)
{
	if (!textureStream.get())
	{
		throw InvalidArgumentError("textureStream", "[textureLoad] Attempted to load from a NULL stream");
	}
	textureStream->open();

	std::unique_ptr<AssetReader<Texture> > assetRd;
	switch (type)
	{
	case TextureFileFormat::KTX:
		assetRd.reset(new assetReaders::TextureReaderKTX(std::move(textureStream)));
		break;
	case TextureFileFormat::PVR:
		assetRd.reset(new assetReaders::TextureReaderPVR(std::move(textureStream)));
		break;
	case TextureFileFormat::TGA:
		assetRd.reset(new assetReaders::TextureReaderTGA(std::move(textureStream)));
		break;
	case TextureFileFormat::BMP:
		assetRd.reset(new assetReaders::TextureReaderBMP(std::move(textureStream)));
		break;
	case TextureFileFormat::DDS:
		assetRd.reset(new assetReaders::TextureReaderDDS(std::move(textureStream)));
		break;
	default:
		throw InvalidArgumentError("type", "Unknown texture file format passed");
	}
	Texture tex;
	assetRd->readAsset(tex);
	assetRd->closeAssetStream();
	return tex;
}

/// <summary>Load a texture from binary data. Synchronous.</summary>
/// <param name="textureStream">A stream from which to load the binary data</param>
/// <param name="type">The type of the texture. Several supported formats.</param>
/// <returns>True if successful, otherwise false</returns>
inline Texture textureLoad(Stream::ptr_type& textureStream, TextureFileFormat type)
{
	return textureLoad(std::move(textureStream), type);
}

} // namespace assets
} // namespace pvr
