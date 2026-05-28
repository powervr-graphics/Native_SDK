/*!
\brief Functionality for loading a texture from disk or other sources
\file PVRCore/texture/TextureLoad.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/textureio/TextureReaderPVR.h"
#include "PVRCore/textureio/TextureReaderBMP.h"
#include "PVRCore/textureio/TextureReaderKTX.h"
#include "PVRCore/textureio/TextureReaderDDS.h"
#include "PVRCore/textureio/TextureReaderXNB.h"
#include "PVRCore/textureio/TextureReaderTGA.h"

namespace pvr {

/// <summary>Load a texture from binary data. Synchronous.</summary>
/// <param name="textureStream">A stream from which to load the binary data</param>
/// <param name="type">The type of the texture. Several supported formats.</param>
/// <returns>Returns a successfully created pvr::Texture object otherwise will throw</returns>
inline Texture textureLoad(const Stream& textureStream, TextureFileFormat type)
{
	switch (type)
	{
	case TextureFileFormat::KTX: return assetReaders::readKTX(textureStream);
	case TextureFileFormat::PVR: return assetReaders::readPVR(textureStream);
	case TextureFileFormat::TGA: return assetReaders::readTGA(textureStream);
	case TextureFileFormat::BMP: return assetReaders::readBMP(textureStream);
	case TextureFileFormat::DDS: return assetReaders::readDDS(textureStream);
	default: throw InvalidArgumentError("type", "Unknown texture file format passed");
	}
}

/// <summary>Load a texture from binary data. Synchronous.</summary>
/// <param name="textureStream">A stream from which to load the binary data</param>
/// <returns>True if successful, otherwise false</returns>
inline Texture textureLoad(const Stream& textureStream) { return textureLoad(textureStream, getTextureFormatFromFilename(textureStream.getFileName().c_str())); }

/// <summary>Read the texture header information from an image, not loading its contents.</summary>
/// <param name="textureStream">A stream from which to load the binary data</param>
/// <param name="type">The type of the texture. Several supported formats.</param>
/// <param name="textureHeader">TextureHeader struct to fill with information.</param>
inline void readImageTextureHeader(const Stream& textureStream, TextureFileFormat type, TextureHeader& textureHeader)
{
	switch (type)
	{
	case TextureFileFormat::KTX: throw InvalidArgumentError("type", "Format not implemented");
	case TextureFileFormat::PVR:
	{
		assetReaders::readPVRTextureHeader(textureStream, textureHeader);
		break;
	}
	case TextureFileFormat::TGA: throw InvalidArgumentError("type", "Format not implemented");
	case TextureFileFormat::BMP: throw InvalidArgumentError("type", "Format not implemented");
	case TextureFileFormat::DDS: throw InvalidArgumentError("type", "Format not implemented");
	default: throw InvalidArgumentError("type", "Unknown texture file format passed");
	}
}

} // namespace pvr
