/*!*********************************************************************************************************************
\file         PVRAssets/AssetWriter.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the AssetWriter class.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace assets {

/*!*********************************************************************************************************************
\brief Base class for Classes that can be used to write back effects to disk.
***********************************************************************************************************************/
template <typename AssetType>
class AssetWriter
{
public:
	/*!*********************************************************************************************************************
	\brief Default constructor.
	***********************************************************************************************************************/
	AssetWriter() : m_assetStream(NULL) {}

public:
	/*!*********************************************************************************************************************
	\brief Open an asset stream for writing. Implement this in your concrete AssetWriter implementation.
	\param assetStream The stream to write to.
	\return return Result::Success if successful
	***********************************************************************************************************************/
	bool openAssetStream(Stream::ptr_type assetStream)
	{
		if (m_assetStream.get())
		{
			(*m_assetStream).close();
		}

		m_assetStream = assetStream;

		if (!m_assetStream.get() || !(*m_assetStream).isWritable())
		{
			return false;
		}
		return (*m_assetStream).open();
	}

	/*!*********************************************************************************************************************
	\brief Add another asset to write. Implement this in your concrete AssetWriter implementation.
	\param asset asset to write
	\return return Result::Success on success
	***********************************************************************************************************************/
	virtual bool addAssetToWrite(const AssetType& asset) = 0;

	/*!*********************************************************************************************************************
	\brief Write out all assets to the Stream. Implement this in your concrete AssetWriter implementation.
	\return return Result::Success on success.
	***********************************************************************************************************************/
	virtual bool writeAllAssets() = 0;

	/*!*********************************************************************************************************************
	\brief Get the number of assets that have been added for writing.  Implement this in your concrete AssetWriter implementation.
	\return The number of assets that have been added for writing.
	***********************************************************************************************************************/
	virtual uint32 assetsAddedSoFar() = 0;

	/*!*********************************************************************************************************************
	\brief Query if this writer supports multiple assets for writing.  Implement this in your concrete AssetWriter implementation.
	\return return true if multiple assets are supported.
	***********************************************************************************************************************/
	virtual bool supportsMultipleAssets() = 0;

	/*!*********************************************************************************************************************
	\brief Query if this writer can write out the specified asset.  Implement this in your concrete AssetWriter implementation.
	\param asset The asset to check for writing.
	\return True if this asset is supported.
	***********************************************************************************************************************/
	virtual bool canWriteAsset(const AssetType& asset) = 0;

	/*!*********************************************************************************************************************
	\brief Get a list of supported file extensions. Implement this in your concrete AssetWriter implementation.
	\return The list of supported file extensions.
	***********************************************************************************************************************/
	virtual std::vector<string> getSupportedFileExtensions() = 0;

	/*!*********************************************************************************************************************
	\brief Get the name of this writer.  Implement this in your concrete AssetWriter implementation.
	  \return writer name
	***********************************************************************************************************************/
	virtual string getWriterName() = 0;

	/*!*********************************************************************************************************************
	\brief Get the version of this writer.  Implement this in your concrete AssetWriter implementation.
	  \return writer version
	***********************************************************************************************************************/
	virtual string getWriterVersion() = 0;

protected:
	std::auto_ptr<Stream>                 m_assetStream; //!< The Stream that this writer uses
	std::vector<const AssetType*> m_assetsToWrite;  //!< The list of assets to write so far
};
}
}
