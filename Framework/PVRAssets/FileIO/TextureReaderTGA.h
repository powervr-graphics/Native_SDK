/*!
\brief An experimental AssetReader that reads pvr::asset::Texture objects from an TGA file.
\file PVRAssets/FileIO/TextureReaderTGA.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Texture.h"
#include "PVRCore/Texture/FileDefinesTGA.h"
#include "PVRCore/IO/AssetReader.h"

//!\cond NO_DOXYGEN
namespace pvr {
namespace assets {
namespace assetReaders {
/// <summary>Experimental TGA Texture reader</summary>
class TextureReaderTGA : public AssetReader<Texture>
{
public:
	TextureReaderTGA();
	TextureReaderTGA(Stream::ptr_type assetStream);

	virtual bool hasAssetsLeftToLoad();
	virtual bool canHaveMultipleAssets();

	virtual bool isSupportedFile(Stream& assetStream);
	virtual std::vector<std::string> getSupportedFileExtensions();
private:
	virtual bool readNextAsset(Texture& asset);
	bool _texturesToLoad;
	bool _fileHeaderLoaded;

	texture_tga::FileHeader _fileHeader;

	bool initializeFile();
	bool readFileHeader(texture_tga::FileHeader& fileheader);
	bool loadImageFromFile(Texture& asset);
	bool loadIndexed(Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bytesPerDataEntry);
	bool loadRunLength(Texture& asset, uint32_t bytesPerDataEntry);
	bool loadRunLengthIndexed(Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bytesPerDataEntry);
};
}
}
}
//!\endcond
