/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/TextureReaderPVR.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An AssetReader that reads pvr::asset::Texture objects from a PVR stream(file). Used extensively in the PowerVR
              Framework and examples.
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/AssetReader.h"
#include "PVRAssets/FileIO/FileDefinesPVR.h"

namespace pvr {
namespace assets {
namespace assetReaders {
/*!*********************************************************************************************************************
\brief    This class creates pvr::assets::Texture object from Streams of PVR texture data. Use the readAsset method to
         create Texture objects from the data in your stream
***********************************************************************************************************************/
class TextureReaderPVR : public AssetReader<Texture>
{
public:
	/*!******************************************************************************************************************
	\brief    Construct empty reader
	********************************************************************************************************************/
	TextureReaderPVR();
	/*!******************************************************************************************************************
	\brief    Construct reader from the specified stream
	********************************************************************************************************************/
	TextureReaderPVR(Stream::ptr_type assetStream);

	/*!******************************************************************************************************************
	\brief    Check if there more assets in the stream
	\return  True if the readAsset() method can be called again to read another asset
	********************************************************************************************************************/
	virtual bool hasAssetsLeftToLoad();

	/*!******************************************************************************************************************
	\brief    Check if this reader supports multiple assets per stream
	\return  True if this reader supports multiple assets per stream
	********************************************************************************************************************/
	virtual bool canHaveMultipleAssets();

	/*!******************************************************************************************************************
	\brief    Check if this reader supports the particular assetStream
	\return  True if this reader supports the particular assetStream
	********************************************************************************************************************/
	virtual bool isSupportedFile(Stream& assetStream);

	/*!******************************************************************************************************************
	\brief    Check what are the expected file extensions for files supported by this reader
	\return  A vector with the expected file extensions for files supported by this reader
	********************************************************************************************************************/
	virtual std::vector<std::string> getSupportedFileExtensions();

	/*!******************************************************************************************************************
	\brief    Convert a PVR Version 2 header to a PVR Version 3 header
	********************************************************************************************************************/
	static bool convertTextureHeader2To3(const texture_legacy::HeaderV2& legacyHeader, TextureHeader& newHeader);

	/*!******************************************************************************************************************
	\brief    Convert a legacy PixelFormat into a new PixelFormat
	********************************************************************************************************************/
	static bool mapLegacyEnumToNewFormat(const texture_legacy::PixelFormat legacyPixelType, PixelFormat& newPixelType,
	                                     types::ColorSpace& newColorSpace, VariableType& newChannelType,
	                                     bool& isPremultiplied);
private:
	virtual bool readNextAsset(Texture& asset);
	bool m_texturesToLoad;
};
}
}
}
