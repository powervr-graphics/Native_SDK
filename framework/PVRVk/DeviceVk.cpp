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

pvrvk::GraphicsPipeline Device_::createGraphicsPipeline(const GraphicsPipelineCreateInfo& desc, const PipelineCache& pipelineCache)
{
	GraphicsPipelinePopulate pipelineFactory;
	VkPipeline vkPipeline;

	if (!pipelineFactory.init(desc))
	{
		return GraphicsPipeline();
	}
	vkThrowIfFailed(getVkBindings().vkCreateGraphicsPipelines(
						getVkHandle(), pipelineCache.isValid() ? pipelineCache->getVkHandle() : VK_NULL_HANDLE, 1, &pipelineFactory.getVkCreateInfo(), nullptr, &vkPipeline),
		"Create GraphicsPipeline Failed.");

	GraphicsPipeline pipeline;
	pipeline.construct(getWeakReference(), vkPipeline, desc);
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

	getVkBindings().vkUpdateDescriptorSets(getVkHandle(), (uint32_t)vkWriteDescSets.size(), vkWriteDescSets.data(), (uint32_t)vkCopyDescriptorSets.size(), vkCopyDescriptorSets.data());
}

namespace {
inline ImageViewType convertToPVRVkImageViewType(ImageType baseType, uint32_t numArrayLayers, bool isCubeMap)
{
	// if it is a cube map it has to be 2D Texture base
	if (isCubeMap && baseType != ImageType::e_2D)
	{
		assertion(baseType == ImageType::e_2D, "Cubemap texture must be 2D");
		return ImageViewType::e_MAX_ENUM;
	}
	// array must be atleast 1
	if (!numArrayLayers)
	{
		assertion(false, "Number of array layers must be greater than equal to 0");
		return ImageViewType::e_MAX_ENUM;
	}
	// if it is array it must be 1D or 2D texture base
	if ((numArrayLayers > 1) && (baseType > ImageType::e_2D))
	{
		assertion(false, "1D and 2D image type supports array texture");
		return ImageViewType::e_MAX_ENUM;
	}

	ImageViewType vkType[] = { ImageViewType::e_1D, ImageViewType::e_1D_ARRAY, ImageViewType::e_2D, ImageViewType::e_2D_ARRAY, ImageViewType::e_3D, ImageViewType::e_CUBE,
		ImageViewType::e_CUBE_ARRAY };
	if (isCubeMap)
	{
		numArrayLayers = (numArrayLayers > 6) * 6;
	}
	return vkType[(static_cast<uint32_t>(baseType) * 2) + (isCubeMap ? 3 : 0) + (numArrayLayers > 1 ? 1 : 0)];
}
} // namespace

inline ImageAspectFlags formatToImageAspect(Format format)
{
	if (format == Format::e_UNDEFINED || format == Format::e_NONE)
	{
		throw ErrorUnknown("Cannot retrieve VkImageAspectFlags from an undefined VkFormat");
	}
	if (format < Format::e_D16_UNORM || format > Format::e_D32_SFLOAT_S8_UINT)
	{
		return ImageAspectFlags::e_COLOR_BIT;
	}
	const ImageAspectFlags formats[] = {
		ImageAspectFlags::e_DEPTH_BIT, // VkFormat::e_D16_UNORM
		ImageAspectFlags::e_DEPTH_BIT, // VkFormat::e_X8_D24_UNORM_PACK32
		ImageAspectFlags::e_DEPTH_BIT, // VkFormat::e_D32_SFLOAT
		ImageAspectFlags::e_STENCIL_BIT, // VkFormat::e_S8_UINT
		ImageAspectFlags::e_DEPTH_BIT | ImageAspectFlags::e_STENCIL_BIT, // VkFormat::e_D16_UNORM_S8_UINT
		ImageAspectFlags::e_DEPTH_BIT | ImageAspectFlags::e_STENCIL_BIT, // VkFormat::e_D24_UNORM_S8_UINT
		ImageAspectFlags::e_DEPTH_BIT | ImageAspectFlags::e_STENCIL_BIT, // VkFormat::e_D32_SFLOAT_S8_UINT
	};
	return formats[static_cast<uint32_t>(format) - static_cast<uint32_t>(Format::e_D16_UNORM)];
}

ImageView Device_::createImageView(const Image& image, ImageViewType viewType, Format format, const ImageSubresourceRange& range, const ComponentMapping& swizzleChannels)
{
	ImageView imageView;
	imageView.construct(image, viewType, format, range, swizzleChannels);
	return imageView;
}

ImageView Device_::createImageView(const Image& image, const ComponentMapping& swizzleChannels)
{
	ImageSubresourceRange range;
	range.setAspectMask(formatToImageAspect(image->getFormat()));
	range.setLevelCount(image->getNumMipLevels());
	range.setLayerCount(image->getNumArrayLayers());
	return createImageView(image, convertToPVRVkImageViewType(image->getImageType(), image->getNumArrayLayers(), image->isCubeMap()), image->getFormat(), range, swizzleChannels);
}

Framebuffer Device_::createFramebuffer(const FramebufferCreateInfo& desc)
{
	Framebuffer framebuffer;
	framebuffer.construct(getWeakReference(), desc);
	return framebuffer;
}

Fence Device_::createFence(FenceCreateFlags fenceCreateFlags)
{
	Fence fence;
	fence.construct(getWeakReference(), fenceCreateFlags);
	return fence;
}

Event Device_::createEvent()
{
	Event event;
	event.construct(getWeakReference());
	return event;
}

Semaphore Device_::createSemaphore()
{
	Semaphore semaphore;
	semaphore.construct(getWeakReference());
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

ShaderModule Device_::createShader(const std::vector<uint32_t>& shaderSrc)
{
	ShaderModule vs;
	vs.construct(getWeakReference(), shaderSrc);
	return vs;
}

Sampler Device_::createSampler(const SamplerCreateInfo& desc)
{
	Sampler sampler;
	sampler.construct(getWeakReference(), desc);
	return sampler;
}

RenderPass Device_::createRenderPass(const RenderPassCreateInfo& renderPass)
{
	RenderPass rp;
	rp.construct(getReference(), renderPass);
	return rp;
}

BufferView Device_::createBufferView(const Buffer& buffer, Format format, VkDeviceSize offset, VkDeviceSize range)
{
	assertion(range == 0xFFFFFFFFu || (range <= buffer->getSize() - offset));
	BufferView bufferview;
	bufferview.construct(getWeakReference(), buffer, format, offset, std::min(range, buffer->getSize() - offset));
	return bufferview;
}

DescriptorPool Device_::createDescriptorPool(const DescriptorPoolCreateInfo& createInfo)
{
	DescriptorPool descPool = impl::DescriptorPool_::createNew(getWeakReference(), createInfo);
	return descPool;
}

CommandPool Device_::createCommandPool(uint32_t queueFamilyId, CommandPoolCreateFlags createFlags)
{
	return impl::CommandPool_::createNew(getWeakReference(), queueFamilyId, createFlags);
}

PipelineLayout Device_::createPipelineLayout(const PipelineLayoutCreateInfo& desc)
{
	PipelineLayout pipelayout;
	pipelayout.construct(getWeakReference(), desc);
	return pipelayout;
}

bool Device_::waitForFences(uint32_t numFences, const Fence* const fences, const bool waitAll, const uint64_t timeout)
{
	VkFence vkFenceArray[10];
	std::vector<VkFence> vkFenceVec(0);
	VkFence* vkFences = vkFenceArray;
	if (numFences > ARRAY_SIZE(vkFenceArray))
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
	if (numFences > ARRAY_SIZE(vkFenceArray))
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

pvrvk::DescriptorSetLayout Device_::createDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& desc)
{
	DescriptorSetLayout layout;
	layout.construct(getWeakReference(), desc);
	return layout;
}

PipelineCache Device_::createPipelineCache(size_t initialDataSize, const void* initialData, PipelineCacheCreateFlags flags)
{
	PipelineCache pipelineCache;
	pipelineCache.construct(getWeakReference(), initialDataSize, initialData, flags);
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

QueryPool Device_::createQueryPool(QueryType queryType, uint32_t queryCount, QueryPipelineStatisticFlags statisticsFlags)
{
	QueryPool queryPool = impl::QueryPool_::createNew(getWeakReference(), queryType, queryCount, statisticsFlags);
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

	deviceCreateInfo.pEnabledFeatures = reinterpret_cast<const VkPhysicalDeviceFeatures*>(_createInfo.getEnabledFeatures());

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
	for (uint32_t i = 0; i < _createInfo.getNumDeviceQueueCreateInfos(); ++i)
	{
		queueFamilyIndex = queueCreateInfos[i].queueFamilyIndex;

		_queueFamilies.push_back(QueueFamily());
		_queueFamilies.back().queueFamily = queueFamilyIndex;
		_queueFamilies.back().queues.resize(queueCreateInfos[i].queueCount);
		VkQueue vkQueue;
		for (queueIndex = 0; queueIndex < queueCreateInfos[i].queueCount; ++queueIndex)
		{
			getVkBindings().vkGetDeviceQueue(getVkHandle(), queueFamilyIndex, queueIndex, &vkQueue);
			_queueFamilies.back().queues[queueIndex].construct(getWeakReference(), vkQueue, queueFamProps[queueFamilyIndex].getQueueFlags(), queueFamilyIndex);
		}
	}
}
} // namespace impl
} // namespace pvrvk

//!\endcond
