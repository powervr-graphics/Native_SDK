/*!
\brief An experimental BMP texture reader.
\file PVRAssets/FileIO/TextureReaderBMP.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Texture.h"
#include "PVRCore/Texture/FileDefinesBMP.h"
#include "PVRCore/IO/AssetReader.h"
//!\cond NO_DOXYGEN
namespace pvr {
namespace assets {
/// <summary>Contains classes whose purpose is to read specific storage formats (bmp, POD, pfx, pvr etc.) into
/// PVRAssets classes (Texture, Model, Effect etc.).</summary>
namespace assetReaders {
/// <summary>Experimental BMP Texture reader</summary>
class TextureReaderBMP : public AssetReader < Texture >
{
public:
	TextureReaderBMP();
	TextureReaderBMP(Stream::ptr_type assetStream);

	virtual bool hasAssetsLeftToLoad();
	virtual bool canHaveMultipleAssets();

	virtual bool isSupportedFile(Stream& assetStream);
	virtual std::vector<std::string>  getSupportedFileExtensions();
private:
	virtual bool readNextAsset(Texture& asset);
	bool _texturesToLoad;
	bool _fileHeaderLoaded;

	texture_bmp::FileHeader _fileHeader;

	bool initializeFile();
	bool loadImageFromFile(Texture& asset);

	bool readFileHeader(texture_bmp::FileHeader& fileheader);
	bool readCoreHeader(uint32_t headerSize, texture_bmp::CoreHeader& coreHeader);
	bool readInfoHeader(uint32_t headerSize, texture_bmp::InfoHeader5& infoHeader);
	bool translateCoreHeader(const texture_bmp::CoreHeader& coreHeader, TextureHeader& header);
	bool translateInfoHeader(const texture_bmp::InfoHeader5& infoHeader, TextureHeader& header);
	bool readImageCoreHeader(const texture_bmp::CoreHeader& coreHeader, Texture& texture);
	bool readImageInfoHeader(const texture_bmp::InfoHeader5& infoHeader, Texture& texture);

	bool loadRowAligned(Texture& asset, uint32_t bytesPerDataEntry, uint32_t rowAlignment);
	bool loadIndexed(Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bitsPerDataEntry,
	                 uint32_t numPaletteEntries, uint32_t rowAlignment);
};

}
}
}
//!\endcond