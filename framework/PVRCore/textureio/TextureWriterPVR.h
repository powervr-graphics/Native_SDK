/*!
\brief An experimental Writer that writes pvr::asset::Texture objects into a PVR file.
\file PVRCore/textureio/TextureWriterPVR.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/texture/Texture.h"
#include "PVRCore/stream/AssetWriter.h"
#include "PVRCore/textureio/FileDefinesPVR.h"

namespace pvr {
namespace assetWriters {
/// <summary>An experimental Writer that writes pvr::asset::Texture objects into a PVR file.</summary>
class TextureWriterPVR : public AssetWriter<Texture>
{
public:
	/// <summary>Constructor</summary>
	TextureWriterPVR() {}

	/// <summary>Constructor</summary>
	/// <param name="assetStream">An asset stream to write the PVR</param>
	TextureWriterPVR(Stream::ptr_type&& assetStream)
	{
		openAssetStream(std::move(assetStream));
	}

	/// <summary>Writes the specified asset used to given stream</summary>
	/// <param name="asset">The asset to write</param>
	virtual void writeAsset(const Texture& asset);

	/// <summary>Determines whther the asset can be written</summary>
	/// <param name="asset">The asset to determine whether it can be written</param>
	/// <returns>True if the asset can be written</returns>
	virtual bool canWriteAsset(const Texture& asset);

	/// <summary>Retrieves the supported file extensions</summary>
	/// <returns>A list of the supported extensions</returns>
	virtual std::vector<std::string> getSupportedFileExtensions();

private:
};
} // namespace assetWriters
} // namespace pvr
