/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\TextureVk.cpp
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
namespace pvr {
using namespace types;
namespace api {
namespace impl {
native::HTexture_& TextureStore_::getNativeObject() { return native_cast(*this); }
const native::HTexture_& TextureStore_::getNativeObject() const { return native_cast(*this); }

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

void TextureStore_::allocate2D(const ImageStorageFormat& format, uint32 width, uint32 height,
                               types::ImageUsageFlags usage, types::ImageLayout newLayout)
{
	newLayout = (static_cast<pvr::uint32>(usage & types::ImageUsageFlags::DepthStencilAttachment) != 0 ?
	             types::ImageLayout::DepthStencilAttachmentOptimal : newLayout);

	allocate2DArrayMS(format, width, height, 1, types::SampleCount::Count1, usage, newLayout);
}

void TextureStore_::allocate2DArray(const ImageStorageFormat& format, uint32 width, uint32 height,
                                    uint32 arraySlices, types::ImageUsageFlags usage,
                                    types::ImageLayout newLayout)
{
	allocate2DArrayMS(format, width, height, arraySlices, types::SampleCount::Count1, usage, newLayout);
}

void TextureStore_::allocate2DMS(const ImageStorageFormat& format, uint32 width, uint32 height,
                                 SampleCount samples, ImageUsageFlags usage,
                                 ImageLayout newLayout)
{
	allocate2DArrayMS(format, width, height, 1, samples, usage, newLayout);
}

void TextureStore_::allocate2DArrayMS(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 arraySize,
                                      types::SampleCount samples, types::ImageUsageFlags usage,
                                      types::ImageLayout newLayout)
{
	bool isCompressed ;
	VkFormat vkFormat =  pvr::api::ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	utils::vulkan::createImageAndMemory(context->getPlatformContext(), types::Extent3D(width, height, 1),
	                                    arraySize, ConvertToVk::sampleCount(samples), format.mipmapLevels,
	                                    true, false, VK_IMAGE_TYPE_2D, vkFormat, ConvertToVk::imageUsageFlags(usage),
	                                    ConvertToVk::imageLayout(newLayout), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                                    this->getNativeObject());
	native_cast(*this).setNumSamples(samples);
	native_cast(*this).setDimensions(types::Extent3D(width, height, 1));
	native_cast(*this).setFormat(format);
	native_cast(*this).setLayers(types::ImageLayersSize((uint16)arraySize, format.mipmapLevels));
}

void TextureStore_::allocateTransient(const ImageStorageFormat& format, uint32 width, uint32 height)
{
	types::ImageUsageFlags usage = types::ImageUsageFlags::ColorAttachment |
	                               types::ImageUsageFlags::InputAttachment |
	                               types::ImageUsageFlags::TransientAttachment;

	types::ImageLayout imageLayout = types::ImageLayout::ColorAttachmentOptimal;

	bool isCompressed ;
	VkFormat vkFormat =  pvr::api::ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	utils::vulkan::createImageAndMemory(context->getPlatformContext(), types::Extent3D(width, height, 1), 1,
	                                    VK_SAMPLE_COUNT_1_BIT, format.mipmapLevels, true, false, VK_IMAGE_TYPE_2D,
	                                    vkFormat, ConvertToVk::imageUsageFlags(usage),
	                                    ConvertToVk::imageLayout(imageLayout), VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
	                                    this->getNativeObject());

	native_cast(*this).setDimensions(types::Extent3D(width, height, 1));
	native_cast(*this).setFormat(format);
	native_cast(*this).setLayers(types::ImageLayersSize(1, format.mipmapLevels));
}

void TextureStore_::allocateStorage(const ImageStorageFormat& format, uint32 width, uint32 height)
{
	types::ImageUsageFlags usage = types::ImageUsageFlags::TransferDest | types::ImageUsageFlags::Storage;

	types::ImageLayout imageLayout = types::ImageLayout::General;

	bool isCompressed ;
	VkFormat vkFormat =  pvr::api::ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	utils::vulkan::createImageAndMemory(context->getPlatformContext(), types::Extent3D(width, height, 1), 1,
	                                    VK_SAMPLE_COUNT_1_BIT, format.mipmapLevels, true, false,
	                                    VK_IMAGE_TYPE_2D, vkFormat, ConvertToVk::imageUsageFlags(usage),
	                                    ConvertToVk::imageLayout(imageLayout),
	                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->getNativeObject());

	native_cast(*this).setDimensions(types::Extent3D(width, height, 1));
	native_cast(*this).setFormat(format);
	native_cast(*this).setLayers(types::ImageLayersSize(1, format.mipmapLevels));

}

void TextureStore_::allocate3D(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth,
                               types::ImageUsageFlags usage, types::ImageLayout newLayout)
{
	bool isCompressed ;
	VkFormat vkFormat =  pvr::api::ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	utils::vulkan::createImageAndMemory(context->getPlatformContext(), types::Extent3D(width, height, depth),
	                                    1, VK_SAMPLE_COUNT_1_BIT, format.mipmapLevels, true, false,
	                                    VK_IMAGE_TYPE_3D, vkFormat, ConvertToVk::imageUsageFlags(usage),
	                                    ConvertToVk::imageLayout(newLayout), VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
	                                    this->getNativeObject());


	native_cast(*this).setFormat(format);
	native_cast(*this).setLayers(types::ImageLayersSize(1, format.mipmapLevels));
}

void TextureStore_::allocate2DCube(const ImageStorageFormat& format, uint32 width, uint32 height,
                                   types::ImageUsageFlags usage, types::ImageLayout newLayout)
{
	bool isCompressed ;
	VkFormat vkFormat =  pvr::api::ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	utils::vulkan::createImageAndMemory(context->getPlatformContext(), types::Extent3D(width, height, 1), 1,
	                                    VK_SAMPLE_COUNT_1_BIT, format.mipmapLevels, true, true, VK_IMAGE_TYPE_2D,
	                                    vkFormat, ConvertToVk::imageUsageFlags(usage), ConvertToVk::imageLayout(newLayout),
	                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->getNativeObject());


	native_cast(*this).setFormat(format);
	native_cast(*this).setLayers(types::ImageLayersSize(1, format.mipmapLevels));
	isCubeMap = true;
}

void TextureStore_::update(const void* data, const ImageDataFormat& format, const TextureArea& area)
{
	if (!isAllocated()) { assertion(false, "Texture must be allocated before updating"); }
	vulkan::TextureStoreVk_& thisVk = native_cast(*this);
	utils::vulkan::ImageUpdateParam imageUpdate;
	Log(Log.Information, "IMAGE UPDATE LEVEL: %d", area.mipLevel);
	imageUpdate.mipLevel = area.mipLevel;
	imageUpdate.data = data;
	imageUpdate.arrayIndex = area.arrayIndex;
	imageUpdate.depth = area.depth;
	imageUpdate.cubeFace = area.cubeFace;
	imageUpdate.width = area.width;
	imageUpdate.height = area.height;
	utils::vulkan::updateImage(getContext().getPlatformContext(), &imageUpdate, 1, thisVk.layersSize.numArrayLevels,
	                           ConvertToVk::pixelFormat(format), isCubeMap, this->getNativeObject().image);
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

TextureViewVk_::TextureViewVk_(const TextureStoreVk& texture, const types::ImageSubresourceRange& range,
                               SwizzleChannels swizzleChannels) : TextureView_(texture)
{
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = getResource()->getNativeObject().image;
	viewCreateInfo.viewType = pvr::api::ConvertToVk::imageBaseTypeToTexViewType(texture->getImageBaseType(),
	                          range.numArrayLevels, texture->is2DCubeMap());

	viewCreateInfo.format = api::ConvertToVk::pixelFormat(getResource()->getFormat().format,
	                        getResource()->getFormat().colorSpace, getResource()->getFormat().dataType);
	viewCreateInfo.components.r = api::ConvertToVk::swizzle(swizzleChannels.r);
	viewCreateInfo.components.g = api::ConvertToVk::swizzle(swizzleChannels.g);
	viewCreateInfo.components.b = api::ConvertToVk::swizzle(swizzleChannels.b);
	viewCreateInfo.components.a = api::ConvertToVk::swizzle(swizzleChannels.a);

	viewCreateInfo.subresourceRange.aspectMask = pvr::api::ConvertToVk::imageAspect(range.aspect);
	viewCreateInfo.subresourceRange.baseMipLevel = range.mipLevelOffset;
	viewCreateInfo.subresourceRange.levelCount = range.numMipLevels;
	viewCreateInfo.subresourceRange.baseArrayLayer = range.arrayLayerOffset;
	viewCreateInfo.subresourceRange.layerCount = range.numArrayLevels;

	if (vk::CreateImageView(native_cast(getResource()->getContext()).getDevice(), &viewCreateInfo, NULL,
	                        &getNativeObject().handle) != VK_SUCCESS)
	{
		assertion(false, "Failed to create ImageView");
	}
}

}// namespace vulkan
}// namespace api
}// namespace pvr
//!\endcond
