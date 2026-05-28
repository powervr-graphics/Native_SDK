/*!
\brief Helper class to allocate Vulkan resources
\file PVRSuperResolution/VulkanResourceAllocator.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once

#include <vector>
#include <string>
#include "../include/vulkan/vulkan.h"
#include "../include/vk_bindings.h"
#include "Log.h"

namespace pvr {

/// <summary>Helper function to destroy Vulkan objects.</summary>
/// <param name="device">Device this Vulkan object was built with.</param>
/// <param name="functionPointer">Function pointer to destroy the Vulkan object.</param>
/// <param name="object">Vulkan object to destroy.</param>
template<class VulkanAPIFunctionPointer, class VulkanObject>
void destroyVulkanObject(VkDevice device, VulkanAPIFunctionPointer functionPointer, VulkanObject object)
{
	assertCondition((object != VK_NULL_HANDLE), "ERROR: Object provided is VK_NULL_HANDLE in destroyVulkanObject");

	functionPointer(device, object, nullptr);
}

/// <summary>Helper function to destroy Vulkan objects in a vector.</summary>
/// <param name="device">Device this Vulkan object was built with.</param>
/// <param name="functionPointer">Function pointer to destroy the Vulkan object.</param>
/// <param name="vectorObject">Vector of Vulkan objects to destroy.</param>
template<class VulkanAPIFunctionPointer, class VulkanObject>
void destroyVulkanObjectVector(VkDevice device, VulkanAPIFunctionPointer functionPointer, const std::vector<VulkanObject>& vectorObject)
{
	for (size_t i = 0; i < vectorObject.size(); ++i)
	{
		assertCondition((vectorObject[i] != VK_NULL_HANDLE), "ERROR: Object provided is VK_NULL_HANDLE in destroyVulkanObjectVector");
		functionPointer(device, vectorObject[i], nullptr);
	}
}

/// <summary>Helper function to destroy Vulkan objects in a vector.</summary>
/// <param name="device">Device this Vulkan object was built with.</param>
/// <param name="functionPointer">Function pointer to destroy the Vulkan object.</param>
/// <param name="vectorVectorObject">Vector of vectors of Vulkan objects to destroy.</param>
template<class VulkanAPIFunctionPointer, class VulkanObject>
void destroyVulkanObjectVectorOfVectors(VkDevice device, VulkanAPIFunctionPointer functionPointer, const std::vector<std::vector<VulkanObject>>& vectorVectorObject)
{
	for (size_t i = 0; i < vectorVectorObject.size(); ++i)
	{
		for (size_t j = 0; j < vectorVectorObject[i].size(); ++j)
		{
			assertCondition((vectorVectorObject[i][j] != VK_NULL_HANDLE), "ERROR: Object provided is VK_NULL_HANDLE in destroyVulkanObjectVectorOfVectors");
			functionPointer(device, vectorVectorObject[i][j], nullptr);
		}
	}
}

class VulkanResourceAllocator
{
public:
	/// <summary>Default constructor.</summary>
	VulkanResourceAllocator(): _device{ VK_NULL_HANDLE }, _physicalDevice{ VK_NULL_HANDLE }, _vk{ VK_NULL_HANDLE }, _vkInstance{ VK_NULL_HANDLE }, _application{ nullptr }
	{}

	/// <summary>Parameter constructor.</summary>
	VulkanResourceAllocator(VkDevice device, VkPhysicalDevice physicalDevice, const VkDeviceBindings* vk, const VkInstanceBindings* vkInstance, void* application)
		:
		_device{ device },
		_physicalDevice{ physicalDevice },
		_vk{ vk },
		_vkInstance{ vkInstance },
		_application{application}
	{}

	/// <summary>Find a memory type in the array VkPhysicalDeviceMemoryProperties::memoryTypes with preferred memory property flags.</summary>
	/// <param name="memoryTypeBits">Result of VkMemoryRequirements::memoryTypeBits after a call to vkGetImageMemoryRequirements,
	/// where the each bit "i" set to "1" means the corresponding VkPhysicalDeviceMemoryProperties::memoryTypes[i] could
	/// allocate memory for the resource being built.</param>
	/// <param name="requestedMemoryProperties">As there could be suboptimal memory types in memoryTypeBits, this parameter
	/// helps find one VkPhysicalDeviceMemoryProperties::memoryTypes[i] with exactly the memory properties wanted.</param>
	/// <returns>Index of the memory type in VkPhysicalDeviceMemoryProperties::memoryTypes[] where to allocate memory from.</returns>
	int getMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlagBits requestedMemoryProperties, const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties);

	/// <summary>Builds a VkBuffer with size, usage flags and memory property flags given by parameter. If pData is not nullptr, it will 
	/// copy the information to the buffer.</summary>
	/// <param name="dataSize">Size of the buffer.</param>
	/// <param name="usage">Usage of the buffer.</param>
	/// <param name="queueFamilyIndexCount">How many queues this buffer will interact with.</param>
	/// <param name="pQueueFamilyIndices">Indices of each one of the queues the buffer will interact with.</param>
	/// <param name="memoryPropertyFlagBits">Properties of the memory to allocate the buffer.</param>
	/// <param name="buffer">Buffer generated.</param>
	/// <param name="deviceMemory">Device memory generated for the buffer generated.</param>
	/// <param name="pData">Data to copy to the buffer, if not nullptr.</param>
	void buildBuffer(VkDeviceSize dataSize, VkBufferUsageFlags usage, uint32_t queueFamilyIndexCount, const uint32_t* pQueueFamilyIndices,
		VkMemoryPropertyFlagBits memoryPropertyFlagBits, void* pData, VkBuffer& buffer, VkDeviceMemory& deviceMemory);

	/// <summary>Copies the information to the buffer with host visible memory.</summary>
	/// <param name="pData">Pointer to the data provided.</param>
	/// <param name="dataSize">Size of the information to copy.</param>
	void copyInformationToHostVisibleMemory(VkDeviceMemory& deviceMemory, void* pData, VkDeviceSize dataSize);

	/// <summary>Copies information to the buffer provided as parameter. It will build a temporal host visible staging buffer 
	/// to copy from this host visible onto the device local one.</summary>
	/// <param name="buffer">Buffer to build.</param>
	/// <param name="deviceMemory">Device memory from the buffer to build.</param>
	/// <param name="pData">Pointer to the data provided.</param>
	/// <param name="dataSize">Size of the information to copy.</param>
	void copyInformationToDeviceLocalMemory(VkDeviceSize dataSize, void* pData, VkBuffer& buffer, VkDeviceMemory& deviceMemory);

	/// <summary>Searches the available queue families for a queue with flag bits for one with the flag bits specificed by parameter.</summary>
	/// <param name="queueFlagBits">Queue flags to look for.</param>
	/// <param name="queueIndex">Index of the queue found, if any.</param>
	/// <returns>True of a compatible queue was found, false otherwise.</returns>
	bool getCompatibleQueueFamilies(VkQueueFlagBits queueFlagBits, uint32_t& queueIndex);

	/// <summary>Builds a VkImage with its associated VkDeviceMemory.</summary>
	/// <param name="format">Format of the image to allocate.</param>
	/// <param name="extent2D">Extent of the image to allocate.</param>
	/// <param name="queueFamilyIndex">Index of the queue where this image will work with (only one queu assumed).</param>
	/// <param name="image">VkImage allocated.</param>
	/// <param name="deviceMemory">VkDeviceMemory allocated.</param>
	void buildImage(VkFormat format, VkExtent2D extent2D, uint32_t queueFamilyIndex, VkImage& image, VkDeviceMemory& deviceMemory);

	/// <summary>Builds an image view out of a VkImage.</summary>
	/// <param name="format">Format of the image view to build.</param>
	/// <param name="image">Image to build the image view from.</param>
	/// <returns>Image view built.</returns>
	VkImageView buildImageView(VkFormat format, VkImage image);

	/// <summary>Builds a compute pipeline.</summary>
	/// <param name="pipelineLayout">Pipeline layout to use for the compute pipeline.</param>
	/// <param name="computeShaderPath">Path to the compute shader file to read.</param>
	/// <returns>Pipeline with the compute shader provided as parameter.</returns>
	VkPipeline buildComputePipeline(VkPipelineLayout pipelineLayout, const std::string& computeShaderPath);

	/// <summary>Builds a descriptor set layout by generating an array of VkDescriptorSetLayoutBinding elements and assigning
	/// to each the information in vectorDescriptorType[i] and vectorDescriptorStage[i].</summary>
	/// <param name="vectorDescriptorType">Vector with the descriptor types to assign.</param>
	/// <param name="vectorDescriptorStage">Vector with the descriptor stage where the descriptor set will be used.</param>
	/// <returns>Descriptor set generated.</returns>
	VkDescriptorSetLayout buildDescriptorSetLayouts(const std::vector<VkDescriptorType>& vectorDescriptorType, const std::vector<VkShaderStageFlagBits>& vectorDescriptorStage);

	/// <summary>Builds as many descriptor set as elements in vectorDescriptorSet from the descriptor set layout and the descriptor pool 
	/// provided to allocate from.</summary>
	/// <param name="descriptorSetLayout">Descritptor set layout.</param>
	/// <param name="descriptorPool">Descritptor pool to allocate from.</param>
	/// <param name="vectorDescriptorSet">Vector with a certain number of elements, each of which will be filled with a new allocated descriptor set.</param>
	/// <returns>Descriptor set generated.</returns>
	void allocateDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, std::vector<VkDescriptorSet>& vectorDescriptorSet);

	/// <summary>Build the _pipelineLayout pipeline layout used in the render pass by the different SuperResolution methods.</summary>
	/// <param name="vectorDescriptorSetLayout">Vector with the descriptor set layouts for the pipeline layout.</param>
	/// <returns>Pipeline layout built.</returns>
	VkPipelineLayout buildPipelineLayout(const std::vector<VkDescriptorSetLayout>& vectorDescriptorSetLayout);

	/// <summary>Builds a descriptor pool.</summary>
	/// <param name="vectorDescriptorType">Vector with the different descriptor types that might be allocated from this pool.</param>
	/// <param name="vectorDescriptorCount">Number of descriptors from each descriptor type the pool should be able to allocate.</param>
	/// <returns>Descriptor pool generated.</returns>
	VkDescriptorPool buildDescriptorPool(const std::vector<VkDescriptorType>& vectorDescriptorType, const std::vector<int>& vectorDescriptorCount);

private:
	/// <summary>Logical device to work with.</summary>
	VkDevice _device{ VK_NULL_HANDLE };

	/// <summary>Physical device to work with.</summary>
	VkPhysicalDevice _physicalDevice {VK_NULL_HANDLE};

	/// <summary>Device bindings to access Vulkan API functions.</summary>
	const VkDeviceBindings* _vk {nullptr};

	/// <summary>Instance bindings to access Vulkan API functions.</summary>
	const VkInstanceBindings* _vkInstance {nullptr};

	/// <summary>Pointer to the application running using this library.</summary>
	void* _application{ nullptr };
};

} // namespace pvr
