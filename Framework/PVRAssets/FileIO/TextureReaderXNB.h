/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/TextureReaderXNB.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An experimental AssetReader that reads pvr::asset::Texture objects from an XNB file.
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/AssetReader.h"
#include "PVRAssets/FileIO/FileDefinesXNB.h"

namespace pvr {
namespace assets {
namespace assetReaders {
/*!*********************************************************************************************************************
\brief Experimental XNB Texture reader
***********************************************************************************************************************/
class TextureReaderXNB : public AssetReader <Texture>
{
public:
	TextureReaderXNB();
	virtual bool hasAssetsLeftToLoad();
	virtual bool canHaveMultipleAssets();

	virtual bool isSupportedFile(Stream& assetStream);
	virtual std::vector<string> getSupportedFileExtensions();
private:
	virtual bool readNextAsset(Texture& asset);

	bool initializeFile();
	uint64 getPVRFormatFromXNBFormat(uint32 xnbFormat);
	VariableType getPVRTypeFromXNBFormat(uint32 xnbFormat);
	bool read7BitEncodedInt(int32& decodedInteger);
	bool readFileHeader(texture_xnb::FileHeader& xnbFileHeader);
	bool readString(string& sstringToRead);
	bool read2DTexture(texture_xnb::Texture2DHeader& assetHeader, Texture& asset);
	bool read3DTexture(texture_xnb::Texture3DHeader& assetHeader, Texture& asset);
	bool readCubeTexture(texture_xnb::TextureCubeHeader& assetHeader, Texture& asset);

private:
	texture_xnb::FileHeader     m_xnbFileHeader;
	uint32                    m_nextAssetToLoad;
	//uint32                    m_totalTextureCount;
	std::vector<string>       m_objectsStrings;
	bool                           m_fileHeaderLoaded;
};
}
}
}
