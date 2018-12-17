/*!
\brief An experimental DDS texture reader.
\file PVRCore/textureReaders/TextureReaderDDS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/textureio/FileDefinesDDS.h"
#include "PVRCore/stream/AssetReader.h"

//!\cond NO_DOXYGEN
namespace pvr {
namespace assetReaders {
/// <summary>Experimental DDS Texture reader</summary>
class TextureReaderDDS : public AssetReader<Texture>
{
public:
	TextureReaderDDS();
	TextureReaderDDS(Stream::ptr_type assetStream);

	virtual bool isSupportedFile(Stream& assetStream);

private:
	virtual void readAsset_(Texture& asset);
	uint32_t getDirect3DFormatFromDDSHeader(texture_dds::FileHeader& textureFileHeader);
	bool _texturesToLoad;
};
} // namespace assetReaders
} // namespace pvr
//!\endcond
