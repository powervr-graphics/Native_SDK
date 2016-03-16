/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\TextureUtilsVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains function definitions for OpenGL ES Texture Utils.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/PVRCore.h"
#include "PVRCore/FileStream.h"
#include "PVRApi/TextureUtils.h"
#include "PVRNativeApi/TextureUtils.h"
#include "PVRApi/Vulkan/TextureVk.h"
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
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include <algorithm>

namespace pvr {
namespace utils {

PixelFormat impl::unused::unused1;
bool impl::unused::unused2;

using namespace ::pvr::types;
using namespace ::pvr::api;

//Result::Enum textureUpload(GraphicsContext& context, const assets::Texture& texture, api::TextureView& outTextureView)
//{
//	textureUpload(context, texture, outTextureView, false);
//}

Result::Enum textureUpload(GraphicsContext& context, const assets::Texture& texture, api::TextureView& outTextureView, bool allowDecompress,
                           PixelFormat& outDeCompressedFormat, bool& isCompressed)
{
	ImageArea imageArea;
	native::HTexture_ htex;
	Result::Enum result = textureUpload(context->getPlatformContext(), texture, htex, imageArea, outDeCompressedFormat, isCompressed, true);
	if (result == Result::Success)
	{
		api::vulkan::TextureStoreVk tex;
		tex.construct(context, htex, texture.getDimension());
		api::ImageStorageFormat& fmt = tex->getFormat();
		fmt = outDeCompressedFormat;
		fmt.colorSpace = texture.getColorSpace();
		fmt.dataType = texture.getChannelType();
		fmt.numSamples = 1;

		types::SwizzleChannels swizzle;
		if (texture.getPixelFormat().getChannelContent(0) == 'l')
		{
			if (texture.getPixelFormat().getChannelContent(1) == 'a')
			{
				swizzle.r = swizzle.g = swizzle.b = Swizzle::R;
				swizzle.a = Swizzle::G;
			}
			else
			{
				swizzle.r = swizzle.g = swizzle.b = Swizzle::R;
				swizzle.a = Swizzle::One;
			}
		}

		else if (texture.getPixelFormat().getChannelContent(0) == 'a')
		{
			swizzle.r = swizzle.g = swizzle.b = Swizzle::Zero;
			swizzle.a = Swizzle::R;
		}

		tex->setDimensions(imageArea);
		tex->setLayers(imageArea);
		fmt.mipmapLevels = (uint8)tex->layersSize.numMipLevels;
		outTextureView = context->createTextureView(tex, imageArea, swizzle);
		if (outTextureView.isNull())
		{
			result = Result::UnknownError;
		}
	}
	return result;
}

}// namespace utils
}// namespace pvr
//!\endcond