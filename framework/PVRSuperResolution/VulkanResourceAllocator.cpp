/*!
\brief Helper class to allocate Vulkan resources
\file PVRSuperResolution/VulkanResourceAllocator.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include <assert.h>
#include <stdio.h>
#include <cstring>
#include "VulkanResourceAllocator.h"
#include "Log.h"
#include "FileIO.h"

namespace pvr {

int VulkanResourceAllocator::getMemoryType(
	uint32_t memoryTypeBits, VkMemoryPropertyFlagBits requestedMemoryProperties, const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties)
{
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i)
	{
		// Verify first bit in memoryTypeBits by swifting each loop iteration one bit
		if ((memoryTypeBits & 1) > 0)
		{
			// Verify whether this memory type has the memory properties which are requested
			if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & requestedMemoryProperties) == requestedMemoryProperties) { return i; }
		}
		memoryTypeBits >>= 1;
	}

	assertCondition(false, "ERROR: No memory type found.");
	return -1;
}

void VulkanResourceAllocator::buildBuffer(VkDeviceSize dataSize, VkBufferUsageFlags usage, uint32_t queueFamilyIndexCount, const uint32_t* pQueueFamilyIndices,
	VkMemoryPropertyFlagBits memoryPropertyFlagBits, void* pData, VkBuffer& buffer, VkDeviceMemory& deviceMemory)
{
	// 1. Build buffer
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = dataSize;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.queueFamilyIndexCount = queueFamilyIndexCount;
	bufferCreateInfo.pQueueFamilyIndices = pQueueFamilyIndices;

	_vk->vkCreateBuffer(_device, &bufferCreateInfo, nullptr, &buffer);

	// 2. Get the memory requirements of the buffer built and allocate memory
	// Get the memory requirements in size, allignment and memory type bits for the buffer resource allocated:
	VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
	_vkInstance->vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &vkPhysicalDeviceMemoryProperties);
	VkMemoryRequirements memoryRequirements = {};
	_vk->vkGetBufferMemoryRequirements(_device, buffer, &memoryRequirements);
	assertCondition((memoryRequirements.size != 0), "ERROR: vkGetBufferMemoryRequirements::size is 0.");

	// getMemoryType() will return the first index in VkPhysicalDeviceMemoryProperties::memoryTypes[] which supports the memory
	// property "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" (if any) from all possible indices in VkPhysicalDeviceMemoryProperties::memoryTypes[]
	// which can be used to allocate memory for the buffer being built. The set of all indices in
	// VkPhysicalDeviceMemoryProperties::memoryTypes[] which allow to allocate memory for the buffer being built are given by
	// VkMemoryRequirements::memoryTypeBits, where each bit set to one indicates such index.
	int memoryType = getMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlagBits, vkPhysicalDeviceMemoryProperties);
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryType;
	assertFunctionResult(_vk->vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &deviceMemory), "ERROR: vkAllocateMemory.\n");

	// 3. Bind the memory allocated for the buffer to the buffer
	assertFunctionResult(_vk->vkBindBufferMemory(_device, buffer, deviceMemory, 0), "ERROR: vkBindBufferMemory.\n");

	// 4. If information to initialize the buffer was provided, copy it to the buffer
	if (pData != nullptr)
	{
		// See if memory is host visible, then copy mapping, otherwise copy via staging buffer
		if ((memoryPropertyFlagBits & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) > 0)
		{
			copyInformationToHostVisibleMemory(deviceMemory, pData, dataSize);
		}
		else
		{
			copyInformationToDeviceLocalMemory(dataSize, pData, buffer, deviceMemory);
		}
	}
}

void VulkanResourceAllocator::copyInformationToHostVisibleMemory(VkDeviceMemory& deviceMemory, void* pData, VkDeviceSize dataSize)
{
	void* _mappedMemory;
	assertFunctionResult(_vk->vkMapMemory(_device, deviceMemory, 0, VK_WHOLE_SIZE, 0, &_mappedMemory), "ERROR: vkMapMemory.\n");
	memcpy(_mappedMemory, pData, dataSize);

	VkMappedMemoryRange mappedMemoryRange = {};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemoryRange.memory = deviceMemory;
	mappedMemoryRange.offset = 0;
	mappedMemoryRange.size = VK_WHOLE_SIZE;

	assertFunctionResult(_vk->vkFlushMappedMemoryRanges(_device, 1, &mappedMemoryRange), "ERROR: vkFlushMappedMemoryRanges.\n");

	_vk->vkUnmapMemory(_device, deviceMemory);
}

void VulkanResourceAllocator::copyInformationToDeviceLocalMemory(VkDeviceSize dataSize, void* pData, VkBuffer& buffer, VkDeviceMemory& deviceMemory)
{
	assertCondition(pData != nullptr, "ERROR: getCompatibleQueueFamilies pData is nullptr.\n");

	// 1. Get a physical deice queue with dedicated transfer operations
	uint32_t transferQueueIndex = static_cast<uint32_t>(-1);
	assertCondition(getCompatibleQueueFamilies(VK_QUEUE_TRANSFER_BIT, transferQueueIndex), "ERROR: getCompatibleQueueFamilies.\n");
	VkQueue transferQueue{ VK_NULL_HANDLE };
	_vk->vkGetDeviceQueue(_device, transferQueueIndex, 0, &transferQueue);

	// 2. Build a staging buffer with host visible and transfer source / transfer destination usage
	VkBuffer stagingBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory stagingDeviceMemory{ VK_NULL_HANDLE };
	buildBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 1, &transferQueueIndex, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pData, stagingBuffer,
		stagingDeviceMemory);

	// 3. Do the copy of information from the staging buffer onto the provided buffer. This requires command buffer allocation and submit
	// Build command pool
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolCreateInfo.queueFamilyIndex = transferQueueIndex;

	VkCommandPool commandPool{VK_NULL_HANDLE};
	assertFunctionResult(_vk->vkCreateCommandPool(_device, &commandPoolCreateInfo, nullptr, &commandPool), "ERROR: vkCreateCommandPool.\n");

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(1);
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	// Allocate one command buffer from the command pool.
	VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
	assertFunctionResult(_vk->vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, &commandBuffer), "ERROR: vkAllocateCommandBuffers.\n");

	// Record the command to copy the contents of the staging buffer onto the provided buffer
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	assertFunctionResult(_vk->vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), "ERROR: vkBeginCommandBuffer.\n");

	VkBufferCopy bufferCopy = {};
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = dataSize;
	_vk->vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &bufferCopy);

	assertFunctionResult(_vk->vkEndCommandBuffer(commandBuffer), "ERROR: vkEndCommandBuffer.\n.");

	VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.pWaitDstStageMask = &pipelineStageFlags;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	assertFunctionResult(_vk->vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE), "ERROR: vkQueueSubmit.\n");
	assertFunctionResult(_vk->vkDeviceWaitIdle(_device), "ERROR: vkDeviceWaitIdle.\n");

	// 4. Release the built resources
	destroyVulkanObject<PFN_vkDestroyBuffer, VkBuffer>(_device, _vk->vkDestroyBuffer, stagingBuffer);
	destroyVulkanObject<PFN_vkFreeMemory, VkDeviceMemory>(_device, _vk->vkFreeMemory, stagingDeviceMemory);
	destroyVulkanObject<PFN_vkDestroyCommandPool, VkCommandPool>(_device, _vk->vkDestroyCommandPool, commandPool);
}

/// <summary>Finds the indices of compatible graphics and present queues and returns them</summary>
/// <param name="queueFlagBits">Queue flag bits to search for</param>
bool VulkanResourceAllocator::getCompatibleQueueFamilies(VkQueueFlagBits queueFlagBits, uint32_t& queueIndex)
{
	// Get the number and details of the queue families the physical device supports.
	uint32_t queueFamiliesCount;
	_vkInstance->vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamiliesCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamiliesCount);
	_vkInstance->vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamiliesCount, queueFamilyProperties.data());

	// This function iterates through all the queue families available on the selected device and selects a queue
	// compatible with the queueFlagBits parameter
	for (int i = 0; i < queueFamilyProperties.size(); ++i)
	{
		// Check for compatible queue family flag.
		if (queueFamilyProperties[i].queueFlags & queueFlagBits)
		{
			queueIndex = i;
			return true;
		}
	}

	return false;
}

void VulkanResourceAllocator::buildImage(VkFormat format, VkExtent2D extent2D, uint32_t queueFamilyIndex, VkImage& image, VkDeviceMemory& deviceMemory)
{
	// First: Build image
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = VkExtent3D{ extent2D.width, extent2D.height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; // TODO: Verify sampled bit is really needed.
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 1;
	imageCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	assertFunctionResult(_vk->vkCreateImage(_device, &imageCreateInfo, nullptr, &image), "ERROR: vkCreateImage.\n");

	// Second: Get the memory requirements of the image built and allocate memory
	// Get the memory requirements in size, allignment and memory type bits for the image resource allocated:
	VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
	_vkInstance->vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &vkPhysicalDeviceMemoryProperties);
	VkMemoryRequirements memoryRequirementsImage = {};
	_vk->vkGetImageMemoryRequirements(_device, image, &memoryRequirementsImage);
	assertCondition((memoryRequirementsImage.size != 0), "ERROR: vkGetImageMemoryRequirements::size is 0.");

	// getMemoryType() will return the first index in VkPhysicalDeviceMemoryProperties::memoryTypes[] which supports the memory
	// property "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" (if any) from all possible indices in VkPhysicalDeviceMemoryProperties::memoryTypes[]
	// which can be used to allocate memory for the image being built. The set of all indices in
	// VkPhysicalDeviceMemoryProperties::memoryTypes[] which allow to allocate memory for the image being built are given by
	// VkMemoryRequirements::memoryTypeBits, where each bit set to one indicates such index.
	int memoryTypeIndexImage = getMemoryType(memoryRequirementsImage.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkPhysicalDeviceMemoryProperties);
	VkMemoryAllocateInfo memoryAllocateInfoYImage = {};
	memoryAllocateInfoYImage.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfoYImage.allocationSize = memoryRequirementsImage.size;
	memoryAllocateInfoYImage.memoryTypeIndex = memoryTypeIndexImage;
	assertFunctionResult(_vk->vkAllocateMemory(_device, &memoryAllocateInfoYImage, nullptr, &deviceMemory), "ERROR: vkAllocateMemory.\n");

	// Third: Bind the memory allocated for the image to the image
	assertFunctionResult(_vk->vkBindImageMemory(_device, image, deviceMemory, 0), "ERROR: vkBindImageMemory.\n");
}

VkImageView VulkanResourceAllocator::buildImageView(VkFormat format, VkImage image)
{
	// Fourth: Build image view (used by the framebuffer)
	VkImageViewCreateInfo imageViewCreateInfo = {};

	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView{ VK_NULL_HANDLE };
	assertFunctionResult(_vk->vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &imageView), "ERROR: vkCreateImageView.\n");

	return imageView;
}

VkPipeline VulkanResourceAllocator::buildComputePipeline(VkPipelineLayout pipelineLayout, const std::string& computeShaderPath)
{
	// Build shader modules
	std::vector<unsigned char> pointerComputeShader;
	loadFile(computeShaderPath, pointerComputeShader, _application);
	assertCondition((pointerComputeShader.size() != 0), "ERROR: Wrong result reading vertex shader AttributelessVertexShader.vsh.spv.");

	// First build vertex shader module
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = static_cast<uint32_t>(pointerComputeShader.size());
	shaderModuleCreateInfo.pCode = static_cast<uint32_t*>((void*)(pointerComputeShader.data()));
	VkShaderModule computeShaderModule = VK_NULL_HANDLE;
	assertFunctionResult(_vk->vkCreateShaderModule(_device, &shaderModuleCreateInfo, nullptr, &computeShaderModule), "ERROR: vkCreateShaderModule.\n");

	VkPipelineShaderStageCreateInfo vectorPipelineShaderStageCreateInfo;
	vectorPipelineShaderStageCreateInfo = {};
	vectorPipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vectorPipelineShaderStageCreateInfo.flags = 0;
	vectorPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	vectorPipelineShaderStageCreateInfo.module = computeShaderModule;
	vectorPipelineShaderStageCreateInfo.pName = "main";
	vectorPipelineShaderStageCreateInfo.pSpecializationInfo = nullptr;

	VkComputePipelineCreateInfo computePipelineCreateInfo = {};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.flags = 0;
	computePipelineCreateInfo.stage = vectorPipelineShaderStageCreateInfo;
	computePipelineCreateInfo.layout = pipelineLayout;

	VkPipeline pipeline = VK_NULL_HANDLE;
	assertFunctionResult(_vk->vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline), "ERROR: vkCreateGraphicsPipelines.\n");

	destroyVulkanObject<PFN_vkDestroyShaderModule, VkShaderModule>(_device, _vk->vkDestroyShaderModule, computeShaderModule);

	return pipeline;
}

VkDescriptorSetLayout VulkanResourceAllocator::buildDescriptorSetLayouts(
	const std::vector<VkDescriptorType>& vectorDescriptorType, const std::vector<VkShaderStageFlagBits>& vectorDescriptorStage)
{
	assertCondition((vectorDescriptorType.size() == vectorDescriptorStage.size()),
		"ERROR: Vectors vectorDescriptorType and vectorDescriptorStage in VulkanResourceAllocator::buildDescriptorSetLayouts do not have the same amount of elements.");
	
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding(vectorDescriptorType.size());

	for (uint32_t i = 0; i < descriptorSetLayoutBinding.size(); ++i)
	{
		descriptorSetLayoutBinding[i].binding = i;
		descriptorSetLayoutBinding[i].descriptorType = vectorDescriptorType[i];
		descriptorSetLayoutBinding[i].descriptorCount = 1;
		descriptorSetLayoutBinding[i].stageFlags = vectorDescriptorStage[i];
		descriptorSetLayoutBinding[i].pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBinding.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBinding.data();
	assertFunctionResult(_vk->vkCreateDescriptorSetLayout(_device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout), "ERROR: vkCreateDescriptorSetLayout.\n");

	return descriptorSetLayout;
}

void VulkanResourceAllocator::allocateDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, std::vector<VkDescriptorSet>& vectorDescriptorSet)
{
	// Allocate descriptor sets
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

	for (size_t i = 0; i < vectorDescriptorSet.size(); ++i)
	{
		assertFunctionResult(_vk->vkAllocateDescriptorSets(_device, &descriptorSetAllocateInfo, &vectorDescriptorSet[i]), "ERROR: vkAllocateDescriptorSets.\n");
	}
}

VkPipelineLayout VulkanResourceAllocator::buildPipelineLayout(const std::vector<VkDescriptorSetLayout>& vectorDescriptorSetLayout)
{
	// Build pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vectorDescriptorSetLayout.size());
	pipelineLayoutCreateInfo.pSetLayouts = vectorDescriptorSetLayout.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	assertFunctionResult(_vk->vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout), "ERROR: vkCreatePipelineLayout.\n");
	return pipelineLayout;
}

VkDescriptorPool VulkanResourceAllocator::buildDescriptorPool(const std::vector<VkDescriptorType>& vectorDescriptorType, const std::vector<int>& vectorDescriptorCount)
{
	// Build descriptor pool
	std::vector<VkDescriptorPoolSize> vectorDescriptorPoolSize(vectorDescriptorType.size());
	uint32_t maxDescriptorCount = 0;              

	for (size_t i = 0; i < vectorDescriptorPoolSize.size(); ++i)
	{
		vectorDescriptorPoolSize[i] = {};
		vectorDescriptorPoolSize[i].type = vectorDescriptorType[i];
		vectorDescriptorPoolSize[i].descriptorCount = vectorDescriptorCount[i];

		maxDescriptorCount += vectorDescriptorCount[i];
	}

	VkDescriptorPool descriptorPool{VK_NULL_HANDLE};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = maxDescriptorCount;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(vectorDescriptorPoolSize.size());
	descriptorPoolCreateInfo.pPoolSizes = vectorDescriptorPoolSize.data();
	assertFunctionResult(_vk->vkCreateDescriptorPool(_device, &descriptorPoolCreateInfo, nullptr, &descriptorPool), "ERROR: vkCreateDescriptorPool.\n");

	return descriptorPool;
}

} // namespace pvr
