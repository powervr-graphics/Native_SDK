/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\TextureGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains definitions for the OpenGL ES texture implementation methods.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRPlatformGlue/PlatformContext.h"
#include "PVRApi/TextureUtils.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/Vulkan/ImageUtilsVk.h"

namespace { /* dummy */

inline bool createTexture(pvr::platform::ContextVk& context, pvr::uint32 width, pvr::uint32 height, pvr::uint32 depth,
                          const pvr::api::ImageStorageFormat& format, pvr::uint32 arrayLayers,
                          VkImageType imageType, bool isCubeMap, bool isTransient, pvr::native::HTexture_& outTexture)
{
	VkImageCreateInfo nfo;
	memset(&nfo, 0, sizeof(nfo));

	nfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	nfo.pNext = NULL;
	nfo.flags = pvr::uint32(isCubeMap) * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	nfo.imageType = imageType;
	nfo.extent.width = width;
	nfo.extent.height = height;
	nfo.extent.depth = depth;
	nfo.mipLevels = format.mipmapLevels;
	nfo.arrayLayers = arrayLayers;
	nfo.samples = VK_SAMPLE_COUNT_1_BIT;
	bool isCompressed;
	nfo.format = pvr::api::ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);
	nfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	nfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	nfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryPropertyFlagBits memoryPropertyFlagBits;

	if (isTransient)
	{
		nfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		memoryPropertyFlagBits = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
		nfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else
	{
		memoryPropertyFlagBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		nfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	}

	//Create the final image
	pvr::vkThrowIfFailed(vk::CreateImage(context.getDevice(), &nfo, NULL, &outTexture.image),
	                     "TextureUtils:TextureUpload createImage");

	VkMemoryRequirements memReqDst;
	if (!pvr::apiutils::vulkan::allocateImageDeviceMemory(context.getDevice(),
	    context.getPlatformContext().getNativePlatformHandles().deviceMemProperties, memoryPropertyFlagBits,
	    outTexture, &memReqDst))
	{
		return false;
	}
	return true;
}

}

namespace pvr {
using namespace types;
namespace api {
namespace impl {
native::HTexture_& TextureStore_::getNativeObject() { return native_cast(*this); }
const native::HTexture_& TextureStore_::getNativeObject() const { return native_cast(*this); }

TextureDimension::Enum TextureStore_::getDimensions() const
{
	return native_cast(*this).getDimension();
}

bool TextureStore_::isAllocated() const
{
	return native_cast(*this).isAllocated();
}

TextureView_::TextureView_(const TextureStore& texture, const native::HImageView_& view): resource(texture)
{
	static_cast<native::HImageView_&>(native_cast(*this)) = view;
}

const native::HImageView_& TextureView_::getNativeObject()const
{
	return native_cast(*this);
}

native::HImageView_& TextureView_::getNativeObject()
{
	return native_cast(*this);
}

TextureView_::TextureView_(const TextureStore& texture): resource(texture) { }

void TextureStore_::allocate2D(const ImageStorageFormat& format, uint32 width, uint32 height)
{
	createTexture(native_cast(*context), width, height, 1, format, 1, VK_IMAGE_TYPE_2D, false, false, this->getNativeObject());
	this->format = format;
	native_cast(*this).setDimensions(types::ImageExtents(width, height, 1));
	native_cast(*this).setLayers(types::ImageLayersSize(1, format.mipmapLevels));
}

void TextureStore_::allocateTransient(const ImageStorageFormat& format, uint32 width, uint32 height)
{
	createTexture(native_cast(*context), width, height, 1, format, 1, VK_IMAGE_TYPE_2D, false, true, this->getNativeObject());
	this->format = format;
	native_cast(*this).setDimensions(types::ImageExtents(width, height, 1));
	native_cast(*this).setLayers(types::ImageLayersSize(1, format.mipmapLevels));
}

void TextureStore_::allocate3D(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth)
{
	createTexture(native_cast(*context), width, height, depth, format, 1, VK_IMAGE_TYPE_3D, false, false, this->getNativeObject());
}
void TextureStore_::allocate2DArray(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySlices)
{
	createTexture(native_cast(*context), width, height, 1, format, arraySlices, VK_IMAGE_TYPE_2D, false, false, this->getNativeObject());
}

void TextureStore_::allocate2DCube(const ImageStorageFormat& format, uint32 width, uint32 height)
{
	createTexture(native_cast(*context), width, height, 1, format, 1, VK_IMAGE_TYPE_3D, false, false, this->getNativeObject());
}

void TextureStore_::update(const void* data, const ImageDataFormat& format, const TextureArea& area)
{
	//NOT IMPLEMENTED//
	//pseudocode:
	//Create linear images for each layer/miplevel
	//Copy data into them
	//Launch image copies
	//Synchronise
}

TextureStore_::~TextureStore_()
{
	if (isAllocated())
	{
		if (context.isValid())
		{
			native::HTexture_& nativeTex = native_cast(*this);

			VkDevice device = native_cast(*context).getDevice();

			if (nativeTex.image != VK_NULL_HANDLE)
			{
				if (!nativeTex.undeletable)
				{
					vk::DestroyImage(device, nativeTex.image, NULL);
				}
				nativeTex.image = VK_NULL_HANDLE;
			}
			if (nativeTex.memory != VK_NULL_HANDLE)
			{
				if (!nativeTex.undeletable)
				{
					vk::FreeMemory(device, nativeTex.memory, NULL);
				}
				nativeTex.memory = VK_NULL_HANDLE;
			}
			//DUE TO SHARED POINTERS, NO VIEWS EXIST IF THIS IS CALLED.
		}
		else
		{
			Log(Log.Warning, "Texture object was not released up before context destruction");
		}
	}
}
}//namespace impl}

namespace vulkan {
void TextureViewVk_::destroy()
{
	if (!undeletable && handle != VK_NULL_HANDLE)
	{
		VkDevice dev = native_cast(getResource()->getContext()).getDevice();
		vk::DestroyImageView(dev, handle, NULL);
	}
	handle = VK_NULL_HANDLE;
}

TextureViewVk_::TextureViewVk_(const TextureStoreVk& texture, const types::ImageSubresourceRange& range, SwizzleChannels swizzleChannels) : TextureView_(texture)
{
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = getResource()->getNativeObject().image;
	viewCreateInfo.viewType = pvr::api::ConvertToVk::textureDimensionImageView(texture->getDimension());
	viewCreateInfo.format = api::ConvertToVk::pixelFormat(getResource()->getFormat().format,
	                        getResource()->getFormat().colorSpace, getResource()->getFormat().dataType);
	viewCreateInfo.components.r = api::ConvertToVk::swizzle(swizzleChannels.r);
	viewCreateInfo.components.g = api::ConvertToVk::swizzle(swizzleChannels.g);
	viewCreateInfo.components.b = api::ConvertToVk::swizzle(swizzleChannels.b);
	viewCreateInfo.components.a = api::ConvertToVk::swizzle(swizzleChannels.a);

	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel = range.mipLevelOffset;
	viewCreateInfo.subresourceRange.levelCount = range.numMipLevels;
	viewCreateInfo.subresourceRange.baseArrayLayer = range.arrayLayerOffset;
	viewCreateInfo.subresourceRange.layerCount = range.numArrayLevels;
	if (vk::CreateImageView(native_cast(getResource()->getContext()).getDevice(), &viewCreateInfo, NULL, &getNativeObject().handle) != VK_SUCCESS)
	{
		assertion(false, "Failed to create ImageView");
	}
}

}// namespace vulkan
}// namespace api
}// namespace pvr
//!\endcond
