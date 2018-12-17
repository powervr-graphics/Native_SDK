#pragma once
#include "PVRAssets/PVRAssets.h"
namespace pvr {
namespace assets {
class Model;
class GltfReader : public AssetReader<Model>
{
public:
	/// <summary>Construct empty reader.</summary>
	GltfReader(IAssetProvider& assetProvider);

	/// <summary>Construct reader from the specified stream.</summary>
	/// <param name="assetStream">The stream to read from</param>
	GltfReader(Stream::ptr_type assetStream, IAssetProvider& assetProvider);

private:
	void readAsset_(Model& asset);
	size_t _fileNameBeginPos;
	IAssetProvider& _assetProvider;
};

inline GltfReader::GltfReader(IAssetProvider& assetProvider) : _assetProvider(assetProvider) {}

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
