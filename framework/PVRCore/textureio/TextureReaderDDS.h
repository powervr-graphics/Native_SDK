/*!
\brief An experimental DDS texture reader.
\file PVRCore/textureio/TextureReaderDDS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/textureio/FileDefinesDDS.h"
#include "PVRCore/stream/AssetReader.h"

namespace pvr {
namespace assetReaders {
/// <summary>Experimental DDS Texture reader</summary>
class TextureReaderDDS : public AssetReader<Texture>
{
public:
	/// <summary>Constructor</summary>
	TextureReaderDDS();

	/// <summary>Constructor</summary>
	/// <param name="assetStream">An asset stream to read the DDS from</param>
	TextureReaderDDS(Stream::ptr_type assetStream);

	/// <summary>Specifies if the DDS file is supported</summary>
	/// <param name="assetStream">An asset stream to read the DDS from</param>
	/// <returns>True if this reader supports the particular assetStream</returns>
	virtual bool isSupportedFile(Stream& assetStream);

private:
	virtual void readAsset_(Texture& asset);
	uint32_t getDirect3DFormatFromDDSHeader(texture_dds::FileHeader& textureFileHeader);
	bool _texturesToLoad;
};
} // namespace assetReaders
} // namespace pvr
