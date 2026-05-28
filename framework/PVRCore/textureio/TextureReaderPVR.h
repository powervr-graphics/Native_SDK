/*!
\brief An AssetReader that reads pvr::asset::Texture objects from a PVR stream(file). Used extensively in the
PowerVR Framework and examples.
\file PVRCore/textureio/TextureReaderPVR.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/texture/Texture.h"
#include "PVRCore/textureio/FileDefinesPVR.h"
#include "PVRCore/stream/Stream.h"

namespace pvr {
namespace assetReaders {

/// <summary>Creates pvr::Texture object from a Stream containing PVR texture data.</summary>
Texture readPVR(const Stream& stream);

bool isPVR(const Stream& assetStream);

/// <summary>Load a texture from binary data. Synchronous.</summary>
/// <param name="stream">A stream from which to load the texture header data.</param>
/// <param name="textureHeader">Where to write the texture header data.</param>
/// <returns>PVR header version.</returns>
uint32_t readPVRTextureHeader(const Stream& stream, TextureHeader& textureHeader);

} // namespace assetReaders
} // namespace pvr
