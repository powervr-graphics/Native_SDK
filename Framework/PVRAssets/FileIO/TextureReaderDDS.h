/*!
\brief An experimental DDS texture reader.
\file PVRAssets/FileIO/TextureReaderDDS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Texture.h"
#include "PVRCore/Texture/FileDefinesDDS.h"
#include "PVRCore/IO/AssetReader.h"

//!\cond NO_DOXYGEN
namespace pvr {
namespace assets {
namespace assetReaders {
/// <summary>Experimental DDS Texture reader</summary>
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
	uint32_t getDirect3DFormatFromDDSHeader(texture_dds::FileHeader& textureFileHeader);
	bool _texturesToLoad;
};
}
}
}
//!\endcond