/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/TextureWriterPVR.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An experimental Writer that writes pvr::asset::Texture objects into a PVR file.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/AssetWriter.h"
#include "PVRAssets/FileIO/FileDefinesPVR.h"
namespace pvr {
namespace assets {
namespace assetWriters {
/*!********************************************************************************************************************
\brief         An experimental Writer that writes pvr::asset::Texture objects into a PVR file.
***********************************************************************************************************************/
class TextureWriterPVR : public AssetWriter<Texture>
{
public:
	virtual bool writeAllAssets();

	virtual uint32 assetsAddedSoFar();
	virtual bool supportsMultipleAssets();

	virtual bool canWriteAsset(const Texture& asset);
	virtual std::vector<string> getSupportedFileExtensions();
	virtual string getWriterName();
	virtual string getWriterVersion();
private:
	virtual bool addAssetToWrite(const Texture& asset);
};
}
}
}
