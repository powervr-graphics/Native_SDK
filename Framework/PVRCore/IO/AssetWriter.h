/*!
\brief Contains the AssetWriter class.
\file PVRCore/IO/AssetWriter.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {

/// <summary>Base class for Classes that can be used to write back effects to disk.</summary>
template <typename AssetType>
class AssetWriter
{
public:
	/// <summary>Default constructor.</summary>
	AssetWriter() : _assetStream(NULL) {}

public:
	/// <summary>Open an asset stream for writing. Implement this in your concrete AssetWriter implementation.
	/// </summary>
	/// <param name="assetStream">The stream to write to.</param>
	/// <returns>return Result::Success if successful</returns>
	bool openAssetStream(Stream::ptr_type assetStream)
	{
		if (_assetStream.get())
		{
			(*_assetStream).close();
		}

		_assetStream = assetStream;

		if (!_assetStream.get() || !(*_assetStream).isWritable())
		{
			return false;
		}
		return (*_assetStream).open();
	}

	/// <summary>Add another asset to write. Implement this in your concrete AssetWriter implementation.</summary>
	/// <param name="asset">asset to write</param>
	/// <returns>return Result::Success on success</returns>
	virtual bool addAssetToWrite(const AssetType& asset) = 0;

	/// <summary>Write out all assets to the Stream. Implement this in your concrete AssetWriter implementation.
	/// </summary>
	/// <returns>return Result::Success on success.</returns>
	virtual bool writeAllAssets() = 0;

	/// <summary>Get the number of assets that have been added for writing. Implement this in your concrete
	/// AssetWriter implementation.</summary>
	/// <returns>The number of assets that have been added for writing.</returns>
	virtual uint32 assetsAddedSoFar() = 0;

	/// <summary>Query if this writer supports multiple assets for writing. Implement this in your concrete AssetWriter
	/// implementation.</summary>
	/// <returns>return true if multiple assets are supported.</returns>
	virtual bool supportsMultipleAssets() = 0;

	/// <summary>Query if this writer can write out the specified asset. Implement this in your concrete AssetWriter
	/// implementation.</summary>
	/// <param name="asset">The asset to check for writing.</param>
	/// <returns>True if this asset is supported.</returns>
	virtual bool canWriteAsset(const AssetType& asset) = 0;

	/// <summary>Get a list of supported file extensions. Implement this in your concrete AssetWriter implementation.
	/// </summary>
	/// <returns>The list of supported file extensions.</returns>
	virtual std::vector<string> getSupportedFileExtensions() = 0;

	/// <summary>Get the name of this writer. Implement this in your concrete AssetWriter implementation.</summary>
	/// <returns>writer name</returns>
	virtual string getWriterName() = 0;

	/// <summary>Get the version of this writer. Implement this in your concrete AssetWriter implementation.
	/// </summary>
	/// <returns>writer version</returns>
	virtual string getWriterVersion() = 0;

protected:
	std::auto_ptr<Stream>                 _assetStream; //!< The Stream that this writer uses
	std::vector<const AssetType*> _assetsToWrite;  //!< The list of assets to write so far
};
}