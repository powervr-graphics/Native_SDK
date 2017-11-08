/*!
\brief An AssetReader that parses and reads PFX effect files into pvr::assets::Effect objects.
\file PVRAssets/FileIO/PFXParser.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/SkipGraph.h"
#include "PVRAssets/Effect_2.h"

#include "../External/pugixml/pugixml.hpp"

namespace pvr {
namespace assets {
// Forward Declarations
namespace pfx {

/// <summary>PFX reader.</summary>
class PfxParser : public AssetReader<effect::Effect>
{
public:
	/// <summary>Constructor. The OSManager is used to load files in a platform-specific way. If the OSManager is NULL, then
	/// only a FileStreams from the current directory will be attempted to be loaded.</summary>
	/// <param name="pfxFilename">The filename (or resource identifier) to read</param>
	/// <param name="assetsProvider">The object providing the "getAssetStream" function to read the stream</param>
	PfxParser(const std::string& pfxFilename, IAssetProvider* assetsProvider);

	/// <summary>Constructor. The OSManager is used to load any shader files in a platform-specific way. If the
	/// OSManager is NULL, then only FileStreams will be attempted to be loaded.</summary>
	/// <param name="pfxStream">The stream to read</param>
	/// <param name="assetsProvider">The object providing the "getAssetStream" function to read streams, and which
	/// will be used to read any shader files referenced in the PFX</param>
	PfxParser(Stream::ptr_type pfxStream, IAssetProvider* assetsProvider);

	/// <summary>Destructor.</summary>
	~PfxParser() {}

	/// <summary>AssetReader implementation. Read an asset from the stream.</summary>
	/// <param name="asset">The data will be read into this asset</param>
	/// <returns>Return true if successful, false if there are no assets in the stream, or the reading failed.</returns>
	virtual bool readNextAsset(effect::Effect& asset);

	/// <summary>Check if there any assets left to load</summary>
	/// <returns>Return true if there is assets left to read, else false</returns>
	virtual bool hasAssetsLeftToLoad() { return false; }


	/// <summary>Check if this PfxParser supports multiple assets</summary>
	/// <returns>Return true if multiple assets supported</returns>
	virtual bool canHaveMultipleAssets() { return false; }

	/// <summary>Get list of supported file extensions</summary>
	/// <returns>Return list of file extensions</returns>
	virtual std::vector<std::string>  getSupportedFileExtensions()
	{
		static std::vector<std::string> extensions({ "pfx", "pfx3" });
		return extensions;
	}
private:
	IAssetProvider* assetProvider;
};

}

}//namespace assets
}//namespace pvr