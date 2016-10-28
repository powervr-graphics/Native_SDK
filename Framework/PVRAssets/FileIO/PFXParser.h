/*!*********************************************************************************************************************
\file         PVRAssets\FileIO\PFXParser.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An AssetReader that parses and reads PFX effect files into pvr::assets::Effect objects.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/AssetReader.h"
#include "PVRAssets/SkipGraph.h"
#include "PVRAssets/Effect_2.h"
#include "PVRCore/OSManager.h"


#include "../External/pugixml/pugixml.hpp"


namespace pvr {
namespace assets {
// Forward Declarations
namespace pfx {

/*!*****************************************************************************************************************
\brief PFX reader.
*******************************************************************************************************************/
class PfxParser : public AssetReader<effect::Effect>
{
public:
	/*!*****************************************************************************************************************
	\brief  Constructor. The OSManager is used to load files in a platform-specific way. If the OSManager is NULL, then
	only a FileStreams from the current directory will be attempted to be loaded.
	*******************************************************************************************************************/
	PfxParser(const std::string& pfxFilename, IAssetProvider* assetsProvider);

	/*!*****************************************************************************************************************
	\brief  Constructor. The OSManager is used to load pfxFilename and any shader files in a platform-specific way.
	If the OSManager is NULL, then only FileStreams will be attempted to be loaded.
	*******************************************************************************************************************/
	PfxParser(Stream::ptr_type pfxStream, IAssetProvider* assetsProvider);

	/*!*****************************************************************************************************************
	\brief  Destructor.
	*******************************************************************************************************************/
	~PfxParser() {}

	/*!*******************************************************************************************************************************
	\brief	AssetReader implementation. Read an asset from the stream. 
	\param[out] asset The data will be read into this asset
	\return	Return true if successful, false if there are no assets in the stream, or the reading failed.
	**********************************************************************************************************************************/
	virtual bool readNextAsset(effect::Effect& asset);

	/*!*******************************************************************************************************************************
	\brief	Check if there any assets left to load
	\return	Return true if there is assets left to read, else false
	**********************************************************************************************************************************/
	virtual bool hasAssetsLeftToLoad() { return false; }


	/*!*******************************************************************************************************************************
	\brief	Check if this PfxParser supports multiple assets
	\return	Return true if multiple assets supported
	**********************************************************************************************************************************/
	virtual bool canHaveMultipleAssets() { return false; }

	/*!*******************************************************************************************************************************
	\brief	Get list of supported file extensions
	\return	Return list of file extensions
	**********************************************************************************************************************************/
	virtual std::vector<std::string>  getSupportedFileExtensions()
	{
		static std::vector<string> extensions({ "pfx", "pfx3" });
		return extensions;
	}
private:
	IAssetProvider* assetProvider;
};

}

}//namespace assets
}//namespace pvr
