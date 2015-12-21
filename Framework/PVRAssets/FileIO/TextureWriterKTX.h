/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\TextureWriterKTX.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An experimental Writer that writes pvr::asset::Texture objects into a KTX file.
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/AssetWriter.h"
#include "PVRAssets/FileIO/FileDefinesKTX.h"
namespace pvr {
namespace assets {
namespace assetWriters {
/*!********************************************************************************************************************
\brief         An experimental Writer that writes pvr::asset::Texture objects into a KTX file.
***********************************************************************************************************************/
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