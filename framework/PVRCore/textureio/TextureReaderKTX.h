/*!
\brief An experimental KTX texture reader.
\file PVRCore/textureReaders/TextureReaderKTX.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/texture/Texture.h"
#include "PVRCore/stream/AssetReader.h"

//!\cond NO_DOXYGEN
namespace pvr {
namespace assetReaders {
/// <summary>Experimental KTX Texture reader</summary>
class TextureReaderKTX : public AssetReader<Texture>
{
public:
	TextureReaderKTX();
	TextureReaderKTX(Stream::ptr_type assetStream);

	virtual bool isSupportedFile(Stream& assetStream);

private:
	virtual void readAsset_(Texture& asset);
	bool _texturesToLoad;
};
} // namespace assetReaders

} // namespace pvr
//!\endcond
