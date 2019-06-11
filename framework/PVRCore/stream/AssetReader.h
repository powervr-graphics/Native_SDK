/*!
\brief Contains the base class for any AssetReader.
\file PVRCore/stream/AssetReader.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/Stream.h"

namespace pvr {

/// <summary>Base class for an AssetReader, a class that can read Assets from a provided Stream.</summary>
/// <typeparam name="AssetType">The typename (class) of assets that will be read from this AssetReader.</typeparam>
/// <remarks>A reader of a specific type of assets will normally inherit from an AssetReader of the specified
/// AssetType. For example, a reader that reads Models from POD files will be: class PODReader:AssetReader\<Model\></remarks>
template<typename AssetType>
class AssetReader
{
public:
	/// <summary>Empty asset reader.</summary>
	AssetReader() : _hasNewAssetStream(true) {}

	/// <summary>Asset reader which will take ownership of the provided Stream object and read Assets from it.</summary>
	/// <param name="assetStream">stream to read</param>
	AssetReader(Stream::ptr_type assetStream) : _assetStream(std::move(assetStream)), _hasNewAssetStream(true) {}

	/// <summary>Destructor. Virtual, as this class will be used polymorphically.</summary>
	virtual ~AssetReader() {}

	/// <summary>A smart, reference counted, pointer type for the Assets. This will be the typical type that
	/// readers will use to wrapping the Asset read and return it to the application.</summary>
	typedef std::shared_ptr<AssetType> AssetHandle;

public:
	/// <summary>Initialize with a new asset stream without open it.</summary>
	/// <param name="assetStream">stream to load</param>
	/// <returns>True if successful, otherwise false (if the stream is invalid or not readable)</returns>
	void newAssetStream(Stream::ptr_type assetStream)
	{
		closeAssetStream();
		_assetStream = std::move(assetStream);
		_hasNewAssetStream = true;
	}

	/// <summary>Opens the asset stream if it is not already open.</summary>
	void openAssetStream()
	{
		if (_assetStream.get() && !_assetStream->isopen())
		{
			_assetStream->open();
		}
	}

	/// <summary>Close the asset stream.</summary>
	void closeAssetStream()
	{
		if (_assetStream.get() && _assetStream->isopen())
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

	/// <summary>Read asset. Asset stream has to be opended before reading asset.</summary>
	/// <param name="asset">type type of asset to read</param>
	/// <returns>true on success</returns>
	void readAsset(AssetType& asset)
	{
		if (!hasAssetStream())
		{
			throw InvalidOperationError("AssetReader::readAsset Attempted to read without an assetStream");
		}
		openAssetStream();
		if (!_assetStream->isReadable())
		{
			throw InvalidOperationError("AssetReader::readAsset Attempted to read a non-readable assetStream");
		}
		readAsset_(asset);
	}

	/// <summary>Read asset. Asset stream has to be opended before reading asset.</summary>
	/// <returns>Retrieves the read asset</returns>
	AssetType readAsset()
	{
		if (!hasAssetStream())
		{
			throw InvalidOperationError("AssetReader::readAsset Attempted to read without an assetStream");
		}
		openAssetStream();
		if (!_assetStream->isReadable())
		{
			throw InvalidOperationError("AssetReader::readAsset Attempted to read a non-readable assetStream");
		}
		AssetType retval;
		readAsset_(retval);
		return retval;
	}

	/// <summary>Create a new Asset. Uses the (static) AssetType::createWithReader function to create a new
	/// asset that will be wrapped in a std::shared_ptr to that asset.</summary>
	/// <returns>A std::shared_ptr to the handle read with this readed.</returns>
	AssetHandle getAssetHandle()
	{
		return AssetType::createWithReader(*this);
	}

protected:
	/// <summary>The stream that this reader is reading</summary>
	Stream::ptr_type _assetStream;
	/// <summary>Use this field to detect if the asset stream has a new stream, which might require initialization</summary>
	bool _hasNewAssetStream;

private:
	/// <summary>Implement this function in your class. Provides the main functionality of reading assets.</summary>
	/// <param name="asset">The asset will be read into this object. You should not make any assumptions of the state of
	/// this object, so you should initialize it completely.</param>
	/// <returns>When overriding this funciton, return true if the read was successful, otherwise return false to signify
	/// any failure.</returns>
	virtual void readAsset_(AssetType& asset) = 0;
};
} // namespace pvr
