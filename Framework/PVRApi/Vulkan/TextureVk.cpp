<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\TextureVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains definitions for the OpenGL ES texture implementation methods.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Contains definitions for the OpenGL ES texture implementation methods.
\file PVRApi/Vulkan/TextureVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/PlatformContext.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/Vulkan/ImageUtilsVk.h"
<<<<<<< HEAD
namespace pvr {
using namespace types;
namespace api {
namespace impl {
native::HTexture_& TextureStore_::getNativeObject() { return native_cast(*this); }
const native::HTexture_& TextureStore_::getNativeObject() const { return native_cast(*this); }

bool TextureStore_::isAllocated() const
=======

namespace pvr {
using namespace types;
using namespace nativeVk;
namespace api {
// TextureViewVk_
namespace impl {
TextureView_::TextureView_(const TextureStore& texture, const native::HImageView_& view): resource(texture)
{
	static_cast<native::HImageView_&>(native_cast(*this)) = view;
}

TextureView_::TextureView_(const TextureStore& texture): resource(texture) { }

}//namespace impl}

namespace vulkan {
bool TextureStoreVk_::isAllocated_() const { return (image != VK_NULL_HANDLE) && (memory != VK_NULL_HANDLE); }

void TextureStoreVk_::allocate2D_(const ImageStorageFormat& format, uint32 width, uint32 height,
                                  types::ImageUsageFlags usage, types::ImageLayout newLayout)
{
	newLayout = (static_cast<pvr::uint32>(usage & types::ImageUsageFlags::DepthStencilAttachment) != 0 ?
	             types::ImageLayout::DepthStencilAttachmentOptimal : newLayout);

	allocate2DArrayMS_(format, width, height, 1, usage, newLayout);
}

void TextureStoreVk_::allocate2DArray_(const ImageStorageFormat& format, uint32 width, uint32 height,
                                       uint32 arraySlices, types::ImageUsageFlags usage, types::ImageLayout newLayout)
{
	allocate2DArrayMS_(format, width, height, arraySlices, usage, newLayout);
}

void TextureStoreVk_::allocate2DMS_(const ImageStorageFormat& format, uint32 width, uint32 height,
                                    ImageUsageFlags usage, ImageLayout newLayout)
>>>>>>> 1776432f... 4.3
{
	allocate2DArrayMS_(format, width, height, 1, usage, newLayout);
}

inline static VkResult helperCreateAndBeginCommandBuffer(platform::NativePlatformHandles_& handles, VkCommandBuffer& outCbuff)
{
	VkResult res;
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = 1;
	nfo.commandPool = handles.universalCommandPool;
	nfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	nfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	if ((res = vk::AllocateCommandBuffers(handles.context.device, &nfo, &outCbuff)) != VK_SUCCESS)
	{
		return res;
	}

	VkCommandBufferBeginInfo beginnfo = {};
	beginnfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	res = vk::BeginCommandBuffer(outCbuff, &beginnfo);
	return res;
}

inline static VkResult helperEndSubmitWaitAndDestroyCommandBuffer(
  platform::NativePlatformHandles_& handles,
  VkQueue queue, VkCommandBuffer cbuff, VkCommandPool pool)
{
	VkResult res;
	if ((res = vk::EndCommandBuffer(cbuff)) != VK_SUCCESS) { return res; }

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pCommandBuffers = &cbuff;
	submit.commandBufferCount = 1;

	VkFence fence;
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if ((res = vk::CreateFence(handles.context.device, &fenceInfo, NULL, &fence)) != VK_SUCCESS) { return res; }
	if ((res = vk::QueueSubmit(queue, 1, &submit, fence)) != VK_SUCCESS) { return res; }
	if ((res = vk::WaitForFences(handles.context.device, 1, &fence, true, uint64(-1))) != VK_SUCCESS) { return res; }
	vk::DestroyFence(handles.context.device, fence, NULL);
	vk::FreeCommandBuffers(handles.context.device, pool, 1, &cbuff);
	return res;
}

inline void allocate(const GraphicsContext& ctx, native::HTexture_& texture,
                     const ImageStorageFormat& format, uint16 width, uint16 height, uint16 depth,
                     uint16 arrayLayers, bool isCube,
                     types::ImageUsageFlags usage, types::ImageLayout newLayout,
                     VkImageType type, bool isTransient)
{
	bool isCompressed;
	VkFormat vkFormat = ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	auto handles = ctx->getPlatformContext().getNativePlatformHandles();
	utils::vulkan::createImageAndMemory(ctx->getPlatformContext().getNativePlatformHandles(),
	                                    types::Extent3D(width, height, depth),
	                                    1, VK_SAMPLE_COUNT_1_BIT, format.mipmapLevels, false,
	                                    type, vkFormat, ConvertToVk::imageUsageFlags(usage),
	                                    isTransient ? VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
	                                    : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture);
	VkCommandBuffer cbuff;
	helperCreateAndBeginCommandBuffer(handles, cbuff);
	utils::vulkan::setImageLayoutAndQueueOwnership(cbuff, VK_NULL_HANDLE, -1, -1,
	    VK_IMAGE_LAYOUT_UNDEFINED, ConvertToVk::imageLayout(newLayout), texture.image,
	    0, format.mipmapLevels, 0, arrayLayers * (1 + isCube * 5),
	    utils::vulkan::inferAspectFromFormat(format.format));
	helperEndSubmitWaitAndDestroyCommandBuffer(handles, handles.universalQueues[handles.universalQueueIndex], cbuff, handles.universalCommandPool);
}


<<<<<<< HEAD
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
=======

void TextureStoreVk_::allocate2DArrayMS_(const ImageStorageFormat& format, uint32 width, uint32 height,
    uint32 arraySize, types::ImageUsageFlags usage, types::ImageLayout imageLayout)
{
	currentLayout = imageLayout;
	bool isCompressed;
	VkFormat vkFormat = ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	auto& handles = _context->getPlatformContext().getNativePlatformHandles();

	utils::vulkan::createImageAndMemory(_context->getPlatformContext().getNativePlatformHandles(),
	                                    types::Extent3D(width, height, 1), arraySize,
	                                    ConvertToVk::sampleCount(types::SampleCount(format.numSamples)), format.mipmapLevels,
	                                    false, VK_IMAGE_TYPE_2D, vkFormat, ConvertToVk::imageUsageFlags(usage),
	                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                                    *this);

	VkCommandBuffer cbuff;
	helperCreateAndBeginCommandBuffer(handles, cbuff);
	utils::vulkan::setImageLayoutAndQueueOwnership(cbuff, VK_NULL_HANDLE, -1, -1,
	    VK_IMAGE_LAYOUT_UNDEFINED, ConvertToVk::imageLayout(imageLayout),
	    image, 0, 1, 0, 1, utils::vulkan::inferAspectFromFormat(nativeVk::ConvertToVk::pixelFormat(format.format)));
	helperEndSubmitWaitAndDestroyCommandBuffer(handles, handles.universalQueues[handles.universalQueueIndex], cbuff, handles.universalCommandPool);

	setNumSamples(types::SampleCount(format.numSamples));
	setDimensions(types::Extent3D(width, height, 1));
	setFormat(format);
	setLayers(types::ImageLayersSize((uint16)arraySize, format.mipmapLevels));
>>>>>>> 1776432f... 4.3
}

void TextureStoreVk_::allocateTransient_(const ImageStorageFormat& format, uint32 width, uint32 height,
    types::ImageUsageFlags usage, types::ImageLayout imageLayout)
{
<<<<<<< HEAD
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
=======
	currentLayout = imageLayout;
	bool isCompressed;
	VkFormat vkFormat = ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	auto handles = _context->getPlatformContext().getNativePlatformHandles();

	utils::vulkan::createImageAndMemory(_context->getPlatformContext().getNativePlatformHandles(),
	                                    types::Extent3D(width, height, 1), 1,
	                                    VK_SAMPLE_COUNT_1_BIT, format.mipmapLevels, false, VK_IMAGE_TYPE_2D,
	                                    vkFormat, ConvertToVk::imageUsageFlags(usage),
	                                    VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, *this);

	VkCommandBuffer cbuff;
	helperCreateAndBeginCommandBuffer(handles, cbuff);
	utils::vulkan::setImageLayoutAndQueueOwnership(cbuff, VK_NULL_HANDLE, -1, -1,
	    VK_IMAGE_LAYOUT_UNDEFINED, ConvertToVk::imageLayout(imageLayout),
	    image, 0, 1, 0, 1, utils::vulkan::inferAspectFromFormat(format.format));
	helperEndSubmitWaitAndDestroyCommandBuffer(handles, handles.universalQueues[handles.universalQueueIndex], cbuff, handles.universalCommandPool);

	setDimensions(types::Extent3D(width, height, 1));
	setFormat(format);
	setLayers(types::ImageLayersSize(1, format.mipmapLevels));
}

void TextureStoreVk_::allocateStorage_(const ImageStorageFormat& format, uint32 width, uint32 height)
{
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

	bool isCompressed;
	VkFormat vkFormat = ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	auto handles = _context->getPlatformContext().getNativePlatformHandles();

	utils::vulkan::createImageAndMemory(_context->getPlatformContext().getNativePlatformHandles(),
	                                    types::Extent3D(width, height, 1), 1,
	                                    VK_SAMPLE_COUNT_1_BIT, format.mipmapLevels, false,
	                                    VK_IMAGE_TYPE_2D, vkFormat, usage,
	                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *this);

	VkCommandBuffer cbuff;
	helperCreateAndBeginCommandBuffer(handles, cbuff);
	utils::vulkan::setImageLayoutAndQueueOwnership(cbuff, VK_NULL_HANDLE, -1, -1,
	    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, image,
	    0, format.mipmapLevels, 0, 1, utils::vulkan::inferAspectFromFormat(format.format));
	helperEndSubmitWaitAndDestroyCommandBuffer(handles, handles.universalQueues[handles.universalQueueIndex], cbuff, handles.universalCommandPool);

	setDimensions(types::Extent3D(width, height, 1));
	setFormat(format);
	setLayers(types::ImageLayersSize(1, format.mipmapLevels));
}

void TextureStoreVk_::allocate3D_(const ImageStorageFormat& format, uint32 width, uint32 height, uint32 depth,
                                  types::ImageUsageFlags usage, types::ImageLayout imageLayout)
{
	currentLayout = imageLayout;
	bool isCompressed;
	VkFormat vkFormat = ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	auto handles = _context->getPlatformContext().getNativePlatformHandles();
	utils::vulkan::createImageAndMemory(_context->getPlatformContext().getNativePlatformHandles(),
	                                    types::Extent3D(width, height, depth),
	                                    1, VK_SAMPLE_COUNT_1_BIT, format.mipmapLevels, false,
	                                    VK_IMAGE_TYPE_3D, vkFormat, ConvertToVk::imageUsageFlags(usage),
	                                    VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
	                                    *this);
	VkCommandBuffer cbuff;
	helperCreateAndBeginCommandBuffer(handles, cbuff);
	utils::vulkan::setImageLayoutAndQueueOwnership(cbuff, VK_NULL_HANDLE, -1, -1,
	    VK_IMAGE_LAYOUT_UNDEFINED, ConvertToVk::imageLayout(imageLayout), image,
	    0, format.mipmapLevels, 0, 1, utils::vulkan::inferAspectFromFormat(format.format));
	helperEndSubmitWaitAndDestroyCommandBuffer(handles, handles.universalQueues[handles.universalQueueIndex], cbuff, handles.universalCommandPool);

	setFormat(format);
	setLayers(types::ImageLayersSize(1, format.mipmapLevels));
}

void TextureStoreVk_::allocate2DCube_(const ImageStorageFormat& format, uint32 width, uint32 height,
                                      types::ImageUsageFlags usage, types::ImageLayout imageLayout)
{
	currentLayout = imageLayout;
	bool isCompressed;
	VkFormat vkFormat = ConvertToVk::pixelFormat(format.format, format.colorSpace, format.dataType, isCompressed);

	auto handles = _context->getPlatformContext().getNativePlatformHandles();

	utils::vulkan::createImageAndMemory(_context->getPlatformContext().getNativePlatformHandles(),
	                                    types::Extent3D(width, height, 1), 1,
	                                    VK_SAMPLE_COUNT_1_BIT, format.mipmapLevels, true, VK_IMAGE_TYPE_2D,
	                                    vkFormat, ConvertToVk::imageUsageFlags(usage),
	                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *this);
	VkCommandBuffer cbuff;
	helperCreateAndBeginCommandBuffer(handles, cbuff);
	utils::vulkan::setImageLayoutAndQueueOwnership(cbuff, VK_NULL_HANDLE, -1, -1,
	    VK_IMAGE_LAYOUT_UNDEFINED, ConvertToVk::imageLayout(imageLayout), image,
	    0, format.mipmapLevels, 0, 1, utils::vulkan::inferAspectFromFormat(format.format));
	helperEndSubmitWaitAndDestroyCommandBuffer(handles, handles.universalQueues[handles.universalQueueIndex], cbuff, handles.universalCommandPool);

	setFormat(format);
	setLayers(types::ImageLayersSize(1, format.mipmapLevels));
	_isCubeMap = true;
>>>>>>> 1776432f... 4.3
}

void TextureStoreVk_::update_(const void* data, const ImageDataFormat& format, const TextureArea& area)
{
	if (!isAllocated()) { assertion(false, "Texture must be allocated before updating"); }
<<<<<<< HEAD
	vulkan::TextureStoreVk_& thisVk = native_cast(*this);
=======

>>>>>>> 1776432f... 4.3
	utils::vulkan::ImageUpdateParam imageUpdate;
	Log(Log.Information, "IMAGE UPDATE LEVEL: %d", area.mipLevel);
	imageUpdate.mipLevel = area.mipLevel;
	imageUpdate.data = data;
	imageUpdate.arrayIndex = area.arrayIndex;
	imageUpdate.depth = area.depth;
	imageUpdate.cubeFace = area.cubeFace;
	imageUpdate.width = area.width;
	imageUpdate.height = area.height;
<<<<<<< HEAD
	utils::vulkan::updateImage(getContext().getPlatformContext(), &imageUpdate, 1, thisVk.layersSize.numArrayLevels,
	                           ConvertToVk::pixelFormat(format), isCubeMap, this->getNativeObject().image);
=======

	utils::vulkan::updateImage(getContext().getPlatformContext(),
	                           &imageUpdate, 1, _layersSize.numArrayLevels,
	                           ConvertToVk::pixelFormat(format), _isCubeMap, image,
	                           ConvertToVk::imageLayout(currentLayout));
>>>>>>> 1776432f... 4.3
}

TextureStoreVk_::~TextureStoreVk_()
{
	if (isAllocated())
	{
		if (_context.isValid())
		{
			native::HTexture_& nativeTex = native_cast(*this);


			if (!nativeTex.undeletable)
			{
				VkDevice device = native_cast(*_context).getDevice();
				if (nativeTex.image != VK_NULL_HANDLE)
				{

					vk::DestroyImage(device, nativeTex.image, NULL);
					nativeTex.image = VK_NULL_HANDLE;
				}
				if (nativeTex.memory != VK_NULL_HANDLE)
				{
					vk::FreeMemory(device, nativeTex.memory, NULL);
					nativeTex.memory = VK_NULL_HANDLE;
				}
			}
			//DUE TO SHARED POINTERS, NO VIEWS EXIST IF THIS IS CALLED.
		}
		else
		{
			Log(Log.Warning, "Texture object was not released up before context destruction");
		}
	}
}
}


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
	auto& res = static_cast<TextureStoreVk_&>(*getResource());
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
<<<<<<< HEAD
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
=======
	viewCreateInfo.image = res.image;
	viewCreateInfo.viewType = ConvertToVk::imageBaseTypeToTexViewType(texture->getImageBaseType(),
	                          range.numArrayLevels, texture->is2DCubeMap());
	viewCreateInfo.format = ConvertToVk::pixelFormat(res.getFormat().format,
	                        res.getFormat().colorSpace, res.getFormat().dataType);
	viewCreateInfo.components.r = ConvertToVk::swizzle(swizzleChannels.r);
	viewCreateInfo.components.g = ConvertToVk::swizzle(swizzleChannels.g);
	viewCreateInfo.components.b = ConvertToVk::swizzle(swizzleChannels.b);
	viewCreateInfo.components.a = ConvertToVk::swizzle(swizzleChannels.a);

	viewCreateInfo.subresourceRange.aspectMask = ConvertToVk::imageAspect(range.aspect);
>>>>>>> 1776432f... 4.3
	viewCreateInfo.subresourceRange.baseMipLevel = range.mipLevelOffset;
	viewCreateInfo.subresourceRange.levelCount = range.numMipLevels;
	viewCreateInfo.subresourceRange.baseArrayLayer = range.arrayLayerOffset;
	viewCreateInfo.subresourceRange.layerCount = range.numArrayLevels;

<<<<<<< HEAD
	if (vk::CreateImageView(native_cast(getResource()->getContext()).getDevice(), &viewCreateInfo, NULL,
	                        &getNativeObject().handle) != VK_SUCCESS)
=======
	if (vk::CreateImageView(native_cast(res.getContext()).getDevice(), &viewCreateInfo, NULL,
	                        &handle) != VK_SUCCESS)
>>>>>>> 1776432f... 4.3
	{
		assertion(false, "Failed to create ImageView");
	}
}
}// namespace vulkan
}// namespace api
}// namespace pvr
