/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/TextureReaderDDS.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An experimental DDS texture reader.
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/AssetReader.h"
#include "PVRAssets/FileIO/FileDefinesDDS.h"

namespace pvr {
namespace assets {
namespace assetReaders {
/*!*********************************************************************************************************************
\brief Experimental DDS Texture reader
***********************************************************************************************************************/
class TextureReaderDDS : public AssetReader <Texture >
{
public:
	TextureReaderDDS();
	TextureReaderDDS(Stream::ptr_type assetStream);

	virtual bool hasAssetsLeftToLoad();
	virtual bool canHaveMultipleAssets();

	virtual bool isSupportedFile(Stream& assetStream);
	virtual std::vector<std::string> getSupportedFileExtensions();
private:
	virtual bool readNextAsset(Texture& asset);
	uint32 getDirect3DFormatFromDDSHeader(texture_dds::FileHeader& textureFileHeader);
	bool m_texturesToLoad;
};
}
}
}