#pragma once
#include "PVRAssets/PVRAssets.h"
namespace pvr {
namespace assets {
class Model;
/// <summary>This class creates pvr::assets::Model object from Streams of GLTF Model data. Use the readAsset method
/// to create Model objects from the data in your stream.</summary>
class GltfReader : public AssetReader<Model>
{
public:
	/// <summary>Construct empty reader.</summary>
	/// <param name="assetProvider">The asset provider to use for working with gltf files</param>
	GltfReader(IAssetProvider& assetProvider);

	/// <summary>Construct reader from the specified stream.</summary>
	/// <param name="assetStream">The stream to read from</param>
	/// <param name="assetProvider">The asset provider to use for working with gltf files</param>
	GltfReader(Stream::ptr_type assetStream, IAssetProvider& assetProvider);

private:
	void readAsset_(Model& asset);
	size_t _fileNameBeginPos;
	IAssetProvider& _assetProvider;
};

inline GltfReader::GltfReader(IAssetProvider& assetProvider) : _assetProvider(assetProvider), _fileNameBeginPos(0) {}

inline GltfReader::GltfReader(Stream::ptr_type assetStream, IAssetProvider& assetProvider) : AssetReader<Model>(std::move(assetStream)), _assetProvider(assetProvider)
{
	const std::string& path = _assetStream->getFileName();
	_fileNameBeginPos = path.find_last_of(
#ifdef _WIN32gltfStream
		'\\'
#else
		'/'
#endif
	);
	if (_fileNameBeginPos != std::string::npos)
	{
		_fileNameBeginPos += 1;
	}
}

} // namespace assets
} // namespace pvr
