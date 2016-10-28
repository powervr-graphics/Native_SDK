/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\TextureUtils.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains function definitions for OpenGL ES Texture Utils.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/PVRCore.h"
#include "PVRCore/FileStream.h"
#include "PVRNativeApi/TextureUtils.h"
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
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/Vulkan/VkErrors.h"
#include "PVRPlatformGlue/PlatformContext.h"
#include "PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h"
#include "PVRNativeApi/Vulkan/ImageUtilsVk.h"
#include "PVRNativeApi/Vulkan/BufferUtilsVk.h"
#include <tuple>
#include <algorithm>

namespace pvr {
using namespace types;
namespace utils {
void decompressPvrtc(const assets::Texture& texture, assets::Texture& cDecompressedTexture)
{
	//Set up the new texture and header.
	assets::TextureHeader cDecompressedHeader(texture);
	// robin: not sure what should happen here. The PVRTGENPIXELID4 macro is used in the old SDK.
	cDecompressedHeader.setPixelFormat(assets::GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);

	cDecompressedHeader.setChannelType(VariableType::UnsignedByteNorm);
	cDecompressedTexture = assets::Texture(cDecompressedHeader);

	//Do decompression, one surface at a time.
	for (uint32 uiMIPLevel = 0; uiMIPLevel < texture.getNumberOfMIPLevels(); ++uiMIPLevel)
	{
		for (uint32 uiArray = 0; uiArray < texture.getNumberOfArrayMembers(); ++uiArray)
		{
			for (uint32 uiFace = 0; uiFace < texture.getNumberOfFaces(); ++uiFace)
			{
				PVRTDecompressPVRTC(texture.getDataPointer(uiMIPLevel, uiArray, uiFace),
				                    (texture.getBitsPerPixel() == 2 ? 1 : 0),
				                    texture.getWidth(uiMIPLevel), texture.getHeight(uiMIPLevel),
				                    cDecompressedTexture.getDataPointer(uiMIPLevel, uiArray, uiFace));
			}
		}
	}
}

namespace {
bool isExtensionSupported(platform::PlatformContext& context, const std::string& extension)
{
	return false;
}
}

Result textureUpload(IPlatformContext& context, const assets::Texture& texture, native::HTexture_ & outTextureName,
                     types::ImageAreaSize& outTextureSize, PixelFormat& outFormat,
                     bool& isDecompressed, bool allowDecompress/*=true*/)
{
	platform::PlatformContext& contextVkGlue = static_cast<platform::PlatformContext&>(context);
	using namespace assets;

	// Check that the texture is valid.
	if (!texture.getDataSize())
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs.\n");
		return Result::UnsupportedRequest;
	}

	// Setup code to get various state
	// Generic error strings for textures being unsupported.
	const char8* cszUnsupportedFormat =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation.\n";

	const char8* cszUnsupportedFormatDecompressionAvailable =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation."
	  " Allowing software decompression (allowDecompress=true) will enable you to use this format.\n";

	VkFormat format = VK_FORMAT_UNDEFINED;

	//Texture to use if we decompress in software.
	assets::Texture decompressedTexture;

	// Texture pointer which points at the texture we should use for the function.
	// Allows switching to, for example, a decompressed version of the texture.
	const assets::Texture* textureToUse = &texture;

	// Check that extension support exists for formats supported in this way.
	{
		// Check format not supportedfor formats only supported by extensions.
		switch (texture.getPixelFormat().getPixelTypeId())
		{
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGB:
		case (uint64)CompressedPixelFormat::PVRTCI_2bpp_RGBA:
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGB:
		case (uint64)CompressedPixelFormat::PVRTCI_4bpp_RGBA:
		{
			bool decompress = !contextVkGlue.getNativePlatformHandles().platformInfo.supportPvrtcImage;
			if (decompress)
			{
				if (allowDecompress)
				{
					Log(Log.Information, "PVRTC texture format support not detected. Decompressing PVRTC to"
					    " corresponding format (RGBA32 or RGB24)");
					decompressPvrtc(texture, decompressedTexture);
					textureToUse = &decompressedTexture;
					isDecompressed = true;
				}
				else
				{
					Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "PVRTC1");
					return Result::UnsupportedRequest;
				}
			}
			break;
		}
		case (uint64)CompressedPixelFormat::PVRTCII_2bpp:
		case (uint64)CompressedPixelFormat::PVRTCII_4bpp:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(contextVkGlue, "GL_IMG_texture_compression_pvrtc2"))
			{
				Log(Log.Error, cszUnsupportedFormat, "PVRTC2");
				return Result::UnsupportedRequest;
			}
			break;
		}
		case (uint64)CompressedPixelFormat::ETC1:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(contextVkGlue, "GL_OES_compressed_ETC1_RGB8_texture"))
			{
				Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "ETC1");
				return Result::UnsupportedRequest;
			}
			break;
		}
		case (uint64)CompressedPixelFormat::DXT1: Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "DXT1");
			return Result::UnsupportedRequest;
		case (uint64)CompressedPixelFormat::DXT3: Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "DXT1");
			return Result::UnsupportedRequest;
		case (uint64)CompressedPixelFormat::DXT5: Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "DXT3");
			return Result::UnsupportedRequest;
		default:
		{}
		}
	}

	// Check that the format is a valid format for this API - Doesn't check specifically between OpenGL/ES,
	// it simply gets the values that would be set for a KTX file.
	if ((format = api::ConvertToVk::pixelFormat(textureToUse->getPixelFormat(), textureToUse->getColorSpace(),
	              textureToUse->getChannelType())) == VK_FORMAT_UNDEFINED)
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Texture's pixel type is not supported by this API.\n");
		return Result::UnsupportedRequest;
	}
	outFormat = textureToUse->getPixelFormat();
	outTextureSize = textureToUse->getTotalDimensions();

	uint32 texWidth = textureToUse->getWidth(), texHeight = textureToUse->getHeight(), texDepth = textureToUse->getDepth(),
	       texMipLevels = textureToUse->getNumberOfMIPLevels(), texArraySlices = textureToUse->getNumberOfArrayMembers(),
	       texFaces = textureToUse->getNumberOfFaces();

	// create the out image and prepear for trasfer operation
	if (!vulkan::createImageAndMemory(context, Extent3D(texWidth, texHeight, texDepth), texArraySlices,
	                                  VK_SAMPLE_COUNT_1_BIT, texMipLevels, true, textureToUse->getNumberOfFaces() > 1,
	                                  texDepth > 1 ? VK_IMAGE_TYPE_3D : texHeight > 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D,
	                                  format,
	                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outTextureName))
	{
		Log("Failed to create the Image");
		return Result::UnknownError;
	}

	//Create a bunch of buffers that will be used as copy destinations - each will be one mip level, one array slice / one face
	//Faces are considered array elements, so each Framework array slice in a cube array will be 6 vulkan array slices.

	//Edit the info to be the small, linear images that we are using.
	VkCommandBuffer cb;
	VkCommandBufferAllocateInfo cbnfo = {};
	cbnfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbnfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbnfo.commandBufferCount = 1;
	cbnfo.commandPool = context.getNativePlatformHandles().commandPool;
	vk::AllocateCommandBuffers(context.getNativePlatformHandles().context.device, &cbnfo, &cb);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vk::BeginCommandBuffer(cb, &beginInfo);
	using utils::vulkan::ImageUpdateParam;
	std::vector<ImageUpdateParam> imageUpdates(texMipLevels * texArraySlices * texFaces);
	uint32 imageUpdateIndex = 0;
	for (uint32 mipLevel = 0; mipLevel < texMipLevels; ++mipLevel)
	{
		texWidth = textureToUse->getWidth(mipLevel);
		texHeight = textureToUse->getHeight(mipLevel);
		texDepth = textureToUse->getDepth(mipLevel);
		for (uint32 arraySlice = 0; arraySlice < texArraySlices; ++arraySlice)
		{
			for (uint32 face = 0; face < texFaces; ++face)
			{
				ImageUpdateParam& update = imageUpdates[imageUpdateIndex];
				update.width = texWidth;
				update.height = texHeight;
				update.depth = texDepth;
				update.arrayIndex = arraySlice;
				update.cubeFace = face;
				update.mipLevel = mipLevel;
				update.data = textureToUse->getDataPointer(mipLevel, arraySlice, face);
				update.dataSize = textureToUse->getDataSize(mipLevel, false, false);
				++imageUpdateIndex;
			}// next face
		}// next arrayslice
	}// next miplevel

	vulkan::updateImage(context, imageUpdates.data(), (uint32)imageUpdates.size(), texArraySlices, format, texFaces > 1, outTextureName.image);
	utils::vulkan::setImageLayout(cb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, outTextureName.image,
	                              VK_IMAGE_ASPECT_COLOR_BIT, 0, texMipLevels, 0, texArraySlices);

	vkThrowIfFailed(vk::EndCommandBuffer(cb), "TextureUtils:TextureUpload End command buffer for the image copy ops");

	VkSubmitInfo sSubmitInfo = {};
	sSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sSubmitInfo.pNext = NULL;
	sSubmitInfo.waitSemaphoreCount = 0;
	sSubmitInfo.commandBufferCount = 1;
	sSubmitInfo.pCommandBuffers = &cb;

	// create the fence
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fenceWait = VK_NULL_HANDLE;
	vkThrowIfFailed(vk::CreateFence(contextVkGlue.getNativePlatformHandles().context.device, &fenceInfo, NULL, &fenceWait),
	                "Failed to create wait fence object");

	vkThrowIfFailed(vk::QueueSubmit(contextVkGlue.getNativePlatformHandles().graphicsQueue, 1, &sSubmitInfo, fenceWait),
	                "TextureUtils:TextureUpload Image copy Queue Submission fail");

	vk::WaitForFences(contextVkGlue.getNativePlatformHandles().context.device, 1, &fenceWait, VK_TRUE, uint32(-1));

	if (vk::GetFenceStatus(contextVkGlue.getNativePlatformHandles().context.device, fenceWait) == VK_SUCCESS)
	{
		vk::FreeCommandBuffers(contextVkGlue.getNativePlatformHandles().context.device,
		                       contextVkGlue.getNativePlatformHandles().commandPool, 1, &cb);

		vk::DestroyFence(contextVkGlue.getNativePlatformHandles().context.device, fenceWait, NULL);
	}

	return Result::Success;
}

}// namespace utils
}// namespace pvr
//!\endcond
