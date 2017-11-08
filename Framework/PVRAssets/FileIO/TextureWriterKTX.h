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

//!\cond NO_DOXYGEN
namespace pvr {
namespace assets {
namespace assetWriters {
/// <summary>An experimental Writer that writes pvr::asset::Texture objects into a KTX file.</summary>
class TextureWriterKTX : public AssetWriter<Texture>
{
public:
	virtual bool writeAllAssets();

	virtual uint32_t assetsAddedSoFar();
	virtual bool supportsMultipleAssets();

	virtual bool                 canWriteAsset(const Texture& asset);
	virtual std::vector<std::string> getSupportedFileExtensions();
	virtual std::string              getWriterName();
	virtual std::string              getWriterVersion();
private:
	virtual bool addAssetToWrite(const Texture& asset);
};
}
}
}
//!\endcond