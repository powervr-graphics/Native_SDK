/*!
\brief An experimental KTX texture reader.
\file PVRCore/textureio/TextureReaderKTX.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/texture/Texture.h"
#include "PVRCore/stream/AssetReader.h"

namespace pvr {
namespace assetReaders {
/// <summary>Experimental KTX Texture reader</summary>
class TextureReaderKTX : public AssetReader<Texture>
{
public:
	/// <summary>Constructor</summary>
	TextureReaderKTX();

	/// <summary>Constructor</summary>
	/// <param name="assetStream">An asset stream to read the KTX from</param>
	TextureReaderKTX(Stream::ptr_type assetStream);

	/// <summary>Specifies if the KTX file is supported</summary>
	/// <param name="assetStream">An asset stream to read the KTX from</param>
	/// <returns>True if this reader supports the particular assetStream</returns>
	virtual bool isSupportedFile(Stream& assetStream);

private:
	virtual void readAsset_(Texture& asset);
	bool _texturesToLoad;
};
} // namespace assetReaders
} // namespace pvr
