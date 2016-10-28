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
\param context The PlatformContext to use to upload the texture.
\param[in] texture The pvr::assets::texture to upload to the GPU
\param[out] outTextureName The native texture handle to upload the texture to (will be created by this function)
\param[out] outTextureSize The dimensions of the texture created
\param[out] outFormat The format of the created texture
\param[out] isDecompressed Will be set to 'true' if the file was of an uncompressed format unsupported by the platform, 
and it was (software) decompressed to a supported uncompressed format
\param[in] allowDecompress Set to true to allow to attempt to de-compress unsupported compressed textures. The textures
will be decompressed if ALL of the following are true: The texture is in a compressed format that can be decompressed
by the framework (PVRTC), the platform does NOT support this format (if it is hardware supported, it will never be 
decompressed), and this flag is set to true. Default:true.
\return   Result::Success on success, errorcode otherwise
***********************************************************************************************************************/
Result textureUpload(IPlatformContext& context, const assets::Texture& texture, native::HTexture_& outTextureName,
                     types::ImageAreaSize& outTextureSize, PixelFormat& outFormat, bool& isDecompressed, bool allowDecompress = true);

}// namespace utils
}// namespace pvr
