/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/TextureReaderKTX.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An experimental KTX texture reader.
***********************************************************************************************************************/

#pragma once
#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/AssetReader.h"
#include "PVRAssets/FileIO/FileDefinesKTX.h"

namespace pvr {
namespace assets {
namespace assetReaders {
/*!*********************************************************************************************************************
\brief Experimental KTX Texture reader
***********************************************************************************************************************/
class TextureReaderKTX : public AssetReader < Texture >
{
public:
	TextureReaderKTX();
	TextureReaderKTX(Stream::ptr_type assetStream);

	virtual bool hasAssetsLeftToLoad();
	virtual bool canHaveMultipleAssets();

	virtual bool isSupportedFile(Stream& assetStream);
	virtual std::vector<std::string> getSupportedFileExtensions();
private:
	virtual bool readNextAsset(Texture& asset);
	bool m_texturesToLoad;
};
}
}
}
