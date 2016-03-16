/*!*********************************************************************************************************************
\file         PVRApi\TextureUtils.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific Helper utilities. Use only if directly using the underlying API's.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRAssets/Model/Mesh.h"
#include "PVRAssets/Texture/Texture.h"

namespace pvr {
namespace utils {

namespace impl {
struct unused
{
	static PixelFormat unused1;
	static bool unused2;
};
}

/*!*********************************************************************************************************************
\brief Upload texture into the GPU, retrieve a pvr::API TextureView object.
\param context The GraphicsContext to use
\param[in] texture The texture to upload
\param[in] allowDecompress Allow de-compress a compressed format if the format is not natively supported. If this is set
to true and an unsupported compressed format before uploading the texture, the implementation will uncompress
the texture on the CPU and upload the uncompressed texture. If set to false, the implementation will return
failure in this case.
\param[out] outTexture The api texture to upload into
\return			Result::Success on success, errorcode otherwise
***********************************************************************************************************************/
//Result::Enum textureUpload(GraphicsContext& context, const assets::Texture& texture, api::TextureView& outTexture, bool allowDecompress = true);

Result::Enum textureUpload(GraphicsContext& context, const assets::Texture& texture, api::TextureView& outTextureView, bool allowDecompress = true,
                           PixelFormat& outDeCompressedFormat = impl::unused::unused1, bool& isCompressed = impl::unused::unused2);

}// namespace utils
}// namespace pvr
