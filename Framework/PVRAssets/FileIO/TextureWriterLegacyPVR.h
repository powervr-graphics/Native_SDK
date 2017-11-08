/*!
\brief An legacy Writer that writes pvr::asset::Texture objects into a PVR file.
\file PVRAssets/FileIO/TextureWriterLegacyPVR.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Texture.h"
#include "PVRCore/IO/AssetWriter.h"
#include "PVRCore/Texture/FileDefinesPVR.h"
#include <vector>

//!\cond NO_DOXYGEN

namespace pvr {
namespace assets {
namespace assetWriters {

/// <summary>An experimentat Writer that writes pvr::asset::Texture objects into a legacy (v2) PVR file.
/// </summary>
class TextureWriterLegacyPVR : public AssetWriter <Texture>
{
public:
	TextureWriterLegacyPVR();
	virtual bool writeAllAssets();

	virtual uint32_t assetsAddedSoFar();
	virtual bool supportsMultipleAssets();

	virtual bool canWriteAsset(const Texture& asset);
	virtual std::vector<std::string> getSupportedFileExtensions();
	virtual std::string getWriterName();
	virtual std::string getWriterVersion();

	void setTargetAPI(texture_legacy::API api);
	texture_legacy::API getTargetAPI();

private:
	virtual bool addAssetToWrite(const Texture& asset);
	bool convertTextureHeader3To2(texture_legacy::HeaderV2& legacyHeader, const TextureHeader& newHeader);
	bool mapNewFormatToLegacyEnum(texture_legacy::PixelFormat& legacyPixelType, const PixelFormat pixelType,
	                              const ColorSpace colorSpace, const VariableType channelType, const bool isPremultiplied);

private:
	texture_legacy::API _targetAPI;
};
}
}
}

//!\endcond