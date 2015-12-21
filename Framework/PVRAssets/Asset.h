/*!*********************************************************************************************************************
\file         PVRAssets/Asset.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Definition of an Asset class template, with common functionality to interoperate with the AssetReader class templates.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetReader.h"
#include "PVRCore/RefCounted.h"

namespace pvr {
namespace assets {
/*!*********************************************************************************************************************
\brief An Asset represents an object that can be stored and loaded. Models, Textures, Effects and similar all inherit from Asset.
       The class provides a Handle typedef for convenience, which is a reference-counted PVR Framework object type.
	   Provides convenience functions for loading assets with assetReaders.
***********************************************************************************************************************/
template<typename AssetType_> class Asset
{
	typedef RefCountedResource<AssetType_> Handle;
public:

	/*!*********************************************************************************************************************
	\brief Create asset with reader.
	\param[in] reader An AssetReader of the correct type. Must have a valid Stream opened.
	\return A handle to the new Asset. Will be null if failed to load.
	***********************************************************************************************************************/
	static Handle createWithReader(assets::AssetReader<AssetType_>& reader)
	{
		Handle handle;
		handle.construct();
		if (!reader.readAsset(*handle))
		{
			handle.reset();
		}
		return handle;
	}

	/*!*********************************************************************************************************************
	\brief Create an asset with an AssetReader, and wrap it into a Handle (reference counted object). Rvalue-ref overload.
	\param[in] reader An (rvalue-ref) AssetReader of the correct type. Must have a valid Stream opened.
	\return A handle of the new Asset. Will be null if failed to load.
	***********************************************************************************************************************/
	static Handle createWithReader(assets::AssetReader<AssetType_>&& reader)
	{
		Handle handle;
		handle.construct();
		if (!reader.readAsset(*handle))
		{
			handle.reset();
		}
		return handle;
	}

	/*!*********************************************************************************************************************
	\brief 	Load the data of this asset from an AssetReader. This function requires an already constructed object, so it is
			commonly used to reuse an asset.
	\param[in] reader An AssetReader that should already owned and opened a valid AssetStream containing the data of this object
	\return True on success, false if failed to load
	***********************************************************************************************************************/
	bool loadWithReader(assets::AssetReader<AssetType_>& reader)
	{
		return reader.readAsset(static_cast<AssetType_&>(*this));
	}

	/*!*********************************************************************************************************************
	\brief 	Load the data of this asset from an AssetReader. This function requires an already constructed object, so it is
			commonly used to reuse an asset.
	\param[in] reader An AssetReader instance. Can be empty (without a stream).
	\param[in] stream A stream that contains the data to load into this asset.
	\return True on success, false if failed to load
	***********************************************************************************************************************/
	bool loadWithReader(assets::AssetReader<AssetType_>& reader, Stream::ptr_type stream)
	{
		reader.openAssetStream(stream);
		return reader.readAsset(static_cast<AssetType_&>(*this));
	}
};
}
}
