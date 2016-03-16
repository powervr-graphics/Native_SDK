/*!*********************************************************************************************************************
\file         PVRNativeApi\TextureUtils.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains native api Helper utilities for textures.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRAssets/Model/Mesh.h"
#include "PVRAssets/Texture/Texture.h"

namespace pvr {
namespace utils {

/*!*********************************************************************************************************************
\brief Upload a texture to the GPU and retrieve the into native handle.
\param context
\param[in] texture texture to upload
\param[in] allowDecompress allow de-compress before upload the texture
\param[out] outTextureName native texture handle to upload into
\return		Result::Success on success, errorcode otherwise
***********************************************************************************************************************/
Result::Enum textureUpload(IPlatformContext& context, const assets::Texture& texture, native::HTexture_& outTextureName,
                           types::ImageAreaSize& outTextureSize, PixelFormat& outFormat, bool& isDecompressed, bool allowDecompress = true);

}// namespace utils
}// namespace pvr
