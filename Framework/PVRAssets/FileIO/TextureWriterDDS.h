/*!
\brief An experimental Writer that writes pvr::asset::Texture objects into a DDS file.
\file PVRAssets/FileIO/TextureWriterDDS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Texture.h"
#include "PVRCore/IO/AssetWriter.h"
#include "PVRCore/Texture/FileDefinesDDS.h"

//!\cond NO_DOXYGEN
namespace pvr {
namespace assets {
/// <summary>Contains experimental serialization classes whose purpose is to write PVRAssets classes (Texture, Model,
/// Effect etc.) into specified storage/data formats (DDS, pvr, pfx etc.)</summary>
namespace assetWriters {
/// <summary>Experimental DDS Texture writer. For demostration purposes.</summary>
class TextureWriterDDS : public AssetWriter < Texture >
{
public:
	virtual bool writeAllAssets();

	virtual uint32_t assetsAddedSoFar();
	virtual bool supportsMultipleAssets();

	virtual bool canWriteAsset(const Texture& asset);
	virtual std::vector<std::string> getSupportedFileExtensions();
	virtual std::string getWriterName();
	virtual std::string getWriterVersion();
private:
	virtual bool addAssetToWrite(const Texture& asset);
	bool setDirect3DFormatToDDSHeader(texture_dds::D3DFormat d3dFormat, texture_dds::FileHeader& ddsFileHeader);

	bool writeFileHeader(const texture_dds::FileHeader& ddsFileHeader);
	bool writeFileHeaderDX10(const texture_dds::FileHeaderDX10& ddsFileHeaderDX10);
};
}
}
}
//!\endcond