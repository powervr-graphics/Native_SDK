/*!*********************************************************************************************************************
\file         PVRApi\OGLES\TextureUtilsGles.cpp
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
Result textureUpload(GraphicsContext& context, const assets::Texture& texture, api::TextureView& outTexture, bool allowDecompress/*=true*/)
{
	native::HTexture_ htex;
	types::ImageArea imageArea;
	bool isCompressed = false;
	PixelFormat outDeCompressedFormat;
	Result result = textureUpload(context->getPlatformContext(), texture, htex, imageArea, outDeCompressedFormat, isCompressed, allowDecompress);
	if (result == Result::Success)
	{
		api::gles::TextureStoreGles texGles;
		texGles.construct(context, htex);

		api::ImageStorageFormat& fmt = texGles->getFormat();
		fmt = outDeCompressedFormat;
		fmt.colorSpace = texture.getColorSpace();
		fmt.dataType = texture.getChannelType();
		fmt.numSamples = 1;


		texGles->setDimensions(imageArea);
		texGles->setLayers(imageArea);
		fmt.mipmapLevels = (uint8)texGles->getNumMipLevels();

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
