/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\TextureUtilsGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains function definitions for OpenGL ES Texture Utils.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/PVRCore.h"
#include "PVRCore/FileStream.h"
#include "PVRApi/TextureUtils.h"
#include "PVRNativeApi/TextureUtils.h"
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRAssets/Texture/Texture.h"
#include "PVRAssets/FileIO/TextureReaderPVR.h"
#include "PVRAssets/FileIO/TextureWriterPVR.h"
#include "PVRAssets/FileIO/TextureReaderTGA.h"
#include "PVRAssets/FileIO/TextureReaderKTX.h"
#include "PVRAssets/FileIO/TextureReaderDDS.h"
#include "PVRAssets/FileIO/TextureReaderBMP.h"
#include "PVRAssets/FileIO/TextureWriterLegacyPVR.h"
#include "PVRAssets/Texture/PVRTDecompress.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include <algorithm>
namespace pvr {
namespace utils {

namespace impl {
PixelFormat unused::unused1;
bool unused::unused2;
}
Result::Enum textureUpload(GraphicsContext& context, const assets::Texture& texture, api::TextureView& outTexture, bool allowDecompress/*=true*/,
                           PixelFormat& outDeCompressedFormat /*= impl::unused::unused1*/, bool& isCompressed /*= impl::unused::unused2*/)
{
	native::HTexture_ htex;
	types::ImageAreaSize texSize;
	Result::Enum result = textureUpload(context->getPlatformContext(), texture, htex, texSize, outDeCompressedFormat, isCompressed, allowDecompress);
	if (result == Result::Success)
	{
		api::gles::TextureStoreGles texGles;
		texGles.construct(context, htex);
		outTexture.construct(texGles);
		if (outTexture.isNull())
		{
			result = Result::UnknownError;
		}
	}
	return result;
}


}// namespace utils
}// namespace pvr
//!\endcond
