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
	AssetReader(Stream::ptr_type assetStream) : _assetStream(std::move(assetStream)), _hasNewAssetStream(true) {}

	/// <summary>Destructor. Virtual, as this class will be used polymorphically.</summary>
	virtual ~AssetReader() {}
	
	/// <summary>A smart, reference counted, pointer type for the Assets. This will be the typical type that
	/// readers will use to wrapping the Asset read and return it to the application.</summary>
	typedef pvr::RefCountedResource<AssetType> AssetHandle;
public:

	/// <summary>Initialize with a new asset stream without open it. </summary>
	/// <param name="assetStream">stream to load</param>
	/// <returns>True if successful, otherwise false (if the stream is invalid or not readable)</returns>
	bool newAssetStream(Stream::ptr_type assetStream)
	{
		closeAssetStream();
		_assetStream = std::move(assetStream);

		if (_assetStream.get() && (*_assetStream).isReadable())
		{
			return true;
		}

		return false;
	}

	/// <summary>Open the (Already set) asset stream.</summary>
	/// <returns>True if successful, otherwise false (if the stream is invalid, not readable
	/// or was unable to open)</returns>
	bool openAssetStream()
	{
		closeAssetStream();

		if (_assetStream.get() && (*_assetStream).isReadable())
		{
			return (*_assetStream).open();
		}

		return false;
	}

	/// <summary>Initialize with a new asset stream and open it.</summary>
	/// <param name="assetStream">The stream to load</param>
	/// <returns>True if successful, otherwise false (if the stream is invalid, not readable
	/// or was unable to open)</returns>
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
	/// <returns>True if has an assetstream otherwise false</returns>
	bool hasAssetStream() { return _assetStream.get() != 0; }

	/// <summary>Read asset. Asset stream has to be opended before reading asset.</summary>
	/// <param name="asset">type type of asset to read</param>
	/// <returns>true on success</returns>
	bool readAsset(AssetType& asset)
	{
		if (!hasAssetStream())
		{
			Log(LogLevel::Warning, "AssetReader::readAsset Attempted to read without an assetStream");
			assertion(0);
			return false;
		}
		if (!_assetStream->isopen())
		{
			_assetStream->open();
		}
		if (!_assetStream->isReadable())
		{
			Log(LogLevel::Error, "AssetReader::readAsset Attempted to read a non-readable assetStream");
			assertion(0);
			return false;
		}
		return readNextAsset(asset);
	}

	/// <summary>Implement this function: Query if this object has assets to read (other has not been used yet, has been
	/// reset, or is reading a stream with multiple assets .</summary>
	/// <returns> Implement this to return true "readNextAsset" can be called, otherwise return false</returns>
	virtual bool hasAssetsLeftToLoad() = 0;

	/// <summary>Implement this to return a list of all supported file extensions. Implement this function in your AssetReader.</summary>
	/// <returns> Return a list of all supported file extensions (without the dot).</returns>
	virtual std::vector<std::string>  getSupportedFileExtensions() = 0;

	/// <summary>Create a new Asset. Uses the (static) AssetType::createWithReader function to create a new
	/// asset that will be wrapped in a RefCountedResource to that asset.</summary>
	/// <returns>A RefCountedResource to the handle read with this readed.</returns>
	AssetHandle getAssetHandle() { return AssetType::createWithReader(*this); }
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
	virtual bool readNextAsset(AssetType& asset) = 0;
};
}