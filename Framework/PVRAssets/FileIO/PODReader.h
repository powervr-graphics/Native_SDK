/*!
\brief An AssetReader that reads POD format streams and creates pvr::assets::Model objects out of them.
\file PVRAssets/FileIO/PODReader.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRAssets/AssetIncludes.h"

namespace pvr {
namespace assets {
class Model;

/// <summary>This class creates pvr::assets::Model object from Streams of POD Model data. Use the readAsset method
/// to create Model objects from the data in your stream.</summary>
class PODReader : public AssetReader<Model>
{
public:
	/// <summary>Construct empty reader.</summary>
	PODReader();

	/// <summary>Construct reader from the specified stream.</summary>
	/// <param name="assetStream">The stream to read from</param>
	PODReader(Stream::ptr_type assetStream) : AssetReader<Model>(std::move(assetStream)) { }

	/// <summary>Check if there more assets in the stream.</summary>
	/// <returns>True if the readAsset() method can be called again to read another asset</returns>
	bool hasAssetsLeftToLoad();

	/// <summary>Check if this reader supports multiple assets per stream.</summary>
	/// <returns>True if this reader supports multiple assets per stream</returns>
	virtual bool canHaveMultipleAssets();

	/// <summary>Check if this reader supports the particular assetStream.</summary>
	/// <param name="assetStream">The stream to check</param>
	/// <returns>True if this reader supports the particular assetStream</returns>
	virtual bool isSupportedFile(Stream& assetStream);

	/// <summary>Check what are the expected file extensions for files supported by this reader.</summary>
	/// <returns>A vector with the expected file extensions for files supported by this reader</returns>
	virtual std::vector<std::string> getSupportedFileExtensions();
private:
	bool readNextAsset(Model& asset);

	bool _modelsToLoad;
};
}
}