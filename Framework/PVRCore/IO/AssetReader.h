/*!
\brief Contains the base class for any AssetReader.
\file PVRCore/IO/AssetReader.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/Stream.h"
namespace pvr {

/// <summary>Base class for an AssetReader, a class that can read Assets from a provided Stream.</summary>
/// <typeparam name="AssetType">The typename (class) of assets that will be read from this AssetReader.
/// </typeparam>
/// <remarks>A reader of a specific type of assets will normally inherit from an AssetReader of the specified
/// AssetType. For example, a reader that reads Models from POD files will be: class PODReader:AssetReader<Model>
/// </remarks>
template <typename AssetType>
class AssetReader
{
public:

	/// <summary>Empty asset reader.</summary>
	AssetReader(): _hasNewAssetStream(true) {}

	/// <summary>Asset reader which will take ownership of the provided Stream object and read Assets from it.
	/// </summary>
	/// <param name="assetStream">stream to read</param>
	AssetReader(Stream::ptr_type assetStream) : _assetStream(assetStream), _hasNewAssetStream(true) {}


	virtual ~AssetReader() {}
	typedef pvr::RefCountedResource<AssetType> AssetHandle;
public:

	/// <summary>Open new asset stream.</summary>
	/// <param name="assetStream">stream to load</param>
	/// <returns>true</returns>
	bool newAssetStream(Stream::ptr_type assetStream)
	{
		closeAssetStream();
		_assetStream = assetStream;

		if (_assetStream.get() && (*_assetStream).isReadable())
		{
			return true;
		}

		return false;
	}

	/// <summary>Open the asset stream.</summary>
	/// <returns>true if successful</returns>
	bool openAssetStream()
	{
		closeAssetStream();

		if (_assetStream.get() && (*_assetStream).isReadable())
		{
			return (*_assetStream).open();
		}

		return false;
	}

	/// <summary>Open new asset stream.</summary>
	/// <param name="assetStream">stream to load</param>
	/// <returns>true</returns>
	bool openAssetStream(Stream::ptr_type assetStream)
	{
		if (newAssetStream(assetStream))
		{
			return openAssetStream();
		}
		return false;
	}

	/// <summary>Close the asset stream.</summary>
	void closeAssetStream() { if (_assetStream.get()) { _assetStream->close(); } }

	/// <summary>Return true if this AssetReader assets stream is loaded.</summary>
	bool hasAssetStream() { return _assetStream.get() != 0; }

	/// <summary>Read asset. Asset stream has to be opended before reading asset.</summary>
	/// <param name="asset">type type of asset to read</param>
	/// <returns>true on success</returns>
	bool readAsset(AssetType& asset)
	{
		if (!hasAssetStream())
		{
			Log(Log.Warning, "AssetReader::readAsset Attempted to read without an assetStream");
			assertion(0);
			return false;
		}
		if (!_assetStream->isopen())
		{
			_assetStream->open();
		}
		if (!_assetStream->isReadable())
		{
			Log(Log.Error, "AssetReader::readAsset Attempted to read a non-readable assetStream");
			assertion(0);
			return false;
		}
		return readNextAsset(asset);
	}

	/// <summary>Return true if assets are left to read. Implement this function in your AssetReader.</summary>
	virtual bool hasAssetsLeftToLoad() = 0;

	/// <summary>Return true if this AssetReader supports multiple assets. Implement this function in your AssetReader.
	/// </summary>
	virtual bool canHaveMultipleAssets() { return false; }

	/// <summary>Return a list of supported file extensions. Implement this function in your AssetReader.</summary>
	virtual std::vector<std::string>  getSupportedFileExtensions() = 0;

	/// <summary>Read the asset in a new Asset wrapped in a Framework smart handle.</summary>
	AssetHandle getAssetHandle() { return AssetType::createWithReader(*this); }
protected:
	Stream::ptr_type _assetStream;
	bool _hasNewAssetStream;
private:
	/// <summary>Implement this function in your class. Provides the main functionality of reading assets.</summary>
	/// <param name="asset">The asset will be read into this object. Any state of the object may be overwritten.
	/// </param>
	virtual bool readNextAsset(AssetType& asset) = 0;
};
}