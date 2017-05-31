/*!
\brief An experimental Writer that writes pvr::asset::Texture objects into a KTX file.
\file PVRAssets/FileIO/TextureWriterKTX.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Texture.h"
#include "PVRCore/IO/AssetWriter.h"
#include "PVRCore/Texture/FileDefinesKTX.h"
namespace pvr {
namespace assets {
namespace assetWriters {
/// <summary>An experimental Writer that writes pvr::asset::Texture objects into a KTX file.</summary>
class TextureWriterKTX : public AssetWriter<Texture>
{
public:
	virtual bool writeAllAssets();

	virtual uint32 assetsAddedSoFar();
	virtual bool supportsMultipleAssets();

	virtual bool                 canWriteAsset(const Texture& asset);
	virtual std::vector<string> getSupportedFileExtensions();
	virtual string              getWriterName();
	virtual string              getWriterVersion();
private:
	virtual bool addAssetToWrite(const Texture& asset);
};
}
}
}