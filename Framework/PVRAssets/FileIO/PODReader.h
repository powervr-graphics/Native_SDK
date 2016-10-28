/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/PODReader.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An AssetReader that reads POD format streams and creates pvr::assets::Model objects out of them.
***********************************************************************************************************************/
#pragma once

#include "PVRAssets/AssetReader.h"

namespace pvr {
namespace assets {
class Model;

/*!*********************************************************************************************************************
\brief    This class creates pvr::assets::Model object from Streams of POD Model data. Use the readAsset method to
         create Model objects from the data in your stream.
***********************************************************************************************************************/
class PODReader : public AssetReader<Model>
{
public:
	/*!******************************************************************************************************************
	\brief    Construct empty reader.
	********************************************************************************************************************/
	PODReader();
	/*!******************************************************************************************************************
	\brief    Construct reader from the specified stream.
	********************************************************************************************************************/
	PODReader(Stream::ptr_type assetStream) : AssetReader<Model>(assetStream) { }

	/*!******************************************************************************************************************
	\brief    Check if there more assets in the stream.
	\return  True if the readAsset() method can be called again to read another asset
	********************************************************************************************************************/
	virtual bool hasAssetsLeftToLoad();

	/*!******************************************************************************************************************
	\brief    Check if this reader supports multiple assets per stream.
	\return  True if this reader supports multiple assets per stream
	********************************************************************************************************************/
	virtual bool canHaveMultipleAssets();

	/*!******************************************************************************************************************
	\brief    Check if this reader supports the particular assetStream.
	\return  True if this reader supports the particular assetStream
	********************************************************************************************************************/
	virtual bool isSupportedFile(Stream& assetStream);

	/*!******************************************************************************************************************
	\brief    Check what are the expected file extensions for files supported by this reader.
	\return  A vector with the expected file extensions for files supported by this reader
	********************************************************************************************************************/
	virtual std::vector<std::string> getSupportedFileExtensions();
private:
	bool readNextAsset(Model& asset);
	static Result getInformation(Stream& stream, std::string* history, std::string* options);

	bool m_modelsToLoad;
};
}
}
