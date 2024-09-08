/*!
\brief Implementation for a number of functions defined in the HelperVk header file.
\file PVRUtils/Vulkan/HelperVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "HelperVk.h"
#include "PVRCore/texture/PVRTDecompress.h"
#include "PVRCore/textureio/TGAWriter.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/QueueVk.h"
#include "PVRVk/HeadersVk.h"
#include "PVRVk/SwapchainVk.h"
#include "PVRVk/MemoryBarrierVk.h"
#include "PVRUtils/Vulkan/MemoryAllocator.h"
#include "PVRVk/MemoryBarrierVk.h"
#include "PVRVk/DisplayVk.h"
#include "PVRVk/DisplayModeVk.h"
#include "pvr_openlib.h"

namespace pvr {
namespace utils {
#pragma region ////////////// BASIC HELPERS /////////////////

void getNumberOfPlanesFromFormat(pvrvk::Format format, uint32_t& numPlanes)
{
	// Read the number of planes
	std::string ycbcrFormat = to_string(format);
	numPlanes = 1;
	if (ycbcrFormat.find("2PLANE") != std::string::npos) { numPlanes = 2; }
	else if (ycbcrFormat.find("3PLANE") != std::string::npos)
	{
		numPlanes = 3;
	}
}

pvrvk::ImageAspectFlags inferAspectFromFormat(pvrvk::Format format, uint32_t planeIndex)
{
	pvrvk::ImageAspectFlags imageAspect = pvrvk::ImageAspectFlags::e_COLOR_BIT;

	if (format >= pvrvk::Format::e_D16_UNORM && format <= pvrvk::Format::e_D32_SFLOAT_S8_UINT)
	{
		const pvrvk::ImageAspectFlags aspects[] = {
			pvrvk::ImageAspectFlags::e_DEPTH_BIT | pvrvk::ImageAspectFlags::e_STENCIL_BIT, //  pvrvk::Format::e_D32_SFLOAT_S8_UINT
			pvrvk::ImageAspectFlags::e_DEPTH_BIT | pvrvk::ImageAspectFlags::e_STENCIL_BIT, //  pvrvk::Format::e_D24_UNORM_S8_UINT
			pvrvk::ImageAspectFlags::e_DEPTH_BIT | pvrvk::ImageAspectFlags::e_STENCIL_BIT, //  pvrvk::Format::e_D16_UNORM_S8_UINT
			pvrvk::ImageAspectFlags::e_STENCIL_BIT, //  pvrvk::Format::e_S8_UINT
			pvrvk::ImageAspectFlags::e_DEPTH_BIT, //  pvrvk::Format::e_D32_SFLOAT
			pvrvk::ImageAspectFlags::e_DEPTH_BIT, //  pvrvk::Format::e_X8_D24_UNORM_PACK32
			pvrvk::ImageAspectFlags::e_DEPTH_BIT, //  pvrvk::Format::e_D16_UNORM
		};
		// (Depthstenil format end) - format
		imageAspect = aspects[static_cast<uint32_t>(pvrvk::Format::e_D32_SFLOAT_S8_UINT) - static_cast<uint32_t>(format)];
	}

	uint32_t numPlanes = 1;
	getNumberOfPlanesFromFormat(format, numPlanes);
	if (numPlanes > 1)
	{
		switch (planeIndex)
		{
			case 0: return pvrvk::ImageAspectFlags::e_PLANE_0_BIT;
			case 1: return pvrvk::ImageAspectFlags::e_PLANE_1_BIT;
			case 2: return pvrvk::ImageAspectFlags::e_PLANE_2_BIT;
		}
	}

	return imageAspect;
}

void getColorBits(pvrvk::Format format, uint32_t& redBits, uint32_t& greenBits, uint32_t& blueBits, uint32_t& alphaBits)
{
	switch (format)
	{
	case pvrvk::Format::e_R8G8B8A8_SRGB:
	case pvrvk::Format::e_R8G8B8A8_UNORM:
	case pvrvk::Format::e_R8G8B8A8_SNORM:
	case pvrvk::Format::e_B8G8R8A8_UNORM:
	case pvrvk::Format::e_B8G8R8A8_SRGB:
		redBits = 8;
		greenBits = 8;
		blueBits = 8;
		alphaBits = 8;
		break;
	case pvrvk::Format::e_B8G8R8_SRGB:
	case pvrvk::Format::e_B8G8R8_UNORM:
	case pvrvk::Format::e_B8G8R8_SNORM:
	case pvrvk::Format::e_R8G8B8_SRGB:
	case pvrvk::Format::e_R8G8B8_UNORM:
	case pvrvk::Format::e_R8G8B8_SNORM:
		redBits = 8;
		greenBits = 8;
		blueBits = 8;
		alphaBits = 0;
		break;
	case pvrvk::Format::e_R5G6B5_UNORM_PACK16:
		redBits = 5;
		greenBits = 6;
		blueBits = 5;
		alphaBits = 0;
		break;
	case pvrvk::Format::e_A2B10G10R10_UNORM_PACK32:
		alphaBits = 2;
		blueBits = 10;
		greenBits = 10;
		redBits = 10;
		break;
	default: assertion(0, "UnSupported pvrvk::Format");
	}
}

void getDepthStencilBits(pvrvk::Format format, uint32_t& depthBits, uint32_t& stencilBits)
{
	switch (format)
	{
	case pvrvk::Format::e_D16_UNORM:
		depthBits = 16;
		stencilBits = 0;
		break;
	case pvrvk::Format::e_D16_UNORM_S8_UINT:
		depthBits = 16;
		stencilBits = 8;
		break;
	case pvrvk::Format::e_D24_UNORM_S8_UINT:
		depthBits = 24;
		stencilBits = 8;
		break;
	case pvrvk::Format::e_D32_SFLOAT:
		depthBits = 32;
		stencilBits = 0;
		break;
	case pvrvk::Format::e_D32_SFLOAT_S8_UINT:
		depthBits = 32;
		stencilBits = 8;
		break;
	case pvrvk::Format::e_X8_D24_UNORM_PACK32:
		depthBits = 24;
		stencilBits = 0;
		break;
	case pvrvk::Format::e_S8_UINT:
		depthBits = 0;
		stencilBits = 8;
		break;
	default: assertion(0, "UnSupported pvrvk::Format");
	}
}

pvrvk::ImageView uploadImageAndViewSubmit(pvrvk::Device& device, const Texture& texture, bool allowDecompress, pvrvk::CommandPool& cmdPool, pvrvk::Queue& queue,
	pvrvk::ImageUsageFlags usageFlags, pvrvk::ImageLayout finalLayout, vma::Allocator stagingBufferAllocator, vma::Allocator imageAllocator,
	vma::AllocationCreateFlags imageAllocationCreateFlags)
{
	pvrvk::CommandBuffer cmdBuffer = cmdPool->allocateCommandBuffer();
	cmdBuffer->begin();
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::uploadImageAndSubmit"));
	pvrvk::ImageView result =
		uploadImageAndView(device, texture, allowDecompress, cmdBuffer, usageFlags, finalLayout, stagingBufferAllocator, imageAllocator, imageAllocationCreateFlags);
	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	cmdBuffer->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cmdBuffer;
	submitInfo.numCommandBuffers = 1;
	pvrvk::Fence fence = device->createFence();
	queue->submit(&submitInfo, 1, fence);
	fence->wait();

	return result;
}

void create3dPlaneMesh(uint32_t width, uint32_t depth, bool generateTexCoords, bool generateNormalCoords, assets::Mesh& outMesh)
{
	const float halfWidth = width * .5f;
	const float halfDepth = depth * .5f;

	glm::vec3 normal[4] = { glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };

	glm::vec2 texCoord[4] = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
	};

	glm::vec3 pos[4] = { glm::vec3(-halfWidth, 0.0f, -halfDepth), glm::vec3(-halfWidth, 0.0f, halfDepth), glm::vec3(halfWidth, 0.0f, halfDepth), glm::vec3(halfWidth, 0.0f, -halfDepth) };

	uint32_t indexData[] = { 0, 1, 2, 0, 2, 3 };

	float vertData[32];
	uint32_t offset = 0;

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
		outMesh.addVertexAttribute("NORMAL", DataType::Float32, 3, offset, 0);
		offset += sizeof(float) * 2;
	}
	if (generateTexCoords) { outMesh.addVertexAttribute("UV0", DataType::Float32, 2, offset, 0); }
	outMesh.setPrimitiveType(PrimitiveTopology::TriangleList);
	outMesh.setStride(0, stride);
	outMesh.setNumFaces(ARRAY_SIZE(indexData) / 3);
	outMesh.setNumVertices(ARRAY_SIZE(pos));
}

#pragma endregion

#pragma region ////////////// LOCAL HELPERS /////////////////
namespace {
void decompressPvrtc(const Texture& texture, Texture& cDecompressedTexture)
{
	// Set up the new texture and header.
	TextureHeader cDecompressedHeader(texture);
	// robin: not sure what should happen here. The PVRTGENPIXELID4 macro is used in the old SDK.
	cDecompressedHeader.setPixelFormat(GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);

	cDecompressedHeader.setChannelType(VariableType::UnsignedByteNorm);
	cDecompressedTexture = Texture(cDecompressedHeader);

	// Do decompression, one surface at a time.
	for (uint32_t uiMipMapLevel = 0; uiMipMapLevel < texture.getNumMipMapLevels(); ++uiMipMapLevel)
	{
		for (uint32_t uiArray = 0; uiArray < texture.getNumArrayMembers(); ++uiArray)
		{
			for (uint32_t uiFace = 0; uiFace < texture.getNumFaces(); ++uiFace)
			{
				PVRTDecompressPVRTC(texture.getDataPointer(uiMipMapLevel, uiArray, uiFace), (texture.getBitsPerPixel() == 2 ? 1 : 0), texture.getWidth(uiMipMapLevel),
					texture.getHeight(uiMipMapLevel), cDecompressedTexture.getDataPointer(uiMipMapLevel, uiArray, uiFace));
			}
		}
	}
}

inline pvrvk::AccessFlags getAccesFlagsFromLayout(pvrvk::ImageLayout layout)
{
	switch (layout)
	{
	case pvrvk::ImageLayout::e_GENERAL:
		return pvrvk::AccessFlags::e_SHADER_READ_BIT | pvrvk::AccessFlags::e_SHADER_WRITE_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT |
			pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT;
	case pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL: return pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT;
	case pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL: return pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT;
	case pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL: return pvrvk::AccessFlags::e_TRANSFER_READ_BIT;
	case pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL: return pvrvk::AccessFlags::e_SHADER_READ_BIT;
	case pvrvk::ImageLayout::e_PRESENT_SRC_KHR: return pvrvk::AccessFlags::e_MEMORY_READ_BIT;
	case pvrvk::ImageLayout::e_PREINITIALIZED: return pvrvk::AccessFlags::e_HOST_WRITE_BIT;
	default: return (pvrvk::AccessFlags)0;
	}
}

inline pvrvk::PipelineStageFlags getPipelineStageFlagsFromLayout(pvrvk::ImageLayout layout)
{
	// Image memory barriers require the correct pipeline stage flags to be set for the access mask
	// However, when using the function getAccesFlagsFromLayout() above, the access flags are determined by layout
	// This means that the stageflags can also be determined by the layout

	// Decide the flags that would be trigger for any shader read or write.
	pvrvk::PipelineStageFlags shaderReadWrite = pvrvk::PipelineStageFlags::e_VERTEX_SHADER_BIT | pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT |
		pvrvk::PipelineStageFlags::e_RAY_TRACING_SHADER_BIT_KHR | pvrvk::PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	switch (layout)
	{
	case pvrvk::ImageLayout::e_UNDEFINED: return pvrvk::PipelineStageFlags::e_TOP_OF_PIPE_BIT;
	case pvrvk::ImageLayout::e_GENERAL: return pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT | shaderReadWrite;
	case pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL: return pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	case pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return pvrvk::PipelineStageFlags::e_EARLY_FRAGMENT_TESTS_BIT | pvrvk::PipelineStageFlags::e_LATE_FRAGMENT_TESTS_BIT;
	case pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL: return pvrvk::PipelineStageFlags::e_TRANSFER_BIT | pvrvk::PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	case pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL: return pvrvk::PipelineStageFlags::e_TRANSFER_BIT | pvrvk::PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	case pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL: return shaderReadWrite;
	case pvrvk::ImageLayout::e_PRESENT_SRC_KHR: return pvrvk::PipelineStageFlags::e_TOP_OF_PIPE_BIT;
	case pvrvk::ImageLayout::e_PREINITIALIZED: return pvrvk::PipelineStageFlags::e_HOST_BIT;
	default: return pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT;
	}
}

inline pvrvk::Format getDepthStencilFormat(const DisplayAttributes& displayAttribs)
{
	uint32_t depthBpp = displayAttribs.depthBPP;
	uint32_t stencilBpp = displayAttribs.stencilBPP;

	pvrvk::Format dsFormat = pvrvk::Format::e_UNDEFINED;

	if (stencilBpp)
	{
		switch (depthBpp)
		{
		case 0: dsFormat = pvrvk::Format::e_S8_UINT; break;
		case 16: dsFormat = pvrvk::Format::e_D16_UNORM_S8_UINT; break;
		case 24: dsFormat = pvrvk::Format::e_D24_UNORM_S8_UINT; break;
		case 32: dsFormat = pvrvk::Format::e_D32_SFLOAT_S8_UINT; break;
		default: assertion(false, "Unsupported Depth Stencil pvrvk::Format");
		}
	}
	else
	{
		switch (depthBpp)
		{
		case 16: dsFormat = pvrvk::Format::e_D16_UNORM; break;
		case 24: dsFormat = pvrvk::Format::e_X8_D24_UNORM_PACK32; break;
		case 32: dsFormat = pvrvk::Format::e_D32_SFLOAT; break;
		default: assertion(false, "Unsupported Depth Stencil pvrvk::Format");
		}
	}
	return dsFormat;
}

// Check a list of formats against the display attributes. Will return the first item that matches. If no items match, will return false and outFormat will be unmodified.
// If both matchColorspace and matchBpp are false, will return the first item of the list. Of course if the list is empty will always return false.
bool checkFormatListAgainstUserPreferences(
	const std::vector<pvrvk::SurfaceFormatKHR>& list, const pvr::DisplayAttributes& displayAttributes, bool matchColorspace, bool matchBpp, pvrvk::SurfaceFormatKHR& outFormat)
{
	for (auto&& sfmt : list)
	{
		pvrvk::Format format = sfmt.getFormat();
		if (matchColorspace)
		{
			if (displayAttributes.frameBufferSrgb != isSrgb(format)) { continue; }
		}
		if (matchBpp)
		{
			uint32_t currentRedBpp, currentGreenBpp, currentBlueBpp, currentAlphaBpp = 0;
			getColorBits(format, currentRedBpp, currentGreenBpp, currentBlueBpp, currentAlphaBpp);
			if (currentRedBpp != displayAttributes.redBits || displayAttributes.greenBits != currentGreenBpp || displayAttributes.blueBits != currentBlueBpp ||
				displayAttributes.alphaBits != currentAlphaBpp)
			{
				continue;
			}
		}
		outFormat = sfmt;
		return true; // This loop will exit as soon as any item passes all of the enabled tests (matching colorspace and or matching bpp).
	}
	return false;
}

pvrvk::SurfaceFormatKHR findSwapchainFormat(
	const std::vector<pvrvk::SurfaceFormatKHR>& supportedFormats, pvr::DisplayAttributes& displayAttributes, const std::vector<pvrvk::Format>& preferredColorFormats)
{
	Log(LogLevel::Information, "Supported Swapchain surface device formats:");
	for (auto&& format : supportedFormats)
	{
		Log(LogLevel::Information, "\tFormat:     %-30s  Colorspace: %s", to_string(format.getFormat()).c_str(), to_string(format.getColorSpace()).c_str());
	}

	pvrvk::SurfaceFormatKHR swapchainFormat;

	std::vector<pvrvk::Format> preferredLinearFormats;
	std::vector<pvrvk::Format> preferredSrgbFormats;

	if (preferredColorFormats.size())
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(preferredColorFormats.size()); ++i)
		{
			if (pvrvk::isSrgb(preferredColorFormats[i])) { preferredSrgbFormats.emplace_back(preferredColorFormats[i]); }
			else
			{
				preferredLinearFormats.emplace_back(preferredColorFormats[i]);
			}
		}
	}
	else
	{
		pvrvk::Format frameworkPreferredLinearFormats[] = { pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Format::e_B8G8R8A8_UNORM, pvrvk::Format::e_R5G6B5_UNORM_PACK16,
			pvrvk::Format::e_UNDEFINED };
		pvrvk::Format frameworkPreferredSrgbFmts[] = { pvrvk::Format::e_R8G8B8A8_SRGB, pvrvk::Format::e_B8G8R8A8_SRGB, pvrvk::Format::e_A8B8G8R8_SRGB_PACK32, pvrvk::Format::e_UNDEFINED };

		preferredLinearFormats.insert(
			preferredLinearFormats.begin(), &frameworkPreferredLinearFormats[0], &frameworkPreferredLinearFormats[ARRAY_SIZE(frameworkPreferredLinearFormats)]);
		preferredSrgbFormats.insert(preferredSrgbFormats.begin(), &frameworkPreferredSrgbFmts[0], &frameworkPreferredSrgbFmts[ARRAY_SIZE(frameworkPreferredSrgbFmts)]);
	}

	std::vector<pvrvk::SurfaceFormatKHR> supportedPreferredLinearFmts;
	std::vector<pvrvk::SurfaceFormatKHR> supportedPreferredSrgbFmts;

	// Our "preferred" formats are typical formats that are widely supported and optimal. Keep two lists: One for Linear formats...
	for (auto&& pfmt : preferredLinearFormats)
	{
		for (auto&& sfmt : supportedFormats)
		{
			if (sfmt.getFormat() == pfmt) { supportedPreferredLinearFmts.emplace_back(sfmt); }
		}
	}

	// ... and one for SRGB formats.
	for (auto&& pfmt : preferredSrgbFormats)
	{
		for (auto&& sfmt : supportedFormats)
		{
			if (sfmt.getFormat() == pfmt) { supportedPreferredSrgbFmts.emplace_back(sfmt); }
		}
	}

	// Order of checks for Device support:
	// 1. Preferred format that matches the user's preferred Colorspace(CS) and Bits per Pixel(BPP)
	// 2. Any format that matches the user's preferred CS and BPP
	// 3. Preferred format that matches the user's preferred CS
	// 4. Any format that matches the user's preferred CS
	// 5. Preferred format that matches the user's BPP
	// 6. Any format that matches the user's preferred BPP
	// 7. Any Preferred format
	// 8. Any format
	// 9. Fail (no supported device formats)

	auto& preferredFormats = displayAttributes.frameBufferSrgb ? supportedPreferredSrgbFmts : supportedPreferredLinearFmts;

	// 1. Preferred format that matches the user's preferred Colorspace(CS) and Bits per Pixel(BPP)
	bool found = checkFormatListAgainstUserPreferences(preferredFormats, displayAttributes, true, true, swapchainFormat);

	if (!found)
	{
		// 2. Any format that matches the user's preferred CS and BPP
		Log(LogLevel::Information, "Requested swapchain format did not match any of the default preferred formats(RGBA8/BGRA8). This is not an error.");
		found = checkFormatListAgainstUserPreferences(supportedFormats, displayAttributes, true, true, swapchainFormat);
	}

	if (!found)
	{
		Log(LogLevel::Warning,
			"Requested swapchain format could not be found with the requested parameters: [R:%d G:%d B:%d A:%d %s colorspace]. Will attempt to find "
			"another supported frambebuffer format.",
			displayAttributes.redBits, displayAttributes.greenBits, displayAttributes.blueBits, displayAttributes.alphaBits, displayAttributes.frameBufferSrgb ? "SRGB" : "Linear");

		if (displayAttributes.forceColorBPP)
		{
			Log(LogLevel::Information,
				"Color Bits per pixel has been forced in user preferences. Will only attempt to find color formats exactly matching the provided color bits configuration.");
		}
		else
		{
			// 3. Preferred format that matches the user's preferred CS
			found = checkFormatListAgainstUserPreferences(preferredFormats, displayAttributes, true, false, swapchainFormat);
			if (!found)
			{
				// 4. Any format that matches the user's preferred CS
				found = checkFormatListAgainstUserPreferences(supportedFormats, displayAttributes, true, false, swapchainFormat);
			}
		}
	}

	// This case will only be hit in the unusual case where the user's platform does not support any format with the requested colorspace type.
	// The only feasible scenario is the user requesting an SRGB framebuffer and the platform does not support any srgb framebuffer.
	if (!found)
	{
		Log(LogLevel::Warning, "Could not find any %s framebuffer format. Will attempt to provide a %s framebuffer matching the requested color bits.",
			displayAttributes.frameBufferSrgb ? "SRGB" : "Linear", displayAttributes.frameBufferSrgb ? "Linear" : "SRGB");

		// 5. Preferred format that matches the user's BPP
		found = checkFormatListAgainstUserPreferences(supportedPreferredLinearFmts, displayAttributes, false, true, swapchainFormat);
		if (!found)
		{
			// Still 5. Preferred format that matches the user's BPP
			found = checkFormatListAgainstUserPreferences(supportedPreferredSrgbFmts, displayAttributes, false, true, swapchainFormat);
		}
		if (!found)
		{
			// 6. Any format that matches the user's preferred BPP
			found = checkFormatListAgainstUserPreferences(supportedFormats, displayAttributes, false, true, swapchainFormat);
		}
	}

	// This case will, finally, be hit if both the requested colorspace could not be matched, AND their requested BPP could not be matched. At this point, we
	// will ignore all user's preferences and just try to give him ANY framebuffer.
	if (!found && !displayAttributes.forceColorBPP)
	{
		Log(LogLevel::Warning, "Could not find any formats matching either the requested colorspace, or the requested bits per pixel. Will attemt to provide ANY supported framebuffer.");

		// 7. Any Preferred format
		found = checkFormatListAgainstUserPreferences(supportedPreferredSrgbFmts, displayAttributes, false, false, swapchainFormat);
		found = checkFormatListAgainstUserPreferences(supportedPreferredLinearFmts, displayAttributes, false, false, swapchainFormat);
		// 8. Any format
		found = checkFormatListAgainstUserPreferences(supportedFormats, displayAttributes, false, false, swapchainFormat);
	}

	if (!found)
	{
		// 9. Fail (no supported device formats)
		if (displayAttributes.forceColorBPP)
		{
			throw InvalidOperationError("Could not find any supported framebuffer with the requested bit depth of R:" + std::to_string(displayAttributes.redBits) +
				" G:" + std::to_string(displayAttributes.greenBits) + " B:" + std::to_string(displayAttributes.blueBits) + " A:" + std::to_string(displayAttributes.alphaBits));
		}
		else
		{
			throw InvalidOperationError("Could not find any supported framebuffers. Check that Vulkan implementation and drivers are correctly installed.");
		}
	}
	Log(LogLevel::Information, "Successfully accepted format: %s Colorspace: %s", pvrvk::to_string(swapchainFormat.getFormat()).c_str(),
		pvrvk::to_string(swapchainFormat.getColorSpace()).c_str());
	return swapchainFormat;
}

pvrvk::Swapchain createSwapchainHelper(const pvrvk::Device& device, const pvrvk::Surface& surface, pvr::DisplayAttributes& displayAttributes,
	const pvrvk::ImageUsageFlags& swapchainImageUsageFlags, const std::vector<pvrvk::Format>& preferredColorFormats = std::vector<pvrvk::Format>())
{
	Log(LogLevel::Information, "Creating Vulkan Swapchain using pvr::DisplayAttributes");

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = device->getPhysicalDevice()->getSurfaceCapabilities(surface);

	Log(LogLevel::Information, "Queried Surface Capabilities:");
	Log(LogLevel::Information, "\tMinimum Image count: %u", surfaceCapabilities.getMinImageCount());
	Log(LogLevel::Information, "\tMaximum Image count: %u", surfaceCapabilities.getMaxImageCount());
	Log(LogLevel::Information, "\tMaximum Image Array Layers: %u", surfaceCapabilities.getMaxImageArrayLayers());
	Log(LogLevel::Information, "\tImage size (now): %ux%u", surfaceCapabilities.getCurrentExtent().getWidth(), surfaceCapabilities.getCurrentExtent().getHeight());
	Log(LogLevel::Information, "\tMinimum Image extent: %dx%d", surfaceCapabilities.getMinImageExtent().getWidth(), surfaceCapabilities.getMinImageExtent().getHeight());
	Log(LogLevel::Information, "\tMaximum Image extent: %dx%d", surfaceCapabilities.getMaxImageExtent().getWidth(), surfaceCapabilities.getMaxImageExtent().getHeight());
	Log(LogLevel::Information, "\tSupported Usage Flags: %s", pvrvk::to_string(surfaceCapabilities.getSupportedUsageFlags()).c_str());
	Log(LogLevel::Information, "\tCurrent transform: %s", pvrvk::to_string(surfaceCapabilities.getCurrentTransform()).c_str());
	Log(LogLevel::Information, "\tSupported transforms: %s", pvrvk::to_string(surfaceCapabilities.getSupportedTransforms()).c_str());
	Log(LogLevel::Information, "\tComposite Alpha Flags: %s", pvrvk::to_string(surfaceCapabilities.getSupportedCompositeAlpha()).c_str());

	uint32_t usedWidth = surfaceCapabilities.getCurrentExtent().getWidth();
	uint32_t usedHeight = surfaceCapabilities.getCurrentExtent().getHeight();
#if !defined(ANDROID)
	usedWidth = std::max<uint32_t>(surfaceCapabilities.getMinImageExtent().getWidth(), std::min<uint32_t>(displayAttributes.width, surfaceCapabilities.getMaxImageExtent().getWidth()));

	usedHeight =
		std::max<uint32_t>(surfaceCapabilities.getMinImageExtent().getHeight(), std::min<uint32_t>(displayAttributes.height, surfaceCapabilities.getMaxImageExtent().getHeight()));
#endif
	// Log modifications made to the surface properties set via DisplayAttributes
	Log(LogLevel::Information, "Modified Surface Properties after inspecting DisplayAttributes:");

	displayAttributes.width = usedWidth;
	displayAttributes.height = usedHeight;

	Log(LogLevel::Information, "\tImage size to be used: %dx%d", displayAttributes.width, displayAttributes.height);

	std::vector<pvrvk::SurfaceFormatKHR> surfaceFormats = device->getPhysicalDevice()->getSurfaceFormats(surface);

	pvrvk::SurfaceFormatKHR imageFormat = findSwapchainFormat(device->getPhysicalDevice()->getSurfaceFormats(surface), displayAttributes, preferredColorFormats);

	// update the display attributes
	displayAttributes.frameBufferSrgb = pvrvk::isSrgb(imageFormat.getFormat());

	std::vector<pvrvk::PresentModeKHR> surfacePresentationModes = device->getPhysicalDevice()->getSurfacePresentModes(surface);

	// With VK_PRESENT_MODE_FIFO_KHR the presentation engine will wait for the next vblank (vertical blanking period) to update the current image. When using FIFO tearing
	// cannot occur. VK_PRESENT_MODE_FIFO_KHR is required to be supported.
	pvrvk::PresentModeKHR swapchainPresentMode = pvrvk::PresentModeKHR::e_FIFO_KHR;
	pvrvk::PresentModeKHR desiredSwapMode = pvrvk::PresentModeKHR::e_FIFO_KHR;

	// We make use of PVRShell for handling command line arguments for configuring vsync modes using the -vsync command line argument.
	switch (displayAttributes.vsyncMode)
	{
	case VsyncMode::Off:
		Log(LogLevel::Information, "Requested presentation mode: Immediate (VsyncMode::Off)");
		desiredSwapMode = pvrvk::PresentModeKHR::e_IMMEDIATE_KHR;
		break;
	case VsyncMode::Mailbox:
		Log(LogLevel::Information, "Requested presentation mode: Mailbox (VsyncMode::Mailbox)");
		desiredSwapMode = pvrvk::PresentModeKHR::e_MAILBOX_KHR;
		break;
	case VsyncMode::Relaxed:
		Log(LogLevel::Information, "Requested presentation mode: Relaxed (VsyncMode::Relaxed)");
		desiredSwapMode = pvrvk::PresentModeKHR::e_FIFO_RELAXED_KHR;
		break;
		// Default vsync mode
	case pvr::VsyncMode::On: Log(LogLevel::Information, "Requested presentation mode: Fifo (VsyncMode::On)"); break;
	case pvr::VsyncMode::Half: Log(LogLevel::Information, "Unsupported presentation mode requested: Half. Defaulting to PresentModeKHR::e_FIFO_KHR");
	}
	std::string supported = "Supported presentation modes: ";
	for (size_t i = 0; i < surfacePresentationModes.size(); i++) { supported += (to_string(surfacePresentationModes[i]) + " "); }
	Log(LogLevel::Information, supported.c_str());
	for (size_t i = 0; i < surfacePresentationModes.size(); i++)
	{
		pvrvk::PresentModeKHR currentPresentMode = surfacePresentationModes[i];

		// Primary matches : Check for a precise match between the desired presentation mode and the presentation modes supported.
		if (currentPresentMode == desiredSwapMode)
		{
			swapchainPresentMode = desiredSwapMode;
			break;
		}
		// Secondary matches : Immediate and Mailbox are better fits for each other than FIFO, so set them as secondaries
		// If the user asked for Mailbox, and we found Immediate, set it (in case Mailbox is not found) and keep looking
		if ((desiredSwapMode == pvrvk::PresentModeKHR::e_MAILBOX_KHR) && (currentPresentMode == pvrvk::PresentModeKHR::e_IMMEDIATE_KHR))
		{
			swapchainPresentMode = pvrvk::PresentModeKHR::e_IMMEDIATE_KHR;
		}
		// ... And vice versa: If the user asked for Immediate, and we found Mailbox, set it (in case Immediate is not found) and keep looking
		if ((desiredSwapMode == pvrvk::PresentModeKHR::e_IMMEDIATE_KHR) && (currentPresentMode == pvrvk::PresentModeKHR::e_MAILBOX_KHR))
		{
			swapchainPresentMode = pvrvk::PresentModeKHR::e_MAILBOX_KHR;
		}
	}
	switch (swapchainPresentMode)
	{
	case pvrvk::PresentModeKHR::e_IMMEDIATE_KHR: Log(LogLevel::Information, "Presentation mode: Immediate (Vsync OFF)"); break;
	case pvrvk::PresentModeKHR::e_MAILBOX_KHR: Log(LogLevel::Information, "Presentation mode: Mailbox (Triple-buffering)"); break;
	case pvrvk::PresentModeKHR::e_FIFO_KHR: Log(LogLevel::Information, "Presentation mode: FIFO (Vsync ON)"); break;
	case pvrvk::PresentModeKHR::e_FIFO_RELAXED_KHR: Log(LogLevel::Information, "Presentation mode: Relaxed FIFO (Relaxed Vsync)"); break;
	default: assertion(false, "Unrecognised presentation mode"); break;
	}

	// Set the swapchain length if it has not already been set.
	if (!displayAttributes.swapLength) { displayAttributes.swapLength = 3; }

	// Check for a supported composite alpha value in a predefined order
	pvrvk::CompositeAlphaFlagsKHR supportedCompositeAlphaFlags = pvrvk::CompositeAlphaFlagsKHR::e_NONE;
	if ((surfaceCapabilities.getSupportedCompositeAlpha() & pvrvk::CompositeAlphaFlagsKHR::e_OPAQUE_BIT_KHR) != 0)
	{
		supportedCompositeAlphaFlags = pvrvk::CompositeAlphaFlagsKHR::e_OPAQUE_BIT_KHR;
	}
	else if ((surfaceCapabilities.getSupportedCompositeAlpha() & pvrvk::CompositeAlphaFlagsKHR::e_INHERIT_BIT_KHR) != 0)
	{
		supportedCompositeAlphaFlags = pvrvk::CompositeAlphaFlagsKHR::e_INHERIT_BIT_KHR;
	}

	pvrvk::SwapchainCreateInfo createInfo;
	createInfo.clipped = true;
	createInfo.compositeAlpha = supportedCompositeAlphaFlags;
	createInfo.surface = surface;

	displayAttributes.swapLength = std::min<uint32_t>(static_cast<uint32_t>(displayAttributes.swapLength), surfaceCapabilities.getMinImageCount());
	if (surfaceCapabilities.getMaxImageCount()) { displayAttributes.swapLength = std::min<uint32_t>(displayAttributes.swapLength, surfaceCapabilities.getMaxImageCount()); }

	createInfo.minImageCount = displayAttributes.swapLength;
	createInfo.imageFormat = imageFormat.getFormat();

	createInfo.imageArrayLayers = 1;
	createInfo.imageColorSpace = imageFormat.getColorSpace();
	createInfo.imageExtent.setWidth(displayAttributes.width);
	createInfo.imageExtent.setHeight(displayAttributes.height);
	createInfo.imageUsage = swapchainImageUsageFlags;

	createInfo.preTransform = pvrvk::SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR;
	if ((surfaceCapabilities.getSupportedTransforms() & pvrvk::SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR) == 0)
	{
		throw InvalidOperationError("Surface does not support VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR transformation");
	}
	createInfo.imageSharingMode = pvrvk::SharingMode::e_EXCLUSIVE;
	createInfo.presentMode = swapchainPresentMode;
	createInfo.numQueueFamilyIndex = 1;
	uint32_t queueFamily = 0;
	createInfo.queueFamilyIndices = &queueFamily;

	pvrvk::Swapchain swapchain;
	swapchain = device->createSwapchain(createInfo, surface);
	displayAttributes.swapLength = swapchain->getSwapchainLength();

	Log(LogLevel::Information, "Swapchain length: %i", displayAttributes.swapLength);

	return swapchain;
}

inline static bool areQueueFamiliesSameOrInvalid(uint32_t lhs, uint32_t rhs)
{
	debug_assertion((lhs != static_cast<uint32_t>(-1) && rhs != static_cast<uint32_t>(-1)) || (lhs == rhs),
		"ImageUtilsVK(areQueueFamiliesSameOrInvalid): Only one queue family was valid. "
		"Either both must be valid, or both must be ignored (-1)"); // Don't pass one non-null only...
	return lhs == rhs || lhs == uint32_t(-1) || rhs == uint32_t(-1);
}
inline static bool isMultiQueue(uint32_t queueFamilySrc, uint32_t queueFamilyDst) { return !areQueueFamiliesSameOrInvalid(queueFamilySrc, queueFamilyDst); }
} // namespace
#pragma endregion

#pragma region ///////////////// INTERNALS //////////////////
namespace impl {
inline bool isSupportedFormat(const pvrvk::PhysicalDevice& pdev, pvrvk::Format fmt)
{
	pvrvk::FormatProperties props = pdev->getFormatProperties(fmt);
	return (props.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_SAMPLED_IMAGE_BIT) != 0;
}

const Texture* decompressIfRequired(
	const Texture& texture, Texture& decompressedTexture, const pvrvk::PhysicalDevice& pdev, bool allowDecompress, pvrvk::Format& outFormat, bool& isDecompressed)
{
	const char* cszUnsupportedFormat = "Texture format is not supported in this implementation.\n";
	const char* cszUnsupportedFormatDecompressionAvailable = "Texture format is not supported in this implementation."
															 " Allowing software decompression (allowDecompress=true) will enable you to use this format.\n";
	outFormat = convertToPVRVkPixelFormat(texture.getPixelFormat(), texture.getColorSpace(), texture.getChannelType(), isDecompressed);

	if (isSupportedFormat(pdev, outFormat))
	{
		isDecompressed = false;
		return &texture;
	}
	else
	{
		if (texture.getPixelFormat().getPixelTypeId() >= uint64_t(CompressedPixelFormat::PVRTCI_2bpp_RGB) &&
			texture.getPixelFormat().getPixelTypeId() <= uint64_t(CompressedPixelFormat::PVRTCI_4bpp_RGBA))
		{
			if (allowDecompress)
			{
				Log(LogLevel::Information,
					"PVRTC texture format support not detected. Decompressing PVRTC to"
					" corresponding format (RGBA32 or RGB24)");
				decompressPvrtc(texture, decompressedTexture);
				isDecompressed = true;
				outFormat = convertToPVRVkPixelFormat(decompressedTexture.getPixelFormat(), decompressedTexture.getColorSpace(), decompressedTexture.getChannelType(), isDecompressed);
				return &decompressedTexture;
			}
			else
			{
				throw TextureDecompressionError(cszUnsupportedFormatDecompressionAvailable, "PVRTC");
			}
		}
		throw TextureDecompressionError(cszUnsupportedFormat, to_string(texture.getPixelFormat()));
	}
}
} // namespace impl
#pragma endregion

#pragma region //////// IMAGE UPLOADING AND UPDATING ////////
bool isSupportedFormat(const pvrvk::PhysicalDevice& pdev, pvrvk::Format fmt)
{
	pvrvk::FormatProperties props = pdev->getFormatProperties(fmt);
	return (props.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_SAMPLED_IMAGE_BIT) != 0;
}

void setImageLayoutAndQueueFamilyOwnership(pvrvk::CommandBufferBase srccmd, pvrvk::CommandBufferBase dstcmd, uint32_t srcQueueFamily, uint32_t dstQueueFamily,
	pvrvk::ImageLayout oldLayout, pvrvk::ImageLayout newLayout, pvrvk::Image& image, uint32_t baseMipLevel, uint32_t numMipLevels, uint32_t baseArrayLayer, uint32_t numArrayLayers,
	pvrvk::ImageAspectFlags aspect)
{
	bool multiQueue = isMultiQueue(srcQueueFamily, dstQueueFamily);

	// No operation required: We don't have a layout transition, and we don't have a queue family change.
	if (newLayout == oldLayout && !multiQueue) { return; } // No transition required

	if (multiQueue)
	{
		assertion(srccmd && dstcmd,
			"Vulkan Utils setImageLayoutAndQueueOwnership: An ownership change was required, "
			"but at least one null command buffers was passed as parameters");
	}
	else
	{
		assertion(srccmd || dstcmd,
			"Vulkan Utils setImageLayoutAndQueueOwnership: An ownership change was not required, "
			"but two non-null command buffers were passed as parameters");
	}
	pvrvk::MemoryBarrierSet barriers;

	pvrvk::ImageMemoryBarrier imageMemBarrier;
	imageMemBarrier.setOldLayout(oldLayout);
	imageMemBarrier.setNewLayout(newLayout);
	imageMemBarrier.setImage(image);
	imageMemBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(aspect, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers));
	imageMemBarrier.setSrcQueueFamilyIndex(static_cast<uint32_t>(-1));
	imageMemBarrier.setDstQueueFamilyIndex(static_cast<uint32_t>(-1));
	imageMemBarrier.setSrcAccessMask(getAccesFlagsFromLayout(oldLayout));
	imageMemBarrier.setDstAccessMask(getAccesFlagsFromLayout(newLayout));

	if (multiQueue)
	{
		imageMemBarrier.setSrcQueueFamilyIndex(srcQueueFamily);
		imageMemBarrier.setDstQueueFamilyIndex(dstQueueFamily);
	}

	barriers.clearAllBarriers();
	// Support any one of the command buffers being NOT null - either first or second is fine.
	if (srccmd)
	{
		barriers.addBarrier(imageMemBarrier);
		srccmd->pipelineBarrier(getPipelineStageFlagsFromLayout(oldLayout), getPipelineStageFlagsFromLayout(newLayout), barriers, true);
	}
	if (dstcmd)
	{
		barriers.addBarrier(imageMemBarrier);
		dstcmd->pipelineBarrier(getPipelineStageFlagsFromLayout(oldLayout), getPipelineStageFlagsFromLayout(newLayout), barriers, true);
	}
} // namespace utils
pvrvk::Image uploadImageHelper(pvrvk::Device& device, const Texture& texture, bool allowDecompress, pvrvk::CommandBufferBase commandBuffer, pvrvk::ImageUsageFlags usageFlags,
	pvrvk::ImageLayout finalLayout, vma::Allocator bufferAllocator = nullptr, vma::Allocator imageAllocator = nullptr,
	vma::AllocationCreateFlags imageAllocationCreateFlags = vma::AllocationCreateFlags::e_NONE)
{
	// Check that the texture is valid.
	if (!texture.getDataSize()) { throw pvrvk::ErrorValidationFailedEXT("TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs."); }
	pvr::utils::beginCommandBufferDebugLabel(commandBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::uploadImage"));
	bool isDecompressed;

	pvrvk::Format format = pvrvk::Format::e_UNDEFINED;

	// Texture to use if we decompress in software.
	Texture decompressedTexture;

	// Texture pointer which points at the texture we should use for the function.
	// Allows switching to, for example, a decompressed version of the texture.
	const Texture* textureToUse = impl::decompressIfRequired(texture, decompressedTexture, device->getPhysicalDevice(), allowDecompress, format, isDecompressed);

	if (format == pvrvk::Format::e_UNDEFINED) { pvrvk::ErrorUnknown("TextureUtils.h:textureUpload:: Texture's pixel type is not supported by this API."); }

	uint32_t texWidth = static_cast<uint32_t>(textureToUse->getWidth());
	uint32_t texHeight = static_cast<uint32_t>(textureToUse->getHeight());
	uint32_t texDepth = static_cast<uint32_t>(textureToUse->getDepth());

	uint32_t dataWidth = static_cast<uint32_t>(textureToUse->getWidth());
	uint32_t dataHeight = static_cast<uint32_t>(textureToUse->getHeight());

	uint16_t texMipLevels = static_cast<uint16_t>(textureToUse->getNumMipMapLevels());
	uint16_t texArraySlices = static_cast<uint16_t>(textureToUse->getNumArrayMembers());
	uint16_t texFaces = static_cast<uint16_t>(textureToUse->getNumFaces());
	uint16_t texPlanes = static_cast<uint16_t>(textureToUse->getNumPlanes());
	pvrvk::Image image;

	usageFlags |= pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT;

	if (texDepth > 1)
	{
		image = createImage(device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_3D, format, pvrvk::Extent3D(texWidth, texHeight, texDepth), usageFlags, static_cast<uint8_t>(texMipLevels), texArraySlices,
				pvrvk::SampleCountFlags::e_1_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, imageAllocator, imageAllocationCreateFlags);
	}
	else if (texHeight > 1)
	{
		image = createImage(device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, format, pvrvk::Extent3D(texWidth, texHeight, 1u), usageFlags, static_cast<uint8_t>(texMipLevels),
				texArraySlices * (texture.getNumFaces() > 1 ? 6 : 1), pvrvk::SampleCountFlags::e_1_BIT,
				pvrvk::ImageCreateFlags::e_CUBE_COMPATIBLE_BIT * (texture.getNumFaces() > 1) |
					pvrvk::ImageCreateFlags::e_2D_ARRAY_COMPATIBLE_BIT_KHR * static_cast<uint32_t>(texArraySlices > 1)),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, imageAllocator, imageAllocationCreateFlags);
	}
	else
	{
		image = createImage(device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_1D, format, pvrvk::Extent3D(texWidth, 1u, 1u), usageFlags, static_cast<uint8_t>(texMipLevels), texArraySlices),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, imageAllocator, imageAllocationCreateFlags);
	}

	// POPULATE, TRANSITION ETC
	{
		// Create a bunch of buffers that will be used as copy destinations - each will be one mip level, one array slice / one face
		// Faces are considered array elements, so each Framework array slice in a cube array will be 6 vulkan array slices.

		// Edit the info to be the small, linear images that we are using.
		std::vector<ImageUpdateInfo> imageUpdates(texMipLevels * texArraySlices * texFaces * texPlanes);
		uint32_t imageUpdateIndex = 0;
		for (uint32_t mipLevel = 0; mipLevel < texMipLevels; ++mipLevel)
		{
			uint32_t minWidth, minHeight, minDepth;
			textureToUse->getMinDimensionsForFormat(minWidth, minHeight, minDepth);
			dataWidth = static_cast<uint32_t>(std::max(textureToUse->getWidth(mipLevel), minWidth));
			dataHeight = static_cast<uint32_t>(std::max(textureToUse->getHeight(mipLevel), minHeight));
			texWidth = textureToUse->getWidth(mipLevel);
			texHeight = textureToUse->getHeight(mipLevel);
			texDepth = textureToUse->getDepth(mipLevel);

			for (uint32_t arraySlice = 0; arraySlice < texArraySlices; ++arraySlice)
			{
				for (uint32_t face = 0; face < texFaces; ++face)
				{
					for (uint32_t plane = 0; plane < texPlanes; ++plane)
					{
						if (plane > 0)
						{
							std::string ycbcrFormat = to_string(texture.getPixelFormat().getPixelTypeId());

							if (ycbcrFormat.find("420") != std::string::npos) // 420
							{
								dataWidth = static_cast<uint32_t>(std::max(textureToUse->getWidth(mipLevel), minWidth)) / 2;
								dataHeight = static_cast<uint32_t>(std::max(textureToUse->getHeight(mipLevel), minHeight)) / 2;
								texWidth = textureToUse->getWidth(mipLevel) / 2;
								texHeight = textureToUse->getHeight(mipLevel) / 2;
							}
							else if (ycbcrFormat.find("422") != std::string::npos) // 422
							{
								dataWidth = static_cast<uint32_t>(std::max(textureToUse->getWidth(mipLevel), minWidth)) / 2;
								texWidth = textureToUse->getWidth(mipLevel) / 2;
							}
						}

						ImageUpdateInfo& update = imageUpdates[imageUpdateIndex];
						update.imageWidth = texWidth;
						update.imageHeight = texHeight;
						update.dataWidth = dataWidth;
						update.dataHeight = dataHeight;
						update.depth = texDepth;
						update.arrayIndex = arraySlice;
						update.cubeFace = face;
						update.mipLevel = mipLevel;
						update.planeIndex = plane;
						update.numPlanes = texPlanes;
						update.data = textureToUse->getDataPointer(mipLevel, arraySlice, face, plane);
						update.dataSize = textureToUse->getDataSize(mipLevel, false, false, false, plane);
						++imageUpdateIndex;
					} // next plane
				} // next face
			} // next arrayslice
		} // next miplevel

		updateImage(device, commandBuffer, imageUpdates.data(), static_cast<uint32_t>(imageUpdates.size()), format, finalLayout, texFaces > 1, image, bufferAllocator);
	}
	pvr::utils::endCommandBufferDebugLabel(commandBuffer);
	return image;
}

pvrvk::ImageView uploadImageAndViewHelper(pvrvk::Device& device, const Texture& texture, bool allowDecompress, pvrvk::CommandBufferBase commandBuffer,
	pvrvk::ImageUsageFlags usageFlags, pvrvk::ImageLayout finalLayout, vma::Allocator bufferAllocator = nullptr, vma::Allocator imageAllocator = nullptr,
	vma::AllocationCreateFlags imageAllocationCreateFlags = vma::AllocationCreateFlags::e_NONE, const void* pNext = nullptr)
{
	pvrvk::ComponentMapping components = {
		pvrvk::ComponentSwizzle::e_IDENTITY,
		pvrvk::ComponentSwizzle::e_IDENTITY,
		pvrvk::ComponentSwizzle::e_IDENTITY,
		pvrvk::ComponentSwizzle::e_IDENTITY,
	};

	if (texture.getPixelFormat().getChannelContent(0) == 'l')
	{
		if (texture.getPixelFormat().getChannelContent(1) == 'a')
		{
			components.setR(pvrvk::ComponentSwizzle::e_R);
			components.setG(pvrvk::ComponentSwizzle::e_R);
			components.setB(pvrvk::ComponentSwizzle::e_R);
			components.setA(pvrvk::ComponentSwizzle::e_G);
		}
		else
		{
			components.setR(pvrvk::ComponentSwizzle::e_R);
			components.setG(pvrvk::ComponentSwizzle::e_R);
			components.setB(pvrvk::ComponentSwizzle::e_R);
			components.setA(pvrvk::ComponentSwizzle::e_ONE);
		}
	}
	else if (texture.getPixelFormat().getChannelContent(0) == 'a')
	{
		components.setR(pvrvk::ComponentSwizzle::e_ZERO);
		components.setG(pvrvk::ComponentSwizzle::e_ZERO);
		components.setB(pvrvk::ComponentSwizzle::e_ZERO);
		components.setA(pvrvk::ComponentSwizzle::e_R);
	}

	return device->createImageView(pvrvk::ImageViewCreateInfo(
		uploadImageHelper(device, texture, allowDecompress, commandBuffer, usageFlags, finalLayout, bufferAllocator, imageAllocator, imageAllocationCreateFlags), components, pNext));
}

inline pvrvk::ImageView loadAndUploadImageAndViewHelper(pvrvk::Device& device, const char* fileName, bool allowDecompress, pvrvk::CommandBufferBase commandBuffer,
	IAssetProvider& assetProvider, pvrvk::ImageUsageFlags usageFlags, pvrvk::ImageLayout finalLayout, Texture* outAssetTexture = nullptr, vma::Allocator imageAllocator = nullptr,
	vma::Allocator bufferAllocator = nullptr, vma::AllocationCreateFlags imageAllocationCreateFlags = vma::AllocationCreateFlags::e_NONE, const void* pNext = nullptr)
{
	Texture outTexture;
	Texture* pOutTexture = &outTexture;
	if (outAssetTexture) { pOutTexture = outAssetTexture; }
	auto assetStream = assetProvider.getAssetStream(fileName);
	*pOutTexture = pvr::textureLoad(*assetStream, pvr::getTextureFormatFromFilename(fileName));
	pvrvk::ImageView imageView =
		uploadImageAndViewHelper(device, *pOutTexture, allowDecompress, commandBuffer, usageFlags, finalLayout, bufferAllocator, imageAllocator, imageAllocationCreateFlags, pNext);
	imageView->setObjectName(fileName);
	return imageView;
}

inline pvrvk::Image loadAndUploadImageHelper(pvrvk::Device& device, const char* fileName, bool allowDecompress, pvrvk::CommandBufferBase commandBuffer, IAssetProvider& assetProvider,
	pvrvk::ImageUsageFlags usageFlags, pvrvk::ImageLayout finalLayout, Texture* outAssetTexture = nullptr, vma::Allocator stagingBufferAllocator = nullptr,
	vma::Allocator imageAllocator = nullptr, vma::AllocationCreateFlags imageAllocationCreateFlags = vma::AllocationCreateFlags::e_NONE)
{
	Texture outTexture;
	Texture* pOutTexture = &outTexture;
	if (outAssetTexture) { pOutTexture = outAssetTexture; }
	auto assetStream = assetProvider.getAssetStream(fileName);
	*pOutTexture = pvr::textureLoad(*assetStream, pvr::getTextureFormatFromFilename(fileName));
	pvrvk::Image image =
		uploadImageHelper(device, *pOutTexture, allowDecompress, commandBuffer, usageFlags, finalLayout, stagingBufferAllocator, imageAllocator, imageAllocationCreateFlags);
	image->setObjectName(fileName);
	return image;
}

pvrvk::ImageView loadAndUploadImageAndView(pvrvk::Device& device, const char* fileName, bool allowDecompress, pvrvk::CommandBuffer& commandBuffer, IAssetProvider& assetProvider,
	pvrvk::ImageUsageFlags usageFlags, pvrvk::ImageLayout finalLayout, Texture* outAssetTexture, vma::Allocator stagingBufferAllocator, vma::Allocator imageAllocator,
	vma::AllocationCreateFlags imageAllocationCreateFlags, const void* pNext)
{
	return loadAndUploadImageAndViewHelper(device, fileName, allowDecompress, pvrvk::CommandBufferBase(commandBuffer), assetProvider, usageFlags, finalLayout, outAssetTexture,
		imageAllocator, stagingBufferAllocator, imageAllocationCreateFlags, pNext);
}

pvrvk::ImageView loadAndUploadImageAndView(pvrvk::Device& device, const char* fileName, bool allowDecompress, pvrvk::SecondaryCommandBuffer& commandBuffer,
	IAssetProvider& assetProvider, pvrvk::ImageUsageFlags usageFlags, pvrvk::ImageLayout finalLayout, Texture* outAssetTexture, vma::Allocator stagingBufferAllocator,
	vma::Allocator imageAllocator, vma::AllocationCreateFlags imageAllocationCreateFlags, const void* pNext)
{
	return loadAndUploadImageAndViewHelper(device, fileName, allowDecompress, pvrvk::CommandBufferBase(commandBuffer), assetProvider, usageFlags, finalLayout, outAssetTexture,
		imageAllocator, stagingBufferAllocator, imageAllocationCreateFlags, pNext);
}

pvrvk::Image loadAndUploadImage(pvrvk::Device& device, const char* fileName, bool allowDecompress, pvrvk::CommandBuffer& commandBuffer, IAssetProvider& assetProvider,
	pvrvk::ImageUsageFlags usageFlags, pvrvk::ImageLayout finalLayout, Texture* outAssetTexture, vma::Allocator stagingBufferAllocator, vma::Allocator imageAllocator,
	vma::AllocationCreateFlags imageAllocationCreateFlags)
{
	return loadAndUploadImageHelper(device, fileName, allowDecompress, pvrvk::CommandBufferBase(commandBuffer), assetProvider, usageFlags, finalLayout, outAssetTexture,
		stagingBufferAllocator, imageAllocator, imageAllocationCreateFlags);
}

pvrvk::Image loadAndUploadImage(pvrvk::Device& device, const std::string& fileName, bool allowDecompress, pvrvk::CommandBuffer& commandBuffer, IAssetProvider& assetProvider,
	pvrvk::ImageUsageFlags usageFlags, pvrvk::ImageLayout finalLayout, Texture* outAssetTexture, vma::Allocator stagingBufferAllocator, vma::Allocator imageAllocator,
	vma::AllocationCreateFlags imageAllocationCreateFlags)
{
	return loadAndUploadImageHelper(device, fileName.c_str(), allowDecompress, pvrvk::CommandBufferBase(commandBuffer), assetProvider, usageFlags, finalLayout, outAssetTexture,
		stagingBufferAllocator, imageAllocator, imageAllocationCreateFlags);
}

pvrvk::Image loadAndUploadImage(pvrvk::Device& device, const char* fileName, bool allowDecompress, pvrvk::SecondaryCommandBuffer& commandBuffer, IAssetProvider& assetProvider,
	pvrvk::ImageUsageFlags usageFlags, pvrvk::ImageLayout finalLayout, Texture* outAssetTexture, vma::Allocator stagingBufferAllocator, vma::Allocator imageAllocator,
	vma::AllocationCreateFlags imageAllocationCreateFlags)
{
	return loadAndUploadImageHelper(device, fileName, allowDecompress, pvrvk::CommandBufferBase(commandBuffer), assetProvider, usageFlags, finalLayout, outAssetTexture,
		stagingBufferAllocator, imageAllocator, imageAllocationCreateFlags);
}

pvrvk::ImageView uploadImageAndView(pvrvk::Device& device, const Texture& texture, bool allowDecompress, pvrvk::SecondaryCommandBuffer& commandBuffer, pvrvk::ImageUsageFlags usageFlags,
	pvrvk::ImageLayout finalLayout, vma::Allocator stagingBufferAllocator, vma::Allocator imageAllocator, vma::AllocationCreateFlags imageAllocationCreateFlags)
{
	return uploadImageAndViewHelper(
		device, texture, allowDecompress, pvrvk::CommandBufferBase(commandBuffer), usageFlags, finalLayout, stagingBufferAllocator, imageAllocator, imageAllocationCreateFlags);
}

pvrvk::ImageView uploadImageAndView(pvrvk::Device& device, const Texture& texture, bool allowDecompress, pvrvk::CommandBuffer& commandBuffer, pvrvk::ImageUsageFlags usageFlags,
	pvrvk::ImageLayout finalLayout, vma::Allocator stagingBufferAllocator, vma::Allocator imageAllocator, vma::AllocationCreateFlags imageAllocationCreateFlags)
{
	return uploadImageAndViewHelper(
		device, texture, allowDecompress, pvrvk::CommandBufferBase(commandBuffer), usageFlags, finalLayout, stagingBufferAllocator, imageAllocator, imageAllocationCreateFlags);
}

pvrvk::Image uploadImage(pvrvk::Device& device, const Texture& texture, bool allowDecompress, pvrvk::CommandBuffer& commandBuffer, pvrvk::ImageUsageFlags usageFlags,
	pvrvk::ImageLayout finalLayout, vma::Allocator stagingBufferAllocator, vma::Allocator imageAllocator, vma::AllocationCreateFlags imageAllocationCreateFlags)
{
	return uploadImageHelper(device, texture, allowDecompress, commandBuffer, usageFlags, finalLayout, stagingBufferAllocator, imageAllocator, imageAllocationCreateFlags);
}

void generateTextureAtlas(pvrvk::Device& device, const pvrvk::Image* inputImages, pvrvk::Rect2Df* outUVs, uint32_t numImages, pvrvk::ImageLayout inputImageLayout,
	pvrvk::ImageView* outImageView, TextureHeader* outDescriptor, pvrvk::CommandBufferBase cmdBuffer, pvrvk::ImageLayout finalLayout, vma::Allocator imageAllocator,
	vma::AllocationCreateFlags imageAllocationCreateFlags)
{
	TextureHeader header;
	struct SortedImage
	{
		uint32_t id;
		pvrvk::Image image;
		uint16_t width;
		uint16_t height;
		uint16_t srcX;
		uint16_t srcY;
		bool hasAlpha;
	};
	std::vector<SortedImage> sortedImage(numImages);
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
		int32_t x;
		int32_t y;
		int32_t w;
		int32_t h;
		int32_t size;
		bool isFilled;

		Area* right;
		Area* left;

	private:
		void setSize(int32_t width, int32_t height)
		{
			w = width;
			h = height;
			size = width * height;
		}

	public:
		Area(int32_t width, int32_t height) : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(width, height); }

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

				// Initializes the child members' size attributes.
				left->size = left->h * left->w;
				right->size = right->h * right->w;

				// Inserts the texture into the left child member.
				return left->insert(width, height);
			}
			// Catch all error return.
			return NULL;
		}

		bool deleteArea()
		{
			if (left != NULL)
			{
				if (left->left != NULL)
				{
					if (!left->deleteArea()) { return false; }
					if (!right->deleteArea()) { return false; }
				}
			}
			if (right != NULL)
			{
				if (right->left != NULL)
				{
					if (!left->deleteArea()) { return false; }
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
	for (uint32_t i = 0; i < numImages; ++i)
	{
		sortedImage[i].image = inputImages[i];
		sortedImage[i].id = i;
		sortedImage[i].width = static_cast<uint16_t>(inputImages[i]->getWidth());
		sortedImage[i].height = static_cast<uint16_t>(inputImages[i]->getHeight());
	}
	//// sort the sprites
	std::sort(sortedImage.begin(), sortedImage.end(), SortCompare());
	// find the best width and height
	int32_t width = 0, height = 0, area = 0;
	uint32_t preferredDim[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
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
	if (sortedImagesIterator >= sizeof(preferredDim) / sizeof(preferredDim[0])) { throw pvrvk::ErrorValidationFailedEXT("Cannot find a best size for the texture atlas"); }

	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::generateTextureAtlas"));

	width = height = preferredDim[sortedImagesIterator];
	float oneOverWidth = 1.f / width;
	float oneOverHeight = 1.f / height;
	Area* head = new Area(width, height);
	Area* pRtrn = nullptr;
	pvrvk::Offset3D dstOffsets[2];

	// create the out texture store
	pvrvk::Format outFmt = pvrvk::Format::e_R8G8B8A8_UNORM;
	pvrvk::Image outTexStore = createImage(device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, outFmt, pvrvk::Extent3D(width, height, 1u), pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, imageAllocator, imageAllocationCreateFlags);

	utils::setImageLayout(outTexStore, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, cmdBuffer);

	pvrvk::ImageView view = device->createImageView(pvrvk::ImageViewCreateInfo(outTexStore));
	cmdBuffer->clearColorImage(view, pvrvk::ClearColorValue(0.0f, 0.f, 0.f, 0.f), pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);

	for (uint32_t i = 0; i < numImages; ++i)
	{
		const SortedImage& image = sortedImage[i];
		pRtrn = head->insert(static_cast<int32_t>(sortedImage[i].width) + totalBorder, static_cast<int32_t>(sortedImage[i].height) + totalBorder);
		if (!pRtrn)
		{
			head->deleteArea();
			delete head;
			throw pvrvk::ErrorUnknown("Cannot find a best size for the texture atlas");
		}
		dstOffsets[0].setX(static_cast<uint16_t>(pRtrn->x + atlasPixelBorder));
		dstOffsets[0].setY(static_cast<uint16_t>(pRtrn->y + atlasPixelBorder));
		dstOffsets[0].setZ(0);

		dstOffsets[1].setX(static_cast<uint16_t>(dstOffsets[0].getX() + sortedImage[i].width));
		dstOffsets[1].setY(static_cast<uint16_t>(dstOffsets[0].getY() + sortedImage[i].height));
		dstOffsets[1].setZ(1);

		pvrvk::Offset2Df offset(dstOffsets[0].getX() * oneOverWidth, dstOffsets[0].getY() * oneOverHeight);
		pvrvk::Extent2Df extent(sortedImage[i].width * oneOverWidth, sortedImage[i].height * oneOverHeight);

		outUVs[image.id].setOffset(offset);
		outUVs[image.id].setExtent(extent);

		pvrvk::Offset3D srcOffsets[2] = { pvrvk::Offset3D(0, 0, 0), pvrvk::Offset3D(image.width, image.height, 1) };
		pvrvk::ImageBlit blit(pvrvk::ImageSubresourceLayers(), srcOffsets, pvrvk::ImageSubresourceLayers(), dstOffsets);

		cmdBuffer->blitImage(sortedImage[i].image, outTexStore, &blit, 1, pvrvk::Filter::e_NEAREST, inputImageLayout, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);
	}
	if (outDescriptor)
	{
		outDescriptor->setWidth(width);
		outDescriptor->setHeight(height);
		outDescriptor->setChannelType(VariableType::UnsignedByteNorm);
		outDescriptor->setColorSpace(ColorSpace::lRGB);
		outDescriptor->setDepth(1);
		outDescriptor->setPixelFormat(PixelFormat::RGBA_8888());
	}
	*outImageView = device->createImageView(pvrvk::ImageViewCreateInfo(outTexStore));

	const uint32_t queueFamilyId = cmdBuffer->getCommandPool()->getQueueFamilyIndex();

	pvrvk::MemoryBarrierSet barrier;
	barrier.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, outTexStore,
		pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, finalLayout, queueFamilyId, queueFamilyId));

	cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT | pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, barrier);

	head->deleteArea();
	delete head;

	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
}

void updateImage(pvrvk::Device& device, pvrvk::CommandBufferBase cbuffTransfer, ImageUpdateInfo* updateInfos, uint32_t numUpdateInfos, pvrvk::Format format,
	pvrvk::ImageLayout layout, bool isCubeMap, pvrvk::Image& image, vma::Allocator bufferAllocator)
{
	using namespace vma;
	if (!(cbuffTransfer && cbuffTransfer->isRecording())) { throw pvrvk::ErrorValidationFailedEXT("updateImage - Commandbuffer must be valid and in recording state"); }

	uint32_t numFace = (isCubeMap ? 6 : 1);

	uint32_t hwSlice;
	std::vector<pvrvk::Buffer> stagingBuffers;

	{
		pvr::utils::beginCommandBufferDebugLabel(cbuffTransfer, pvrvk::DebugUtilsLabel("PVRUtilsVk::updateImage"));

		stagingBuffers.resize(numUpdateInfos);
		pvrvk::BufferImageCopy imgcp = {};

		for (uint32_t i = 0; i < numUpdateInfos; ++i)
		{
			const ImageUpdateInfo& mipLevelUpdate = updateInfos[i];
			assertion(mipLevelUpdate.data && mipLevelUpdate.dataSize, "Data and Data size must be valid");

			hwSlice = mipLevelUpdate.arrayIndex * numFace + mipLevelUpdate.cubeFace;

			// Will write the switch layout commands from the universal queue to the transfer queue to both the
			// transfer command buffer and the universal command buffer
			setImageLayoutAndQueueFamilyOwnership(pvrvk::CommandBufferBase(), cbuffTransfer, static_cast<uint32_t>(-1), static_cast<uint32_t>(-1), pvrvk::ImageLayout::e_UNDEFINED,
				pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, image, mipLevelUpdate.mipLevel, 1, hwSlice, 1, inferAspectFromFormat(format, updateInfos[i].planeIndex));

			// Create a staging buffer to use as the source of a copyBufferToImage
			stagingBuffers[i] = createBuffer(device, pvrvk::BufferCreateInfo(mipLevelUpdate.dataSize, pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT),
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, bufferAllocator, vma::AllocationCreateFlags::e_MAPPED_BIT);

			stagingBuffers[i]->setObjectName("PVRUtilsVk::updateImage::Temporary Image Upload Buffer");
			imgcp.setImageOffset(pvrvk::Offset3D(mipLevelUpdate.offsetX, mipLevelUpdate.offsetY, mipLevelUpdate.offsetZ));
			imgcp.setImageExtent(pvrvk::Extent3D(mipLevelUpdate.imageWidth, mipLevelUpdate.imageHeight, 1));

			imgcp.setImageSubresource(pvrvk::ImageSubresourceLayers(inferAspectFromFormat(format, updateInfos[i].planeIndex), updateInfos[i].mipLevel, hwSlice, 1));
			imgcp.setBufferRowLength(mipLevelUpdate.dataWidth);
			imgcp.setBufferImageHeight(mipLevelUpdate.dataHeight);

			const uint8_t* srcData;
			uint32_t srcDataSize;
			srcData = static_cast<const uint8_t*>(mipLevelUpdate.data);
			srcDataSize = mipLevelUpdate.dataSize;

			updateHostVisibleBuffer(stagingBuffers[i], srcData, 0, srcDataSize, true);

			cbuffTransfer->copyBufferToImage(stagingBuffers[i], image, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, 1, &imgcp);

			// CAUTION: We swapped src and dst queue families as, if there was no ownership transfer, no problem - queue families
			// will be ignored.
			// Will write the switch layout commands from the transfer queue to the universal queue to both the
			// transfer command buffer and the universal command buffer
			setImageLayoutAndQueueFamilyOwnership(cbuffTransfer, pvrvk::CommandBufferBase(), static_cast<uint32_t>(-1), static_cast<uint32_t>(-1),
				pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, layout, image, mipLevelUpdate.mipLevel, 1, hwSlice, 1, inferAspectFromFormat(format, updateInfos[i].planeIndex));
		}
		pvr::utils::endCommandBufferDebugLabel(cbuffTransfer);
	}
}

#pragma endregion

#pragma region //////////// DEVICES AND QUEUES //////////////

pvrvk::Device createDeviceAndQueues(pvrvk::PhysicalDevice physicalDevice, const QueuePopulateInfo* queueCreateInfos, uint32_t numQueueCreateInfos, QueueAccessInfo* outAccessInfo,
	const DeviceExtensions& deviceExtensions)
{
	std::vector<pvrvk::DeviceQueueCreateInfo> queueCreateInfo;
	const std::vector<pvrvk::QueueFamilyProperties>& queueFamilyProperties = physicalDevice->getQueueFamilyProperties();

	const char* graphics = "GRAPHICS ";
	const char* compute = "COMPUTE ";
	const char* present = "PRESENT ";
	const char* transfer = "TRANSFER ";
	const char* sparse = "SPARSE_BINDING ";
	const char* nothing = "";

	// Log the supported queue families
	Log(LogLevel::Information, "Supported Queue Families:");
	for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); ++i)
	{
		Log(LogLevel::Information, "\tqueue family %d (#queues %d)  FLAGS: %d ( %s%s%s%s%s)", i, queueFamilyProperties[i].getQueueCount(), queueFamilyProperties[i].getQueueFlags(),
			((queueFamilyProperties[i].getQueueFlags() & pvrvk::QueueFlags::e_GRAPHICS_BIT) != 0) ? graphics : nothing,
			((queueFamilyProperties[i].getQueueFlags() & pvrvk::QueueFlags::e_COMPUTE_BIT) != 0) ? compute : nothing,
			((queueFamilyProperties[i].getQueueFlags() & pvrvk::QueueFlags::e_TRANSFER_BIT) != 0) ? transfer : nothing,
			((queueFamilyProperties[i].getQueueFlags() & pvrvk::QueueFlags::e_SPARSE_BINDING_BIT) != 0) ? sparse : nothing, nothing, nothing);
	}

	std::vector<int32_t> queueIndices(queueFamilyProperties.size(), -1);
	std::vector<float> queuePrioties;
	for (uint32_t i = 0; i < numQueueCreateInfos; ++i)
	{
		for (uint32_t j = 0; j < queueFamilyProperties.size(); ++j)
		{
			// if requested, look for presentation support
			if (!queueCreateInfos[i].surface || physicalDevice->getSurfaceSupport(j, queueCreateInfos[i].surface))
			{
				uint32_t supportedFlags = static_cast<uint32_t>(queueFamilyProperties[j].getQueueFlags());
				uint32_t requestedFlags = static_cast<uint32_t>(queueCreateInfos[i].queueFlags);

				// look for the supported flags
				if ((supportedFlags & requestedFlags) == requestedFlags)
				{
					if (static_cast<uint32_t>(queueIndices[j] + 1) < queueFamilyProperties[j].getQueueCount()) { ++queueIndices[j]; }

					outAccessInfo[i].familyId = j;
					outAccessInfo[i].queueId = static_cast<uint32_t>(queueIndices[j]);
					queuePrioties.emplace_back(queueCreateInfos[i].priority);

					break;
				}
			}
		}
	}

	uint32_t priorityIndex = 0;
	// populate the queue create info
	for (uint32_t i = 0; i < queueIndices.size(); ++i)
	{
		if (queueIndices[i] != -1)
		{
			queueCreateInfo.emplace_back(pvrvk::DeviceQueueCreateInfo());
			pvrvk::DeviceQueueCreateInfo& createInfo = queueCreateInfo.back();
			createInfo.setQueueFamilyIndex(i);
			for (uint32_t j = 0; j < static_cast<uint32_t>(queueIndices[i] + 1); ++j)
			{
				createInfo.addQueue(queuePrioties[priorityIndex]);
				priorityIndex++;
			}
		}
	}

	// create the device
	pvrvk::DeviceCreateInfo deviceInfo;
	pvrvk::PhysicalDeviceFeatures features = physicalDevice->getFeatures();

	// Ensure that robustBufferAccess is disabled
	features.setRobustBufferAccess(false);
	deviceInfo.setEnabledFeatures(&features);

	deviceInfo.setDeviceQueueCreateInfos(queueCreateInfo);

	// Print out the supported device extensions
	const std::vector<pvrvk::ExtensionProperties>& extensionProperties = physicalDevice->getDeviceExtensionsProperties();

	Log(LogLevel::Information, "Supported Device Extensions:");
	for (uint32_t i = 0; i < static_cast<uint32_t>(extensionProperties.size()); ++i)
	{
		Log(LogLevel::Information, "\t%s : version [%u]", extensionProperties[i].getExtensionName(), extensionProperties[i].getSpecVersion());
	}

	// Filter the given set of extensions so only the set of device extensions which are supported by the device remain
	if (deviceExtensions.getNumExtensions())
	{
		pvrvk::VulkanExtensionList supportedRequestedExtensions = pvrvk::Extensions::filterExtensions(extensionProperties, deviceExtensions);

		// Determine whether VK_EXT_debug_utils is supported and enabled by the instance
		bool debugUtilsSupported = physicalDevice->getInstance()->getEnabledExtensionTable().extDebugUtilsEnabled;

		if (debugUtilsSupported)
		{
			// Determine whether VK_EXT_debug_maker is supported
			bool debugReportSupported = supportedRequestedExtensions.containsExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

			// If VK_EXT_debug_utils is supported then remove VK_EXT_debug_maker from the list of extension to enable therefore we are prioritising use of VK_EXT_debug_utils
			if (debugUtilsSupported && debugReportSupported)
			{
				Log(LogLevel::Information, "VK_EXT_debug_utils and VK_EXT_debug_maker are both supported. We will be using VK_EXT_debug_utils.");
				supportedRequestedExtensions.removeExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			}
		}

		deviceInfo.setExtensionList(supportedRequestedExtensions);
		deviceInfo.setLastRequestedExtensionFeature(deviceExtensions.getLastRequestedExtensionFeature());

		Log(LogLevel::Information, "Supported Device Extensions to be Enabled:");
		for (uint32_t i = 0; i < static_cast<uint32_t>(deviceInfo.getExtensionList().getNumExtensions()); ++i)
		{
			Log(LogLevel::Information, "\t%s : version [%u]", deviceInfo.getExtensionList().getExtension(i).getName().c_str(),
				deviceInfo.getExtensionList().getExtension(i).getSpecVersion());
		}

		if (deviceInfo.getExtensionList().getNumExtensions() != deviceExtensions.getNumExtensions())
		{
			Log(LogLevel::Warning, "Note that not all requested Logical device extensions are supported");
		}
	}

	// add device extension features to DeviceCreateInfo pnext chain
	if (deviceExtensions.getLastRequestedExtensionFeature()) { deviceInfo.setLastRequestedExtensionFeature(deviceExtensions.getLastRequestedExtensionFeature()); }

	pvrvk::Device outDevice = physicalDevice->createDevice(deviceInfo);
	outDevice->retrieveQueues();

	// Log the retrieved queues
	Log(LogLevel::Information, "Queues Created:");
	for (uint32_t i = 0; i < queueCreateInfo.size(); ++i)
	{
		bool supportsWsi = (queueCreateInfos[i].surface && physicalDevice->getSurfaceSupport(i, queueCreateInfos[i].surface));

		Log(LogLevel::Information, "\t queue Family: %d ( %s%s%s%s%s) \tqueue count: %d", queueCreateInfo[i].getQueueFamilyIndex(),
			((queueFamilyProperties[queueCreateInfo[i].getQueueFamilyIndex()].getQueueFlags() & pvrvk::QueueFlags::e_GRAPHICS_BIT) != 0) ? graphics : nothing,
			((queueFamilyProperties[queueCreateInfo[i].getQueueFamilyIndex()].getQueueFlags() & pvrvk::QueueFlags::e_COMPUTE_BIT) != 0) ? compute : nothing,
			((queueFamilyProperties[queueCreateInfo[i].getQueueFamilyIndex()].getQueueFlags() & pvrvk::QueueFlags::e_TRANSFER_BIT) != 0) ? transfer : nothing,
			((queueFamilyProperties[queueCreateInfo[i].getQueueFamilyIndex()].getQueueFlags() & pvrvk::QueueFlags::e_SPARSE_BINDING_BIT) != 0) ? sparse : nothing,
			(supportsWsi ? present : nothing), queueCreateInfo[i].getNumQueues());
	}

	return outDevice;
}
#pragma endregion

#pragma region ///////// SWAPCHAINS AND FRAMEBUFFERS ////////

bool isSupportedDepthStencilFormat(const pvrvk::Device& device, pvrvk::Format format)
{
	auto prop = device->getPhysicalDevice()->getFormatProperties(format);
	return (prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
};

pvrvk::Format getSupportedDepthStencilFormat(const pvrvk::Device& device, std::vector<pvrvk::Format> preferredDepthFormats)
{
	if (preferredDepthFormats.empty())
	{
		preferredDepthFormats = {
			pvrvk::Format::e_D32_SFLOAT_S8_UINT,
			pvrvk::Format::e_D24_UNORM_S8_UINT,
			pvrvk::Format::e_D16_UNORM_S8_UINT,
			pvrvk::Format::e_D32_SFLOAT,
			pvrvk::Format::e_D16_UNORM,
			pvrvk::Format::e_X8_D24_UNORM_PACK32,
		};
	}

	pvrvk::Format supportedDepthStencilFormat = pvrvk::Format::e_UNDEFINED;

	auto it =
		std::find_if(preferredDepthFormats.begin(), preferredDepthFormats.end(), [&device](pvrvk::Format format) -> bool { return isSupportedDepthStencilFormat(device, format); });
	if (it != preferredDepthFormats.end()) { supportedDepthStencilFormat = *it; }

	return supportedDepthStencilFormat;
}

pvrvk::Format getSupportedDepthStencilFormat(const pvrvk::Device& device, pvr::DisplayAttributes& displayAttributes, std::vector<pvrvk::Format> preferredDepthFormats)
{
	pvrvk::Format depthStencilFormatRequested = getDepthStencilFormat(displayAttributes);
	pvrvk::Format supportedDepthStencilFormat = pvrvk::Format::e_UNDEFINED;

	// start by checking for the requested depth stencil format
	if (isSupportedDepthStencilFormat(device, depthStencilFormatRequested)) { supportedDepthStencilFormat = depthStencilFormatRequested; }
	else
	{
		supportedDepthStencilFormat = getSupportedDepthStencilFormat(device, preferredDepthFormats);
		Log(LogLevel::Information, "Requested DepthStencil VkFormat %s is not supported. Falling back to %s", to_string(depthStencilFormatRequested).c_str(),
			to_string(supportedDepthStencilFormat).c_str());
	}

	getDepthStencilBits(supportedDepthStencilFormat, displayAttributes.depthBPP, displayAttributes.stencilBPP);
	Log(LogLevel::Information, "DepthStencil VkFormat: %s", to_string(supportedDepthStencilFormat).c_str());

	return supportedDepthStencilFormat;
}

void createSwapchainAndDepthStencilImageAndViews(const pvrvk::Device& device, const pvrvk::Surface& surface, DisplayAttributes& displayAttributes, pvrvk::Swapchain& outSwapchain,
	Multi<pvrvk::ImageView>& outDepthStencilImages, const pvrvk::ImageUsageFlags& swapchainImageUsageFlags, const pvrvk::ImageUsageFlags& dsImageUsageFlags,
	const vma::Allocator& dsImageAllocator, vma::AllocationCreateFlags dsImageAllocationCreateFlags)
{
	outSwapchain = createSwapchain(device, surface, displayAttributes, swapchainImageUsageFlags);

	pvrvk::Format supportedDepthStencilFormat = getSupportedDepthStencilFormat(device, displayAttributes);
	createAttachmentImages(outDepthStencilImages, device, displayAttributes.swapLength, supportedDepthStencilFormat, outSwapchain->getDimension(), dsImageUsageFlags,
		convertToPVRVkNumSamples((uint8_t)displayAttributes.aaSamples), dsImageAllocator, dsImageAllocationCreateFlags, "PVRUtilsVk::DepthStencil");
}

pvrvk::Swapchain createSwapchain(const pvrvk::Device& device, const pvrvk::Surface& surface, pvr::DisplayAttributes& displayAttributes,
	pvrvk::ImageUsageFlags swapchainImageUsageFlags, const std::vector<pvrvk::Format>& preferredColorFormats)
{
	return createSwapchainHelper(device, surface, displayAttributes, swapchainImageUsageFlags, preferredColorFormats);
}

pvrvk::RenderPass createOnScreenRenderPass(const pvrvk::Swapchain& swapchain, bool hasDepthStencil, const pvrvk::Format depthStencilFormat,
	pvrvk::ImageLayout initialSwapchainLayout, pvrvk::ImageLayout initialDepthStencilLayout, pvrvk::AttachmentLoadOp colorLoadOp, pvrvk::AttachmentStoreOp colorStoreOp,
	pvrvk::AttachmentLoadOp depthStencilLoadOp, pvrvk::AttachmentStoreOp depthStencilStoreOp, pvrvk::SampleCountFlags samples)
{
	pvrvk::RenderPassCreateInfo rpInfo;
	pvrvk::SubpassDescription subpass;

	bool multisample = (samples != pvrvk::SampleCountFlags::e_1_BIT);

	int coloridx = 0, depthidx = 0, colorresolveidx = 0, depthresolveidx = 0;
	details::assignAttachmentIndexes(hasDepthStencil, multisample, coloridx, depthidx, colorresolveidx, depthresolveidx);

	if (!multisample)
	{
		rpInfo.setAttachmentDescription(0,
			pvrvk::AttachmentDescription::createColorDescription(
				swapchain->getImageFormat(), initialSwapchainLayout, pvrvk::ImageLayout::e_PRESENT_SRC_KHR, colorLoadOp, colorStoreOp, pvrvk::SampleCountFlags::e_1_BIT));
		subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(coloridx, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
		if (hasDepthStencil)
		{
			rpInfo.setAttachmentDescription(1,
				pvrvk::AttachmentDescription::createDepthStencilDescription(depthStencilFormat, initialDepthStencilLayout, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					depthStencilLoadOp, depthStencilStoreOp, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE,
					swapchain->getImage(0)->getCreateInfo().getNumSamples()));
			subpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(depthidx, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
		}
	}
	else // MULTISAMPLING
	{
		// We do not care about storeop, as the resolve attachment is the one that will be stored. The layout should always be color attachment optimal, as only resolve will be presented
		rpInfo.setAttachmentDescription(coloridx,
			pvrvk::AttachmentDescription::createColorDescription(
				swapchain->getImageFormat(), initialSwapchainLayout, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, colorLoadOp, pvrvk::AttachmentStoreOp::e_DONT_CARE, samples));

		subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(coloridx, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

		// Conversely, we do not care about the loadop, since the whole attachment will get overwritten during resolving. The layout should be Present Src, as only the color attachment gets rendered into
		rpInfo.setAttachmentDescription(colorresolveidx,
			pvrvk::AttachmentDescription::createColorDescription(swapchain->getImageFormat(), initialSwapchainLayout, pvrvk::ImageLayout::e_PRESENT_SRC_KHR,
				pvrvk::AttachmentLoadOp::e_DONT_CARE, colorStoreOp, pvrvk::SampleCountFlags::e_1_BIT));

		subpass.setResolveAttachmentReference(0, pvrvk::AttachmentReference(colorresolveidx, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

		if (hasDepthStencil)
		{
			rpInfo.setAttachmentDescription(depthidx,
				pvrvk::AttachmentDescription::createDepthStencilDescription(depthStencilFormat, initialDepthStencilLayout, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					depthStencilLoadOp, pvrvk::AttachmentStoreOp::e_DONT_CARE, depthStencilLoadOp, pvrvk::AttachmentStoreOp::e_DONT_CARE, samples));
			subpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(depthidx, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

			rpInfo.setAttachmentDescription(depthresolveidx,
				pvrvk::AttachmentDescription::createDepthStencilDescription(depthStencilFormat, initialDepthStencilLayout, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					pvrvk::AttachmentLoadOp::e_DONT_CARE, depthStencilStoreOp, pvrvk::AttachmentLoadOp::e_DONT_CARE, depthStencilStoreOp, pvrvk::SampleCountFlags::e_1_BIT));
			subpass.setResolveAttachmentReference(1, pvrvk::AttachmentReference(depthresolveidx, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
		}
	}

	std::vector<pvrvk::SubpassDependency> dependencies;
	dependencies.emplace_back(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::AccessFlags::e_NONE, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::DependencyFlags::e_BY_REGION_BIT);

	dependencies.emplace_back(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
		pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_NONE, pvrvk::DependencyFlags::e_BY_REGION_BIT);

	if (hasDepthStencil)
	{
		dependencies.emplace_back(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_LATE_FRAGMENT_TESTS_BIT, pvrvk::PipelineStageFlags::e_EARLY_FRAGMENT_TESTS_BIT,
			pvrvk::AccessFlags::e_NONE, pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			pvrvk::DependencyFlags::e_BY_REGION_BIT);
	}

	rpInfo.addSubpassDependencies(dependencies.data(), static_cast<uint32_t>(dependencies.size()));
	rpInfo.setSubpass(0, subpass);

	pvrvk::RenderPass renderPass = swapchain->getDevice()->createRenderPass(rpInfo);
	renderPass->setObjectName("PVRUtilsVk::OnScreenRenderPass");

	return renderPass;
}

OnScreenObjects createSwapchainRenderpassFramebuffers(
	const pvrvk::Device& device, const pvrvk::Surface& surface, pvr::DisplayAttributes& displayAttributes, const CreateSwapchainParameters& configuration)
{
	OnScreenObjects retval;
	retval.swapchain = createSwapchain(device, surface, displayAttributes, configuration.colorImageUsageFlags, configuration.preferredColorFormats);
	pvrvk::SampleCountFlags samples = convertToPVRVkNumSamples((uint8_t)displayAttributes.aaSamples);
	pvrvk::Format supportedDepthStencilFormat = pvrvk::Format::e_UNDEFINED;
	if (displayAttributes.aaSamples > 1)
	{
		retval.colorMultisampledAttachmentImages.resize(displayAttributes.swapLength);

		// Since AA, THIS is actually the color attachemt. The backbuffer is the "resolve" attachment.
		createAttachmentImages(retval.colorMultisampledAttachmentImages, device, displayAttributes.swapLength, retval.swapchain->getImageFormat(), retval.swapchain->getDimension(),
			configuration.colorAttachmentFlagsIfMultisampled, samples, configuration.imageAllocator, configuration.imageAllocatorFlags, "PVRUtilsVk::ColorMSAAAttachment");
	}
	if (configuration.createDepthBuffer)
	{
		retval.depthStencilImages.resize(displayAttributes.swapLength);

		// The only attachment, or the Resolve attachment if AA
		supportedDepthStencilFormat = getSupportedDepthStencilFormat(device, displayAttributes, configuration.preferredDepthStencilFormats);
		createAttachmentImages(retval.depthStencilImages, device, displayAttributes.swapLength, supportedDepthStencilFormat, retval.swapchain->getDimension(),
			configuration.depthStencilImageUsageFlags, pvrvk::SampleCountFlags::e_1_BIT, configuration.imageAllocator, configuration.imageAllocatorFlags, "PVRUtilsVk::DepthStencil");
		if (displayAttributes.aaSamples > 1)
		{
			retval.depthStencilMultisampledAttachmentImages.resize(displayAttributes.swapLength);

			// Since AA, THIS is the depth attachment
			createAttachmentImages(retval.depthStencilMultisampledAttachmentImages, device, displayAttributes.swapLength, supportedDepthStencilFormat,
				retval.swapchain->getDimension(), configuration.depthStencilAttachmentFlagsIfMultisampled, samples, configuration.imageAllocator, configuration.imageAllocatorFlags,
				"PVRUtilsVk::DepthStencilMSAAAttachment");
		}
	}

	retval.renderPass = createOnScreenRenderPass(retval.swapchain, configuration.createDepthBuffer, supportedDepthStencilFormat, configuration.initialSwapchainLayout,
		configuration.initialDepthStencilLayout, configuration.colorLoadOp, configuration.colorStoreOp, configuration.depthStencilLoadOp, configuration.depthStencilStoreOp, samples);
	retval.framebuffer = createOnscreenFramebuffers<std::vector<pvrvk::Framebuffer>>(retval.swapchain, retval.renderPass, retval.depthStencilImages.data(),
		retval.colorMultisampledAttachmentImages.data(), retval.depthStencilMultisampledAttachmentImages.data());
	return retval;
}

#pragma endregion

#pragma region //////////////// SCREENSHOTS /////////////////

std::vector<unsigned char> captureImageRegion(pvrvk::Queue& queue, pvrvk::CommandPool& cmdPool, pvrvk::Image& image, pvrvk::Offset3D srcOffset, pvrvk::Extent3D srcExtent,
	pvrvk::Format destinationImageFormat, pvrvk::ImageLayout imageInitialLayout, pvrvk::ImageLayout imageFinalLayout, vma::Allocator bufferAllocator, vma::Allocator imageAllocator)
{
	pvrvk::Device device = image->getDevice();
	pvrvk::CommandBuffer cmdBuffer = cmdPool->allocateCommandBuffer();
	// create the destination texture which does the format conversion
	const pvrvk::FormatProperties& formatProps = device->getPhysicalDevice()->getFormatProperties(destinationImageFormat);
	if ((formatProps.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_BLIT_DST_BIT) == 0)
	{
		throw pvrvk::ErrorValidationFailedEXT("Screen Capture requested Image format is not supported");
	}

	pvrvk::Extent3D copyRegion = pvrvk::Extent3D(srcExtent.getWidth() - srcOffset.getX(), srcExtent.getHeight() - srcOffset.getY(), srcExtent.getDepth() - srcOffset.getZ());

	// Create the intermediate image which will be used as the format conversion
	// when copying from source image and then copied into the buffer
	pvrvk::Image dstImage = createImage(device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, destinationImageFormat, copyRegion, pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, imageAllocator);

	const pvrvk::Offset3D srcOffsets[2] = { srcOffset, pvrvk::Offset3D(srcExtent.getWidth(), srcExtent.getHeight(), srcExtent.getDepth()) };
	const pvrvk::Offset3D dstOffsets[2] = { pvrvk::Offset3D(srcOffset.getX(), srcExtent.getHeight(), 0),
		pvrvk::Offset3D(copyRegion.getWidth(), srcOffset.getY(), copyRegion.getDepth()) };

	std::vector<unsigned char> outData;
	outData.resize(static_cast<const unsigned int>(dstImage->getMemoryRequirement().getSize()));

	// create the final destination buffer for reading
	pvrvk::Buffer buffer = createBuffer(device, pvrvk::BufferCreateInfo(dstImage->getMemoryRequirement().getSize(), pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, bufferAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	buffer->setObjectName("PVRUtilsVk::screenCaptureRegion::Temporary Screen Capture Buffer");

	cmdBuffer->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("PVRUtilsVk::screenCaptureRegion"));
	pvrvk::ImageBlit copyRange(pvrvk::ImageSubresourceLayers(), srcOffsets, pvrvk::ImageSubresourceLayers(), dstOffsets);

	// transform the layout from the color attachment to transfer src
	if (imageInitialLayout != pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL) { setImageLayout(image, imageInitialLayout, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, cmdBuffer); }
	setImageLayout(dstImage, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, cmdBuffer);

	cmdBuffer->blitImage(image, dstImage, &copyRange, 1, pvrvk::Filter::e_NEAREST, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);

	pvrvk::ImageSubresourceLayers subResource;
	subResource.setAspectMask(pvrvk::ImageAspectFlags::e_COLOR_BIT);
	pvrvk::BufferImageCopy region(0, 0, 0, subResource, pvrvk::Offset3D(0, 0, 0), copyRegion);

	if (imageInitialLayout != pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL) { setImageLayout(image, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, imageFinalLayout, cmdBuffer); }
	setImageLayout(dstImage, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, cmdBuffer);

	cmdBuffer->copyImageToBuffer(dstImage, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, buffer, &region, 1);
	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	cmdBuffer->end();
	// create a fence for wait.
	pvrvk::Fence fenceWait = device->createFence(pvrvk::FenceCreateFlags(0));
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cmdBuffer;
	submitInfo.numCommandBuffers = 1;
	queue->submit(&submitInfo, 1, fenceWait);
	fenceWait->wait(); // wait for the submit to finish so that the command buffer get destroyed properly

	// map the buffer and copy the data
	void* memory = 0;
	unsigned char* data = nullptr;
	bool unmap = false;
	if (!buffer->getDeviceMemory()->isMapped())
	{
		memory = buffer->getDeviceMemory()->map(0, dstImage->getMemoryRequirement().getSize());
		unmap = true;
	}
	else
	{
		memory = buffer->getDeviceMemory()->getMappedData();
	}
	data = static_cast<unsigned char*>(memory);
	memcpy(outData.data(), data, static_cast<size_t>(dstImage->getMemoryRequirement().getSize()));

	if (static_cast<uint32_t>(buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		buffer->getDeviceMemory()->invalidateRange(0, dstImage->getMemoryRequirement().getSize());
	}
	if (unmap) { buffer->getDeviceMemory()->unmap(); }
	return outData;
}

bool takeScreenshot(pvrvk::Queue& queue, pvrvk::CommandPool& cmdPool, pvrvk::Swapchain& swapchain, const uint32_t swapIndex, const std::string& screenshotFileName,
	vma::Allocator bufferAllocator, vma::Allocator imageAllocator, const uint32_t screenshotScale)
{
	pvr::utils::beginQueueDebugLabel(queue, pvrvk::DebugUtilsLabel("PVRUtilsVk::takeScreenshot"));

	if (!swapchain->supportsUsage(pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		Log(LogLevel::Warning, "Could not take screenshot as the swapchain does not support TRANSFER_SRC_BIT");
		return false;
	}
	// force the queue to wait idle prior to taking a copy of the swap chain image
	queue->waitIdle();

	saveImage(queue, cmdPool, swapchain->getImage(swapIndex), pvrvk::ImageLayout::e_PRESENT_SRC_KHR, pvrvk::ImageLayout::e_PRESENT_SRC_KHR, screenshotFileName, bufferAllocator,
		imageAllocator, screenshotScale);

	pvr::utils::endQueueDebugLabel(queue);

	return true;
}

void saveImage(pvrvk::Queue& queue, pvrvk::CommandPool& cmdPool, pvrvk::Image& image, const pvrvk::ImageLayout imageInitialLayout, const pvrvk::ImageLayout imageFinalLayout,
	const std::string& filename, vma::Allocator bufferAllocator, vma::Allocator imageAllocator, const uint32_t screenshotScale)
{
	pvrvk::Format destinationImageFormat = pvrvk::Format::e_B8G8R8A8_SRGB;

	// Handle Linear Images.
	if (!pvrvk::isSrgb(image->getFormat())) { destinationImageFormat = pvrvk::Format::e_B8G8R8A8_UNORM; }

	std::vector<unsigned char> imageData = captureImageRegion(queue, cmdPool, image, pvrvk::Offset3D(0, 0, 0),
		pvrvk::Extent3D(image->getExtent().getWidth(), image->getExtent().getHeight(), image->getExtent().getDepth()), destinationImageFormat, imageInitialLayout, imageFinalLayout,
		bufferAllocator, imageAllocator);
	Log(LogLevel::Information, "Writing TGA screenshot, filename %s.", filename.c_str());
	writeTGA(filename.c_str(), image->getExtent().getWidth(), image->getExtent().getHeight(), imageData.data(), 4, screenshotScale);
}

#pragma endregion

#pragma region //////////////// DEBUG_UTILS /////////////////

// Nasty global, but fit for purpose as a configuration switch.
bool PVRUtils_Throw_On_Validation_Error = true;

namespace {
std::string debugUtilsMessengerCallbackToString(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT msgTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
{
	std::string messageSeverityString = pvrvk::to_string(static_cast<pvrvk::DebugUtilsMessageSeverityFlagsEXT>(messageSeverity));
	std::string messageTypeString = pvrvk::to_string(static_cast<pvrvk::DebugUtilsMessageTypeFlagsEXT>(msgTypes));

	std::string exceptionMessage = pvr::strings::createFormatted("%s (%s) - ID: %i, Name: \"%s\":\n\tMESSAGE: %s", messageSeverityString.c_str(), messageTypeString.c_str(),
		pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);

	if (pCallbackData->objectCount > 0)
	{
		exceptionMessage += "\n";
		std::string objectsMessage = pvr::strings::createFormatted("\tAssociated Objects - (%u)\n", pCallbackData->objectCount);

		for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
		{
			std::string objectType = pvrvk::to_string(static_cast<pvrvk::ObjectType>(pCallbackData->pObjects[i].objectType));

			objectsMessage += pvr::strings::createFormatted("\t\tObject[%u] - Type %s, Value %p, Name \"%s\"\n", i, objectType.c_str(),
				(void*)(pCallbackData->pObjects[i].objectHandle), pCallbackData->pObjects[i].pObjectName ? pCallbackData->pObjects[i].pObjectName : "unnamed-object");
		}

		exceptionMessage += objectsMessage;
	}

	if (pCallbackData->cmdBufLabelCount > 0)
	{
		exceptionMessage += "\n";
		std::string cmdBufferLabelsMessage = pvr::strings::createFormatted("\tAssociated Command Buffer Labels - (%u)\n", pCallbackData->cmdBufLabelCount);

		for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
		{
			cmdBufferLabelsMessage += pvr::strings::createFormatted("\t\tCommand Buffer Label[%u] - %s, Color: {%f, %f, %f, %f}\n", i, pCallbackData->pCmdBufLabels[i].pLabelName,
				pCallbackData->pCmdBufLabels[i].color[0], pCallbackData->pCmdBufLabels[i].color[1], pCallbackData->pCmdBufLabels[i].color[2], pCallbackData->pCmdBufLabels[i].color[3]);
		}

		exceptionMessage += cmdBufferLabelsMessage;
	}

	if (pCallbackData->queueLabelCount > 0)
	{
		exceptionMessage += "\n";
		std::string queueLabelsMessage = pvr::strings::createFormatted("\tAssociated Queue Labels - (%u)\n", pCallbackData->queueLabelCount);

		for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
		{
			queueLabelsMessage += pvr::strings::createFormatted("\t\tQueue Label[%u] - %s, Color: {%f, %f, %f, %f}\n", i, pCallbackData->pQueueLabels[i].pLabelName,
				pCallbackData->pQueueLabels[i].color[0], pCallbackData->pQueueLabels[i].color[1], pCallbackData->pQueueLabels[i].color[2], pCallbackData->pQueueLabels[i].color[3]);
		}

		exceptionMessage += queueLabelsMessage;
	}
	return exceptionMessage;
}
} // namespace

// An application defined callback used as the callback function specified in as pfnCallback in the
// create info VkDebugUtilsMessengerCreateInfoEXT used when creating the debug utils messenger callback vkCreateDebugUtilsMessengerEXT
VKAPI_ATTR VkBool32 VKAPI_CALL throwOnErrorDebugUtilsMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT msgTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	// throw an exception if the type of DebugUtilsMessageSeverityFlagsEXT contains the ERROR_BIT
	if (PVRUtils_Throw_On_Validation_Error &&
		((static_cast<pvrvk::DebugUtilsMessageSeverityFlagsEXT>(messageSeverity) & (pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_ERROR_BIT_EXT)) !=
			pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_NONE))
	{
		throw pvrvk::ErrorValidationFailedEXT(debugUtilsMessengerCallbackToString(messageSeverity, msgTypes, pCallbackData));
	}
	return VK_FALSE;
}

// The application defined callback used as the callback function specified in as pfnCallback in the
// create info VkDebugUtilsMessengerCreateInfoEXT used when creating the debug utils messenger callback vkCreateDebugUtilsMessengerEXT
VKAPI_ATTR VkBool32 VKAPI_CALL logMessageDebugUtilsMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT msgTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::vector<int>* vectorValidationIDFilter = static_cast<std::vector<int>*>(pUserData);
	if ((vectorValidationIDFilter != nullptr) &&
		(std::find(vectorValidationIDFilter->begin(), vectorValidationIDFilter->end(), pCallbackData->messageIdNumber) != vectorValidationIDFilter->end()))
	{
		return VK_FALSE;
	}

	Log(mapDebugUtilsMessageSeverityFlagsToLogLevel(static_cast<pvrvk::DebugUtilsMessageSeverityFlagsEXT>(messageSeverity)),
		debugUtilsMessengerCallbackToString(messageSeverity, msgTypes, pCallbackData).c_str());

	return VK_FALSE;
}

// The application defined callback used as the callback function specified in as pfnCallback in the
// create info VkDebugReportCallbackCreateInfoEXT used when creating the debug report callback vkCreateDebugReportCallbackEXT
VKAPI_ATTR VkBool32 VKAPI_CALL throwOnErrorDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
	int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	(void)object;
	(void)location;
	(void)messageCode;
	(void)pLayerPrefix;
	(void)pUserData;

	// throw an exception if the type of VkDebugReportFlagsEXT contains the ERROR_BIT
	if (PVRUtils_Throw_On_Validation_Error && ((static_cast<pvrvk::DebugReportFlagsEXT>(flags) & pvrvk::DebugReportFlagsEXT::e_ERROR_BIT_EXT) != pvrvk::DebugReportFlagsEXT::e_NONE))
	{
		throw pvrvk::ErrorValidationFailedEXT(
			std::string(pvrvk::to_string(static_cast<pvrvk::DebugReportObjectTypeEXT>(objectType)) + std::string(". VULKAN_LAYER_VALIDATION: ") + pMessage));
	}
	return VK_FALSE;
}

// The application defined callback used as the callback function specified in as pfnCallback in the
// create info VkDebugReportCallbackCreateInfoEXT used when creating the debug report callback vkCreateDebugReportCallbackEXT
VKAPI_ATTR VkBool32 VKAPI_CALL logMessageDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
	int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	(void)object;
	(void)location;
	(void)messageCode;
	(void)pLayerPrefix;
	(void)pUserData;
	// map the VkDebugReportFlagsEXT to a suitable log type
	// map the VkDebugReportObjectTypeEXT to a stringified representation
	// Log the message generated by a lower layer
	Log(mapDebugReportFlagsToLogLevel(static_cast<pvrvk::DebugReportFlagsEXT>(flags)),
		std::string(pvrvk::to_string(static_cast<pvrvk::DebugReportObjectTypeEXT>(objectType)) + std::string(". VULKAN_LAYER_VALIDATION: %s")).c_str(), pMessage);

	return VK_FALSE;
}

DebugUtilsCallbacks createDebugUtilsCallbacks(pvrvk::Instance& instance, void* pUserData)
{
	DebugUtilsCallbacks debugUtilsCallbacks;

	if (instance->getEnabledExtensionTable().extDebugUtilsEnabled)
	{
		{
			// Create a second Debug Utils Messenger for throwing exceptions for Error events.
			pvrvk::DebugUtilsMessengerCreateInfo createInfo = pvrvk::DebugUtilsMessengerCreateInfo(
				pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_ERROR_BIT_EXT, pvrvk::DebugUtilsMessageTypeFlagsEXT::e_ALL_BITS, pvr::utils::throwOnErrorDebugUtilsMessengerCallback);
			debugUtilsCallbacks.debugUtilsMessengers[1] = instance->createDebugUtilsMessenger(createInfo);
		}
		// Create Debug Utils Messengers
		{
			// Create a Debug Utils Messenger which will trigger our callback for logging messages for events of warning and error types of all severities
			pvrvk::DebugUtilsMessengerCreateInfo createInfo =
				pvrvk::DebugUtilsMessengerCreateInfo(pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_ERROR_BIT_EXT | pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_WARNING_BIT_EXT,
					pvrvk::DebugUtilsMessageTypeFlagsEXT::e_ALL_BITS, logMessageDebugUtilsMessengerCallback, pUserData);

			debugUtilsCallbacks.debugUtilsMessengers[0] = instance->createDebugUtilsMessenger(createInfo);
		}
	}
	else if (instance->getEnabledExtensionTable().extDebugReportEnabled)
	{
		{
			// Create a second Debug Report Callback for throwing exceptions for Error events.
			pvrvk::DebugReportCallbackCreateInfo createInfo =
				pvrvk::DebugReportCallbackCreateInfo(pvrvk::DebugReportFlagsEXT::e_ERROR_BIT_EXT, pvr::utils::throwOnErrorDebugReportCallback);

			debugUtilsCallbacks.debugCallbacks[1] = instance->createDebugReportCallback(createInfo);
		}
		// Create Debug Report Callbacks
		{
			// Create a Debug Report Callback for logging messages for events of error, performance or warning types.
			pvrvk::DebugReportCallbackCreateInfo createInfo = pvrvk::DebugReportCallbackCreateInfo(
				pvrvk::DebugReportFlagsEXT::e_ERROR_BIT_EXT | pvrvk::DebugReportFlagsEXT::e_PERFORMANCE_WARNING_BIT_EXT | pvrvk::DebugReportFlagsEXT::e_WARNING_BIT_EXT,
				logMessageDebugReportCallback);

			debugUtilsCallbacks.debugCallbacks[0] = instance->createDebugReportCallback(createInfo);
		}
	}

	return debugUtilsCallbacks;
}
#pragma endregion

#pragma region ////////////// OBJECT_CREATION ///////////////
pvrvk::Instance createInstance(const std::string& applicationName, VulkanVersion version, const InstanceExtensions& instanceExtensions, const InstanceLayers& instanceLayers,
	const pvrvk::DebugUtilsMessageSeverityFlagsEXT InstanceValidationFlags)
{
	pvrvk::InstanceCreateInfo instanceInfo;
	pvrvk::ApplicationInfo appInfo;
	appInfo.setApplicationName(applicationName);
	appInfo.setApplicationVersion(1);
	appInfo.setEngineName("PVRVk");
	appInfo.setEngineVersion(0);

	// Retrieve the vulkan bindings

	uint32_t major = -1;
	uint32_t minor = -1;
	uint32_t patch = -1;

	// Retrive the version of the vulkan loader, this can be done through vkEnumerateInstanceVersion, this function was supplied by Vulkan 1.1, so if the function pointer is not
	// valid, then we know Vulkan loader is only version 1.0. Otherwise we can use vkEnumerateInstanceVersion to determine the maximum instance version supported by the loader.
	if (pvrvk::getVkBindings().vkEnumerateInstanceVersion)
	{
		uint32_t supportedApiVersion;
		pvrvk::getVkBindings().vkEnumerateInstanceVersion(&supportedApiVersion);

		major = VK_VERSION_MAJOR(supportedApiVersion);
		minor = VK_VERSION_MINOR(supportedApiVersion);
		patch = VK_VERSION_PATCH(supportedApiVersion);

		Log(LogLevel::Information, "The function pointer for 'vkEnumerateInstanceVersion' was valid. Vulkan loader instance version : ([%d].[%d].[%d]).", major, minor, patch);
	}
	else
	{
		major = 1;
		minor = 0;
		patch = 0;
		Log(LogLevel::Information, "Could not find a function pointer for 'vkEnumerateInstanceVersion'. Vulkan loader instance version is : ([%d].[%d].[%d]).", major, minor, patch);
	}

	// Check the maximum supported instance version against the version requested by the application
	VulkanVersion LoaderVersion = VulkanVersion(major, minor, patch);
	if (LoaderVersion.toVulkanVersion() < version.toVulkanVersion())
	{
		Log(LogLevel::Warning, "The instance version supported by the Vulkan loader on your device is less than the Application's requested Vulkan version : ([%d].[%d].[%d])",
			version.majorV, version.minorV, version.patchV);
		// Attempt to continue by forcing the application to use the maximum supported instance version
		version = LoaderVersion;
	}
	else
	{
		Log(LogLevel::Information, "The instance version requested by the application ([%d].[%d].[%d]) is supported by the Vulkan loader on your device ", version.majorV,
			version.minorV, version.patchV);
	}

	// Print out the supported instance extensions
	std::vector<pvrvk::ExtensionProperties> extensionProperties;
	pvrvk::Extensions::enumerateInstanceExtensions(extensionProperties);

	std::vector<pvrvk::LayerProperties> layerProperties;
	pvrvk::Layers::enumerateInstanceLayers(layerProperties);

	if (instanceLayers.getNumLayers())
	{
		pvrvk::VulkanLayerList supportedLayers = pvrvk::Layers::filterLayers(layerProperties, instanceLayers);

		std::string standardValidationLayerString = "VK_LAYER_LUNARG_standard_validation";

		bool requestedStandardValidation = instanceLayers.containsLayer(standardValidationLayerString);
		bool supportsStandardValidation = supportedLayers.containsLayer(standardValidationLayerString);
		bool supportsKhronosValidation = supportedLayers.containsLayer("VK_LAYER_KHRONOS_validation");
		uint32_t standardValidationRequiredIndex = -1;

		// This code is to cover cases where VK_LAYER_LUNARG_standard_validation is requested but is not supported, where on some platforms the
		// component layers enabled via VK_LAYER_LUNARG_standard_validation may still be supported even though VK_LAYER_LUNARG_standard_validation is not.
		// Only perform the expansion if VK_LAYER_LUNARG_standard_validation is requested and not supported and the newer equivalent layer VK_LAYER_KHRONOS_validation is also not supported
		if (requestedStandardValidation && !supportsStandardValidation && !supportsKhronosValidation)
		{
			for (auto it = layerProperties.begin(); !supportsStandardValidation && it != layerProperties.end(); ++it)
			{
				supportsStandardValidation = !strcmp(it->getLayerName(), standardValidationLayerString.c_str());
			}
			if (!supportsStandardValidation)
			{
				for (uint32_t i = 0; standardValidationRequiredIndex == static_cast<uint32_t>(-1) && i < layerProperties.size(); ++i)
				{
					if (!strcmp(instanceLayers.getLayer(i).getName().c_str(), standardValidationLayerString.c_str())) { standardValidationRequiredIndex = i; }
				}

				for (uint32_t j = 0; j < instanceLayers.getNumLayers(); ++j)
				{
					if (standardValidationRequiredIndex == j && !supportsStandardValidation)
					{
						const char* stdValComponents[] = { "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker",
							"VK_LAYER_LUNARG_core_validation", "VK_LAYER_GOOGLE_unique_objects" };
						for (uint32_t k = 0; k < sizeof(stdValComponents) / sizeof(stdValComponents[0]); ++k)
						{
							for (uint32_t i = 0; i < layerProperties.size(); ++i)
							{
								if (!strcmp(stdValComponents[k], layerProperties[i].getLayerName()))
								{
									supportedLayers.addLayer(pvrvk::VulkanLayer(std::string(stdValComponents[k])));
									break;
								}
							}
						}
					}
				}

				// filter the layers again checking for support for the component layers enabled via VK_LAYER_LUNARG_standard_validation
				supportedLayers = pvrvk::Layers::filterLayers(layerProperties, supportedLayers);
			}
		}

		instanceInfo.setLayerList(supportedLayers);

		// For each layer retrieve each of the extensions it provides implementations for and add it into the main list
		for (uint32_t i = 0; i < instanceInfo.getLayerList().getNumLayers(); ++i)
		{
			std::vector<pvrvk::ExtensionProperties> perLayerExtensionProperties;
			pvrvk::Extensions::enumerateInstanceExtensions(perLayerExtensionProperties, instanceInfo.getLayerList().getLayer(i).getName());

			extensionProperties.insert(extensionProperties.end(), perLayerExtensionProperties.begin(), perLayerExtensionProperties.end());
		}
	}

	if (instanceExtensions.getNumExtensions())
	{
		pvrvk::VulkanExtensionList supportedRequestedExtensions = pvrvk::Extensions::filterExtensions(extensionProperties, instanceExtensions);

		// Determine whether VK_EXT_debug_utils is supported
		bool debugUtilsSupported = supportedRequestedExtensions.containsExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		if (debugUtilsSupported)
		{
			pvrvk::DebugUtilsMessengerCreateInfo debugUtilsMessengerCreateInfo(
				InstanceValidationFlags, pvrvk::DebugUtilsMessageTypeFlagsEXT::e_ALL_BITS, pvr::utils::logMessageDebugUtilsMessengerCallback);
			instanceInfo.setDebugUtilsMessengerCreateInfo(debugUtilsMessengerCreateInfo);
		}

		// Determine whether VK_EXT_debug_report is supported
		bool debugReportSupported = supportedRequestedExtensions.containsExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		// If VK_EXT_debug_utils is supported then remove VK_EXT_debug_report from the list of extension to enable therefore we are prioritising use of VK_EXT_debug_utils
		if (debugUtilsSupported && debugReportSupported)
		{
			Log(LogLevel::Information, "VK_EXT_debug_utils and VK_EXT_debug_report are both supported. We will be using VK_EXT_debug_utils.");
			supportedRequestedExtensions.removeExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		// Determine whether VK_EXT_validation_features is supported
		bool validationFeaturesSupported = supportedRequestedExtensions.containsExtension(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
		if (validationFeaturesSupported)
		{
			pvrvk::ValidationFeatures validationFeatures;
			validationFeatures.addEnabledValidationFeature(pvrvk::ValidationFeatureEnableEXT::e_GPU_ASSISTED_EXT);
			validationFeatures.addEnabledValidationFeature(pvrvk::ValidationFeatureEnableEXT::e_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT);
			validationFeatures.addEnabledValidationFeature(pvrvk::ValidationFeatureEnableEXT::e_BEST_PRACTICES_EXT);
			// sets a list of validation features to enable when creating the instance
			instanceInfo.setValidationFeatures(validationFeatures);
		}

		instanceInfo.setExtensionList(supportedRequestedExtensions);
	}

	Log(LogLevel::Information, "Supported Instance Extensions:");
	for (uint32_t i = 0; i < static_cast<uint32_t>(extensionProperties.size()); ++i)
	{
		Log(LogLevel::Information, "\t%s : version [%u]", extensionProperties[i].getExtensionName(), extensionProperties[i].getSpecVersion());
	}

	if (instanceExtensions.getNumExtensions())
	{
		Log(LogLevel::Information, "Supported Instance Extensions to be Enabled:");
		for (uint32_t i = 0; i < static_cast<uint32_t>(instanceInfo.getExtensionList().getNumExtensions()); ++i)
		{
			Log(LogLevel::Information, "\t%s : version [%u]", instanceInfo.getExtensionList().getExtension(i).getName().c_str(),
				instanceInfo.getExtensionList().getExtension(i).getSpecVersion());
		}
	}

	Log(LogLevel::Information, "Supported Instance Layers:");
	for (uint32_t i = 0; i < static_cast<uint32_t>(layerProperties.size()); ++i)
	{
		Log(LogLevel::Information, "\t%s : Spec version [%u], Implementation version [%u]", layerProperties[i].getLayerName(), layerProperties[i].getSpecVersion(),
			layerProperties[i].getImplementationVersion());
	}

	if (instanceLayers.getNumLayers())
	{
		Log(LogLevel::Information, "Supported Instance Layers to be Enabled:");
		for (uint32_t i = 0; i < instanceInfo.getLayerList().getNumLayers(); ++i)
		{
			Log(LogLevel::Information, "\t%s : Spec version [%u], Spec version [%u]", instanceInfo.getLayerList().getLayer(i).getName().c_str(),
				instanceInfo.getLayerList().getLayer(i).getSpecVersion(), instanceInfo.getLayerList().getLayer(i).getImplementationVersion());
		}
	}

	appInfo.setApiVersion(version.toVulkanVersion());
	instanceInfo.setApplicationInfo(appInfo);

	pvrvk::Instance outInstance = pvrvk::createInstance(instanceInfo);
	outInstance->retrievePhysicalDevices();

	const pvrvk::ApplicationInfo& instanceAppInfo = outInstance->getCreateInfo().getApplicationInfo();
	Log(LogLevel::Information, "Created Vulkan Instance:");
	Log(LogLevel::Information, "	Application Name: %s.", instanceAppInfo.getApplicationName().c_str());
	Log(LogLevel::Information, "	Application Version: %d.", instanceAppInfo.getApplicationVersion());
	Log(LogLevel::Information, "	Engine Name: %s.", instanceAppInfo.getEngineName().c_str());
	Log(LogLevel::Information, "	Engine Version: %d.", instanceAppInfo.getEngineVersion());
	Log(LogLevel::Information, "	Version: %d / ([%d].[%d].[%d]).", instanceAppInfo.getApiVersion(), version.majorV, version.minorV, version.patchV);

	const std::vector<pvrvk::PhysicalDevice>& physicalDevices = outInstance->getPhysicalDevices();

	Log(LogLevel::Information, "Supported Vulkan Physical devices:");

	for (uint32_t i = 0; i < physicalDevices.size(); ++i)
	{
		pvrvk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevices[i]->getProperties();

		uint32_t deviceMajor = VK_VERSION_MAJOR(physicalDeviceProperties.getApiVersion());
		uint32_t deviceMinor = VK_VERSION_MINOR(physicalDeviceProperties.getApiVersion());
		uint32_t devicePatch = VK_VERSION_PATCH(physicalDeviceProperties.getApiVersion());

		Log(LogLevel::Information, "	Device Name: %s.", physicalDeviceProperties.getDeviceName());
		Log(LogLevel::Information, "	Device ID: 0x%X.", physicalDeviceProperties.getDeviceID());
		Log(LogLevel::Information, "	Api Version Supported: %d / ([%d].[%d].[%d]).", physicalDeviceProperties.getApiVersion(), deviceMajor, deviceMinor, devicePatch);
		Log(LogLevel::Information, "	Device Type: %s.", pvrvk::to_string(physicalDeviceProperties.getDeviceType()).c_str());
		Log(LogLevel::Information, "	Driver version: 0x%X.", physicalDeviceProperties.getDriverVersion());
		Log(LogLevel::Information, "	Vendor ID: %d.", physicalDeviceProperties.getVendorID());

		Log(LogLevel::Information, "	Memory Configuration:");
		auto memprop = physicalDevices[i]->getMemoryProperties();

		for (uint32_t heapIdx = 0; heapIdx < memprop.getMemoryHeapCount(); ++heapIdx)
		{
			auto heap = memprop.getMemoryHeaps()[heapIdx];
			std::string s = to_string(heap.getFlags());
			Log(LogLevel::Information, "		Heap:[%d] Size:[%zuMB] Flags: [%d (%s) ]", heapIdx, static_cast<uint32_t>(heap.getSize() / 1024ull * 1024ull),
				static_cast<uint32_t>(heap.getFlags()), s.c_str());
			for (uint32_t typeIdx = 0; typeIdx < memprop.getMemoryTypeCount(); ++typeIdx)
			{
				auto type = memprop.getMemoryTypes()[typeIdx];
				if (type.getHeapIndex() == heapIdx)
					Log(LogLevel::Information, "			Memory Type: [%d] Flags: [ %d (%s) ] ", typeIdx, type.getPropertyFlags(), to_string(type.getPropertyFlags()).c_str());
			}
		}
	}
	return outInstance;
}

pvrvk::Surface createSurface(const pvrvk::Instance& instance, const pvrvk::PhysicalDevice& physicalDevice, void* window, void* display, void* connection)
{
	(void)physicalDevice; // hide warning
	(void)connection;
	(void)display;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	Log(LogLevel::Information, "Using platform define: VK_USE_PLATFORM_ANDROID_KHR");
	if (instance->getEnabledExtensionTable().khrAndroidSurfaceEnabled)
	{
		Log(LogLevel::Information, "Using Instance surface extension: VK_KHR_android_surface");
		return pvrvk::Surface(instance->createAndroidSurface(reinterpret_cast<ANativeWindow*>(window)));
	}
#elif defined VK_USE_PLATFORM_WIN32_KHR
	Log(LogLevel::Information, "Using platform define: VK_USE_PLATFORM_WIN32_KHR");
	if (instance->getEnabledExtensionTable().khrWin32SurfaceEnabled)
	{
		Log(LogLevel::Information, "Using Instance surface extension: VK_KHR_win32_surface");
		return pvrvk::Surface(instance->createWin32Surface(GetModuleHandle(NULL), static_cast<HWND>(window)));
	}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	Log(LogLevel::Information, "Using platform define: VK_USE_PLATFORM_XCB_KHR");
	if (instance->getEnabledExtensionTable().khrXcbSurfaceEnabled)
	{
		Log(LogLevel::Information, "Using Instance surface extension: VK_KHR_xcb_surface");
		return pvrvk::Surface(instance->createXcbSurface(static_cast<xcb_connection_t*>(connection), *((xcb_window_t*)(&window))));
	}
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	Log(LogLevel::Information, "Using platform define: VK_USE_PLATFORM_XLIB_KHR");
	if (instance->getEnabledExtensionTable().khrXlibSurfaceEnabled)
	{
		Log(LogLevel::Information, "Using Instance surface extension: VK_KHR_xlib_surface");
		return pvrvk::Surface(instance->createXlibSurface(static_cast<Display*>(display), reinterpret_cast<Window>(window)));
	}
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	Log(LogLevel::Information, "Using platform define: VK_USE_PLATFORM_WAYLAND_KHR");
	if (instance->getEnabledExtensionTable().khrWaylandSurfaceEnabled)
	{
		Log(LogLevel::Information, "Using Instance surface extension: VK_KHR_wayland_surface");
		return pvrvk::Surface(instance->createWaylandSurface(reinterpret_cast<wl_display*>(display), reinterpret_cast<wl_surface*>(window)));
	}
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	Log(LogLevel::Information, "Using platform define: VK_USE_PLATFORM_MACOS_MVK");
	(void)display;
	if (instance->getEnabledExtensionTable().mvkMacosSurfaceEnabled)
	{
		Log(LogLevel::Information, "Using Instance surface extension: VK_MVK_macos_surface");
		return pvrvk::Surface(instance->createMacOSSurface(window));
	}
#else // NullWS
	if (instance->getEnabledExtensionTable().khrDisplayEnabled)
	{
		Log(LogLevel::Information, "Using Instance surface extension: VK_KHR_display");
		Log(LogLevel::Information, "%u Displays supported by the physical device", physicalDevice->getNumDisplays());
		Log(LogLevel::Information, "Display properties:");

		for (uint32_t i = 0; i < physicalDevice->getNumDisplays(); ++i)
		{
			const pvrvk::Display& display = physicalDevice->getDisplay(i);
			Log(LogLevel::Information, "Properties for Display [%u]:", i);
			Log(LogLevel::Information, "	Display Name: '%s':", display->getDisplayName());
			Log(LogLevel::Information, "	Supports Persistent Content: %u", display->getPersistentContent());
			Log(LogLevel::Information, "	Physical Dimensions: (%u, %u)", display->getPhysicalDimensions().getWidth(), display->getPhysicalDimensions().getHeight());
			Log(LogLevel::Information, "	Physical Resolution: (%u, %u)", display->getPhysicalResolution().getWidth(), display->getPhysicalResolution().getHeight());
			Log(LogLevel::Information, "	Supported Transforms: %s", pvrvk::to_string(display->getSupportedTransforms()).c_str());
			Log(LogLevel::Information, "	Supports Plane Reorder: %u", display->getPlaneReorderPossible());

			Log(LogLevel::Information, "	Display supports [%u] display modes:", display->getNumDisplayModes());
			for (uint32_t j = 0; j < display->getNumDisplayModes(); ++j)
			{
				Log(LogLevel::Information, "	Properties for Display Mode [%u]:", j);
				const pvrvk::DisplayMode& displayMode = display->getDisplayMode(j);
				Log(LogLevel::Information, "		Refresh Rate: %f", displayMode->getParameters().getRefreshRate());
				Log(LogLevel::Information, "		Visible Region: (%u, %u)", displayMode->getParameters().getVisibleRegion().getWidth(),
					displayMode->getParameters().getVisibleRegion().getHeight());
			}
		}

		if (physicalDevice->getNumDisplays() == 0) { throw pvrvk::ErrorInitializationFailed("Could not find a suitable Vulkan Display."); }

		// We simply loop through the display planes and find a supported display and display mode
		for (uint32_t i = 0; i < physicalDevice->getNumDisplayPlanes(); ++i)
		{
			uint32_t currentStackIndex = -1;
			pvrvk::Display display = physicalDevice->getDisplayPlaneProperties(i, currentStackIndex);
			std::vector<pvrvk::Display> supportedDisplaysForPlane = physicalDevice->getDisplayPlaneSupportedDisplays(i);
			pvrvk::DisplayMode displayMode;

			// if a valid display can be found and its supported then make use of it
			if (display && std::find(supportedDisplaysForPlane.begin(), supportedDisplaysForPlane.end(), display) != supportedDisplaysForPlane.end())
			{
				displayMode = display->getDisplayMode(0);
			} // else find the first supported display and grab its first display mode
			else if (supportedDisplaysForPlane.size())
			{
				pvrvk::Display& currentDisplay = supportedDisplaysForPlane[0];
				displayMode = currentDisplay->getDisplayMode(0);
			}

			if (displayMode)
			{
				pvrvk::DisplayPlaneCapabilitiesKHR capabilities = physicalDevice->getDisplayPlaneCapabilities(displayMode, i);
				Log(LogLevel::Information, "Capabilities for the chosen display mode for Display Plane [%u]:", i);
				Log(LogLevel::Information, "	Supported Alpha Flags: %s", pvrvk::to_string(capabilities.getSupportedAlpha()).c_str());
				Log(LogLevel::Information, "	Supported Min Src Position: (%u, %u)", capabilities.getMinSrcPosition().getX(), capabilities.getMinSrcPosition().getY());
				Log(LogLevel::Information, "	Supported Max Src Position: (%u, %u)", capabilities.getMaxSrcPosition().getX(), capabilities.getMaxSrcPosition().getY());
				Log(LogLevel::Information, "	Supported Min Src Extent: (%u, %u)", capabilities.getMinSrcExtent().getWidth(), capabilities.getMinSrcExtent().getHeight());
				Log(LogLevel::Information, "	Supported Max Src Extent: (%u, %u)", capabilities.getMaxSrcExtent().getWidth(), capabilities.getMaxSrcExtent().getHeight());
				Log(LogLevel::Information, "	Supported Min Dst Position: (%u, %u)", capabilities.getMinDstPosition().getX(), capabilities.getMinDstPosition().getY());
				Log(LogLevel::Information, "	Supported Max Dst Position: (%u, %u)", capabilities.getMaxDstPosition().getX(), capabilities.getMaxDstPosition().getY());
				Log(LogLevel::Information, "	Supported Min Dst Extent: (%u, %u)", capabilities.getMinDstExtent().getWidth(), capabilities.getMinDstExtent().getHeight());
				Log(LogLevel::Information, "	Supported Max Dst Extent: (%u, %u)", capabilities.getMaxDstExtent().getWidth(), capabilities.getMaxDstExtent().getHeight());

				return pvrvk::Surface(instance->createDisplayPlaneSurface(
					displayMode, displayMode->getParameters().getVisibleRegion(), pvrvk::DisplaySurfaceCreateFlagsKHR::e_NONE, i, currentStackIndex));
			}
		}
	}
#endif

	throw pvrvk::ErrorInitializationFailed("We were unable to create a suitable Surface for the given physical device, provided platform defines and supported surface "
										   "extensions.");
}

pvrvk::Buffer createBuffer(const pvrvk::Device& device, const pvrvk::BufferCreateInfo& createInfo, pvrvk::MemoryPropertyFlags requiredMemoryFlags,
	pvrvk::MemoryPropertyFlags optimalMemoryFlags, const vma::Allocator& bufferAllocator, vma::AllocationCreateFlags vmaAllocationCreateFlags,
	pvrvk::MemoryAllocateFlags memoryAllocateFlags)
{
	// create the PVRVk Buffer
	pvrvk::Buffer buffer = device->createBuffer(createInfo);

	// if the required memory flags is pvrvk::MemoryPropertyFlags::e_NONE then no backing will be provided for the buffer
	if (requiredMemoryFlags != pvrvk::MemoryPropertyFlags::e_NONE)
	{
		// use the allocator
		if (bufferAllocator)
		{
			vma::AllocationCreateInfo allocationInfo;
			allocationInfo.usage = vma::MemoryUsage::e_UNKNOWN;
			allocationInfo.requiredFlags = requiredMemoryFlags;
			allocationInfo.preferredFlags = optimalMemoryFlags | requiredMemoryFlags;
			allocationInfo.flags = vmaAllocationCreateFlags;
			allocationInfo.memoryTypeBits = buffer->getMemoryRequirement().getMemoryTypeBits();
			vma::Allocation allocation;
			allocation = bufferAllocator->allocateMemoryForBuffer(buffer, allocationInfo);
			buffer->bindMemory(pvrvk::DeviceMemory(allocation), allocation->getOffset());
		}
		else
		{
			// get the buffer memory requirements, memory type index and memory property flags required for backing the PVRVk buffer
			const pvrvk::MemoryRequirements& memoryRequirements = buffer->getMemoryRequirement();
			uint32_t memoryTypeIndex;
			pvrvk::MemoryPropertyFlags memoryPropertyFlags;
			getMemoryTypeIndex(device->getPhysicalDevice(), memoryRequirements.getMemoryTypeBits(), requiredMemoryFlags, optimalMemoryFlags, memoryTypeIndex, memoryPropertyFlags);

			// allocate the buffer memory using the retrieved memory type index and memory property flags
			pvrvk::DeviceMemory deviceMemory = device->allocateMemory(pvrvk::MemoryAllocationInfo(buffer->getMemoryRequirement().getSize(), memoryTypeIndex), memoryAllocateFlags);

			// attach the memory to the buffer
			buffer->bindMemory(deviceMemory, 0);
		}
	}
	return buffer;
}

pvrvk::Image createImage(const pvrvk::Device& device, const pvrvk::ImageCreateInfo& createInfo, pvrvk::MemoryPropertyFlags requiredMemoryFlags,
	pvrvk::MemoryPropertyFlags optimalMemoryFlags, const vma::Allocator& imageAllocator, vma::AllocationCreateFlags vmaAllocationCreateFlags)
{
	// create the PVRVk Image
	pvrvk::Image image = device->createImage(createInfo);

	// if the required memory flags is pvrvk::MemoryPropertyFlags::e_NONE then no backing will be provided for the image
	if (requiredMemoryFlags != pvrvk::MemoryPropertyFlags::e_NONE)
	{
		// if no flags are provided for the optimal flags then just reuse the required set of memory property flags to optimise the getMemoryTypeIndex
		if (optimalMemoryFlags == pvrvk::MemoryPropertyFlags::e_NONE) { optimalMemoryFlags = requiredMemoryFlags; }

		// Create a memory block if it is non sparse and a valid memory propery flag.
		if ((createInfo.getFlags() &
				(pvrvk::ImageCreateFlags::e_SPARSE_ALIASED_BIT | pvrvk::ImageCreateFlags::e_SPARSE_BINDING_BIT | pvrvk::ImageCreateFlags::e_SPARSE_RESIDENCY_BIT)) == 0 &&
			(requiredMemoryFlags != pvrvk::MemoryPropertyFlags(0)))
		// If it's not sparse, create memory backing
		{
			if (imageAllocator)
			{
				vma::AllocationCreateInfo allocInfo = {};
				allocInfo.memoryTypeBits = image->getMemoryRequirement().getMemoryTypeBits();
				allocInfo.requiredFlags = requiredMemoryFlags;
				allocInfo.preferredFlags = requiredMemoryFlags | optimalMemoryFlags;
				allocInfo.flags = vmaAllocationCreateFlags;
				vma::Allocation allocation = imageAllocator->allocateMemoryForImage(image, allocInfo);
				image->bindMemoryNonSparse(allocation, allocation->getOffset());
			}
			else
			{
				// get the image memory requirements, memory type index and memory property flags required for backing the PVRVk image
				const pvrvk::MemoryRequirements& memoryRequirements = image->getMemoryRequirement();
				uint32_t memoryTypeIndex;
				pvrvk::MemoryPropertyFlags memoryPropertyFlags;
				getMemoryTypeIndex(device->getPhysicalDevice(), memoryRequirements.getMemoryTypeBits(), requiredMemoryFlags, optimalMemoryFlags, memoryTypeIndex, memoryPropertyFlags);

				// allocate the image memory using the retrieved memory type index and memory property flags
				pvrvk::DeviceMemory memBlock = device->allocateMemory(pvrvk::MemoryAllocationInfo(memoryRequirements.getSize(), memoryTypeIndex));

				// attach the memory to the image
				image->bindMemoryNonSparse(memBlock);
			}
		}
	}
	return image;
}
#pragma endregion

#pragma region //////////////// MEMORY HEAPS ////////////////
uint32_t numberOfSetBits(uint32_t bits)
{
	bits = bits - ((bits >> 1) & 0x55555555);
	bits = (bits & 0x33333333) + ((bits >> 2) & 0x33333333);
	return (((bits + (bits >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void getMemoryTypeIndex(const pvrvk::PhysicalDevice& physicalDevice, const uint32_t allowedMemoryTypeBits, const pvrvk::MemoryPropertyFlags requiredMemoryProperties,
	const pvrvk::MemoryPropertyFlags optimalMemoryProperties, uint32_t& outMemoryTypeIndex, pvrvk::MemoryPropertyFlags& outMemoryPropertyFlags)
{
	// attempt to find a memory type index which supports the optimal set of memory property flags
	pvrvk::MemoryPropertyFlags memoryPropertyFlags = optimalMemoryProperties;

	// ensure that the optimal set of memory property flags is a superset of the required set of memory property flags.
	// This also handles cases where the optimal set of memory property flags hasn't been set but the required set has
	memoryPropertyFlags |= requiredMemoryProperties;

	uint32_t minCost = std::numeric_limits<uint32_t>::max();

	// iterate through each memory type supported by the physical device and attempt to find the best possible memory type supporting as many of the optimal bits as possible
	for (uint32_t memoryIndex = 0u; memoryIndex < physicalDevice->getMemoryProperties().getMemoryTypeCount(); ++memoryIndex)
	{
		const uint32_t memoryTypeBits = (1u << memoryIndex);
		// ensure the memory type is compatible with the require memory for the given allocation
		const bool isRequiredMemoryType = static_cast<uint32_t>(allowedMemoryTypeBits & memoryTypeBits) != 0;

		if (isRequiredMemoryType)
		{
			const pvrvk::MemoryPropertyFlags currentMemoryPropertyFlags = physicalDevice->getMemoryProperties().getMemoryTypes()[memoryIndex].getPropertyFlags();
			// ensure the memory property flags for the current memory type supports the required set of memory property flags
			const bool hasRequiredProperties = static_cast<uint32_t>(currentMemoryPropertyFlags & requiredMemoryProperties) == requiredMemoryProperties;
			if (hasRequiredProperties)
			{
				// calculate a cost value based on the number of bits from the optimal set of bits which are not present in the current memory type
				uint32_t currentCost = numberOfSetBits(static_cast<uint32_t>(memoryPropertyFlags & ~currentMemoryPropertyFlags));

				// update the return values if the current cost is less than the current maximum cost value
				if (currentCost < minCost)
				{
					outMemoryTypeIndex = static_cast<uint32_t>(memoryIndex);
					outMemoryPropertyFlags = currentMemoryPropertyFlags;

					// early return if we have a perfect match
					if (currentCost == 0) { return; }
					// keep track of the current minimum cost
					minCost = currentCost;
				}
			}
		}
	}

	assertion(minCost != std::numeric_limits<uint32_t>::max(), "Could not find compatible memory type");
}
#pragma endregion

#pragma region //////////// EXTENSIONS AND LAYERS ///////////
DeviceExtensions::DeviceExtensions(VulkanVersion vkVersion) : VulkanExtensionList()
{
#ifdef VK_KHR_swapchain
	// enable the swap chain extension
	addExtension(pvrvk::VulkanExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, (uint32_t)-1));
#endif

#ifdef VK_IMG_format_pvrtc
	// attempt to enable pvrtc extension
	addExtension(pvrvk::VulkanExtension(VK_IMG_FORMAT_PVRTC_EXTENSION_NAME, (uint32_t)-1));
#endif

#ifdef VK_IMG_filter_cubic
	// attempt to enable IMG cubic filtering
	addExtension(pvrvk::VulkanExtension(VK_IMG_FILTER_CUBIC_EXTENSION_NAME, (uint32_t)-1));
#endif

#ifdef DEBUG
#ifdef VK_EXT_debug_marker
	// if the build is Debug then enable the DEBUG_MARKER extension to aid with debugging
	addExtension(pvrvk::VulkanExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, (uint32_t)-1));
#endif
#endif

	// After changing extension loading : Ensure that only device extenions that were depricated in 1.1 are only added when targetting Vulkan 1.0
	// if (vkVersion.toVulkanVersion() < VK_MAKE_VERSION(1, 1, 0))
	{
#ifdef VK_KHR_get_memory_requirements2
		// attempt to enable VK_KHR_get_memory_requirements2 extension
		addExtension(pvrvk::VulkanExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, (uint32_t)-1));
#endif
#ifdef VK_KHR_dedicated_allocation
		// attempt to enable VK_KHR_dedicated_allocation extension
		addExtension(pvrvk::VulkanExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, (uint32_t)-1));
#endif
	}
}

DeviceExtensions& DeviceExtensions::addExtensionFeature(pvrvk::ExtensionFeatures& extensionFeature)
{
	// Insert the extension feature into the extension feature map
	void* extensionFeaturePtr = extensionFeature.getVkPtr();
	_extensionFeatures.insert({ static_cast<VkStructureType>(extensionFeature.getSType()), extensionFeaturePtr });

	// If an extension feature has already been requested, we shift the linked list down by one
	// Making this current extension the new base pointer
	if (_lastRequestedExtensionFeature) { extensionFeature.setPNext(_lastRequestedExtensionFeature); }
	_lastRequestedExtensionFeature = extensionFeaturePtr;

	return *this;
}

DeviceExtensions& DeviceExtensions::addFragmentShadingRateExtensionAndFeature(pvrvk::PhysicalDevice& physicalDevice)
{
	// add the extension
	addExtension(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

	// store shared pointer to ensure the features struct survives for the lifetime of this instance
	std::shared_ptr<pvrvk::FragmentShadingRateFeatures> featuresPtr = std::make_shared<pvrvk::FragmentShadingRateFeatures>(pvrvk::FragmentShadingRateFeatures());
	_featureReferences.emplace_back(featuresPtr);

	// get extension features
	physicalDevice->populateExtensionFeatures(*featuresPtr.get());

	// add the extension feature
	addExtensionFeature(*featuresPtr.get());

	return *this;
}

InstanceLayers::InstanceLayers(bool forceLayers)
{
	if (forceLayers)
	{
		// Enable both VK_LAYER_KHRONOS_validation and the deprecated VK_LAYER_LUNARG_standard_validation as the Loader will handle removing duplicate layers
		addLayer(pvrvk::VulkanLayer("VK_LAYER_KHRONOS_validation", static_cast<uint32_t>(-1)));
		addLayer(pvrvk::VulkanLayer("VK_LAYER_LUNARG_standard_validation", static_cast<uint32_t>(-1)));
		addLayer(pvrvk::VulkanLayer("VK_LAYER_LUNARG_assistant_layer", static_cast<uint32_t>(-1)));
		// Add IMG specific best practices layer, if supported
		addLayer(pvrvk::VulkanLayer("VK_LAYER_IMG_powervr_perf_doc", static_cast<uint32_t>(-1)));
	}
}

InstanceExtensions::InstanceExtensions(VulkanVersion vkVersion)
{
#ifdef VK_KHR_surface
	addExtension(pvrvk::VulkanExtension(VK_KHR_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	addExtension(pvrvk::VulkanExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined VK_USE_PLATFORM_WIN32_KHR
	addExtension(pvrvk::VulkanExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	addExtension(pvrvk::VulkanExtension(VK_KHR_XCB_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	addExtension(pvrvk::VulkanExtension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	addExtension(pvrvk::VulkanExtension(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	addExtension(pvrvk::VulkanExtension(VK_MVK_MACOS_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_KHR_display) // NullWS
	addExtension(pvrvk::VulkanExtension(VK_KHR_DISPLAY_EXTENSION_NAME, (uint32_t)-1));
#endif

#ifdef DEBUG
	// if the build is Debug then attempt to enable the VK_EXT_debug_report extension to aid with debugging
#if defined(VK_EXT_debug_report) && !defined(VK_USE_PLATFORM_MACOS_MVK)
	addExtension(pvrvk::VulkanExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, (uint32_t)-1));
#endif
	// if the build is Debug then attempt to enable the VK_EXT_debug_utils extension to aid with debugging
#if defined(VK_EXT_debug_utils) && !defined(VK_USE_PLATFORM_MACOS_MVK)
	addExtension(pvrvk::VulkanExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, (uint32_t)-1));
#endif
	// if the build is Debug then attempt to enable the VK_EXT_validation_features extension to aid with debugging
#ifdef VK_EXT_validation_features
	addExtension(pvrvk::VulkanExtension(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, (uint32_t)-1));
#endif
#endif

	// After changing extension loading : Ensure that instance extensions depreciated in 1.1 are only added if targetting vulkan version 1.0
	// if (vkVersion.toVulkanVersion() < VK_MAKE_VERSION(1, 1, 0))
	{
#ifdef VK_KHR_get_physical_device_properties2
		addExtension(pvrvk::VulkanExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, (uint32_t)-1));
#endif
	}
}

std::vector<int> validatePhysicalDeviceExtensions(const pvrvk::Instance instance, const std::vector<std::string>& vectorExtensionNames)
{
	std::vector<int> vectorResult;

	for (uint32_t i = 0; i < instance->getNumPhysicalDevices(); i++)
	{
		pvrvk::PhysicalDevice physicalDevice = instance->getPhysicalDevice(i);

		bool deviceHasAllExtensions = true;

		// Get the full list of extensions supported by the current physical device
		std::vector<pvrvk::ExtensionProperties> supportedExtensions = physicalDevice->getDeviceExtensionsProperties();

		// For each of the requested extensions, check that its name is contained within the list of extensions supported by the device
		for (std::string requested : vectorExtensionNames)
		{
			bool found =
				std::any_of(supportedExtensions.begin(), supportedExtensions.end(), [&requested = requested](auto& supported) { return requested == supported.getExtensionName(); });

			if (!found)
			{
				Log(LogLevel::Information, "Physical Device : %s Failed to find the extension : %s ", physicalDevice->getProperties().getDeviceName(), requested.c_str());
				deviceHasAllExtensions = false;
				break;
			}
		}

		if (deviceHasAllExtensions) { vectorResult.push_back(i); }
	}

	return vectorResult;
}

bool formatWithTilingSupportsFeatureFlags(pvrvk::Format imageFormat, pvrvk::ImageTiling imageTiling, pvrvk::FormatFeatureFlags formatFeatureFlags, const pvrvk::Instance instance, pvrvk::PhysicalDevice physicalDevice)
{
	VkFormatProperties formatProperties = {};

	instance->getVkBindings().vkGetPhysicalDeviceFormatProperties(physicalDevice->getVkHandle(), static_cast<VkFormat>(imageFormat), &formatProperties);

	switch (imageTiling)
	{
		case pvrvk::ImageTiling::e_LINEAR:
		{
			if (formatProperties.linearTilingFeatures & static_cast<VkFormatFeatureFlags>(formatFeatureFlags))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		case pvrvk::ImageTiling::e_OPTIMAL:
		{
			if (formatProperties.optimalTilingFeatures & static_cast<VkFormatFeatureFlags>(formatFeatureFlags))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		default:
		{
			return false;
		}
	}

	return false;
}

#pragma endregion
} // namespace utils
} // namespace pvr
//!\endcond
