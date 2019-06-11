/*!
\brief Definition of an Asset class template, with common functionality to interoperate with the AssetReader class
templates.
\file PVRCore/stream/Asset.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/AssetReader.h"

namespace pvr {
/// <summary>An Asset represents an object that can be stored and loaded. Models, Textures, Effects and similar
/// all inherit from Asset. The class provides a Handle typedef for convenience, which is a reference-counted PVR
/// Framework object type. Provides convenience functions for loading assets with assetReaders.</summary>
template<typename AssetType_>
class Asset
{
	typedef std::shared_ptr<AssetType_> Handle;

public:
	/// <summary>Create asset with reader.</summary>
	/// <param name="reader">An AssetReader of the correct type. Must have a valid Stream opened.</param>
	/// <returns>A handle to the new Asset. Will be null if failed to load.</returns>
	static Handle createWithReader(AssetReader<AssetType_>& reader)
	{
		Handle handle = std::make_shared<AssetType_>();
		reader.readAsset(*handle);
		return handle;
	}

	/// <summary>Create an asset with an AssetReader, and wrap it into a Handle (reference counted object). Rvalue-ref
	/// overload.</summary>
	/// <param name="reader">An (rvalue-ref) AssetReader of the correct type. Must have a valid Stream opened.</param>
	/// <returns>A handle of the new Asset. Will be null if failed to load.</returns>
	static Handle createWithReader(AssetReader<AssetType_>&& reader)
	{
		Handle handle = std::make_shared<AssetType_>();
		reader.readAsset(*handle);
		return handle;
	}

	/// <summary>Load the data of this asset from an AssetReader. This function requires an already constructed object,
	/// so it is commonly used to reuse an asset.</summary>
	/// <param name="reader">An AssetReader that should already owned and opened a valid AssetStream containing the
	/// data of this object</param>
	void loadWithReader(AssetReader<AssetType_>& reader)
	{
		reader.readAsset(static_cast<AssetType_&>(*this));
	}

	/// <summary>Load the data of this asset from an AssetReader. This function requires an already constructed object,
	/// so it is commonly used to reuse an asset.</summary>
	/// <param name="reader">An AssetReader instance. Can be empty (without a stream).</param>
	/// <param name="stream">A stream that contains the data to load into this asset.</param>
	void loadWithReader(AssetReader<AssetType_>& reader, Stream::ptr_type stream)
	{
		reader.openAssetStream(stream);
		reader.readAsset(static_cast<AssetType_&>(*this));
	}
};
} // namespace pvr
