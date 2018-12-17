/*!
\brief Contains the AssetWriter class.
\file PVRCore/io/AssetWriter.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/Stream.h"

namespace pvr {

/// <summary>Base class for Classes that can be used to write back effects to disk.</summary>
template<typename AssetType>
class AssetWriter
{
public:
	/// <summary>Initialize with a new asset stream without open it.</summary>
	/// <param name="assetStream">stream to load</param>
	/// <returns>True if successful, otherwise false (if the stream is invalid or not readable)</returns>
	void newAssetStream(Stream::ptr_type assetStream)
	{
		closeAssetStream();
		_assetStream = std::move(assetStream);
	}

	/// <summary>Close the asset stream.</summary>
	void closeAssetStream()
	{
		if (_assetStream.get())
		{
			_assetStream->close();
		}
	}

	/// <summary>Return true if this AssetReader assets stream is loaded.</summary>
	/// <returns>True if has an assetstream otherwise false</returns>
	bool hasAssetStream()
	{
		return _assetStream.get() != 0;
	}

	/// <summary>Open an asset stream for writing. Implement this in your concrete AssetWriter implementation.
	/// </summary>
	/// <param name="assetStream">The stream to write to.</param>
	/// <returns>return Result::Success if successful</returns>
	void openAssetStream(Stream::ptr_type assetStream)
	{
		if (!assetStream.get())
		{
			throw InvalidArgumentError("assetStream", "AssetWriter::openAssetStream: Cannot open assetStream as it was NULL");
		}
		if (!assetStream->isWritable())
		{
			throw InvalidArgumentError("assetStream", "AssetWriter::openAssetStream: Cannot open assetStream as it is not writable");
		}
		if (_assetStream.get())
		{
			(*_assetStream).close();
		}
		_assetStream = std::move(assetStream);
		return (*_assetStream).open();
	}

	/// <summary>Write out all assets to the Stream. Implement this in your concrete AssetWriter implementation.
	/// </summary>
	/// <returns>return Result::Success on success.</returns>
	virtual void writeAsset(const AssetType& asset) = 0;

	/// <summary>Query if this writer can write out the specified asset. Implement this in your concrete AssetWriter
	/// implementation.</summary>
	/// <param name="asset">The asset to check for writing.</param>
	/// <returns>True if this asset is supported.</returns>
	virtual bool canWriteAsset(const AssetType& asset) = 0;

protected:
	std::unique_ptr<Stream> _assetStream; //!< The Stream that this writer uses
};
} // namespace pvr
