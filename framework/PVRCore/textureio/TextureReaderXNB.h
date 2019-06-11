/*!
\brief An experimental AssetReader that reads pvr::asset::Texture objects from an XNB file.
\file PVRCore/textureio/TextureReaderXNB.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/textureio/FileDefinesXNB.h"
#include "PVRCore/stream/AssetReader.h"

namespace pvr {
namespace assetReaders {
/// <summary>Experimental XNB Texture reader</summary>
class TextureReaderXNB : public AssetReader<Texture>
{
public:
	/// <summary>Constructor</summary>
	TextureReaderXNB();

	/// <summary>Specifies if the XNB file is supported</summary>
	/// <param name="assetStream">An asset stream to read the XNB from</param>
	/// <returns>True if this reader supports the particular assetStream</returns>
	virtual bool isSupportedFile(Stream& assetStream);

private:
	virtual void readAsset_(Texture& asset);

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
} // namespace pvr
