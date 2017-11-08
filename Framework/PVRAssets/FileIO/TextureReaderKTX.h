/*!
\brief An experimental KTX texture reader.
\file PVRAssets/FileIO/TextureReaderKTX.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/Texture.h"
#include "PVRCore/IO/AssetReader.h"

//!\cond NO_DOXYGEN
namespace pvr {
namespace assets {
namespace assetReaders {
/// <summary>Experimental KTX Texture reader</summary>
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
	bool _texturesToLoad;
};
}
}
}
//!\endcond