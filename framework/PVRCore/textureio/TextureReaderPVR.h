/*!
\brief An AssetReader that reads pvr::asset::Texture objects from a PVR stream(file). Used extensively in the
PowerVR Framework and examples.
\file PVRCore/textureReaders/TextureReaderPVR.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/textureio/FileDefinesPVR.h"
#include "PVRCore/stream/AssetReader.h"

//!\cond NO_DOXYGEN
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
	TextureReaderPVR(Stream::ptr_type assetStream);

	/// <summary>Check if this reader supports the particular assetStream</summary>
	/// <returns>True if this reader supports the particular assetStream</returns>
	virtual bool isSupportedFile(Stream& assetStream);

	/// <summary>Convert a PVR Version 2 header to a PVR Version 3 header</summary>
	static void convertTextureHeader2To3(const texture_legacy::HeaderV2& legacyHeader, TextureHeader& newHeader);

	/// <summary>Convert a legacy PixelFormat into a new PixelFormat</summary>
	static void mapLegacyEnumToNewFormat(
		const texture_legacy::PixelFormat legacyPixelType, PixelFormat& newPixelType, ColorSpace& newColorSpace, VariableType& newChannelType, bool& isPremultiplied);

private:
	virtual void readAsset_(Texture& asset);
	bool _texturesToLoad;
};
} // namespace assetReaders
} // namespace pvr
//!\endcond
