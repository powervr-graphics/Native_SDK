/*!*********************************************************************************************************************
\file         PVRAssets/AssetReader.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the base class for any AssetReader.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/Stream.h"
namespace pvr {
namespace assets {

/*!*********************************************************************************************************************
\brief    Base class for an AssetReader, a class that can read Assets from a provided Stream.
\param    AssetType The typename (class) of assets that will be read from this AssetReader.
\brief   A reader of a specific type of assets will normally inherit from an AssetReader of the specified AssetType.
          For example, a reader that reads Models from POD files will be:    class PODReader:AssetReader<Model>
***********************************************************************************************************************/
template <typename AssetType>
class AssetReader
{
public:

	/*!******************************************************************************************************************
	\brief Empty asset reader.
	********************************************************************************************************************/
	AssetReader(): m_hasNewAssetStream(true) {}

	/*!******************************************************************************************************************
	\brief Asset reader which will take ownership of the provided Stream object and read Assets from it.
	  \param assetStream stream to read
	********************************************************************************************************************/
	AssetReader(Stream::ptr_type assetStream) : m_assetStream(assetStream), m_hasNewAssetStream(true) {}

	typedef pvr::RefCountedResource<AssetType> AssetHandle;
public:

	/*!****************************************************************************************************************
	\brief Open new asset stream.
	\param assetStream stream to load
	\return true
	******************************************************************************************************************/
	bool newAssetStream(Stream::ptr_type assetStream)
	{
		closeAssetStream();
		m_assetStream = assetStream;

		if (m_assetStream.get() && (*m_assetStream).isReadable())
		{
			return true;
		}

		return false;
	}

	/*!****************************************************************************************************************
	\brief Open the asset stream.
	\return true if successful
	******************************************************************************************************************/
	bool openAssetStream()
	{
		closeAssetStream();

		if (m_assetStream.get() && (*m_assetStream).isReadable())
		{
			return (*m_assetStream).open();
		}

		return false;
	}

	/*!****************************************************************************************************************
	\brief Open new asset stream.
	\param assetStream stream to load
	\return true
	******************************************************************************************************************/
	bool openAssetStream(Stream::ptr_type assetStream)
	{
		if (newAssetStream(assetStream))
		{
			return openAssetStream();
		}
		return false;
	}

	/*!*********************************************************************************************************************
	\brief Close the asset stream.
	***********************************************************************************************************************/
	void closeAssetStream() { if (m_assetStream.get()) { m_assetStream->close(); } }

	/*!*********************************************************************************************************************
	\brief Return true if this AssetReader assets stream is loaded.
	***********************************************************************************************************************/
	bool hasAssetStream() { return m_assetStream.get() != 0; }

	/*!*********************************************************************************************************************
	\brief Read asset. Asset stream has to be opended before reading asset.
	\param asset type type of asset to read
	\return true on success
	***********************************************************************************************************************/
	bool readAsset(AssetType& asset)
	{
		if (!hasAssetStream())
		{
			Log(Log.Warning, "AssetReader::readAsset Attempted to read without an assetStream");
			assertion(0);
			return false;
		}
		if (!m_assetStream->isopen())
		{
			m_assetStream->open();
		}
		if (!m_assetStream->isReadable())
		{
			Log(Log.Error, "AssetReader::readAsset Attempted to read a non-readable assetStream");
			assertion(0);
			return false;
		}
		return readNextAsset(asset);
	}

	/*!*********************************************************************************************************************
	\brief Return true if assets are left to read. Implement this function in your AssetReader.
	***********************************************************************************************************************/
	virtual bool hasAssetsLeftToLoad() = 0;

	/*!*********************************************************************************************************************
	\brief Return true if this AssetReader supports multiple assets. Implement this function in your AssetReader.
	***********************************************************************************************************************/
	virtual bool canHaveMultipleAssets() { return false; }

	/*!*********************************************************************************************************************
	\brief Return a list of supported file extensions. Implement this function in your AssetReader.
	***********************************************************************************************************************/
	virtual std::vector<std::string>  getSupportedFileExtensions() = 0;

	/*!*********************************************************************************************************************
	\brief Read the asset in a new Asset wrapped in a Framework smart handle.
	***********************************************************************************************************************/
	AssetHandle getAssetHandle() { return AssetType::createWithReader(*this); }
protected:
	Stream::ptr_type m_assetStream;
	bool m_hasNewAssetStream;
private:
	/*!*********************************************************************************************************************
	\param Implement this function in your class. Provides the main functionality of reading assets.
	***********************************************************************************************************************/
	virtual bool readNextAsset(AssetType& asset) = 0;
};
}
}
