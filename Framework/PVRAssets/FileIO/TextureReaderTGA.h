/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/TextureReaderTGA.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An experimental AssetReader that reads pvr::asset::Texture objects from an TGA file.
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/AssetReader.h"
#include "PVRAssets/FileIO/FileDefinesTGA.h"

namespace pvr {
namespace assets {
namespace assetReaders {
/*!*********************************************************************************************************************
\brief Experimental TGA Texture reader
***********************************************************************************************************************/
class TextureReaderTGA : public AssetReader<Texture>
{
public:
	TextureReaderTGA();
	TextureReaderTGA(Stream::ptr_type assetStream);

	virtual bool hasAssetsLeftToLoad();
	virtual bool canHaveMultipleAssets();

	virtual bool isSupportedFile(Stream& assetStream);
	virtual std::vector<string> getSupportedFileExtensions();
private:
	virtual bool readNextAsset(Texture& asset);
	bool m_texturesToLoad;
	bool m_fileHeaderLoaded;

	texture_tga::FileHeader m_fileHeader;

	bool initializeFile();
	bool readFileHeader(texture_tga::FileHeader& fileheader);
	bool loadImageFromFile(Texture& asset);
	bool loadIndexed(Texture& asset, uint32 bytesPerPaletteEntry, uint32 bytesPerDataEntry);
	bool loadRunLength(Texture& asset, uint32 bytesPerDataEntry);
	bool loadRunLengthIndexed(Texture& asset, uint32 bytesPerPaletteEntry, uint32 bytesPerDataEntry);
};
}
}
}
