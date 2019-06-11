/*!
\brief An AssetReader that reads pvr::asset::Texture objects from a PVR stream(file). Used extensively in the
PowerVR Framework and examples.
\file PVRCore/textureio/TextureReaderPVR.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/textureio/FileDefinesPVR.h"
#include "PVRCore/stream/AssetReader.h"

namespace pvr {
namespace assetReaders {
/// <summary>This class creates pvr::Texture object from Streams of PVR texture data. Use the readAsset
/// method to create Texture objects from the data in your stream</summary>
class TextureReaderPVR : public AssetReader<Texture>
{
public:
	/// <summary>Construct empty reader</summary>
	TextureReaderPVR();

	/// <summary>Construct reader from the specified stream</summary>
	/// <param name="assetStream">An asset stream to read the PVR from</param>
	TextureReaderPVR(Stream::ptr_type assetStream);

	/// <summary>Check if this reader supports the particular assetStream</summary>
	/// <param name="assetStream">An asset stream to read the PVR from</param>
	/// <returns>True if this reader supports the particular assetStream</returns>
	virtual bool isSupportedFile(Stream& assetStream);

	/// <summary>Convert a PVR Version 2 header to a PVR Version 3 header</summary>
	/// <param name="legacyHeader">A legacy header to use</param>
	/// <param name="newHeader">A new style header to use</param>
	static void convertTextureHeader2To3(const texture_legacy::HeaderV2& legacyHeader, TextureHeader& newHeader);

	/// <summary>maps legacy enums to new style formats</summary>
	/// <param name="legacyPixelType">A legacy pixel type</param>
	/// <param name="newPixelType">A new style pixel type</param>
	/// <param name="newColorSpace">A new style color space</param>
	/// <param name="newChannelType">A new style channel type</param>
	/// <param name="isPremultiplied">A new style premultiplied alpha</param>
	static void mapLegacyEnumToNewFormat(
		const texture_legacy::PixelFormat legacyPixelType, PixelFormat& newPixelType, ColorSpace& newColorSpace, VariableType& newChannelType, bool& isPremultiplied);

private:
	virtual void readAsset_(Texture& asset);
	bool _texturesToLoad;
};
} // namespace assetReaders
} // namespace pvr
