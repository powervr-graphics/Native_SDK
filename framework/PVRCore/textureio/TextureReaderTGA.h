/*!
\brief An experimental AssetReader that reads pvr::asset::Texture objects from an TGA file.
\file PVRCore/textureReaders/TextureReaderTGA.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/textureio/FileDefinesTGA.h"
#include "PVRCore/stream/AssetReader.h"

//!\cond NO_DOXYGEN
namespace pvr {
namespace assetReaders {
/// <summary>Experimental TGA Texture reader</summary>
class TextureReaderTGA : public AssetReader<Texture>
{
public:
	TextureReaderTGA();
	TextureReaderTGA(Stream::ptr_type assetStream);

	virtual bool isSupportedFile(Stream& assetStream);

private:
	virtual void readAsset_(Texture& asset);
	bool _texturesToLoad;
	bool _fileHeaderLoaded;

	texture_tga::FileHeader _fileHeader;

	void initializeFile();
	void readFileHeader(texture_tga::FileHeader& fileheader);
	void loadImageFromFile(Texture& asset);
	void loadIndexed(Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bytesPerDataEntry);
	void loadRunLength(Texture& asset, uint32_t bytesPerDataEntry);
	void loadRunLengthIndexed(Texture& asset, uint32_t bytesPerPaletteEntry, uint32_t bytesPerDataEntry);
};
} // namespace assetReaders
} // namespace pvr
//!\endcond
