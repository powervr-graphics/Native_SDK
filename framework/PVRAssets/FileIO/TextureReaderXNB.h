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

//!\cond NO_DOXYGEN
namespace pvr {
namespace assets {
namespace assetReaders {
/// <summary>Experimental XNB Texture reader</summary>
class TextureReaderXNB : public AssetReader<Texture>
{
public:
	TextureReaderXNB();
	virtual bool hasAssetsLeftToLoad();
	virtual bool canHaveMultipleAssets();

	virtual bool isSupportedFile(Stream& assetStream);
	virtual std::vector<std::string> getSupportedFileExtensions();

private:
	virtual void readNextAsset(Texture& asset);

	void initializeFile();
	uint64_t getPVRFormatFromXNBFormat(uint32_t xnbFormat);
	VariableType getPVRTypeFromXNBFormat(uint32_t xnbFormat);
	void read7BitEncodedInt(int32_t& decodedInteger);
	void readFileHeader(texture_xnb::FileHeader& xnbFileHeader);
	void readString(std::string& sstringToRead);
	void read2DTexture(texture_xnb::Texture2DHeader& assetHeader, Texture& asset);
	void read3DTexture(texture_xnb::Texture3DHeader& assetHeader, Texture& asset);
	void readCubeTexture(texture_xnb::TextureCubeHeader& assetHeader, Texture& asset);

private:
	texture_xnb::FileHeader _xnbFileHeader;
	uint32_t _nextAssetToLoad;
	std::vector<std::string> _objectsStrings;
	bool _fileHeaderLoaded;
};
} // namespace assetReaders
} // namespace assets
} // namespace pvr
//!\endcond