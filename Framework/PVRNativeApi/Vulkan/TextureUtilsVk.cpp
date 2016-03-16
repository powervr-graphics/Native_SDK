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
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRApi/ApiObjects/CommandBuffer.h"
#include "PVRApi/Vulkan/CommandPoolVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/Vulkan/VkErrors.h"
#include "PVRPlatformGlue/PlatformContext.h"
#include "PVRNativeApi/Vulkan/ImageUtilsVk.h"
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
bool isExtensionSupported(system::PlatformContext& context, const std::string& extension)
{
	return false;
}
}



Result::Enum textureUpload(IPlatformContext& context, const assets::Texture& texture, native::HTexture_ & outTextureName, types::ImageAreaSize& outTextureSize, PixelFormat& outFormat,
                           bool& isDecompressed, bool allowDecompress/*=true*/)
{
	system::PlatformContext& contextVkGlue = static_cast<system::PlatformContext&>(context);
	VkDevice device = context.getNativePlatformHandles().context.device;
	using namespace assets;

	// Check that the texture is valid.
	if (!texture.getDataSize())
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs.\n");
		return Result::UnsupportedRequest;
	}

	// Setup code to get various state
	// Generic error strings for textures being unsupported.
	const char8* cszUnsupportedFormat = "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation.\n";
	const char8* cszUnsupportedFormatDecompressionAvailable =
	    "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation. Allowing software decompression (allowDecompress=true) will enable you to use this format.\n";

	VkFormat format = VK_FORMAT_UNDEFINED;

	//Texture to use if we decompress in software.
	assets::Texture decompressedTexture;

	// Texture pointer which points at the texture we should use for the function. Allows switching to, for example, a decompressed version of the texture.
	const assets::Texture* textureToUse = &texture;

	// Check that extension support exists for formats supported in this way.
	{
		// Check format not supportedfor formats only supported by extensions.
		switch (texture.getPixelFormat().getPixelTypeId())
		{
		case CompressedPixelFormat::PVRTCI_2bpp_RGB:
		case CompressedPixelFormat::PVRTCI_2bpp_RGBA:
		case CompressedPixelFormat::PVRTCI_4bpp_RGB:
		case CompressedPixelFormat::PVRTCI_4bpp_RGBA:
		{
			bool decompress = !contextVkGlue.getNativePlatformHandles().supportPvrtcImage;
			if (decompress)
			{
				if (allowDecompress)
				{
					Log(Log.Information, "PVRTC texture format support not detected. Decompressing PVRTC to corresponding format (RGBA32 or RGB24)");
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
		case CompressedPixelFormat::PVRTCII_2bpp:
		case CompressedPixelFormat::PVRTCII_4bpp:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(contextVkGlue, "GL_IMG_texture_compression_pvrtc2"))
			{
				Log(Log.Error, cszUnsupportedFormat, "PVRTC2");
				return Result::UnsupportedRequest;
			}
			break;
		}
		case CompressedPixelFormat::ETC1:
		{
			//useTexStorage = false;
			if (!isExtensionSupported(contextVkGlue, "GL_OES_compressed_ETC1_RGB8_texture"))
			{
				Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "ETC1");
				return Result::UnsupportedRequest;
			}
			break;
		}
		case CompressedPixelFormat::DXT1: Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "DXT1"); return Result::UnsupportedRequest;
		case CompressedPixelFormat::DXT3: Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "DXT1"); return Result::UnsupportedRequest;
		case CompressedPixelFormat::DXT5: Log(Log.Error, cszUnsupportedFormatDecompressionAvailable, "DXT3"); return Result::UnsupportedRequest;
		default:
		{}
		}
	}

	// Check that the format is a valid format for this API - Doesn't check specifically between OpenGL/ES, it simply gets the values that would be set for a KTX file.
	if ((format = api::ConvertToVk::pixelFormat(textureToUse->getPixelFormat(), textureToUse->getColorSpace(), textureToUse->getChannelType())) == VK_FORMAT_UNDEFINED)
	{
		Log(Log.Error, "TextureUtils.h:textureUpload:: Texture's pixel type is not supported by this API.\n");
		return Result::UnsupportedRequest;
	}
	outFormat = textureToUse->getPixelFormat();
	outTextureSize = textureToUse->getTotalDimensions();

	uint32 texWidth = textureToUse->getWidth(), texHeight = textureToUse->getHeight(), texDepth = textureToUse->getDepth(),
	       texMipLevels = textureToUse->getNumberOfMIPLevels(), texArraySlices = textureToUse->getNumberOfArrayMembers(),
	       texFaces = textureToUse->getNumberOfFaces();

	VkImageCreateInfo nfo;
	memset(&nfo, 0, sizeof(nfo));

	nfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	nfo.pNext = NULL;
	nfo.flags = ((textureToUse->getNumberOfFaces() > 1) * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	nfo.imageType = texDepth > 1 ? VK_IMAGE_TYPE_3D : texHeight > 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D;
	nfo.extent.width = texWidth;
	nfo.extent.height = texHeight;
	nfo.extent.depth = texDepth;
	nfo.mipLevels = texMipLevels;
	nfo.arrayLayers = texArraySlices;
	nfo.samples = VK_SAMPLE_COUNT_1_BIT;
	nfo.format = format;
	nfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	nfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	nfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	nfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	//Create the final image
	vkThrowIfFailed(vk::CreateImage(device, &nfo, NULL, &outTextureName.image), "TextureUtils:TextureUpload createImage");

	//Get what memory it needs, allocate it and bind it.
	VkMemoryRequirements memReqDst;

	if (!apiutils::vulkan::allocateImageDeviceMemory(contextVkGlue.getNativePlatformHandles().context.device,
	        contextVkGlue.getNativePlatformHandles().deviceMemProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	        outTextureName, &memReqDst))
	{
		return Result::UnknownError;
	}


	//Create a bunch of linear images that will be used as copy destinations - each will be one mip level, one array slice / one face
	//Faces are considered array elements, so each Framework array slice in a cube array will be 6 vulkan array slices.

	//Edit the info to be the small, linear images that we are using.
	nfo.tiling = (format == VK_FORMAT_RGBA_PVRTC1_4BPP_BLOCK || format == VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK) ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;
	nfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	nfo.arrayLayers = 1;
	nfo.mipLevels = 1;

	VkCommandBuffer cb;
	VkCommandBufferAllocateInfo cbnfo = {};
	cbnfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbnfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbnfo.commandBufferCount = 1;
	cbnfo.commandPool = contextVkGlue.getNativePlatformHandles().commandPool;
	vk::AllocateCommandBuffers(contextVkGlue.getNativePlatformHandles().context.device, &cbnfo, &cb);
	VkCommandBufferBeginInfo beginInfo;
	memset(&beginInfo, 0, sizeof(VkCommandBufferBeginInfo));
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vk::BeginCommandBuffer(cb, &beginInfo);
	std::vector<native::HTexture_> tmpImgs;
	{
		tmpImgs.resize(texMipLevels * texArraySlices * texFaces);
		for (uint32 mipLevel = 0; mipLevel < texMipLevels; ++mipLevel)
		{
			texWidth = textureToUse->getWidth(mipLevel);
			texHeight = textureToUse->getHeight(mipLevel);
			texDepth = textureToUse->getDepth(mipLevel);

			for (uint32 arraySlice = 0; arraySlice < texArraySlices; ++arraySlice)
			{
				for (uint32 face = 0; face < texFaces; ++face)
				{
					VkImageCopy imgcp;
					uint32 hwSlice = arraySlice * texFaces + face;
					uint32 index = hwSlice + mipLevel * texArraySlices * texFaces;

					imgcp.extent.width = nfo.extent.width = texWidth;
					imgcp.extent.height = nfo.extent.height = texHeight;
					imgcp.extent.depth = nfo.extent.depth = texDepth;
					imgcp.srcOffset.x = imgcp.srcOffset.y = imgcp.srcOffset.z = 0;
					imgcp.dstOffset = imgcp.srcOffset;

					imgcp.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imgcp.srcSubresource.baseArrayLayer = 0;
					imgcp.srcSubresource.layerCount = 1;
					imgcp.srcSubresource.mipLevel = 0;

					imgcp.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imgcp.dstSubresource.baseArrayLayer = hwSlice;
					imgcp.dstSubresource.layerCount = 1;
					imgcp.dstSubresource.mipLevel = mipLevel;

					VkResult res = vk::CreateImage(device, &nfo, NULL, &tmpImgs[index].image);
					if (res != VK_SUCCESS)
					{
						//Just to avoid all the string ops in case the optimizer cannot remove them on success
						vkThrowIfFailed(res , strings::createFormatted("TextureUtils:TextureUpload Linear Temp Image CreateImage MipLevel:[%d]  ArraySlice:[%d] Face[%d]", mipLevel, arraySlice, face).c_str());
					}

					VkMemoryRequirements memReqSrc;
					if (!pvr::apiutils::vulkan::allocateImageDeviceMemory(contextVkGlue.getNativePlatformHandles().context.device,
					        contextVkGlue.getNativePlatformHandles().deviceMemProperties,
					        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, tmpImgs[index], &memReqSrc))
					{
						return Result::UnknownError;
					}
					VkSubresourceLayout layout;
					VkImageSubresource sres; sres.arrayLayer = hwSlice; sres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					sres.mipLevel = mipLevel;
					vk::GetImageSubresourceLayout(device, tmpImgs[index].image, &sres, &layout);
					uint8* mappedData;
					const uint8* srcData;
					uint32 srcDataSize;
					srcData = textureToUse->getDataPointer(mipLevel, arraySlice, face);
					srcDataSize = textureToUse->getDataSize(mipLevel, false, false);
					vkThrowIfFailed(vk::MapMemory(device, tmpImgs[index].memory, 0, memReqSrc.size, 0, (void**)&mappedData), "TextureUtils:TextureUpload Linear Temp Image Map Memory");
					for (uint32_t slice3d = 0; !slice3d || (slice3d < textureToUse->getDepth()); ++slice3d)
					{
						memcpy(mappedData, srcData, srcDataSize);
						mappedData += layout.depthPitch;
						srcData += srcDataSize;
					}

					vk::UnmapMemory(device, tmpImgs[index].memory);
					vk::CmdCopyImage(cb, tmpImgs[index].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, outTextureName.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgcp);
				}
			}
		}
	}
	vkThrowIfFailed(vk::EndCommandBuffer(cb), "TextureUtils:TextureUpload End command buffer for the image copy ops");

	VkSubmitInfo sSubmitInfo = {};

	sSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sSubmitInfo.pNext = NULL;
	sSubmitInfo.waitSemaphoreCount = 0;
	sSubmitInfo.commandBufferCount = 1;
	sSubmitInfo.pCommandBuffers = &cb;

	vkThrowIfFailed(vk::QueueSubmit(contextVkGlue.getNativePlatformHandles().graphicsQueue, 1, &sSubmitInfo, VK_NULL_HANDLE),
	                "TextureUtils:TextureUpload Image copy ops CreateFence");
	vk::QueueWaitIdle(contextVkGlue.getNativePlatformHandles().graphicsQueue);
	vk::FreeCommandBuffers(contextVkGlue.getNativePlatformHandles().context.device, contextVkGlue.getNativePlatformHandles().commandPool, 1, &cb);

	for (auto it = tmpImgs.begin(); it != tmpImgs.end(); ++it)
	{
		vk::DestroyImage(device, it->image, NULL);
		vk::FreeMemory(device, it->memory, NULL);
	}
	return Result::Success;
}

}// namespace utils
}// namespace pvr
//!\endcond
