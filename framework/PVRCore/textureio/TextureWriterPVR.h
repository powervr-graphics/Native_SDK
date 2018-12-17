/*!
\brief An experimental Writer that writes pvr::asset::Texture objects into a PVR file.
\file PVRAssets/FileIO/TextureWriterPVR.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/texture/Texture.h"
#include "PVRCore/stream/AssetWriter.h"
#include "PVRCore/textureio/FileDefinesPVR.h"

//!\cond NO_DOXYGEN
namespace pvr {
namespace assetWriters {
/// <summary>An experimental Writer that writes pvr::asset::Texture objects into a PVR file.</summary>
class TextureWriterPVR : public AssetWriter<Texture>
{
public:
	virtual void writeAsset(const Texture& asset);

	virtual bool canWriteAsset(const Texture& asset);
	virtual std::vector<std::string> getSupportedFileExtensions();

private:
};
} // namespace assetWriters
} // namespace pvr
//!\endcond
