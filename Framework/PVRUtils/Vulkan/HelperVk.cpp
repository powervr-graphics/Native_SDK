/*!
\brief Implementation for a number of functions defined in the HelperVk header file.
\file PVRUtils/Vulkan/HelperVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "HelperVk.h"
#include "PVRCore/Texture/PVRTDecompress.h"
#include "PVRCore/TGAWriter.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/QueueVk.h"
#include "PVRVk/BindingsVk.h"
#include "PVRVk/SwapchainVk.h"
#include "PVRUtils/Vulkan/MemoryAllocator.h"


namespace pvr {
namespace utils {

pvrvk::Buffer createBuffer(pvrvk::Device device, VkDeviceSize size, VkBufferUsageFlags bufferUsage,
                           VkMemoryPropertyFlags memoryProps, VkBufferCreateFlags bufferCreateFlags,
                           bool sharingExclusive, const uint32_t* queueFamilyIndices,
                           uint32_t numQueueFamilyIndices)
{
	pvrvk::Buffer buffer = device->createBuffer(size, bufferUsage, bufferCreateFlags, sharingExclusive, queueFamilyIndices, numQueueFamilyIndices);
	if (buffer.isNull())
	{
		return buffer;
	}
	if (memoryProps != 0)
	{
		pvrvk::DeviceMemory deviceMemory = device->allocateMemory(buffer->getMemoryRequirement().size, buffer->getMemoryRequirement().memoryTypeBits, memoryProps);

		if (deviceMemory.isNull() || buffer->bindMemory(deviceMemory, 0) != VkResult::e_SUCCESS)
		{
			buffer.reset();
			deviceMemory.reset();
		}
	}

	return buffer;
}
pvrvk::Image createImage(
  pvrvk::Device device, VkImageType imageType, VkFormat format,
  const pvrvk::Extent3D& dimension, VkImageUsageFlags usage,
  VkImageCreateFlags flags,
  const pvrvk::ImageLayersSize& layerSize,
  VkSampleCountFlags samples,
  VkMemoryPropertyFlags allocMemFlags,
  bool sharingExclusive,
  const uint32_t* queueFamilyIndices,
  uint32_t numQueueFamilyIndices)
{
	pvrvk::Image image = device->createImage(imageType, format, dimension, usage, flags, layerSize, samples, sharingExclusive, queueFamilyIndices, numQueueFamilyIndices);
	if (image.isNull()) { return image; }
	// Create a memory block if it is non sparse and a valid memory propery flag.
	if ((flags & (
	       VkImageCreateFlags::e_SPARSE_ALIASED_BIT |
	       VkImageCreateFlags::e_SPARSE_BINDING_BIT |
	       VkImageCreateFlags::e_SPARSE_RESIDENCY_BIT)) == 0 &&
	    (allocMemFlags != VkMemoryPropertyFlags(0)))
		// If it's not sparse, create memory backing
	{
		// create memory block
		auto& memreq = image->getMemoryRequirement();
		pvrvk::DeviceMemory memBlock = device->allocateMemory(memreq.size, memreq.memoryTypeBits, allocMemFlags);

		if (memBlock.isNull() || !image->bindMemoryNonSparse(memBlock))
		{
			image.reset();
			memBlock.reset();
		}
	}
	return image;

}

using namespace pvrvk;
VkImageAspectFlags inferAspectFromFormat(VkFormat format)
{
	VkImageAspectFlags imageAspect = VkImageAspectFlags::e_COLOR_BIT;

	if (format >= VkFormat::e_D16_UNORM && format <= VkFormat::e_D32_SFLOAT_S8_UINT)
	{
		const VkImageAspectFlags aspects[] =
		{
			VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT,  //  VkFormat::e_D32_SFLOAT_S8_UINT
			VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT,  //  VkFormat::e_D24_UNORM_S8_UINT
			VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT,  //  VkFormat::e_D16_UNORM_S8_UINT
			VkImageAspectFlags::e_STENCIL_BIT,          //  VkFormat::e_S8_UINT
			VkImageAspectFlags::e_DEPTH_BIT,          //  VkFormat::e_D32_SFLOAT
			VkImageAspectFlags::e_DEPTH_BIT,          //  VkFormat::e_X8_D24_UNORM_PACK32
			VkImageAspectFlags::e_DEPTH_BIT,          //  VkFormat::e_D16_UNORM
		};
		// (Depthstenil format end) - format
		imageAspect = aspects[(int)(VkFormat::e_D32_SFLOAT_S8_UINT) - (int)(format)];
	}
	return imageAspect;
}

ImageView uploadImageAndSubmit(Device& device, const Texture& texture, bool allowDecompress,
                               CommandPool& pool, Queue& queue, VkImageUsageFlags flags, MemorySuballocator* allocator)
{
	CommandBuffer cmdBuffer = pool->allocateCommandBuffer();
	cmdBuffer->begin();
	ImageUploadResults results = uploadImage(device, texture, allowDecompress, cmdBuffer, flags, allocator);
	cmdBuffer->end();

	if (results.getResult() == pvr::Result::Success)
	{
		SubmitInfo submitInfo;
		submitInfo.commandBuffers = &cmdBuffer;
		submitInfo.numCommandBuffers = 1;
		Fence fence = device->createFence();
		queue->submit(&submitInfo, 1, fence);
		fence->wait();

		return results.getImageView();
	}
	return  ImageView();
}
namespace {
void decompressPvrtc(const Texture& texture, Texture& cDecompressedTexture)
{
	//Set up the new texture and header.
	TextureHeader cDecompressedHeader(texture);
	// robin: not sure what should happen here. The PVRTGENPIXELID4 macro is used in the old SDK.
	cDecompressedHeader.setPixelFormat(GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);

	cDecompressedHeader.setChannelType(VariableType::UnsignedByteNorm);
	cDecompressedTexture = Texture(cDecompressedHeader);

	//Do decompression, one surface at a time.
	for (uint32_t uiMipMapLevel = 0; uiMipMapLevel < texture.getNumMipMapLevels(); ++uiMipMapLevel)
	{
		for (uint32_t uiArray = 0; uiArray < texture.getNumArrayMembers(); ++uiArray)
		{
			for (uint32_t uiFace = 0; uiFace < texture.getNumFaces(); ++uiFace)
			{
				PVRTDecompressPVRTC(texture.getDataPointer(uiMipMapLevel, uiArray, uiFace),
				                    (texture.getBitsPerPixel() == 2 ? 1 : 0),
				                    texture.getWidth(uiMipMapLevel), texture.getHeight(uiMipMapLevel),
				                    cDecompressedTexture.getDataPointer(uiMipMapLevel, uiArray, uiFace));
			}
		}
	}
}

inline void getColorBits(VkFormat format, uint32_t& redBits,
                         uint32_t& greenBits, uint32_t& blueBits, uint32_t& alphaBits)
{
	switch (format)
	{
	case VkFormat::e_R8G8B8A8_SRGB:
	case VkFormat::e_R8G8B8A8_UNORM:
	case VkFormat::e_R8G8B8A8_SNORM:
	case VkFormat::e_B8G8R8A8_UNORM:
	case VkFormat::e_B8G8R8A8_SRGB:
		redBits = 8; greenBits = 8; blueBits = 8; alphaBits = 8;  break;
	case VkFormat::e_B8G8R8_SRGB:
	case VkFormat::e_B8G8R8_UNORM:
	case VkFormat::e_B8G8R8_SNORM:
	case VkFormat::e_R8G8B8_SRGB:
	case VkFormat::e_R8G8B8_UNORM:
	case VkFormat::e_R8G8B8_SNORM:
		redBits = 8; greenBits = 8; blueBits = 8; alphaBits = 0;  break;
	case VkFormat::e_R5G6B5_UNORM_PACK16:
		redBits = 5; greenBits = 6; blueBits = 5; alphaBits = 0;  break;
	default: assertion(0, "UnSupported VkFormat");
	}
}

inline void getDepthStencilBits(VkFormat format, uint32_t& depthBits, uint32_t& stencilBits)
{
	switch (format)
	{
	case VkFormat::e_D16_UNORM: depthBits = 16; stencilBits = 0; break;
	case VkFormat::e_D16_UNORM_S8_UINT: depthBits = 16; stencilBits = 8; break;
	case VkFormat::e_D24_UNORM_S8_UINT: depthBits = 24; stencilBits = 8; break;
	case VkFormat::e_D32_SFLOAT: depthBits = 32; stencilBits = 0; break;
	case VkFormat::e_D32_SFLOAT_S8_UINT: depthBits = 32; stencilBits = 8; break;
	case VkFormat::e_X8_D24_UNORM_PACK32: depthBits = 24; stencilBits = 0; break;
	case VkFormat::e_S8_UINT: depthBits = 0; stencilBits = 8; break;
	default: assertion(0, "UnSupported VkFormat");
	}
}

inline VkFormat getDepthStencilFormat(const DisplayAttributes& displayAttribs)
{
	uint32_t depthBpp = displayAttribs.depthBPP;
	uint32_t stencilBpp = displayAttribs.stencilBPP;

	VkFormat dsFormat = VkFormat::e_UNDEFINED;

	if (stencilBpp)
	{
		switch (depthBpp)
		{
		case 0: dsFormat = VkFormat::e_S8_UINT; break;
		case 16: dsFormat = VkFormat::e_D16_UNORM_S8_UINT; break;
		case 24: dsFormat = VkFormat::e_D24_UNORM_S8_UINT; break;
		case 32: dsFormat = VkFormat::e_D32_SFLOAT_S8_UINT; break;
		default: assertion("Unsupported Depth Stencil VkFormat");
		}
	}
	else
	{
		switch (depthBpp)
		{
		case 16: dsFormat = VkFormat::e_D16_UNORM; break;
		case 24: dsFormat = VkFormat::e_X8_D24_UNORM_PACK32; break;
		case 32: dsFormat = VkFormat::e_D32_SFLOAT; break;
		default: assertion("Unsupported Depth Stencil VkFormat");
		}
	}
	return dsFormat;
}

const inline std::string depthStencilFormatToString(VkFormat format)
{
	const std::string preferredDepthStencilFormat[] =
	{
		"VkFormat::e_D16_UNORM",
		"VkFormat::e_X8_D24_UNORM_PACK32",
		"VkFormat::e_D32_SFLOAT",
		"VkFormat::e_S8_UINT",
		"VkFormat::e_D16_UNORM_S8_UINT",
		"VkFormat::e_D24_UNORM_S8_UINT",
		"VkFormat::e_D32_SFLOAT_S8_UINT",
	};
	return preferredDepthStencilFormat[(int)format - (int)VkFormat::e_D16_UNORM];
}

Swapchain createSwapchainHelper(Device& device, const Surface& surface,
                                pvr::DisplayAttributes& displayAttributes, const VkImageUsageFlags& swapchainImageUsageFlags,
                                VkFormat* preferredColorFormats, uint32_t numPreferredColorFormats)
{
	Log(LogLevel::Information, "Creating Vulkan Swapchain using pvr::DisplayAttributes");

	SurfaceCapabilitiesKHR surfaceCapabilities = device->getPhysicalDevice()->getSurfaceCapabilities(surface);

	Log(LogLevel::Information, "Queried Surface Capabilities:");
	Log(LogLevel::Information, "\tMin-max swap image count: %u - %u", surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
	Log(LogLevel::Information, "\tArray size: %u", surfaceCapabilities.maxImageArrayLayers);
	Log(LogLevel::Information, "\tImage size (now): %dx%d", surfaceCapabilities.currentExtent.width,
	    surfaceCapabilities.currentExtent.height);
	Log(LogLevel::Information, "\tImage size (extent): %dx%d - %dx%d", surfaceCapabilities.minImageExtent.width,
	    surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.width,
	    surfaceCapabilities.maxImageExtent.height);
	Log(LogLevel::Information, "\tUsage: %x", surfaceCapabilities.supportedUsageFlags);
	Log(LogLevel::Information, "\tCurrent transform: %u", surfaceCapabilities.currentTransform);

#if !defined(ANDROID)
	surfaceCapabilities.currentExtent.width = std::max<uint32_t>(surfaceCapabilities.minImageExtent.width,
	    std::min<uint32_t>(displayAttributes.width, surfaceCapabilities.maxImageExtent.width));

	surfaceCapabilities.currentExtent.height = std::max<uint32_t>(surfaceCapabilities.minImageExtent.height,
	    std::min<uint32_t>(displayAttributes.height, surfaceCapabilities.maxImageExtent.height));
#endif
	// Log modifications made to the surface properties set via DisplayAttributes
	Log(LogLevel::Information, "Surface Properties after DisplayAttributes:");

	displayAttributes.width = surfaceCapabilities.currentExtent.width;
	displayAttributes.height = surfaceCapabilities.currentExtent.height;

	Log(LogLevel::Information, "\tImage size (now): %dx%d", displayAttributes.width, displayAttributes.height);

	uint32_t numFormats = 0;
	vk::GetPhysicalDeviceSurfaceFormatsKHR(device->getPhysicalDevice()->getNativeObject(),
	                                       surface->getNativeObject(), &numFormats, NULL);

	VkSurfaceFormatKHR tmpformats[16];
	std::vector<VkSurfaceFormatKHR> tmpFormatsVector;
	VkSurfaceFormatKHR* allFormats = tmpformats;
	if (numFormats > 16)
	{
		tmpFormatsVector.resize(numFormats);
		allFormats = tmpFormatsVector.data();
	}
	vk::GetPhysicalDeviceSurfaceFormatsKHR(device->getPhysicalDevice()->getNativeObject(),
	                                       surface->getNativeObject(), &numFormats, allFormats);

	VkSurfaceFormatKHR imageFormat = allFormats[0];

	VkFormat frameworkPreferredColorFormats[7] =
	{
		VkFormat::e_R8G8B8A8_UNORM, VkFormat::e_R8G8B8A8_SRGB, VkFormat::e_R8G8B8A8_SNORM,
		VkFormat::e_B8G8R8_SNORM, VkFormat::e_B8G8R8A8_UNORM, VkFormat::e_B8G8R8A8_SRGB, VkFormat::e_R5G6B5_UNORM_PACK16
	};
	std::vector<VkFormat> colorFormats;

	if (numPreferredColorFormats)
	{
		colorFormats.insert(colorFormats.begin(), &preferredColorFormats[0], &preferredColorFormats[numPreferredColorFormats]);
	}
	else
	{
		colorFormats.insert(colorFormats.begin(), &frameworkPreferredColorFormats[0], &frameworkPreferredColorFormats[7]);
	}

	uint32_t requestedRedBpp = displayAttributes.redBits;
	uint32_t requestedGreenBpp = displayAttributes.greenBits;
	uint32_t requestedBlueBpp = displayAttributes.blueBits;
	uint32_t requestedAlphaBpp = displayAttributes.alphaBits;
	bool foundFormat = false;
	for (unsigned int i = 0; i < colorFormats.size() && !foundFormat; ++i)
	{
		for (uint32_t f = 0; f < numFormats; ++f)
		{
			if (allFormats[f].format == colorFormats[i])
			{
				if (displayAttributes.forceColorBPP)
				{
					uint32_t currentRedBpp, currentGreenBpp, currentBlueBpp, currentAlphaBpp = 0;
					getColorBits(allFormats[f].format, currentRedBpp, currentGreenBpp, currentBlueBpp, currentAlphaBpp);
					if (currentRedBpp == requestedRedBpp &&
					    requestedGreenBpp == currentGreenBpp &&
					    requestedBlueBpp == currentBlueBpp &&
					    requestedAlphaBpp == currentAlphaBpp)
					{
						imageFormat = allFormats[f]; foundFormat = true; break;
					}
				}
				else
				{
					imageFormat = allFormats[f]; foundFormat = true; break;
				}
			}
		}
	}
	if (!foundFormat)
	{
		Log(LogLevel::Warning, "Swapchain - Unable to find supported "
		    "preferred color format. Using color format: %s", imageFormat);
	}

	uint32_t numPresentModes;
	VkResult res = vk::GetPhysicalDeviceSurfacePresentModesKHR(device->getPhysicalDevice()->getNativeObject(),
	               surface->getNativeObject(), &numPresentModes, NULL);
	assertion(res == VkResult::e_SUCCESS, "Failed to get the number of present modes count");

	assertion(numPresentModes > 0, "0 presentation modes returned");
	std::vector<VkPresentModeKHR> presentModes(numPresentModes);
	res = vk::GetPhysicalDeviceSurfacePresentModesKHR(device->getPhysicalDevice()->getNativeObject(),
	      surface->getNativeObject(), &numPresentModes, &presentModes[0]);
	assertion(res == VkResult::e_SUCCESS, "Failed to get the present modes");

	// Default is FIFO - Which is typical Vsync.

	VkPresentModeKHR swapchainPresentMode = VkPresentModeKHR::e_FIFO_KHR;
	VkPresentModeKHR desiredSwapMode = VkPresentModeKHR::e_FIFO_KHR;
	switch (displayAttributes.vsyncMode)
	{
	case VsyncMode::Off:
		desiredSwapMode = VkPresentModeKHR::e_IMMEDIATE_KHR;
		break;
	case VsyncMode::Mailbox:
		desiredSwapMode = VkPresentModeKHR::e_MAILBOX_KHR;
		break;
	case VsyncMode::Relaxed:
		desiredSwapMode = VkPresentModeKHR::e_FIFO_RELAXED_KHR;
		break;
	}
	for (size_t i = 0; i < numPresentModes; i++)
	{
		VkPresentModeKHR currentPresentMode = presentModes[i];

		if (currentPresentMode == desiredSwapMode)
		{
			//Precise match - Break!
			swapchainPresentMode = desiredSwapMode;
			break;
		}
		//Secondary matches : Immediate and Mailbox are better fits for each other than Fifo, so set them as secondaries
		// If the user asked for Mailbox, and we found Immediate, set it (in case Mailbox is not found) and keep looking
		if ((desiredSwapMode == VkPresentModeKHR::e_MAILBOX_KHR) && (currentPresentMode == VkPresentModeKHR::e_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VkPresentModeKHR::e_IMMEDIATE_KHR;
		}
		// ... And vice versa: If the user asked for Immediate, and we found Mailbox,
		// set it (in case Immediate is not found) and keep looking
		if ((desiredSwapMode == VkPresentModeKHR::e_IMMEDIATE_KHR) && (currentPresentMode == VkPresentModeKHR::e_MAILBOX_KHR))
		{
			swapchainPresentMode = VkPresentModeKHR::e_MAILBOX_KHR;
		}
	}
	switch (swapchainPresentMode)
	{
	case VkPresentModeKHR::e_IMMEDIATE_KHR:
		Log(LogLevel::Information, "Presentation mode: Immediate (Vsync OFF)"); break;
	case VkPresentModeKHR::e_MAILBOX_KHR:
		Log(LogLevel::Information, "Presentation mode: Mailbox (Triple-buffering)"); break;
	case VkPresentModeKHR::e_FIFO_KHR:
		Log(LogLevel::Information, "Presentation mode: FIFO (Vsync ON)"); break;
	case VkPresentModeKHR::e_FIFO_RELAXED_KHR:
		Log(LogLevel::Information, "Presentation mode: Relaxed FIFO (Improved Vsync)"); break;
	default: assertion(false, "Unrecognised presentation mode"); break;
	}

	if (!displayAttributes.swapLength)
	{
		switch (swapchainPresentMode)
		{
		case VkPresentModeKHR::e_IMMEDIATE_KHR: displayAttributes.swapLength = 2; break;
		case VkPresentModeKHR::e_MAILBOX_KHR: displayAttributes.swapLength = 3;  break;
		case VkPresentModeKHR::e_FIFO_KHR: displayAttributes.swapLength = 2; break;
		case VkPresentModeKHR::e_FIFO_RELAXED_KHR: displayAttributes.swapLength = 2; break;
		}
	}

	SwapchainCreateInfo createInfo;
	createInfo.clipped = true;
	createInfo.compositeAlpha = VkCompositeAlphaFlagsKHR::e_OPAQUE_BIT_KHR;
	createInfo.surface = surface;

	displayAttributes.swapLength = std::max<uint32_t>(displayAttributes.swapLength,
	                               surfaceCapabilities.minImageCount);
	if (surfaceCapabilities.maxImageCount)
	{
		displayAttributes.swapLength = std::min<uint32_t>(displayAttributes.swapLength,
		                               surfaceCapabilities.maxImageCount);
	}

	displayAttributes.swapLength = std::min<uint32_t>(displayAttributes.swapLength,
	                               FrameworkCaps::MaxSwapChains);

	createInfo.minImageCount = displayAttributes.swapLength;
	createInfo.imageFormat = imageFormat.format;

	createInfo.imageArrayLayers = 1;
	createInfo.imageColorSpace = imageFormat.colorSpace;
	createInfo.imageExtent.width = surfaceCapabilities.currentExtent.width;
	createInfo.imageExtent.height = surfaceCapabilities.currentExtent.height;
	createInfo.imageUsage = swapchainImageUsageFlags;

	createInfo.preTransform = VkSurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR;
	createInfo.imageSharingMode = VkSharingMode::e_EXCLUSIVE;
	createInfo.presentMode = swapchainPresentMode;
	createInfo.numQueueFamilyIndex = 1;
	uint32_t queueFamily = 0;
	createInfo.queueFamilyIndices = &queueFamily;

	Swapchain swapchain;
	swapchain = device->createSwapchain(createInfo, surface);

	return swapchain;
}

bool createDepthStencilImagesHelper(
  Device& device, pvr::DisplayAttributes& displayAttributes,
  VkFormat* preferredDepthFormats, uint32_t numDepthFormats, const pvrvk::Extent2D& imageExtent,
  Multi<ImageView>& depthStencilImages, VkFormat& outFormat,
  const VkImageUsageFlags& imageUsageFlags, VkSampleCountFlags sampleCount)
{
	VkFormat depthStencilFormatRequested = getDepthStencilFormat(displayAttributes);
	VkFormat supportedDepthStencilFormat = VkFormat::e_UNDEFINED;

	VkFormat frameworkPreferredDepthStencilFormat[6] =
	{
		VkFormat::e_D32_SFLOAT_S8_UINT,
		VkFormat::e_D24_UNORM_S8_UINT,
		VkFormat::e_D16_UNORM_S8_UINT,
		VkFormat::e_D32_SFLOAT,
		VkFormat::e_D16_UNORM,
		VkFormat::e_X8_D24_UNORM_PACK32,
	};

	std::vector<VkFormat> depthFormats;

	if (numDepthFormats)
	{
		depthFormats.insert(depthFormats.begin(), &preferredDepthFormats[0], &preferredDepthFormats[numDepthFormats]);
	}
	else
	{
		depthFormats.insert(depthFormats.begin(), &frameworkPreferredDepthStencilFormat[0], &frameworkPreferredDepthStencilFormat[6]);
	}

	// start by checking for the requested depth stencil format
	VkFormat currentDepthStencilFormat = depthStencilFormatRequested;
	for (uint32_t f = 0; f < depthFormats.size(); ++f)
	{
		VkFormatProperties prop = device->getPhysicalDevice()->getFormatProperties(currentDepthStencilFormat);
		if ((prop.optimalTilingFeatures & VkFormatFeatureFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
		{
			supportedDepthStencilFormat = currentDepthStencilFormat;
			break;
		}
		currentDepthStencilFormat = depthFormats[f];
	}

	if (depthStencilFormatRequested != supportedDepthStencilFormat)
	{
		Log(LogLevel::Information, "Requested DepthStencil VkFormat %s is not supported. Falling back to %s",
		    depthStencilFormatToString(depthStencilFormatRequested).c_str(), depthStencilFormatToString(supportedDepthStencilFormat).c_str());
	}
	getDepthStencilBits(supportedDepthStencilFormat, displayAttributes.depthBPP, displayAttributes.stencilBPP);
	Log(LogLevel::Information, "DepthStencil VkFormat: %s", depthStencilFormatToString(supportedDepthStencilFormat).c_str());

	// create the depth stencil images
	depthStencilImages.resize(displayAttributes.swapLength);
	const VkMemoryPropertyFlags memProp = (imageUsageFlags & VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT) != 0 ?
	                                      VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT :
	                                      VkMemoryPropertyFlags::e_DEVICE_LOCAL_BIT;
	for (int32_t i = 0; i < displayAttributes.swapLength; ++i)
	{
		Image depthStencilImage = createImage(device, VkImageType::e_2D,
		                                      supportedDepthStencilFormat, pvrvk::Extent3D(imageExtent, 1u), imageUsageFlags,
		                                      VkImageCreateFlags(0), pvrvk::ImageLayersSize(), sampleCount, memProp);

		if (depthStencilImage.isNull())
		{
			Log("Failed to create the depth stencil images");
			return false;
		}

		depthStencilImages[i] = device->createImageView(depthStencilImage);
		if (depthStencilImages[i].isNull())
		{
			Log("Failed to create the depth stencil image views");
			return false;
		}
	}

	outFormat = supportedDepthStencilFormat;

	return true;
}



}
namespace impl {
const Texture* decompressIfRequired(const Texture& texture, Texture& decompressedTexture,
                                    bool allowDecompress, bool supportPvrtc, bool& isDecompressed)
{
	const Texture* textureToUse = &texture;
	// Setup code to get various state
	// Generic error strings for textures being unsupported.
	const char* cszUnsupportedFormat =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation.\n";
	const char* cszUnsupportedFormatDecompressionAvailable =
	  "TextureUtils.h:textureUpload:: Texture format %s is not supported in this implementation."
	  " Allowing software decompression (allowDecompress=true) will enable you to use this format.\n";

	// Check that extension support exists for formats supported in this way.
	// Check format not supportedfor formats only supported by extensions.
	switch (texture.getPixelFormat().getPixelTypeId())
	{
	case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB):
	case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA):
	case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB):
	case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA):
	{
		bool decompress = !supportPvrtc;
		if (decompress)
		{
			if (allowDecompress)
			{
				Log(LogLevel::Information, "PVRTC texture format support not detected. Decompressing PVRTC to"
				    " corresponding format (RGBA32 or RGB24)");
				decompressPvrtc(texture, decompressedTexture);
				textureToUse = &decompressedTexture;
				isDecompressed = true;
			}
			else
			{
				Log(LogLevel::Error, cszUnsupportedFormatDecompressionAvailable, "PVRTC");
				return nullptr;
			}
		}
		break;
	}
	case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp):
	case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp):
	{
		if (!supportPvrtc)
		{
			Log(LogLevel::Error, cszUnsupportedFormat, "PVRTC2");
			return nullptr;
		}
		break;
	}
	case static_cast<uint64_t>(CompressedPixelFormat::ETC1): Log(LogLevel::Error, cszUnsupportedFormatDecompressionAvailable, "ETC1");
		return nullptr;
	case static_cast<uint64_t>(CompressedPixelFormat::DXT1): Log(LogLevel::Error, cszUnsupportedFormatDecompressionAvailable, "DXT1");
		return nullptr;
	case static_cast<uint64_t>(CompressedPixelFormat::DXT3): Log(LogLevel::Error, cszUnsupportedFormatDecompressionAvailable, "DXT1");
		return nullptr;
	case static_cast<uint64_t>(CompressedPixelFormat::DXT5): Log(LogLevel::Error, cszUnsupportedFormatDecompressionAvailable, "DXT3");
		return nullptr;
	default:
	{}
	}
	return textureToUse;
}
}

ImageUploadResults uploadImageHelper(Device& device, const Texture& texture, bool allowDecompress,
                                     CommandBufferBase commandBuffer, VkImageUsageFlags usageFlags,
                                     MemorySuballocator* bufferAllocator = nullptr, MemorySuballocator* textureAllocator = nullptr)
{
	pvr::Result results = pvr::Result::UnknownError;
	ImageUpdateResults updateResult;
	bool isDecompressed;
	// Check that the texture is valid.
	if (!texture.getDataSize())
	{
		Log(LogLevel::Error, "TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs.\n");
		results = pvr::Result::UnsupportedRequest;
		return ImageUploadResults();
	}

	VkFormat format = VkFormat::e_UNDEFINED;

	//Texture to use if we decompress in software.
	Texture decompressedTexture;

	// Texture pointer which points at the texture we should use for the function.
	// Allows switching to, for example, a decompressed version of the texture.
	const Texture* textureToUse = impl::decompressIfRequired(texture, decompressedTexture,
	                              allowDecompress, device->supportsPVRTC(), isDecompressed);

	format = convertToVk(textureToUse->getPixelFormat());
	if (format == VkFormat::e_UNDEFINED)
	{
		Log(LogLevel::Error, "TextureUtils.h:textureUpload:: Texture's pixel type is not supported by this API.\n");
		results = pvr::Result::UnsupportedRequest;
		return ImageUploadResults();
	}

	uint32_t texWidth = static_cast<uint32_t>(textureToUse->getWidth());
	uint32_t texHeight = static_cast<uint32_t>(textureToUse->getHeight());
	uint32_t texDepth = static_cast<uint32_t>(textureToUse->getDepth());

	uint32_t dataWidth = static_cast<uint32_t>(textureToUse->getWidth());
	uint32_t dataHeight = static_cast<uint32_t>(textureToUse->getHeight());
	uint32_t dataDepth = static_cast<uint32_t>(textureToUse->getDepth());

	uint16_t texMipLevels = static_cast<uint16_t>(textureToUse->getNumMipMapLevels());
	uint16_t texArraySlices = static_cast<uint16_t>(textureToUse->getNumArrayMembers());
	uint16_t texFaces = static_cast<uint16_t>(textureToUse->getNumFaces());
	Image image;

	usageFlags |= VkImageUsageFlags::e_TRANSFER_DST_BIT;

	if (texDepth > 1)
	{
		image  = createImage(device, VkImageType::e_3D, format, pvrvk::Extent3D(texWidth, texHeight, texDepth),
		                     usageFlags, VkImageCreateFlags(0), pvrvk::ImageLayersSize(texArraySlices, static_cast<uint8_t>(texMipLevels)));
	}
	else if (texHeight > 1)
	{
		image  = createImage(device, VkImageType::e_2D, format, pvrvk::Extent3D(texWidth, texHeight, 1u),
		                     usageFlags, VkImageCreateFlags::e_CUBE_COMPATIBLE_BIT * (texture.getNumFaces() > 1) |
		                     VkImageCreateFlags::e_2D_ARRAY_COMPATIBLE_BIT_KHR * static_cast<uint32_t>(texArraySlices > 1),
		                     pvrvk::ImageLayersSize(texArraySlices * (texture.getNumFaces() > 1 ? 6 : 1), static_cast<uint8_t>(texMipLevels)));
	}
	else
	{
		image  = createImage(device, VkImageType::e_1D, format, pvrvk::Extent3D(texWidth, 1u, 1u), usageFlags,
		                     VkImageCreateFlags(0), pvrvk::ImageLayersSize(texArraySlices, static_cast<uint8_t>(texMipLevels)));
	}

	if (!image.isValid())
	{
		return ImageUploadResults();
	}
	// POPULATE, TRANSITION ETC
	{
		//Create a bunch of buffers that will be used as copy destinations - each will be one mip level, one array slice / one face
		//Faces are considered array elements, so each Framework array slice in a cube array will be 6 vulkan array slices.

		//Edit the info to be the small, linear images that we are using.
		std::vector<ImageUpdateInfo> imageUpdates(texMipLevels * texArraySlices * texFaces);
		uint32_t imageUpdateIndex = 0;
		for (uint32_t mipLevel = 0; mipLevel < texMipLevels; ++mipLevel)
		{
			uint32_t minWidth, minHeight, minDepth;
			textureToUse->getMinDimensionsForFormat(minWidth, minHeight, minDepth);
			dataWidth = static_cast<uint32_t>(std::max(textureToUse->getWidth(mipLevel), minWidth));
			dataHeight = static_cast<uint32_t>(std::max(textureToUse->getHeight(mipLevel), minHeight));
			dataDepth = static_cast<uint32_t>(std::max(textureToUse->getDepth(mipLevel), minDepth));
			texWidth = textureToUse->getWidth(mipLevel);
			texHeight = textureToUse->getHeight(mipLevel);
			texDepth = textureToUse->getDepth(mipLevel);
			for (uint32_t arraySlice = 0; arraySlice < texArraySlices; ++arraySlice)
			{
				for (uint32_t face = 0; face < texFaces; ++face)
				{
					ImageUpdateInfo& update = imageUpdates[imageUpdateIndex];
					update.imageWidth = texWidth;
					update.imageHeight = texHeight;
					update.dataWidth = dataWidth;
					update.dataHeight = dataHeight;
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

		updateResult = updateImage(device, commandBuffer, imageUpdates.data(),
		                           static_cast<uint32_t>(imageUpdates.size()), format, VkImageLayout::e_GENERAL, texFaces > 1,  image,
		                           bufferAllocator);
	}
	results = pvr::Result::Success;


	//create the wrapper objects
	if (updateResult.getResult() == pvr::Result::Success)
	{
		ComponentMapping swizzle =
		{
			VkComponentSwizzle::e_IDENTITY,
			VkComponentSwizzle::e_IDENTITY,
			VkComponentSwizzle::e_IDENTITY,
			VkComponentSwizzle::e_IDENTITY,
		};
		if (texture.getPixelFormat().getChannelContent(0) == 'l')
		{
			if (texture.getPixelFormat().getChannelContent(1) == 'a')
			{
				swizzle.r = swizzle.g = swizzle.b = VkComponentSwizzle::e_R;
				swizzle.a = VkComponentSwizzle::e_G;
			}
			else
			{
				swizzle.r = swizzle.g = swizzle.b = VkComponentSwizzle::e_R;
				swizzle.a = VkComponentSwizzle::e_ONE;
			}
		}
		else if (texture.getPixelFormat().getChannelContent(0) == 'a')
		{
			swizzle.r = swizzle.g = swizzle.b = VkComponentSwizzle::e_ZERO;
			swizzle.a = VkComponentSwizzle::e_R;
		}
		return ImageUploadResults(updateResult, device->createImageView(image, swizzle),
		                          isDecompressed, results);
	}
	return ImageUploadResults();
}

inline ImageUploadResults loadAndUploadImageHelper(Device& device, const char* fileName,
    bool allowDecompress, CommandBufferBase commandBuffer, IAssetProvider& assetProvider,
    VkImageUsageFlags usageFlags, Texture* outAssetTexture = nullptr, MemorySuballocator* allocator = nullptr)
{
	Texture outTexture;
	Texture* pOutTexture = &outTexture;
	if (outAssetTexture) { pOutTexture = outAssetTexture; }
	auto assetStream = assetProvider.getAssetStream(fileName);
	if (!pvr::assets::textureLoad(assetStream, pvr::getTextureFormatFromFilename(fileName), *pOutTexture))
	{
		Log("Failed to load texture %s", fileName);
		return ImageUploadResults();
	}
	return uploadImageHelper(device, *pOutTexture, allowDecompress, commandBuffer, usageFlags);
}

ImageUploadResults loadAndUploadImage(Device& device, const char* fileName,
                                      bool allowDecompress, CommandBuffer& commandBuffer, IAssetProvider& assetProvider,
                                      VkImageUsageFlags usageFlags, Texture* outAssetTexture, MemorySuballocator* allocator)
{
	return loadAndUploadImageHelper(device, fileName, allowDecompress,
	                                CommandBufferBase(commandBuffer), assetProvider, usageFlags, outAssetTexture, allocator);
}

ImageUploadResults loadAndUploadImage(Device& device, const char* fileName, bool allowDecompress,
                                      SecondaryCommandBuffer& commandBuffer, IAssetProvider& assetProvider, VkImageUsageFlags usageFlags,
                                      Texture* outAssetTexture, MemorySuballocator* allocator)
{
	return loadAndUploadImageHelper(device, fileName, allowDecompress,
	                                CommandBufferBase(commandBuffer), assetProvider, usageFlags, outAssetTexture, allocator);
}

ImageUploadResults uploadImage(Device& device, const Texture& texture, bool allowDecompress,
                               SecondaryCommandBuffer& commandBuffer, VkImageUsageFlags usageFlags, MemorySuballocator* allocator)
{
	return uploadImageHelper(device, texture, allowDecompress, CommandBufferBase(commandBuffer), usageFlags, allocator);
}

ImageUploadResults uploadImage(Device& device, const Texture& texture, bool allowDecompress,
                               CommandBuffer& commandBuffer, VkImageUsageFlags usageFlags, MemorySuballocator* allocator)
{
	return uploadImageHelper(device, texture, allowDecompress, CommandBufferBase(commandBuffer), usageFlags, allocator);
}

bool generateTextureAtlas(Device& device, const Image* textures,
                          Rect2Df* outUVs, uint32_t numTextures, ImageView* outTexture,
                          TextureHeader* outDescriptor, CommandBufferBase cmdBuffer)
{
	TextureHeader header;
	struct SortedImage
	{
		uint32_t          id;
		Image       tex;
		uint16_t     width;
		uint16_t     height;
		uint16_t     srcX;
		uint16_t     srcY;
		bool            hasAlpha;
	};
	std::vector<SortedImage> sortedImage(numTextures);
	struct SortCompare
	{
		bool operator()(const SortedImage& a, const SortedImage& b)
		{
			uint32_t aSize = a.width * a.height;
			uint32_t bSize = b.width * b.height;
			return (aSize > bSize);
		}
	};

	struct Area
	{
		int32_t    x;
		int32_t    y;
		int32_t    w;
		int32_t    h;
		int32_t    size;
		bool          isFilled;

		Area*         right;
		Area*         left;

	private:
		void setSize(int32_t width, int32_t height) { w = width;  h = height; size = width * height;  }
	public:
		Area(int32_t width, int32_t height) :
			x(0), y(0), isFilled(false), right(NULL), left(NULL)
		{
			setSize(width, height);
		}

		Area() : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(0, 0); }

		Area* insert(int32_t width, int32_t height)
		{
			// If this area has branches below it (i.e. is not a leaf) then traverse those.
			// Check the left branch first.
			if (left)
			{
				Area* tempPtr = NULL;
				tempPtr = left->insert(width, height);
				if (tempPtr != NULL) { return tempPtr; }
			}
			// Now check right
			if (right) { return right->insert(width, height); }
			// Already filled!
			if (isFilled) { return NULL; }

			// Too small
			if (size < width * height || w < width || h < height) { return NULL; }

			// Just right!
			if (size == width * height && w == width && h == height)
			{
				isFilled = true;
				return this;
			}
			// Too big. Split up.
			if (size > width * height && w >= width && h >= height)
			{
				// Initializes the children, and sets the left child's coordinates as these don't change.
				left = new Area;
				right = new Area;
				left->x = x;
				left->y = y;

				// --- Splits the current area depending on the size and position of the placed texture.
				// Splits vertically if larger free distance across the texture.
				if ((w - width) > (h - height))
				{
					left->w = width;
					left->h = h;

					right->x = x + width;
					right->y = y;
					right->w = w - width;
					right->h = h;
				}
				// Splits horizontally if larger or equal free distance downwards.
				else
				{
					left->w = w;
					left->h = height;

					right->x = x;
					right->y = y + height;
					right->w = w;
					right->h = h - height;
				}

				//Initializes the child members' size attributes.
				left->size = left->h  * left->w;
				right->size = right->h * right->w;

				//Inserts the texture into the left child member.
				return left->insert(width, height);
			}
			//Catch all error return.
			return NULL;
		}

		bool deleteArea()
		{
			if (left != NULL)
			{
				if (left->left != NULL)
				{
					if (!left->deleteArea())  { return false; }
					if (!right->deleteArea()) { return false; }
				}
			}
			if (right != NULL)
			{
				if (right->left != NULL)
				{
					if (!left->deleteArea())  { return false; }
					if (!right->deleteArea()) { return false; }
				}
			}
			delete right;
			right = NULL;
			delete left;
			left = NULL;
			return true;
		}
	};

	// load the textures
	for (uint32_t i = 0; i < numTextures; ++i)
	{
		sortedImage[i].tex = textures[i];
		sortedImage[i].id = i;
		sortedImage[i].width = static_cast<uint16_t>(textures[i]->getWidth());
		sortedImage[i].height = static_cast<uint16_t>(textures[i]->getHeight());
	}
	//// sort the sprites
	std::sort(sortedImage.begin(), sortedImage.end(), SortCompare());
	// find the best width and height
	int32_t width = 0, height = 0, area = 0;
	uint32_t preferredDim[] = {8, 16, 32, 64, 128, 256, 512, 1024};
	const uint32_t atlasPixelBorder = 1;
	const uint32_t totalBorder = atlasPixelBorder * 2;
	uint32_t sortedImagesIterator = 0;
	// calculate the total area
	for (; sortedImagesIterator < sortedImage.size(); ++sortedImagesIterator)
	{
		area += (sortedImage[sortedImagesIterator].width + totalBorder) * (sortedImage[sortedImagesIterator].height + totalBorder);
	}
	sortedImagesIterator = 0;
	while ((static_cast<int32_t>(preferredDim[sortedImagesIterator]) * static_cast<int32_t>(preferredDim[sortedImagesIterator])) < area &&
	       sortedImagesIterator < sizeof(preferredDim) / sizeof(preferredDim[0]))
	{
		++sortedImagesIterator;
	}
	if (sortedImagesIterator >= sizeof(preferredDim) / sizeof(preferredDim[0]))
	{
		Log("Cannot find a best size for the texture atlas");
		return false;
	}
	width = height = preferredDim[sortedImagesIterator];
	float oneOverWidth = 1.f / width;
	float oneOverHeight = 1.f / height;
	Area* head = new Area(width, height);
	Area* pRtrn = nullptr;
	pvrvk::Offset3D dstOffset[2];


	// create the out texture store
	VkFormat outFmt = VkFormat::e_R8G8B8A8_UNORM;
	Image outTexStore = createImage(device, VkImageType::e_2D,
	                                        outFmt, pvrvk::Extent3D(width, height, 1u),
	                                        VkImageUsageFlags::e_SAMPLED_BIT | VkImageUsageFlags::e_TRANSFER_DST_BIT);

	utils::setImageLayout(
	  outTexStore, VkImageLayout::e_UNDEFINED, VkImageLayout::e_TRANSFER_DST_OPTIMAL,
	  cmdBuffer);

	ImageView view = device->createImageView(outTexStore);
	cmdBuffer->clearColorImage(view, ClearColorValue(0.0f, 0.f, 0.f, 0.f), VkImageLayout::e_TRANSFER_DST_OPTIMAL);

	for (uint32_t i = 0; i < numTextures; ++i)
	{
		const SortedImage& image = sortedImage[i];
		pRtrn = head->insert(static_cast<int32_t>(sortedImage[i].width) + totalBorder,
		                     static_cast<int32_t>(sortedImage[i].height) + totalBorder);
		if (!pRtrn)
		{
			Log("ERROR: Not enough room in texture atlas!\n");
			head->deleteArea();
			delete head;
			return false;
		}
		dstOffset[0].x = static_cast<uint16_t>(pRtrn->x + atlasPixelBorder);
		dstOffset[0].y = static_cast<uint16_t>(pRtrn->y + atlasPixelBorder);
		dstOffset[0].z = 0;

		dstOffset[1].x = static_cast<uint16_t>(dstOffset[0].x + sortedImage[i].width);
		dstOffset[1].y = static_cast<uint16_t>(dstOffset[0].y + sortedImage[i].height);
		dstOffset[1].z = 1;

		outUVs[image.id].offset.x = dstOffset[0].x * oneOverWidth;
		outUVs[image.id].offset.y = dstOffset[0].y * oneOverHeight;
		outUVs[image.id].extent.width = sortedImage[i].width * oneOverWidth;
		outUVs[image.id].extent.height = sortedImage[i].height * oneOverHeight;

		ImageBlitRange blit(pvrvk::Offset3D(0, 0, 0),
		                    pvrvk::Offset3D(image.width, image.height, 1), dstOffset[0], dstOffset[1]);

		cmdBuffer->blitImage(sortedImage[i].tex, outTexStore, &blit, 1, VkFilter::e_NEAREST,
		                     VkImageLayout::e_TRANSFER_SRC_OPTIMAL, VkImageLayout::e_TRANSFER_DST_OPTIMAL);
	}
	if (outDescriptor)
	{
		outDescriptor->setWidth(width); outDescriptor->setHeight(height);
		outDescriptor->setChannelType(VariableType::UnsignedByteNorm);
		outDescriptor->setColorSpace(ColorSpace::lRGB);
		outDescriptor->setDepth(1);
		outDescriptor->setPixelFormat(PixelFormat::RGBA_8888);
	}
	(*outTexture) = device->createImageView(outTexStore);

	const uint32_t queueFamilyId = cmdBuffer->getCommandPool()->getQueueFamilyId();

	MemoryBarrierSet barrier;
	barrier.addBarrier(pvrvk::ImageMemoryBarrier(VkAccessFlags::e_TRANSFER_WRITE_BIT, VkAccessFlags::e_SHADER_READ_BIT, outTexStore,
	                   pvrvk::ImageSubresourceRange(VkImageAspectFlags::e_COLOR_BIT),
	                   VkImageLayout::e_TRANSFER_DST_OPTIMAL, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
	                   queueFamilyId, queueFamilyId));

	cmdBuffer->pipelineBarrier(VkPipelineStageFlags::e_TRANSFER_BIT,
	                           VkPipelineStageFlags::e_FRAGMENT_SHADER_BIT | VkPipelineStageFlags::e_COMPUTE_SHADER_BIT, barrier);

	head->deleteArea();
	delete head;
	return true;
}

pvrvk::Device createDeviceAndQueues(PhysicalDevice physicalDevice,
                                    const QueuePopulateInfo* queueCreateFlags, uint32_t numQueueCreateFlags, QueueAccessInfo* outAccessInfo,
                                    const DeviceExtensions& deviceExtensions)
{
	std::vector<DeviceQueueCreateInfo> queueCreateInfo;
	std::vector<QueueFamilyProperties> queueProps = physicalDevice->getQueueFamilyProperties();
	std::vector<int32_t> queueIndex(queueProps.size(), -1);
	std::vector<VkQueueFlags> queueFlags(queueProps.size(), VkQueueFlags(0));
	for (uint32_t i = 0; i < numQueueCreateFlags; ++i)
	{
		for (uint32_t j = 0; j < queueProps.size(); ++j)
		{
			QueueFamilyProperties& queueProp = queueProps[j];
			// look for the flags
			if (((static_cast<uint32_t>(queueProp.queueFlags) & static_cast<uint32_t>(queueCreateFlags[i].queueFlags)) == static_cast<uint32_t>(queueCreateFlags[i].queueFlags)) &&
			    queueProp.numQueues)
			{
				if (queueCreateFlags[i].surface.isValid()) // look for presentation
				{
					std::vector<VkBool32> presentationQueueFamily;
					physicalDevice->getPresentationQueueFamily(
					  queueCreateFlags[i].surface, presentationQueueFamily);

					if (presentationQueueFamily[j])
					{
						outAccessInfo[i].familyId = j;
						outAccessInfo[i].queueId = ++queueIndex[j];
						queueFlags[j] |= queueCreateFlags[i].queueFlags;
						--queueProps[j].numQueues;
						break;
					}
				}
				else
				{
					outAccessInfo[i].familyId = j;
					outAccessInfo[i].queueId = ++queueIndex[j];
					--queueProps[j].numQueues;
					break;
				}
			}
		}
	}

	// populate the queue create info
	for (uint32_t i = 0; i < queueIndex.size(); ++i)
	{
		if (queueIndex[i] != -1)
		{
			queueCreateInfo.push_back(DeviceQueueCreateInfo());
			DeviceQueueCreateInfo& createInfo = queueCreateInfo.back();
			createInfo.queueCount = queueIndex[i] + 1;
			createInfo.queueFamilyIndex = i;
			for (uint32_t j = 0; j < createInfo.queueCount; ++j)
			{
				createInfo.queuePriorities[j] = 1.f;
			}
		}
	}
	// create the _device

	DeviceCreateInfo deviceInfo;
	PhysicalDeviceFeatures feature = physicalDevice->getFeatures();
	feature.robustBufferAccess = false;
	deviceInfo.enabledFeatures = &feature;
	deviceInfo.queueCreateInfos = queueCreateInfo;
	deviceInfo.enabledExtensionNames = deviceExtensions.extensionStrings;
	return physicalDevice->createDevice(deviceInfo);
}

bool createSwapchainAndDepthStencilImageView(Device& device, const Surface& surface,
    DisplayAttributes& displayAttributes, Swapchain& outSwapchain, Multi<ImageView>& outDepthStencil,
    const VkImageUsageFlags& swapchainImageUsageFlags, const VkImageUsageFlags& dsImageUsageFlags)
{
	outSwapchain = createSwapchain(device, surface, displayAttributes, swapchainImageUsageFlags);
	if (outSwapchain.isNull()) { return false; }

	VkFormat outDepthFormat;
	return createDepthStencilImagesHelper(device, displayAttributes, nullptr, 0,
	                                      outSwapchain->getDimension(), outDepthStencil, outDepthFormat, dsImageUsageFlags, VkSampleCountFlags::e_1_BIT);
}

bool createSwapchainAndDepthStencilImageView(Device& device, const Surface& surface,
    DisplayAttributes& displayAttributes, Swapchain& outSwapchain, Multi<ImageView>& outDepthStencil,
    VkFormat* preferredColorFormats, uint32_t numColorFormats, VkFormat* preferredDepthFormats,
    uint32_t numDepthFormats, const VkImageUsageFlags& swapchainImageUsageFlags,
    const VkImageUsageFlags& dsImageUsageFlags)
{
	outSwapchain = createSwapchain(device, surface, displayAttributes, preferredColorFormats,
	                               numColorFormats, swapchainImageUsageFlags);
	if (outSwapchain.isNull()) { return false; }
	VkFormat dsFormat;
	return createDepthStencilImagesHelper(device, displayAttributes, preferredDepthFormats,
	                                      numDepthFormats, outSwapchain->getDimension(), outDepthStencil, dsFormat, dsImageUsageFlags, VkSampleCountFlags::e_1_BIT);
}

Swapchain createSwapchain(Device& device, const Surface& surface,
                          pvr::DisplayAttributes& displayAttributes, VkFormat* preferredColorFormats, uint32_t numColorFormats,
                          VkImageUsageFlags swapchainImageUsageFlags)
{
	return createSwapchainHelper(device, surface, displayAttributes, swapchainImageUsageFlags, preferredColorFormats, numColorFormats);
}

Swapchain createSwapchain(Device& device,
                          const Surface& surface, pvr::DisplayAttributes& displayAttributes,
                          VkImageUsageFlags swapchainImageUsageFlags)
{
	VkFormat formats[1];
	return createSwapchainHelper(device, surface, displayAttributes, swapchainImageUsageFlags, formats, 0);
}

bool createDepthStencilImages(Device device,
                              pvr::DisplayAttributes& displayAttributes, const pvrvk::Extent2D& imageExtent,
                              Multi<ImageView>& depthStencilImages, VkFormat& outFormat,
                              const VkImageUsageFlags& swapchainImageUsageFlags,
                              VkSampleCountFlags sampleCount)
{
	VkFormat formats[1];
	return createDepthStencilImagesHelper(device, displayAttributes, formats, 0,
	                                      imageExtent, depthStencilImages, outFormat, swapchainImageUsageFlags, sampleCount);
}

bool createDepthStencilImages(Device device, pvr::DisplayAttributes& displayAttributes,
                              VkFormat* preferredDepthFormats, uint32_t numDepthFormats,
                              const pvrvk::Extent2D& imageExtent, Multi<ImageView>& depthStencilImages, VkFormat& outFormat,
                              const VkImageUsageFlags& swapchainImageUsageFlags, VkSampleCountFlags sampleCount)
{
	return createDepthStencilImagesHelper(device, displayAttributes, preferredDepthFormats,
	                                      numDepthFormats, imageExtent, depthStencilImages, outFormat, swapchainImageUsageFlags, sampleCount);
}

namespace {
bool screenCaptureRegion(Device device,
                         Image swapChainImage, CommandPool& cmdPool, Queue& queue,
                         uint32_t x, uint32_t y, uint32_t w, uint32_t h, char* outBuffer,
                         uint32_t strideInBytes, VkFormat requestedImageFormat,
                         VkImageLayout initialLayout, VkImageLayout finalLayout)
{
	CommandBuffer cmdBuffer = cmdPool->allocateCommandBuffer();
	const uint16_t width = static_cast<uint16_t>(w - x);
	const uint16_t height = static_cast<uint16_t>(h - y);
	const uint32_t dataSize = strideInBytes * width * height;
	// create the destination texture which does the format conversion

	const VkFormatProperties& formatProps =
	  device->getPhysicalDevice()->getFormatProperties(requestedImageFormat);
	if ((formatProps.optimalTilingFeatures & VkFormatFeatureFlags::e_BLIT_DST_BIT) == 0)
	{
		Log("Screen Capture requested Image format is not supported");
		return false;
	}

	// Create the intermediate image which will be used as the format conversion
	// when copying from swapchain image and then copied into the buffer
	Image dstImage = createImage(device, VkImageType::e_2D, requestedImageFormat,
	                                     pvrvk::Extent3D(width, height, 1u), VkImageUsageFlags::e_TRANSFER_DST_BIT |
	                                     VkImageUsageFlags::e_TRANSFER_SRC_BIT);

	const pvrvk::Offset3D srcOffsets[2] =
	{
		pvrvk::Offset3D(static_cast<uint16_t>(x), static_cast<uint16_t>(y), 0),
		pvrvk::Offset3D(static_cast<uint16_t>(w), static_cast<uint16_t>(h), 1)
	};

	const pvrvk::Offset3D dstOffsets[2] =
	{
		pvrvk::Offset3D(static_cast<uint16_t>(x), static_cast<uint16_t>(h), 0),
		pvrvk::Offset3D(static_cast<uint16_t>(w), static_cast<uint16_t>(y), 1)
	};

	//create the final destination buffer for reading
	Buffer buffer = createBuffer(device, dataSize, VkBufferUsageFlags::e_TRANSFER_DST_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);

	cmdBuffer->begin(VkCommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);
	ImageBlitRange copyRange(srcOffsets, dstOffsets);

	// transform the layout from the color attachment to transfer src
	setImageLayout(swapChainImage, initialLayout, VkImageLayout::e_TRANSFER_SRC_OPTIMAL, cmdBuffer);
	setImageLayout(dstImage, VkImageLayout::e_UNDEFINED, VkImageLayout::e_TRANSFER_DST_OPTIMAL, cmdBuffer);

	cmdBuffer->blitImage(swapChainImage, dstImage, &copyRange, 1, VkFilter::e_LINEAR,
	                     VkImageLayout::e_TRANSFER_SRC_OPTIMAL, VkImageLayout::e_TRANSFER_DST_OPTIMAL);

	pvrvk::ImageSubresourceLayers subResource;
	subResource.aspectMask = VkImageAspectFlags::e_COLOR_BIT;
	BufferImageCopy region(0, 0, 0, subResource, pvrvk::Offset3D(x, y, 0), pvrvk::Extent3D(w, h, 1));

	setImageLayout(swapChainImage, VkImageLayout::e_TRANSFER_SRC_OPTIMAL, finalLayout, cmdBuffer);
	setImageLayout(dstImage, VkImageLayout::e_TRANSFER_DST_OPTIMAL, VkImageLayout::e_TRANSFER_SRC_OPTIMAL, cmdBuffer);

	cmdBuffer->copyImageToBuffer(dstImage, VkImageLayout::e_TRANSFER_SRC_OPTIMAL, buffer, &region, 1);
	cmdBuffer->end();
	// create a fence for wait.
	Fence fenceWait = device->createFence(VkFenceCreateFlags(0));
	SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cmdBuffer;
	submitInfo.numCommandBuffers = 1;
	queue->submit(&submitInfo, 1, fenceWait);
	fenceWait->wait();// wait for the submit to finish so that the command buffer get destroyed properly
	// map the buffer and copy the data
	void* memory = 0;
	unsigned char* data;
	if ((buffer->getDeviceMemory()->map(&memory, 0, dataSize) != VkResult::e_SUCCESS) || !memory)
	{
		return false;
	}
	data = static_cast<unsigned char*>(memory);
	memcpy(outBuffer, data, dataSize);
	buffer->getDeviceMemory()->invalidateRange(0, dataSize);
	buffer->getDeviceMemory()->unmap();
	return true;
}
}

void takeScreenshot(Swapchain& swapChain, const uint32_t swapIndex, CommandPool& cmdPool, Queue& queue,
                    const std::string& screenshotFileName, const uint32_t screenshotScale)
{
	if (swapChain->supportsUsage(VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		// force the queue to wait idle prior to taking a copy of the swap chain image
		queue->waitIdle();

		saveImage(swapChain->getImage(swapIndex), VkImageLayout::e_PRESENT_SRC_KHR, VkImageLayout::e_PRESENT_SRC_KHR,
		          cmdPool, queue, screenshotFileName, screenshotScale);
	}
	else
	{
		Log(LogLevel::Warning, "Could not take screenshot as the swapchain does not support TRANSFER_SRC_BIT");
	}
}

void saveImage(Image image, const VkImageLayout imageInitialLayout, const VkImageLayout imageFinalLayout, CommandPool& cmdPool,
               Queue& queue, const std::string& filename, const uint32_t screenshotScale)
{
	const Extent2D dim(image->getWidth(), image->getHeight());
	const uint32_t stride = 4;
	std::vector<char> buffer(dim.width * dim.height * stride);
	if (screenCaptureRegion(image->getDevice(), image, cmdPool, queue,
	                        0, 0, dim.width, dim.height, buffer.data(), stride, image->getFormat(),
	                        imageInitialLayout, imageFinalLayout))
	{
		Log(LogLevel::Information, "Writing TGA screenshot, filename %s.", filename.c_str());
		writeTGA(filename.c_str(), dim.width, dim.height, reinterpret_cast<const unsigned char*>(buffer.data()), 4, screenshotScale);
	}
}

ImageUpdateResults updateImage(Device& device, CommandBufferBase cbuffTransfer,
                               ImageUpdateInfo* updateInfos, uint32_t numUpdateInfos, VkFormat format, VkImageLayout layout,
                               bool isCubeMap, Image& image, MemorySuballocator* bufferAllocator)
{
	debug_assertion(cbuffTransfer.isValid() && cbuffTransfer->isRecording(), "updateImage - Commandbuffer must be valid"
	                "and in recording state");
	ImageCleanupObject_* res = new ImageCleanupObject_();
	CleanupObject retval(res);
	uint32_t numFace = (isCubeMap ? 6 : 1);

	uint32_t hwSlice;
	std::vector<Buffer> stagingBuffers;

	struct BufferIteratorAdapter
	{
		std::vector<Buffer>::iterator myit;
	public:
		BufferIteratorAdapter(std::vector<Buffer>::iterator it) : myit(it) {}
		Buffer& operator*() { return *myit; }
		Buffer* operator->() { return &*myit; }
		BufferIteratorAdapter& operator++() { ++myit; return *this; }
		BufferIteratorAdapter operator++(int)
		{
			BufferIteratorAdapter ret(*this); ++(*this); return  ret;
		}
		bool operator!=(const BufferIteratorAdapter& rhs)
		{
			return myit != rhs.myit;
		}
		bool operator==(const BufferIteratorAdapter& rhs)
		{
			return myit == rhs.myit;
		}
	};

	{
		stagingBuffers.resize(numUpdateInfos);
		BufferImageCopy imgcp = {};

		for (uint32_t i = 0; i < numUpdateInfos; ++i)
		{
			const ImageUpdateInfo& mipLevelUpdate = updateInfos[i];
			assertion(mipLevelUpdate.data && mipLevelUpdate.dataSize, "Data and Data size must be valid");

			hwSlice = mipLevelUpdate.arrayIndex * numFace + mipLevelUpdate.cubeFace;

			// Will write the switch layout commands from the universal queue to the transfer queue to both the
			// transfer command buffer and the universal command buffer
			setImageLayoutAndQueueFamilyOwnership(
			  CommandBufferBase(), cbuffTransfer, static_cast<uint32_t>(-1), static_cast<uint32_t>(-1),
			  VkImageLayout::e_UNDEFINED, VkImageLayout::e_TRANSFER_DST_OPTIMAL,
			  image, mipLevelUpdate.mipLevel, 1, hwSlice, 1, inferAspectFromFormat(format));

			Buffer& buffer = stagingBuffers[i];
			// CREATE BUFFER WITHOUT MEMORY
			buffer = createBuffer(device, mipLevelUpdate.dataSize, VkBufferUsageFlags::e_TRANSFER_SRC_BIT, bufferAllocator ? VkMemoryPropertyFlags(0) : VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
			if (bufferAllocator)
			{
				auto suballoc = (*bufferAllocator)->suballocate(buffer->getMemoryRequirement().size);
				if (!suballoc.isValid())
				{
					Log("Failed to suballocate memory for staging buffer mip level %d slice %d", mipLevelUpdate.mipLevel, hwSlice);
					buffer.reset();
				}
				buffer->bindMemory(suballoc, suballoc->offset());
			}

			if (buffer.isNull())
			{
				Log("Failed to create staging buffer for mip level %d slice %d", mipLevelUpdate.mipLevel, hwSlice);
				return ImageUpdateResults();
			}

			imgcp.imageOffset = pvrvk::Offset3D(mipLevelUpdate.offsetX, mipLevelUpdate.offsetY, mipLevelUpdate.offsetZ);
			imgcp.imageExtent = pvrvk::Extent3D(mipLevelUpdate.imageWidth, mipLevelUpdate.imageHeight, 1);
			imgcp.imageSubresource.aspectMask = inferAspectFromFormat(format);
			imgcp.imageSubresource.baseArrayLayer = hwSlice;
			imgcp.imageSubresource.layerCount = 1;
			imgcp.imageSubresource.mipLevel = updateInfos[i].mipLevel;
			imgcp.bufferRowLength = mipLevelUpdate.dataWidth;
			imgcp.bufferImageHeight = mipLevelUpdate.dataHeight;

			const uint8_t* srcData;
			uint32_t srcDataSize;
			srcData = static_cast<const uint8_t*>(mipLevelUpdate.data);
			srcDataSize = mipLevelUpdate.dataSize;
			void* memory = 0;
			uint8_t* mappedData;
			if (buffer->getDeviceMemory()->map(&memory) != VkResult::e_SUCCESS)
			{
				Log("ImageUtils:updateImage Linear staging buffer Map Memory Failed");
				return ImageUpdateResults();
			}
			mappedData = static_cast<uint8_t*>(memory);
			if (mappedData == nullptr)
			{
				Log("ImageUtils:updateImage Linear staging buffer Map Memory Failed");
				return ImageUpdateResults();
			}
			for (uint32_t slice3d = 0; !slice3d || (slice3d < mipLevelUpdate.depth); ++slice3d)
			{
				memcpy(mappedData, srcData, srcDataSize);
				mappedData += srcDataSize;
				srcData += srcDataSize;
			}
			buffer->getDeviceMemory()->flushRange();
			buffer->getDeviceMemory()->unmap();

			cbuffTransfer->copyBufferToImage(buffer, image, VkImageLayout::e_TRANSFER_DST_OPTIMAL, 1, &imgcp);

			// CAUTION: We swapped src and dst queue families as, if there was no ownership transfer, no problem - queue families
			// will be ignored.
			// Will write the switch layout commands from the transfer queue to the universal queue to both the
			// transfer command buffer and the universal command buffer
			setImageLayoutAndQueueFamilyOwnership(cbuffTransfer, CommandBufferBase(),
			                                      static_cast<uint32_t>(-1), static_cast<uint32_t>(-1), VkImageLayout::e_TRANSFER_DST_OPTIMAL, layout, image, mipLevelUpdate.mipLevel, 1,
			                                      hwSlice, 1, inferAspectFromFormat(format));
		}
	}
	res->addBuffers(BufferIteratorAdapter(stagingBuffers.begin()), BufferIteratorAdapter(stagingBuffers.end()));
	return ImageUpdateResults(retval, image, pvr::Result::Success);
}

void create3dPlaneMesh(uint32_t width, uint32_t depth, bool generateTexCoords, bool generateNormalCoords, assets::Mesh& outMesh)
{
	const float halfWidth = width * .5f;
	const float halfDepth = depth * .5f;

	glm::vec3 normal[4] =
	{
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	};

	glm::vec2 texCoord[4] =
	{
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
	};

	glm::vec3 pos[4] =
	{
		glm::vec3(-halfWidth, 0.0f, -halfDepth),
		glm::vec3(-halfWidth, 0.0f, halfDepth),
		glm::vec3(halfWidth, 0.0f, halfDepth),
		glm::vec3(halfWidth, 0.0f, -halfDepth)
	};

	uint32_t indexData[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	float vertData[32];
	uint32_t offset = 0 ;

	for (uint32_t i = 0; i < 4; ++i)
	{
		memcpy(&vertData[offset], &pos[i], sizeof(pos[i]));
		offset += 3;
		if (generateNormalCoords)
		{
			memcpy(&vertData[offset], &normal[i], sizeof(normal[i]));
			offset += 3;
		}
		if (generateTexCoords)
		{
			memcpy(&vertData[offset], &texCoord[i], sizeof(texCoord[i]));
			offset += 2;
		}
	}

	uint32_t stride = sizeof(glm::vec3) + (generateNormalCoords ? sizeof(glm::vec3) : 0) + (generateTexCoords ? sizeof(glm::vec2) : 0);

	outMesh.addData(reinterpret_cast<const uint8_t*>(vertData), sizeof(vertData), stride, 0);
	outMesh.addFaces(reinterpret_cast<const uint8_t*>(indexData), sizeof(indexData), IndexType::IndexType32Bit);
	offset = 0;
	outMesh.addVertexAttribute("POSITION", DataType::Float32, 3, offset, 0);
	offset += sizeof(float) * 3;
	if (generateNormalCoords)
	{
		outMesh.addVertexAttribute("NORMAL", DataType::Float32, 3,  offset, 0);
		offset += sizeof(float) * 2;
	}
	if (generateTexCoords)
	{
		outMesh.addVertexAttribute("UV0", DataType::Float32, 2, offset, 0);
	}
	outMesh.setPrimitiveType(PrimitiveTopology::TriangleList);
	outMesh.setStride(0, stride);
	outMesh.setNumFaces(ARRAY_SIZE(indexData) / 3);
	outMesh.setNumVertices(ARRAY_SIZE(pos));
}
namespace {
inline static bool areQueueFamiliesSameOrInvalid(uint32_t lhs, uint32_t rhs)
{
	debug_assertion((lhs != -1 && rhs != -1) || (lhs == rhs),
	                "ImageUtilsVK(areQueueFamiliesSameOrInvalid): Only one queue family was valid. "
	                "Either both must be valid, or both must be ignored (-1)"); // Don't pass one non-null only...
	return lhs == rhs || lhs == -1 || rhs == -1;
}
inline static bool isMultiQueue(uint32_t queueFamilySrc, uint32_t queueFamilyDst)
{
	return !areQueueFamiliesSameOrInvalid(queueFamilySrc, queueFamilyDst);
}

inline VkAccessFlags getAccesFlagsFromLayout(VkImageLayout layout)
{
	switch (layout)
	{
	case VkImageLayout::e_GENERAL: return VkAccessFlags::e_SHADER_READ_BIT | VkAccessFlags::e_SHADER_WRITE_BIT | VkAccessFlags::e_COLOR_ATTACHMENT_READ_BIT | VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT;
	case VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL: return VkAccessFlags::e_COLOR_ATTACHMENT_READ_BIT | VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT;
	case VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case VkImageLayout::e_TRANSFER_DST_OPTIMAL: return VkAccessFlags::e_TRANSFER_WRITE_BIT;
	case VkImageLayout::e_TRANSFER_SRC_OPTIMAL: return VkAccessFlags::e_TRANSFER_READ_BIT;
	case VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL: return VkAccessFlags::e_SHADER_READ_BIT;
	case VkImageLayout::e_PRESENT_SRC_KHR: return VkAccessFlags::e_MEMORY_READ_BIT;
	case VkImageLayout::e_PREINITIALIZED: return VkAccessFlags::e_HOST_WRITE_BIT;
	default: return (VkAccessFlags)0;
	}
}
}

void setImageLayoutAndQueueFamilyOwnership(CommandBufferBase srccmd, CommandBufferBase dstcmd,
    uint32_t srcQueueFamily, uint32_t dstQueueFamily, VkImageLayout oldLayout,
    VkImageLayout newLayout, Image image, uint32_t baseMipLevel, uint32_t numMipLevels,
    uint32_t baseArrayLayer, uint32_t numArrayLayers, VkImageAspectFlags aspect)
{
	bool multiQueue = isMultiQueue(srcQueueFamily, dstQueueFamily);

	// No operation required: We don't have a layout transition, and we don't have a queue family change.
	if (newLayout == oldLayout && !multiQueue)
	{
		return;
	} // No transition required

	if (multiQueue)
	{
		assertion(srccmd.isValid() && dstcmd.isValid(),
		          "Vulkan Utils setImageLayoutAndQueueOwnership: An ownership change was required, "
		          "but at least one null command buffers was passed as parameters");
	}
	else
	{
		assertion(srccmd.isNull() || dstcmd.isNull(),
		          "Vulkan Utils setImageLayoutAndQueueOwnership: An ownership change was not required, "
		          "but two non-null command buffers were passed as parameters");
	}
	MemoryBarrierSet barriers;

	ImageMemoryBarrier imageMemBarrier;
	imageMemBarrier.oldLayout = oldLayout;
	imageMemBarrier.newLayout = newLayout;
	imageMemBarrier.image = image;
	imageMemBarrier.subresourceRange =
	  pvrvk::ImageSubresourceRange(aspect, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers);
	imageMemBarrier.srcQueueFamilyIndex = static_cast<uint32_t>(-1);
	imageMemBarrier.dstQueueFamilyIndex = static_cast<uint32_t>(-1);
	imageMemBarrier.srcAccessMask = getAccesFlagsFromLayout(oldLayout);
	imageMemBarrier.dstAccessMask = getAccesFlagsFromLayout(newLayout);

	// TODO - this needs checking - WORKAROUND - MAKE THE LAYERS SHUT UP (due to spec bug)
#if 1
	{
		if (multiQueue && newLayout != oldLayout)
		{
			barriers.addBarrier(imageMemBarrier);
			srccmd->pipelineBarrier(VkPipelineStageFlags::e_ALL_COMMANDS_BIT,
			                        VkPipelineStageFlags::e_ALL_COMMANDS_BIT, barriers, true);
		}
#endif

		if (multiQueue)
		{
			imageMemBarrier.srcQueueFamilyIndex = srcQueueFamily;
			imageMemBarrier.dstQueueFamilyIndex = dstQueueFamily;
		}
		barriers.clearAllBarriers();
		//Support any one of the command buffers being NOT null - either first or second is fine.
		if (srccmd.isValid())
		{
			barriers.addBarrier(imageMemBarrier);
			srccmd->pipelineBarrier(VkPipelineStageFlags::e_ALL_COMMANDS_BIT,
			                        VkPipelineStageFlags::e_ALL_COMMANDS_BIT, barriers, true);
		}
		if (dstcmd.isValid())
		{
			barriers.addBarrier(imageMemBarrier);
			dstcmd->pipelineBarrier(VkPipelineStageFlags::e_ALL_COMMANDS_BIT,
			                        VkPipelineStageFlags::e_ALL_COMMANDS_BIT, barriers, true);
		}
	}

}

/// <summary> Create instance and surface.
/// The instance will be created with the platform surface extension</summary>
/// <param name="numInstanceExtensions"> Additional instance extension to enable </param>
/// <param name="instanceLayerNames"> Layers to enable. By default all layers are enabled for DEBUG build</param>
/// <param name="numInstanceLayers"> Number of instance layers to enable</param>
/// <returns> Return true if success </returns>
bool createInstanceAndSurface(
  const std::string& applicationName, void* window, void* display, Instance& outInstance, Surface& outSurface,
  VulkanVersion version, const InstanceExtensions& instanceExtensions,
  const InstanceLayers& layers)
{
	InstanceCreateInfo instanceInfo;
	ApplicationInfo appInfo;
	instanceInfo.applicationInfo = &appInfo;
	appInfo.applicationVersion = 1;
	appInfo.applicationName = applicationName.c_str();
	appInfo.engineVersion = 0;
	appInfo.engineName = "PVRVulkan";
	appInfo.apiVersion = version.toVulkanVersion();
	instanceInfo.enabledExtensionNames = instanceExtensions.extensionStrings;
	instanceInfo.enabledLayerNames = layers.layersStrings;
	outInstance = pvrvk::createInstance(instanceInfo);
	if (outInstance.isNull()) { return false; }

	outSurface = outInstance->createSurface(outInstance->getPhysicalDevice(0), window, display);

	return !outSurface.isNull();
}
}
}
//!\endcond
