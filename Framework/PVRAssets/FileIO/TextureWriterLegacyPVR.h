/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/TextureWriterLegacyPVR.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An legacy Writer that writes pvr::asset::Texture objects into a PVR file.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/AssetWriter.h"
#include "PVRAssets/FileIO/FileDefinesPVR.h"
#include <vector>
namespace pvr {
namespace assets {
namespace assetWriters {

/*!********************************************************************************************************************
\brief  An experimentat Writer that writes pvr::asset::Texture objects into a legacy (v2) PVR file.
***********************************************************************************************************************/
class TextureWriterLegacyPVR : public AssetWriter <Texture>
{
public:
	TextureWriterLegacyPVR();
	virtual bool writeAllAssets();

	virtual uint32 assetsAddedSoFar();
	virtual bool supportsMultipleAssets();

	virtual bool canWriteAsset(const Texture& asset);
	virtual std::vector<string> getSupportedFileExtensions();
	virtual string getWriterName();
	virtual string getWriterVersion();

	void setTargetAPI(texture_legacy::API api);
	texture_legacy::API getTargetAPI();

private:
	virtual bool addAssetToWrite(const Texture& asset);
	bool convertTextureHeader3To2(texture_legacy::HeaderV2& legacyHeader, const TextureHeader& newHeader);
	bool mapNewFormatToLegacyEnum(texture_legacy::PixelFormat& legacyPixelType, const PixelFormat pixelType,
	                              const types::ColorSpace colorSpace, const VariableType channelType, const bool isPremultiplied);

private:
	texture_legacy::API m_targetAPI;
};
}
}
}
