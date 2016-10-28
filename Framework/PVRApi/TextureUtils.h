/*!*********************************************************************************************************************
\file         PVRApi/TextureUtils.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains utilities for uploading textures into the API.
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
\brief Upload a texture into the GPU, retrieve a pvr::api::TextureView object.
\param context The GraphicsContext to use
\param[in] texture The texture to upload
\param[in] allowDecompress Allow de-compress a compressed format if the format is not natively supported. If this is set
to true and an unsupported compressed format before uploading the texture, the implementation will uncompress
the texture on the CPU and upload the uncompressed texture. If set to false, the implementation will return
failure in this case.
\param[out] outTextureView The api texture to upload into. Will contain a newly created textureview even if it contained
another one before.
\return			Result::Success on success, errorcode otherwise
***********************************************************************************************************************/

Result textureUpload(GraphicsContext& context, const assets::Texture& texture, api::TextureView& outTextureView, bool allowDecompress = true);

}// namespace utils
}// namespace pvr
