/*!
\brief Contains OpenGL ES specific Helper utilities. Use only if directly using the underlying API's.
\file PVRNativeApi/OGLES/TextureUtilsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRCore/Interfaces/IGraphicsContext.h"
#include "PVRCore/PixelFormat.h"

namespace pvr {
namespace nativeGles {
struct TextureUploadResults
{
	/// <summary>The dimensions of the texture created</summary>
	types::ImageAreaSize textureSize;
	/// <summary>A native texture handle where the texture was uploaded</summary>
	native::HTexture_ image;
	/// <summary>The format of the created texture</summary>
	PixelFormat format;

	GLsync fenceSync;
	/// <summary>Will be set to 'true' if the file was of an uncompressed format unsupported by the
	/// platform, and it was (software) decompressed to a supported uncompressed format</summary>
	bool isDecompressed;
	Result result;
};

/// <summary>Upload a texture to the GPU on the current context, and retrieve the into native handle.</summary>
/// <param name="context">The PlatformContext to use to upload the texture. This will only be used for queries.</param>
/// <param name="texture">The pvr::assets::texture to upload to the GPU</param>
/// <param name="allowDecompress">Set to true to allow to attempt to de-compress unsupported compressed textures.
/// The textures will be decompressed if ALL of the following are true: The texture is in a compressed format that
/// can be decompressed by the framework (PVRTC), the platform does NOT support this format (if it is hardware
/// supported, it will never be decompressed), and this flag is set to true. Default:true.</param>
/// <returns>A TextureUploadResults object containing the uploaded texture and all necessary information (size, formats,
/// whether it was actually decompressed, a sync object to wait on. The "result" field will contain Result::Success
/// on success, errorcode otherwise. See the Texture</returns>
TextureUploadResults textureUpload(IPlatformContext& context, const Texture& texture, bool allowDecompress = true);
}
}