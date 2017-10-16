#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/FileIO/TextureReaderPVR.h"
#include "PVRAssets/FileIO/TextureReaderBMP.h"
#include "PVRAssets/FileIO/TextureReaderKTX.h"
#include "PVRAssets/FileIO/TextureReaderDDS.h"
#include "PVRAssets/FileIO/TextureReaderXNB.h"
#include "PVRAssets/FileIO/TextureReaderTGA.h"

namespace pvr {
namespace assets {
inline Result textureLoad(Stream::ptr_type textureStream, TextureFileFormat type, Texture& outTex)
{
	if (!textureStream.get() || !textureStream->open())
	{
		return Result::UnableToOpen;
	}
	Result rslt = Result::Success;

	std::auto_ptr<AssetReader<Texture>> assetRd;
	switch (type)
	{
	case TextureFileFormat::KTX: assetRd.reset(new assetReaders::TextureReaderKTX(textureStream)); break;
	case TextureFileFormat::PVR: assetRd.reset(new assetReaders::TextureReaderPVR(textureStream)); break;
	case TextureFileFormat::TGA: assetRd.reset(new assetReaders::TextureReaderTGA(textureStream)); break;
	case TextureFileFormat::BMP: assetRd.reset(new assetReaders::TextureReaderBMP(textureStream)); break;
	case TextureFileFormat::DDS: assetRd.reset(new assetReaders::TextureReaderDDS(textureStream)); break;
	default: assertion(0); return Result::UnsupportedRequest; break;
	}

	rslt = (assetRd->readAsset(outTex) ? Result::Success : Result::NotFound);
	assetRd->closeAssetStream();
	return rslt;
}
}
}