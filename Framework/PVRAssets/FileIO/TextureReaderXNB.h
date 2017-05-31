/*!
\brief An experimental AssetReader that reads pvr::asset::Texture objects from an XNB file.
\file PVRAssets/FileIO/TextureReaderXNB.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Texture.h"
#include "PVRCore/Texture/FileDefinesXNB.h"
#include "PVRCore/IO/AssetReader.h"

namespace pvr {
namespace assets {
namespace assetReaders {
/// <summary>Experimental XNB Texture reader</summary>
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
	texture_xnb::FileHeader     _xnbFileHeader;
	uint32                    _nextAssetToLoad;
	//uint32                    _totalTextureCount;
	std::vector<string>       _objectsStrings;
	bool                           _fileHeaderLoaded;
};
}
}
}