/*!
\brief An AssetReader that reads pvr::asset::Texture objects from a PVR stream(file). Used extensively in the
PowerVR Framework and examples.
\file PVRAssets/FileIO/TextureReaderPVR.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Texture.h"
#include "PVRCore/Texture/FileDefinesPVR.h"
#include "PVRCore/IO/AssetReader.h"

//!\cond NO_DOXYGEN
namespace pvr {
namespace assets {
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

	/// <summary>Check if there more assets in the stream</summary>
	/// <returns>True if the readAsset() method can be called again to read another asset</returns>
	virtual bool hasAssetsLeftToLoad();

	/// <summary>Check if this reader supports multiple assets per stream</summary>
	/// <returns>True if this reader supports multiple assets per stream</returns>
	virtual bool canHaveMultipleAssets();

	/// <summary>Check if this reader supports the particular assetStream</summary>
	/// <returns>True if this reader supports the particular assetStream</returns>
	virtual bool isSupportedFile(Stream& assetStream);

	/// <summary>Check what are the expected file extensions for files supported by this reader</summary>
	/// <returns>A vector with the expected file extensions for files supported by this reader</returns>
	virtual std::vector<std::string> getSupportedFileExtensions();

	/// <summary>Convert a PVR Version 2 header to a PVR Version 3 header</summary>
	static bool convertTextureHeader2To3(const texture_legacy::HeaderV2& legacyHeader, TextureHeader& newHeader);

	/// <summary>Convert a legacy PixelFormat into a new PixelFormat</summary>
	static bool mapLegacyEnumToNewFormat(const texture_legacy::PixelFormat legacyPixelType, PixelFormat& newPixelType,
	                                     ColorSpace& newColorSpace, VariableType& newChannelType,
	                                     bool& isPremultiplied);
private:
	virtual bool readNextAsset(Texture& asset);
	bool _texturesToLoad;
};
}
}
}
//!\endcond