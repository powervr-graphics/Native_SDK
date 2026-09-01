/*!
\brief Enums used in the library and initialization data struct
\file PVRSuperResolution/PostProcessingInitializationData.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <vector>
#include "DynamicMap.h"
#include "../include/vk_bindings.h"

namespace pvr {

/// <summary>Enum to indicate what type of post processing method will be used.</summary>
enum class PostProcessingMethod
{
	SupernovaV1Mode1X = 0, /// <summary>Supernova V1 method where no upscaling is performed.</summary>
	SupernovaV1Mode2X, /// <summary>Supernova V1 method with 2x upscaling performed.</summary>
	MentisV2NeuralSuperResolution, /// <summary>Mentis Neural Super Resolution upscaler.</summary>
	YUVAColorConversion, /// <summary>YUVA color conversion.</summary>
	PostProcessingMethodSize /// <summary>Helper enum to know the amount available enums in PostProcessingMethod.</summary>
};

/// <summary>Enum to indicate the order of each SuperResolution pass used.</summary>
enum class PostprocessingPassOrder
{
	FirstPass = 0, /// <summary>The pass is the first one being used, being at least two passes.</summary>
	IntermediatePass, /// <summary>The pass is not the first nor the last, being at least three passes.</summary>
	LastPass, /// <summary>The pass is the last one being used, being at least two passes.</summary>
	SinglePass, /// <summary>The pass is the only one present.</summary>
	Size /// <summary>Helper enum to know the amount available enums in PostprocessingPassOrder.</summary>
};

/// <summary>Enum to indicate the API used fro a particular post processing method.</summary>
enum class PostProcessingAPI
{
	PostProcessingGraphicsAPIVulkan = 0, /// <summary>Post processing method using the Vulkan API with a graphics queue.</summary>
	PostProcessingComputeAPIVulkan, /// <summary>Post processing method using the Vulkan API with a compute queue.</summary>
	PostProcessingAPISize /// <summary>Helper enum to know the amount available enums in PostProcessingAPI.</summary>
};

/// <summary>Struct used to initialize Vulkan post processing algorithms.</summary>
struct VulkanInitializationData
{
	/// <summary>Vulkan logical device.</summary>
	VkDevice device{ VK_NULL_HANDLE };

	/// <summary>Vulkan physical device.</summary>
	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };

	/// <summary>Vulkan image views used as input for the SuperResolution method selected, for each swachain index.</summary>
	std::vector<std::vector<VkImageView>> vectorInputImageView;

	/// <summary>Format of the 2D image used as input.</summary>
	std::vector<VkFormat> inputImageFormat;

	/// <summary>Layout of the 2D image used as input.</summary>
	std::vector<VkImageLayout> inputImageLayout;

	/// <summary>Extent of the 2D image used as input.</summary>
	VkExtent2D inputImageExtent = {};

	/// <summary>Vulkan image where to store the result. Its dimensions have to match the ones expected by the SuperResolution method selected (SupernovaV1ModeX1 does not do upsampling, while SupernovaV1ModeX2 and SupernovaV2 do x2 upsample).</summary>
	std::vector<VkImageView> vectorOutputImageView;

	/// <summary>Format of the 2D image used to  output results.</summary>
	VkFormat outputImageFormat{ VK_FORMAT_UNDEFINED };

	/// <summary>Initial layout of the 2D image used to output results.</summary>
	VkImageLayout outputImageInitialLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

	/// <summary>Final layout of the 2D image used to output results.</summary>
	VkImageLayout outputImageFinalLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

	/// <summary>Extent of the 2D image used to output results.</summary>
	VkExtent2D outputImageExtent = {};

	/// <summary>Flag bits for the queue provided (whether it has graphics, compute or transfer capabilities.</summary>
	VkQueueFlagBits queueFlagBits{ VK_QUEUE_FLAG_BITS_MAX_ENUM };

	/// <summary>Graphics capabilities queue to submit internal initialization commands to.</summary>
	VkQueue queue{ VK_NULL_HANDLE };

	/// <summary>Graphics capabilities queue index to submit internal initialization commands to.</summary>
	uint32_t queueFamilyIndex{ static_cast<uint32_t>(-1) };

	/// <summary>Number of command buffers that will be used to record to the commands for the chosen SuperResolution
	/// algorithm. Each time a call to recordCommands() is done, an internal counter increases and if it surpasses the
	/// value of numberCommandBuffer, an assert will be triggered.</summary>
	uint32_t numberCommandBuffer{ 0 };

	/// <summary>Vulkan device bindings for all the Vulkan API calls that will be done.</summary>
	const VkDeviceBindings* vk{ nullptr };

	/// <summary>Vulkan instance bindings for all the instance Vulkan API calls that will be done.</summary>ç
	const VkInstanceBindings* vkInstance{ nullptr };

	/// <summary>Pointer to the application running using this library.</summary>
	void* application{ nullptr };

	/// <summary>Post processing method to be used.</summary>
	PostProcessingMethod postProcessingMethod{ PostProcessingMethod::PostProcessingMethodSize };

	/// <summary>Pointer to SuperResolution::_dynamicMap for all passes used in each algorithm implemented by the 
	/// PVRSuperResolution algorithm to be able to access any required values from the client application required by the library.</summary>
	DynamicMap* dynamicMap { nullptr };
};

/// <summary>Utility class to adapt vectors from the VulkanInitializationData used by an application to the SuperResolution library internal 
/// format (where each image view used as input or output has to be in a separate std::<vector>).</summary>
/// <param name="commandBuffer">Command buffer to record to.</param>
/// <param name="commandBufferIndex">When the SuperResolution algorithm is initialized, a value is provided in
template<class T>
void adaptVectorDataToVectorVectorData(const std::vector<T>& vectorData, std::vector<std::vector<T>>& vectorVectorData)
{
	const size_t vectorSize = vectorData.size();
	vectorVectorData.resize(vectorSize);
	for (size_t i = 0; i < vectorSize; ++i) { vectorVectorData[i].push_back(vectorData[i]); }
}

} // namespace pvr
