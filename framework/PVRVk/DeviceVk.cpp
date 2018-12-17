/*!
\brief Function implementations of the Vulkan Device class.
\file PVRVk/DeviceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRVk/DeviceVk.h"
#include "PVRVk/InstanceVk.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/FramebufferVk.h"
#include "PVRVk/DeviceMemoryVk.h"
#include "PVRVk/QueueVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/SamplerVk.h"
#include "PVRVk/ShaderModuleVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/RenderPassVk.h"
#include "PVRVk/EventVk.h"
#include "PVRVk/FenceVk.h"
#include "PVRVk/SemaphoreVk.h"
#include "PVRVk/GraphicsPipelineVk.h"
#include "PVRVk/ComputePipelineVk.h"
#include "PVRVk/PopulateCreateInfoVk.h"
#include "PVRVk/SwapchainVk.h"
#include "PVRVk/PipelineCacheVk.h"
#include "PVRVk/QueryPoolVk.h"

namespace pvrvk {

static inline VkDeviceQueueCreateInfo createQueueCreateInfo()
{
	static const float priority[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo createInfo = {};
	createInfo.queueCount = 1;
	createInfo.sType = static_cast<VkStructureType>(StructureType::e_DEVICE_QUEUE_CREATE_INFO);
	createInfo.pNext = NULL;
	createInfo.queueFamilyIndex = static_cast<uint32_t>(-1);
	createInfo.pQueuePriorities = priority;
	createInfo.flags = static_cast<VkDeviceQueueCreateFlags>(DeviceQueueCreateFlags::e_NONE);

	return createInfo;
}
} // namespace pvrvk
// Object creation
namespace pvrvk {
namespace impl {
std::vector<const char*> filterExtensions(const std::vector<VkExtensionProperties>& vec, const char* const* filters, uint32_t numfilters)
{
	std::vector<const char*> retval;
	for (uint32_t i = 0; i < vec.size(); ++i)
	{
		for (uint32_t j = 0; j < numfilters; ++j)
		{
			if (!strcmp(vec[i].extensionName, filters[j]))
			{
				retval.push_back(filters[j]);
				break;
			}
		}
	}
	return retval;
}

std::vector<const char*> filterLayers(const std::vector<VkLayerProperties>& vec, const char* const* filters, uint32_t numfilters)
{
	std::vector<const char*> retval;
	for (uint32_t i = 0; i < vec.size(); ++i)
	{
		for (uint32_t j = 0; j < numfilters; ++j)
		{
			if (!strcmp(vec[i].layerName, filters[j]))
			{
				retval.push_back(filters[j]);
			}
		}
	}
	return retval;
}

pvrvk::GraphicsPipeline Device_::createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo, const PipelineCache& pipelineCache)
{
	GraphicsPipelinePopulate pipelineFactory;
	VkPipeline vkPipeline;

	if (!pipelineFactory.init(createInfo))
	{
		return GraphicsPipeline();
	}
	vkThrowIfFailed(getVkBindings().vkCreateGraphicsPipelines(
						getVkHandle(), pipelineCache.isValid() ? pipelineCache->getVkHandle() : VK_NULL_HANDLE, 1, &pipelineFactory.getVkCreateInfo(), nullptr, &vkPipeline),
		"Create GraphicsPipeline Failed.");

	GraphicsPipeline pipeline;
	pipeline.construct(getWeakReference(), vkPipeline, createInfo);
	return pipeline;
}

void Device_::createGraphicsPipelines(const GraphicsPipelineCreateInfo* createInfos, uint32_t numCreateInfos, const PipelineCache& pipelineCache, GraphicsPipeline* outPipelines)
{
	ArrayOrVector<GraphicsPipelinePopulate, 4> pipelineFactories(numCreateInfos);
	ArrayOrVector<VkGraphicsPipelineCreateInfo, 4> vkCreateInfos(numCreateInfos);
	ArrayOrVector<VkPipeline, 4> vkPipelines(numCreateInfos);

	for (uint32_t i = 0; i < numCreateInfos; ++i)
	{
		pipelineFactories[i].init(createInfos[i]);
		vkCreateInfos[i] = pipelineFactories[i].getVkCreateInfo();
	}
	vkThrowIfFailed(getVkBindings().vkCreateGraphicsPipelines(
						getVkHandle(), pipelineCache.isValid() ? pipelineCache->getVkHandle() : VK_NULL_HANDLE, numCreateInfos, vkCreateInfos.get(), nullptr, vkPipelines.get()),
		"Create GraphicsPipeline Failed");

	// create the pipeline wrapper
	for (uint32_t i = 0; i < numCreateInfos; ++i)
	{
		outPipelines[i].construct(getWeakReference(), vkPipelines[i], createInfos[i]);
	}
}

ComputePipeline Device_::createComputePipeline(const ComputePipelineCreateInfo& createInfo, const PipelineCache& pipelineCache)
{
	ComputePipelinePopulate pipelineFactory;
	VkPipeline vkPipeline;

	pipelineFactory.init(createInfo);
	vkThrowIfFailed(getVkBindings().vkCreateComputePipelines(
						getVkHandle(), pipelineCache.isValid() ? pipelineCache->getVkHandle() : VK_NULL_HANDLE, 1, &pipelineFactory.createInfo, nullptr, &vkPipeline),
		"Create ComputePipeline Failed.");

	ComputePipeline pipeline;
	pipeline.construct(getWeakReference(), vkPipeline, createInfo);
	return pipeline;
}

void Device_::createComputePipelines(const ComputePipelineCreateInfo* createInfos, uint32_t numCreateInfos, const PipelineCache& pipelineCache, ComputePipeline* outPipelines)
{
	std::vector<ComputePipelinePopulate> pipelineFactories(numCreateInfos);
	std::vector<VkComputePipelineCreateInfo> vkCreateInfos(numCreateInfos);
	std::vector<VkPipeline> vkPipelines(numCreateInfos, VK_NULL_HANDLE);

	for (uint32_t i = 0; i < numCreateInfos; ++i)
	{
		pipelineFactories[i].init(createInfos[i]);
		vkCreateInfos[i] = pipelineFactories[i].createInfo;
	}
	vkThrowIfFailed(getVkBindings().vkCreateComputePipelines(getVkHandle(), pipelineCache.isValid() ? pipelineCache->getVkHandle() : VK_NULL_HANDLE,
						static_cast<uint32_t>(vkCreateInfos.size()), vkCreateInfos.data(), nullptr, vkPipelines.data()),
		"Create ComputePipelines Failed");

	// create the pipeline wrapper
	for (uint32_t i = 0; i < numCreateInfos; ++i)
	{
		outPipelines[i].construct(getWeakReference(), vkPipelines[i], createInfos[i]);
	}
}

pvrvk::Image Device_::createImage(const ImageCreateInfo& createInfo)
{
	Image image;
	image.construct(getWeakReference(), createInfo);
	return image;
}

void Device_::updateDescriptorSets(const WriteDescriptorSet* writeDescSets, uint32_t numWriteDescSets, const CopyDescriptorSet* copyDescSets, uint32_t numCopyDescSets)
{
	// WRITE DESCRIPTORSET
	std::vector<VkWriteDescriptorSet> vkWriteDescSets(numWriteDescSets);
	// Count number of image, buffer and texel buffer view needed
	uint32_t numImageInfos = 0, numBufferInfos = 0, numTexelBufferView = 0;

	for (uint32_t i = 0; i < numWriteDescSets; ++i)
	{
		if ((writeDescSets[i].getDescriptorType() >= DescriptorType::e_SAMPLER && writeDescSets[i].getDescriptorType() <= DescriptorType::e_STORAGE_IMAGE) ||
			writeDescSets[i].getDescriptorType() == DescriptorType::e_INPUT_ATTACHMENT)
		{
			// Validate the bindings
			numImageInfos += writeDescSets[i].getNumDescriptors();
		}
		else if (writeDescSets[i].getDescriptorType() >= DescriptorType::e_UNIFORM_BUFFER && writeDescSets[i].getDescriptorType() <= DescriptorType::e_STORAGE_BUFFER_DYNAMIC)
		{
#ifdef DEBUG
			std::for_each(writeDescSets[i]._infos.begin(), writeDescSets[i]._infos.end(),
				[](const WriteDescriptorSet::Infos& infos) { debug_assertion(infos.bufferInfo.buffer.isValid(), "Buffer Must be valid"); });
#endif
			numBufferInfos += writeDescSets[i].getNumDescriptors();
		}
		else if (writeDescSets[i].getDescriptorType() == DescriptorType::e_UNIFORM_TEXEL_BUFFER ||
			writeDescSets[i].getDescriptorType() <= DescriptorType::e_STORAGE_TEXEL_BUFFER) // Texel buffer
		{
			numTexelBufferView += writeDescSets[i].getNumDescriptors();
		}
		else
		{
			debug_assertion(false, "");
		}
	}

	// now allocate
	std::vector<VkDescriptorBufferInfo> bufferInfoVk(numBufferInfos);
	std::vector<VkDescriptorImageInfo> imageInfoVk(numImageInfos);
	std::vector<VkBufferView> texelBufferVk(numTexelBufferView);
	uint32_t vkImageInfoOffset = 0;
	uint32_t vkBufferInfoOffset = 0;

	// now process
	for (uint32_t i = 0; i < numWriteDescSets; ++i)
	{
		VkWriteDescriptorSet& vkWriteDescSet = vkWriteDescSets[i];
		const WriteDescriptorSet& writeDescSet = writeDescSets[i];
		memset(&vkWriteDescSet, 0, sizeof(vkWriteDescSet));
		vkWriteDescSet.descriptorType = static_cast<VkDescriptorType>(writeDescSet.getDescriptorType());
		vkWriteDescSet.sType = static_cast<VkStructureType>(StructureType::e_WRITE_DESCRIPTOR_SET);
		vkWriteDescSet.dstArrayElement = writeDescSet.getDestArrayElement();
		vkWriteDescSet.dstBinding = writeDescSet.getDestBinding();
		vkWriteDescSet.dstSet = writeDescSet.getDescriptorSet()->getVkHandle();
		writeDescSet.updateKeepAliveIntoDestinationDescriptorSet();

		// Do the buffer info
		if (writeDescSet._infoType == WriteDescriptorSet::InfoType::BufferInfo)
		{
			vkWriteDescSet.pBufferInfo = bufferInfoVk.data() + vkBufferInfoOffset;
			std::transform(writeDescSet._infos.begin(), writeDescSet._infos.end(), bufferInfoVk.begin() + vkBufferInfoOffset,
				[&](const WriteDescriptorSet::Infos& writeDescSet) -> VkDescriptorBufferInfo {
					return VkDescriptorBufferInfo{ writeDescSet.bufferInfo.buffer->getVkHandle(), writeDescSet.bufferInfo.offset, writeDescSet.bufferInfo.range };
				});
			vkWriteDescSet.descriptorCount = writeDescSet._infos.size();
			vkBufferInfoOffset += writeDescSet._infos.size();
		}
		else if (writeDescSet._infoType == WriteDescriptorSet::InfoType::ImageInfo)
		{
			vkWriteDescSet.pImageInfo = imageInfoVk.data() + vkImageInfoOffset;
			std::transform(writeDescSet._infos.begin(), writeDescSet._infos.end(), imageInfoVk.begin() + vkImageInfoOffset,
				[&](const WriteDescriptorSet::Infos& writeDescSet) -> VkDescriptorImageInfo {
					return VkDescriptorImageInfo{ (writeDescSet.imageInfo.sampler.isValid() ? writeDescSet.imageInfo.sampler->getVkHandle() : VK_NULL_HANDLE),
						(writeDescSet.imageInfo.imageView.isValid() ? writeDescSet.imageInfo.imageView->getVkHandle() : VK_NULL_HANDLE),
						static_cast<VkImageLayout>(writeDescSet.imageInfo.imageLayout) };
				});
			vkWriteDescSet.descriptorCount = writeDescSet._infos.size();
			vkImageInfoOffset += writeDescSet._infos.size();
		}
	}

	// COPY DESCRIPTOR SET
	std::vector<VkCopyDescriptorSet> vkCopyDescriptorSets(numCopyDescSets);
	std::transform(copyDescSets, copyDescSets + numCopyDescSets, vkCopyDescriptorSets.begin(), [&](const CopyDescriptorSet& copyDescSet) {
		return VkCopyDescriptorSet{ static_cast<VkStructureType>(StructureType::e_COPY_DESCRIPTOR_SET), nullptr, copyDescSet.srcSet->getVkHandle(), copyDescSet.srcBinding,
			copyDescSet.srcArrayElement, copyDescSet.dstSet->getVkHandle(), copyDescSet.dstBinding, copyDescSet.dstArrayElement, copyDescSet.descriptorCount };
	});

	getVkBindings().vkUpdateDescriptorSets(
		getVkHandle(), static_cast<uint32_t>(vkWriteDescSets.size()), vkWriteDescSets.data(), static_cast<uint32_t>(vkCopyDescriptorSets.size()), vkCopyDescriptorSets.data());
}

ImageView Device_::createImageView(const ImageViewCreateInfo& createInfo)
{
	ImageView imageView;
	imageView.construct(getWeakReference(), createInfo);
	return imageView;
}

Framebuffer Device_::createFramebuffer(const FramebufferCreateInfo& createInfo)
{
	Framebuffer framebuffer;
	framebuffer.construct(getWeakReference(), createInfo);
	return framebuffer;
}

Fence Device_::createFence(const FenceCreateInfo& createInfo)
{
	Fence fence;
	fence.construct(getWeakReference(), createInfo);
	return fence;
}

Event Device_::createEvent(const EventCreateInfo& createInfo)
{
	Event event;
	event.construct(getWeakReference(), createInfo);
	return event;
}

Semaphore Device_::createSemaphore(const SemaphoreCreateInfo& createInfo)
{
	Semaphore semaphore;
	semaphore.construct(getWeakReference(), createInfo);
	return semaphore;
}

Buffer Device_::createBuffer(const BufferCreateInfo& createInfo)
{
	Buffer buffer;
	buffer.construct(getWeakReference(), createInfo);
	return buffer;
}

DeviceMemory Device_::allocateMemory(const MemoryAllocationInfo& allocationInfo)
{
	debug_assertion(allocationInfo.getMemoryTypeIndex() != uint32_t(-1) && allocationInfo.getAllocationSize() > 0u, "Invalid MemoryAllocationInfo");
	DeviceMemoryImpl mem;
	const MemoryPropertyFlags memFlags = _physicalDevice->getMemoryProperties().getMemoryTypes()[allocationInfo.getMemoryTypeIndex()].getPropertyFlags();
	mem.construct(getWeakReference(), allocationInfo, memFlags);
	return mem;
}

ShaderModule Device_::createShaderModule(const ShaderModuleCreateInfo& createInfo)
{
	ShaderModule shaderModule;
	shaderModule.construct(getWeakReference(), createInfo);
	return shaderModule;
}

Sampler Device_::createSampler(const SamplerCreateInfo& createInfo)
{
	Sampler sampler;
	sampler.construct(getWeakReference(), createInfo);
	return sampler;
}

RenderPass Device_::createRenderPass(const RenderPassCreateInfo& createInfo)
{
	RenderPass renderPass;
	renderPass.construct(getReference(), createInfo);
	return renderPass;
}

BufferView Device_::createBufferView(const BufferViewCreateInfo& createInfo)
{
	BufferView bufferview;
	bufferview.construct(getWeakReference(), createInfo);
	return bufferview;
}

DescriptorPool Device_::createDescriptorPool(const DescriptorPoolCreateInfo& createInfo)
{
	DescriptorPool descPool = impl::DescriptorPool_::createNew(getWeakReference(), createInfo);
	return descPool;
}

CommandPool Device_::createCommandPool(const CommandPoolCreateInfo& createInfo)
{
	return impl::CommandPool_::createNew(getWeakReference(), createInfo);
}

PipelineLayout Device_::createPipelineLayout(const PipelineLayoutCreateInfo& createInfo)
{
	PipelineLayout pipelayout;
	pipelayout.construct(getWeakReference(), createInfo);
	return pipelayout;
}

bool Device_::waitForFences(uint32_t numFences, const Fence* const fences, const bool waitAll, const uint64_t timeout)
{
	VkFence vkFenceArray[10];
	std::vector<VkFence> vkFenceVec(0);
	VkFence* vkFences = vkFenceArray;
	if (numFences > sizeof(vkFenceArray) / sizeof(vkFenceArray[0]))
	{
		vkFenceVec.resize(numFences);
		vkFences = vkFenceVec.data();
	}

	for (uint32_t i = 0; i < numFences; i++)
	{
		vkFences[i] = fences[i]->getVkHandle();
	}

	Result res;
	vkThrowIfError(res = static_cast<pvrvk::Result>(getVkBindings().vkWaitForFences(getVkHandle(), numFences, vkFences, waitAll, timeout)), "WaitForFences failed");
	if (res == Result::e_SUCCESS)
	{
		return true;
	}
	debug_assertion(res == Result::e_TIMEOUT, "WaitForFences returned neither success nor timeout, yet did not throw!");
	return false;
}

void Device_::resetFences(uint32_t numFences, const Fence* const fences)
{
	VkFence vkFenceArray[10];
	std::vector<VkFence> vkFenceVec(0);
	VkFence* vkFences = vkFenceArray;
	if (numFences > sizeof(vkFenceArray) / sizeof(vkFenceArray[0]))
	{
		vkFenceVec.resize(numFences);
		vkFences = vkFenceVec.data();
	}

	for (uint32_t i = 0; i < numFences; i++)
	{
		vkFences[i] = fences[i]->getVkHandle();
	}

	vkThrowIfFailed(getVkBindings().vkResetFences(getVkHandle(), numFences, vkFences), "Reset fences failed");
}

pvrvk::DescriptorSetLayout Device_::createDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo)
{
	DescriptorSetLayout layout;
	layout.construct(getWeakReference(), createInfo);
	return layout;
}

PipelineCache Device_::createPipelineCache(const PipelineCacheCreateInfo& createInfo)
{
	PipelineCache pipelineCache;
	pipelineCache.construct(getWeakReference(), createInfo);
	return pipelineCache;
}

void Device_::mergePipelineCache(const PipelineCache* srcPipeCaches, uint32_t numSrcPipeCaches, PipelineCache destPipeCache)
{
	std::vector<VkPipelineCache> vkSrcPipeCaches(numSrcPipeCaches);
	std::transform(srcPipeCaches, srcPipeCaches + numSrcPipeCaches, vkSrcPipeCaches.begin(), [&](const PipelineCache& pipelineCache) { return pipelineCache->getVkHandle(); });

	vkThrowIfFailed(getVkBindings().vkMergePipelineCaches(getVkHandle(), destPipeCache->getVkHandle(), numSrcPipeCaches, vkSrcPipeCaches.data()), "Failed to merge Pipeline Caches");
}

Swapchain Device_::createSwapchain(const SwapchainCreateInfo& createInfo, const Surface& surface)
{
	Swapchain swapchain;
	swapchain.construct(getWeakReference(), surface, createInfo);
	return swapchain;
}

QueryPool Device_::createQueryPool(const QueryPoolCreateInfo& createInfo)
{
	QueryPool queryPool = impl::QueryPool_::createNew(getWeakReference(), createInfo);
	return queryPool;
}

void Device_::waitIdle()
{
	getVkBindings().vkDeviceWaitIdle(getVkHandle());
}

// CAUTION - We will be abusing queueFamilyProperties[...].numQueues as a counter for queues remaining.
struct QueueFamilyCreateInfo
{
	uint32_t queueFamilyId;
	uint32_t queueId;
	bool supportPresentation;
	explicit QueueFamilyCreateInfo(uint32_t queueFamilyId = -1, uint32_t queueId = -1, bool supportPresentation = 0)
		: queueFamilyId(queueFamilyId), queueId(queueId), supportPresentation(supportPresentation)
	{}
};

Device_::Device_(PhysicalDeviceWeakPtr physicalDevice, const DeviceCreateInfo& createInfo) : PhysicalDeviceObjectHandle(physicalDevice)
{
	_createInfo = createInfo;

	debug_assertion(_physicalDevice->getQueueFamilyProperties().size() >= static_cast<size_t>(1), "A Vulkan device must support at least 1 queue family.");

	ArrayOrVector<VkDeviceQueueCreateInfo, 4> queueCreateInfos(_createInfo.getNumDeviceQueueCreateInfos());
	for (uint32_t i = 0; i < _createInfo.getNumDeviceQueueCreateInfos(); ++i)
	{
		const DeviceQueueCreateInfo& queueCreateInfo = _createInfo.getDeviceQueueCreateInfo(i);

		VkDeviceQueueCreateInfo queueCreateInfoVk = {};
		queueCreateInfoVk.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfoVk.queueFamilyIndex = queueCreateInfo.getQueueFamilyIndex();
		queueCreateInfoVk.queueCount = queueCreateInfo.getNumQueues();
		queueCreateInfoVk.pQueuePriorities = queueCreateInfo.getQueuePriorities().data();
		queueCreateInfos[i] = queueCreateInfoVk;
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DEVICE_CREATE_INFO);
	deviceCreateInfo.flags = static_cast<VkDeviceCreateFlags>(_createInfo.getFlags());
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(_createInfo.getNumDeviceQueueCreateInfos());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.get();

	deviceCreateInfo.pEnabledFeatures = &_createInfo.getEnabledFeatures()->get();

	// Extensions
	std::vector<const char*> enabledExtensions;
	if (_createInfo.getNumEnabledExtensionNames())
	{
		for (uint32_t i = 0; i < _createInfo.getNumEnabledExtensionNames(); ++i)
		{
			enabledExtensions.push_back(_createInfo.getEnabledExtensionName(i).c_str());
		}

		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}

	vkThrowIfFailed(
		_physicalDevice->getInstance()->getVkBindings().vkCreateDevice(_physicalDevice->getVkHandle(), &deviceCreateInfo, nullptr, &_vkHandle), "Vulkan Device Creation failed");

	// CHECK PVRTC SUPPORT
	_supportsPVRTC = isExtensionEnabled("VK_IMG_format_pvrtc");

	initVkDeviceBindings(getVkHandle(), &_vkBindings, _physicalDevice->getInstance()->getVkBindings().vkGetDeviceProcAddr);
	const std::vector<QueueFamilyProperties>& queueFamProps = _physicalDevice->getQueueFamilyProperties();

	uint32_t queueFamilyIndex;
	uint32_t queueIndex;
	float queuePriority;
	for (uint32_t i = 0; i < _createInfo.getNumDeviceQueueCreateInfos(); ++i)
	{
		queueFamilyIndex = queueCreateInfos[i].queueFamilyIndex;

		_queueFamilies.push_back(QueueFamily());
		_queueFamilies.back().queueFamily = queueFamilyIndex;
		_queueFamilies.back().queues.resize(queueCreateInfos[i].queueCount);
		VkQueue vkQueue;
		for (queueIndex = 0; queueIndex < queueCreateInfos[i].queueCount; ++queueIndex)
		{
			queuePriority = queueCreateInfos[i].pQueuePriorities[queueIndex];
			getVkBindings().vkGetDeviceQueue(getVkHandle(), queueFamilyIndex, queueIndex, &vkQueue);
			_queueFamilies.back().queues[queueIndex].construct(getWeakReference(), vkQueue, queueFamProps[queueFamilyIndex].getQueueFlags(), queueFamilyIndex, queuePriority);
		}
	}
}
} // namespace impl
} // namespace pvrvk

//!\endcond
