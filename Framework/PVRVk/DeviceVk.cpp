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
#include "PVRVk/ShaderVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/RenderPassVk.h"
#include "PVRVk/SyncVk.h"
#include "PVRVk/GraphicsPipelineVk.h"
#include "PVRVk/ComputePipelineVk.h"
#include "PVRVk/PopulateCreateInfoVk.h"
#include "PVRVk/SwapchainVk.h"
#include "PVRVk/PipelineCacheVk.h"


namespace pvrvk {
inline char const* vkErrorToStr(VkResult errorCode)
{
	switch (errorCode)
	{
	case VkResult::e_SUCCESS: return "VkResult::e_SUCCESS";
	case VkResult::e_NOT_READY: return "VkResult::e_NOT_READY";
	case VkResult::e_TIMEOUT: return "VkResult::e_TIMEOUT";
	case VkResult::e_EVENT_SET: return "VkResult::e_EVENT_SET";
	case VkResult::e_EVENT_RESET: return "VkResult::e_EVENT_RESET";
	case VkResult::e_INCOMPLETE: return "VkResult::e_INCOMPLETE";
	case VkResult::e_ERROR_OUT_OF_HOST_MEMORY: return "VkResult::e_ERROR_OUT_OF_HOST_MEMORY";
	case VkResult::e_ERROR_OUT_OF_DEVICE_MEMORY: return "VkResult::e_ERROR_OUT_OF_DEVICE_MEMORY";
	case VkResult::e_ERROR_INITIALIZATION_FAILED: return "VkResult::e_ERROR_INITIALIZATION_FAILED";
	case VkResult::e_ERROR_DEVICE_LOST: return "VkResult::e_ERROR_DEVICE_LOST";
	case VkResult::e_ERROR_MEMORY_MAP_FAILED: return "VkResult::e_ERROR_MEMORY_MAP_FAILED";
	case VkResult::e_ERROR_LAYER_NOT_PRESENT: return "VkResult::e_ERROR_LAYER_NOT_PRESENT";
	case VkResult::e_ERROR_EXTENSION_NOT_PRESENT: return "VkResult::e_ERROR_EXTENSION_NOT_PRESENT";
	case VkResult::e_ERROR_FEATURE_NOT_PRESENT: return "VkResult::e_ERROR_FEATURE_NOT_PRESENT";
	case VkResult::e_ERROR_INCOMPATIBLE_DRIVER: return "VkResult::e_ERROR_INCOMPATIBLE_DRIVER";
	case VkResult::e_ERROR_TOO_MANY_OBJECTS: return "VkResult::e_ERROR_TOO_MANY_OBJECTS";
	case VkResult::e_ERROR_FORMAT_NOT_SUPPORTED: return "VkResult::e_ERROR_FORMAT_NOT_SUPPORTED";
	case VkResult::e_ERROR_SURFACE_LOST_KHR: return "VkResult::e_ERROR_SURFACE_LOST_KHR";
	case VkResult::e_SUBOPTIMAL_KHR: return "VkResult::e_SUBOPTIMAL_KHR";
	case VkResult::e_ERROR_OUT_OF_DATE_KHR: return "VkResult::e_ERROR_OUT_OF_DATE_KHR";
	case VkResult::e_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VkResult::e_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VkResult::e_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VkResult::e_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VkResult::e_ERROR_VALIDATION_FAILED_EXT: return "VkResult::e_ERROR_VALIDATION_FAILED_EXT";
	case VkResult::e_ERROR_FRAGMENTED_POOL: return "VkResult::e_ERROR_FRAGMENTED_POOL";
	case VkResult::e_ERROR_INVALID_SHADER_NV: return "VkResult::e_ERROR_INVALID_SHADER_NV";
	// Declared here to Hide the Warnings.
	default: return "";
	}
}

inline bool vkIsSuccessful(VkResult result, const char* msg)
{
	if (result != VkResult::e_SUCCESS)
	{
		Log(LogLevel::Error, "Failed: %s. Vulkan has raised an error: %s", msg, vkErrorToStr(result));
		assertion(0);
		return false;
	}
	return true;
}

static inline VkDeviceQueueCreateInfo createQueueCreateInfo()
{
	static const float priority[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo createInfo = {};
	createInfo.queueCount = 1;
	createInfo.sType = VkStructureType::e_DEVICE_QUEUE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.queueFamilyIndex = static_cast<uint32_t>(-1);
	createInfo.pQueuePriorities = priority;
	createInfo.flags = 0;

	return createInfo;
}
}
// Object creation
namespace pvrvk {
namespace impl {
std::vector<const char*> filterExtensions(const std::vector<VkExtensionProperties>& vec,
    const char* const* filters, uint32_t numfilters)
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

std::vector<const char*> filterLayers(const std::vector<VkLayerProperties>& vec,
                                      const char* const* filters, uint32_t numfilters)
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


pvrvk::GraphicsPipeline Device_::createGraphicsPipeline(const GraphicsPipelineCreateInfo& desc, const PipelineCache& pipeCache)
{
	GraphicsPipelinePopulate createFactory;
	VkPipeline vkPipeline;

	if (!createFactory.init(desc))
	{
		return GraphicsPipeline();
	}
	bool result = vkIsSuccessful(vk::CreateGraphicsPipelines(getNativeObject(),
	                             pipeCache.isValid() ? pipeCache->getNativeObject() : VK_NULL_HANDLE,
	                             1, &createFactory.getVkCreateInfo(), nullptr, &vkPipeline), "Create GraphicsPipeline");
	if (!result)
	{
		return GraphicsPipeline();
	}

	GraphicsPipeline pipeline; pipeline.construct(getWeakReference());
	pipeline->init(vkPipeline, desc);
	return pipeline;
}

bool Device_::createGraphicsPipelines(const GraphicsPipelineCreateInfo* createInfos,
                                      uint32_t numCreateInfos, const PipelineCache& pipeCache, GraphicsPipeline* outPipelines)
{
	std::vector<GraphicsPipelinePopulate> createFactory(numCreateInfos);
	std::vector<VkGraphicsPipelineCreateInfo> vkCreateInfo(numCreateInfos);
	std::vector<VkPipeline> vkPipelines(numCreateInfos);

	for (uint32_t i = 0; i < numCreateInfos; ++i)
	{
		if (!createFactory[i].init(createInfos[i]))
		{
			return false;
		}
		vkCreateInfo[i] = createFactory[i].getVkCreateInfo();
	}
	bool result = vkIsSuccessful(vk::CreateGraphicsPipelines(getNativeObject(), pipeCache.isValid() ? pipeCache->getNativeObject() : VK_NULL_HANDLE,
	                             numCreateInfos, vkCreateInfo.data(), nullptr, vkPipelines.data()), "Create GraphicsPipeline");
	if (!result)
	{
		return false;
	}

	// create the pipeline wrapper
	for (uint32_t i = 0; i < numCreateInfos; ++i)
	{
		outPipelines[i].construct(getWeakReference());
		result = outPipelines[i]->init(vkPipelines[i], createInfos[i]);
	}
	return result;
}

bool Device_::createComputePipelines(const ComputePipelineCreateInfo* createInfo,
                                     uint32_t numCreateInfos, const PipelineCache& pipeCache, ComputePipeline* outPipelines)
{
	std::vector<impl::ComputePipelinePopulate> createInfoPopulate(numCreateInfos);
	std::vector<VkComputePipelineCreateInfo> vkCreateInfo(numCreateInfos);
	std::vector<VkPipeline> vkPipeline(numCreateInfos, VK_NULL_HANDLE);

	for (uint32_t  i = 0; i < numCreateInfos; ++i)
	{
		if (!createInfoPopulate[i].init(createInfo[i]))
		{
			return false;
		}
		vkCreateInfo[i] = createInfoPopulate[i].createInfo;
	}
	bool rslt =  vkIsSuccessful(vk::CreateComputePipelines(getNativeObject(),
	                            pipeCache.isValid() ? pipeCache->getNativeObject() : VK_NULL_HANDLE,
	                            static_cast<uint32_t>(vkCreateInfo.size()), vkCreateInfo.data(), nullptr, vkPipeline.data()),
	                            "Create ComputePipeline");
	if (!rslt)
	{
		Log(LogLevel::Error, "Failed to create compute pipeline.");
		return false;
	}
	for (uint32_t i = 0; i < numCreateInfos; ++i)
	{
		outPipelines[i].construct(getWeakReference(), createInfo[i], vkPipeline[i]);
	}
	return true;
}

ComputePipeline Device_::createComputePipeline(
  const ComputePipelineCreateInfo& createInfo, const PipelineCache& pipeCache)
{
	impl::ComputePipelinePopulate createInfoPopulate;
	if (!createInfoPopulate.init(createInfo))
	{
		return ComputePipeline();
	}
	VkPipeline vkPipeline;
	bool rslt =  vkIsSuccessful(vk::CreateComputePipelines(getNativeObject(),
	                            pipeCache.isValid() ? pipeCache->getNativeObject() : VK_NULL_HANDLE, 1, &createInfoPopulate.createInfo,
	                            nullptr, &vkPipeline), "Create ComputePipeline");

	if (!rslt)
	{
		Log(LogLevel::Error, "Failed to create compute pipeline.");
		return ComputePipeline();
	}

	ComputePipeline pipeline;
	pipeline.construct(getWeakReference(), createInfo, vkPipeline);
	return pipeline;
}

pvrvk::Image Device_::createImage(VkImageType imageType, VkFormat format,
                                  const Extent3D& dimension, VkImageUsageFlags usage,
                                  VkImageCreateFlags flags, const ImageLayersSize& layerSize,
                                  VkSampleCountFlags samples,
                                  bool sharingExclusive, const uint32_t* queueFamilyIndices,
                                  uint32_t numQueueFamilyIndices)
{
	Image image; image.construct(getWeakReference());
	if (!image->init(imageType, ImageAreaSize(layerSize, dimension),
	                 format, usage, flags, samples, sharingExclusive, queueFamilyIndices, numQueueFamilyIndices))
	{
		image.reset();
	}
	return image;
}

void Device_::updateDescriptorSets(const WriteDescriptorSet* writeDescSets,
                                   uint32_t numWriteDescSets, const CopyDescriptorSet* copyDescSets, uint32_t numCopyDescSets)
{
	// WRITE DESCRIPTORSET
	std::vector<VkWriteDescriptorSet> vkWriteDescSets(numWriteDescSets);
	// Count number of image, buffer and texel buffer view needed
	uint32_t numImageInfos = 0, numBufferInfos = 0, numTexelBufferView = 0;

	for (uint32_t i = 0; i < numWriteDescSets; ++i)
	{
		if ((writeDescSets[i].getDescriptorType() >= VkDescriptorType::e_SAMPLER &&
		     writeDescSets[i].getDescriptorType() <= VkDescriptorType::e_STORAGE_IMAGE) ||
		    writeDescSets[i].getDescriptorType() == VkDescriptorType::e_INPUT_ATTACHMENT)
		{
			// Validate the bindings

			numImageInfos += writeDescSets[i].getNumDescriptors();
		}
		else if (writeDescSets[i].getDescriptorType() >= VkDescriptorType::e_UNIFORM_BUFFER &&
		         writeDescSets[i].getDescriptorType() <= VkDescriptorType::e_STORAGE_BUFFER_DYNAMIC)
		{
#ifdef DEBUG
			std::for_each(writeDescSets[i]._infos.begin(), writeDescSets[i]._infos.end(),
			              [](const WriteDescriptorSet::Infos & infos)
			{
				debug_assertion(infos.bufferInfo.buffer.isValid(), "Buffer Must be valid");
			});
#endif
			numBufferInfos += writeDescSets[i].getNumDescriptors();
		}
		else if (writeDescSets[i].getDescriptorType() == VkDescriptorType::e_UNIFORM_TEXEL_BUFFER ||
		         writeDescSets[i].getDescriptorType() <= VkDescriptorType::e_STORAGE_TEXEL_BUFFER) // Texel buffer
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
	std::vector<VkDescriptorImageInfo>  imageInfoVk(numImageInfos);
	std::vector<VkBufferView> texelBufferVk(numTexelBufferView);
	uint32_t vkImageInfoOffset = 0;
	uint32_t vkBufferInfoOffset = 0;

	// now process
	for (uint32_t i = 0; i < numWriteDescSets; ++i)
	{
		VkWriteDescriptorSet& vkWriteDescSet = vkWriteDescSets[i];
		const WriteDescriptorSet& writeDescSet = writeDescSets[i];
		memset(&vkWriteDescSet, 0, sizeof(vkWriteDescSet));
		vkWriteDescSet.descriptorType = writeDescSet.getDescriptorType();
		vkWriteDescSet.sType = VkStructureType::e_WRITE_DESCRIPTOR_SET;
		vkWriteDescSet.dstArrayElement = writeDescSet.getDestArrayElement();
		vkWriteDescSet.dstBinding = writeDescSet.getDestBinding();
		vkWriteDescSet.dstSet = writeDescSet.getDescriptorSet()->getNativeObject();
		writeDescSet.updateKeepAliveIntoDestinationDescriptorSet();

		// Do the buffer info
		if (writeDescSet._infoType == WriteDescriptorSet::InfoType::BufferInfo)
		{
			vkWriteDescSet.pBufferInfo = bufferInfoVk.data() + vkBufferInfoOffset;
			std::transform(writeDescSet._infos.begin(), writeDescSet._infos.end(),
			               bufferInfoVk.begin() + vkBufferInfoOffset,
			               [&](const WriteDescriptorSet::Infos & writeDescSet) -> VkDescriptorBufferInfo
			{
				return VkDescriptorBufferInfo
				{
					writeDescSet.bufferInfo.buffer->getNativeObject(),
					writeDescSet.bufferInfo.offset,
					writeDescSet.bufferInfo.range
				};
			}
			              );
			vkWriteDescSet.descriptorCount = writeDescSet._infos.size();
			vkBufferInfoOffset += writeDescSet._infos.size();
		}
		else if (writeDescSet._infoType == WriteDescriptorSet::InfoType::ImageInfo)
		{
			vkWriteDescSet.pImageInfo =  imageInfoVk.data() + vkImageInfoOffset;

			std::transform(writeDescSet._infos.begin(), writeDescSet._infos.end(),
			               imageInfoVk.begin() + vkImageInfoOffset,
			               [&](const WriteDescriptorSet::Infos & writeDescSet) -> VkDescriptorImageInfo
			{
				return VkDescriptorImageInfo
				{
					(writeDescSet.imageInfo.sampler.isValid() ? writeDescSet.imageInfo.sampler->getNativeObject() : VK_NULL_HANDLE),
					(writeDescSet.imageInfo.imageView.isValid() ? writeDescSet.imageInfo.imageView->getNativeObject() : VK_NULL_HANDLE),
					writeDescSet.imageInfo.imageLayout
				};
			});
			vkWriteDescSet.descriptorCount = writeDescSet._infos.size();
			vkImageInfoOffset += writeDescSet._infos.size();
		}
	}

	// COPY DESCRIPTOR SET
	std::vector<VkCopyDescriptorSet> vkCopyDescriptorSets(numCopyDescSets);
	std::transform(copyDescSets, copyDescSets + numCopyDescSets, vkCopyDescriptorSets.begin(),
	               [&](const CopyDescriptorSet & copyDescSet)
	{
		return VkCopyDescriptorSet
		{
			VkStructureType::e_COPY_DESCRIPTOR_SET,
			nullptr,
			copyDescSet.srcSet->getNativeObject(),
			copyDescSet.srcBinding,
			copyDescSet.srcArrayElement,
			copyDescSet.dstSet->getNativeObject(),
			copyDescSet.dstBinding,
			copyDescSet.dstArrayElement,
			copyDescSet.descriptorCount
		};
	});

	vk::UpdateDescriptorSets(getNativeObject(), (uint32_t)vkWriteDescSets.size(), vkWriteDescSets.data(),
	                         (uint32_t)vkCopyDescriptorSets.size(), vkCopyDescriptorSets.data());
}

namespace {
inline VkImageViewType convertToVkImageViewType(VkImageType baseType, uint32_t numArrayLayers, bool isCubeMap)
{
	// if it is a cube map it has to be 2D Texture base
	if (isCubeMap && baseType != VkImageType::e_2D)
	{
		assertion(baseType == VkImageType::e_2D, "Cubemap texture must be 2D");
		return VkImageViewType::e_MAX_ENUM;
	}
	// array must be atleast 1
	if (!numArrayLayers)
	{
		assertion(false, "Number of array layers must be greater than equal to 0");
		return VkImageViewType::e_MAX_ENUM;
	}
	// if it is array it must be 1D or 2D texture base
	if ((numArrayLayers > 1) && (baseType > VkImageType::e_2D))
	{
		assertion(false, "1D and 2D image type supports array texture");
		return VkImageViewType::e_MAX_ENUM;
	}

	VkImageViewType vkType[] =
	{
		VkImageViewType::e_1D,
		VkImageViewType::e_1D_ARRAY,
		VkImageViewType::e_2D,
		VkImageViewType::e_2D_ARRAY,
		VkImageViewType::e_3D,
		VkImageViewType::e_CUBE,
		VkImageViewType::e_CUBE_ARRAY
	};
	if (isCubeMap)
	{
		numArrayLayers = (numArrayLayers > 6) * 6;
	}
	return vkType[(static_cast<uint32_t>(baseType) * 2) + (isCubeMap ? 3 : 0) + (numArrayLayers > 1 ? 1 : 0)];
}
}

inline VkImageAspectFlags formatToImageAspect(VkFormat format)
{
	if (format < VkFormat::e_D16_UNORM || format > VkFormat::e_D32_SFLOAT_S8_UINT)
	{
		return VkImageAspectFlags::e_COLOR_BIT;
	}
	const VkImageAspectFlags formats[] =
	{
		VkImageAspectFlags::e_DEPTH_BIT, //VkFormat::e_D16_UNORM
		VkImageAspectFlags::e_DEPTH_BIT, //VkFormat::e_X8_D24_UNORM_PACK32
		VkImageAspectFlags::e_DEPTH_BIT, //VkFormat::e_D32_SFLOAT
		VkImageAspectFlags::e_STENCIL_BIT, //VkFormat::e_S8_UINT
		VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT, //VkFormat::e_D16_UNORM_S8_UINT
		VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT, //VkFormat::e_D24_UNORM_S8_UINT
		VkImageAspectFlags::e_DEPTH_BIT | VkImageAspectFlags::e_STENCIL_BIT, //VkFormat::e_D32_SFLOAT_S8_UINT
	};
	return formats[static_cast<uint32_t>(format) - static_cast<uint32_t>(VkFormat::e_D16_UNORM)];
}

ImageView Device_::createImageView(const Image& image, VkImageViewType viewType,
                                   VkFormat format, const ImageSubresourceRange& range, const ComponentMapping& swizzleChannels)
{
	ImageView imageView; imageView.construct();
	if (!imageView->init(image, viewType, format, range, swizzleChannels))
	{
		Log("Failed to create ImageView");
		imageView.reset();
	}
	return imageView;
}

ImageView Device_::createImageView(const Image& image,
                                   const ComponentMapping& swizzleChannels)
{
	ImageSubresourceRange range;

	range.aspectMask = formatToImageAspect(image->getFormat());
	range.levelCount = image->getNumMipMapLevels();
	range.layerCount = image->getNumArrayLayers();
	return createImageView(image, convertToVkImageViewType(image->getImageType(),
	                       image->getNumArrayLayers(), image->isCubeMap()), image->getFormat(), range, swizzleChannels);
}


Framebuffer Device_::createFramebuffer(const FramebufferCreateInfo& desc)
{
	Framebuffer framebuffer;
	// create framebuffer
	framebuffer.construct(getWeakReference());
	if (!framebuffer->init(desc))
	{
		Log("Failed to create Framebuffer");
		framebuffer.reset();
	}
	return framebuffer;
}

Fence Device_::createFence(VkFenceCreateFlags fenceCreateFlags)
{
	Fence fence;
	fence.construct(getWeakReference());
	if (!fence->init(fenceCreateFlags))
	{
		Log("Failed to create Fence");
		fence.reset();
	}
	return fence;
}

Semaphore Device_::createSemaphore()
{
	Semaphore semaphore;
	semaphore.construct(getWeakReference());
	if (!semaphore->init())
	{
		Log("Failed to create Semaphore");
		semaphore.reset();
	}
	return semaphore;
}

Buffer Device_::createBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VkBufferCreateFlags bufferCreateFlags,
                             bool sharingExclusive, const uint32_t* queueFamilyIndices, uint32_t numQueueFamilyIndices)
{
	Buffer buffer; buffer.construct(getWeakReference());
	if (!buffer->init(size, bufferUsage, bufferCreateFlags, sharingExclusive, queueFamilyIndices, numQueueFamilyIndices))
	{
		buffer.reset();
		Log("Failed to create buffer");
		return buffer;
	}

	return buffer;
}

DeviceMemory Device_::allocateMemory(VkDeviceSize size, uint32_t allowedMemoryBits, VkMemoryPropertyFlags memoryProps)
{
	DeviceMemoryImpl mem; mem.construct(getWeakReference());
	if (!mem->init(size, allowedMemoryBits, memoryProps))
	{
		Log("Failed to create memory block");
		mem.reset();
	}
	return mem;
}

Shader Device_::createShader(const std::vector<uint32_t>& shaderSrc)
{
	Shader vs; vs.construct(getWeakReference());
	if (!vs->init(shaderSrc))
	{
		Log(LogLevel::Error, "Failed to create VertexShader.");
		vs.reset();
	}
	return vs;
}

Sampler Device_::createSampler(const SamplerCreateInfo& desc)
{
	Sampler sampler;
	sampler.construct(getWeakReference());
	if (!sampler->init(desc))
	{
		sampler.reset();
		Log("failed to create Sampler object");
	}
	return sampler;
}


RenderPass Device_::createRenderPass(const RenderPassCreateInfo& renderPass)
{
	RenderPass rp; rp.construct(getReference());
	if (!rp->init(renderPass))
	{
		Log("Failed to create thes Renderpass");
		rp.reset();
	}
	return rp;
}

BufferView Device_::createBufferView(const Buffer& buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range)
{
	BufferView bufferview;
	bufferview.construct(getWeakReference(), buffer, format, offset, std::min(range, buffer->getSize() - offset));
	assertion(range == 0xFFFFFFFFu || (range <= buffer->getSize() - offset));
	return bufferview;
}

DescriptorPool Device_::createDescriptorPool(const DescriptorPoolCreateInfo& createInfo)
{
	DescriptorPool descPool = impl::DescriptorPool_::createNew(getWeakReference());
	if (!descPool->init(createInfo))
	{
		descPool.reset();
		Log("Failed to create DescriptorPool");
	}
	return descPool;
}

CommandPool Device_::createCommandPool(uint32_t queueFamilyId, VkCommandPoolCreateFlags createFlags)
{
	CommandPool cmdpool = impl::CommandPool_::createNew(getWeakReference());

	if (!cmdpool->init(queueFamilyId, createFlags))
	{
		cmdpool.reset();
		Log("Failed to create CommandPool");
	}
	return cmdpool;
}

PipelineLayout Device_::createPipelineLayout(const PipelineLayoutCreateInfo& desc)
{
	PipelineLayout pipelayout;
	pipelayout.construct(getWeakReference());
	if (!pipelayout->init(desc))
	{
		pipelayout.reset();
	}
	return pipelayout;
}

bool Device_::waitForFences(const uint32_t numFences,
                            const Fence* const fences, const bool waitAll, const uint64_t timeout)
{
	std::vector<VkFence> vkFences;
	vkFences.reserve(numFences);
	for (uint32_t i = 0; i < numFences; i++)
	{
		vkFences.emplace_back(fences[i]->getNativeObject());
	}

	if (vk::WaitForFences(_device, numFences, vkFences.data(), waitAll, timeout) == VkResult::e_SUCCESS)
	{
		return true;
	}

	return false;
}

bool Device_::resetFences(const uint32_t numFences, const Fence* const fences)
{
	std::vector<VkFence> vkFences;
	vkFences.reserve(numFences);
	for (uint32_t i = 0; i < numFences; i++)
	{
		vkFences.emplace_back(fences[i]->getNativeObject());
	}

	if (vk::ResetFences(_device, numFences, vkFences.data()) == VkResult::e_SUCCESS)
	{
		return true;
	}

	return false;
}

pvrvk::DescriptorSetLayout Device_::createDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& desc)
{
	DescriptorSetLayout layout;
	layout.construct(getWeakReference());
	if (!layout->init(desc))
	{
		layout.reset();
		Log("Failed to create DescriptorSetLayout");
	}
	return layout;
}

PipelineCache Device_::createPipelineCache(size_t initialDataSize, const void* initialData, VkPipelineCacheCreateFlags flags)
{
	PipelineCache pipeCache;
	pipeCache.construct(getWeakReference());
	if (pipeCache->init(initialDataSize, initialData, flags) != VkResult::e_SUCCESS)
	{
		pipeCache.reset();
		Log("Failed to create PipelineCache");
	}
	return pipeCache;
}

VkResult Device_::mergePipelineCache(const PipelineCache* srcPipeCaches,
                                     uint32_t numSrcPipeCaches, PipelineCache destPipeCache)
{
	std::vector<VkPipelineCache> vkSrcPipeCaches(numSrcPipeCaches);
	std::transform(srcPipeCaches, srcPipeCaches + numSrcPipeCaches, vkSrcPipeCaches.begin(),
	               [&](const PipelineCache & pipelineCache)
	{
		return pipelineCache->getNativeObject();
	});

	return vk::MergePipelineCaches(getNativeObject(), destPipeCache->getNativeObject(), numSrcPipeCaches,
	                               vkSrcPipeCaches.data());
}


Swapchain Device_::createSwapchain(
  const SwapchainCreateInfo& createInfo, const Surface& surface)
{
	Swapchain swapchain;
	swapchain.construct(getWeakReference());
	if (!swapchain->init(surface, createInfo))
	{
		Log("Failed to create Swapchain");
		swapchain.reset();
	}
	return swapchain;
}

void Device_::waitIdle()
{
	vk::DeviceWaitIdle(_device);
}

// CAUTION - We will be abusing queueFamilyProperties[...].numQueues as a counter for queues remaining.
struct QueueFamilyCreateInfo
{
	uint32_t queueFamilyId;
	uint32_t queueId;
	bool supportPresentation;
};

std::vector<VkDeviceQueueCreateInfo> logQueueFamilies(
  const PhysicalDeviceWeakPtr physicalDevice,
  const DeviceQueueCreateInfo* queueRequests,
  uint32_t numQueueCreateInfos)
{
	static const float priority[] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
	std::map<int, int> queueCounts;

	std::vector<QueueFamilyProperties> queueFamilyProperties = physicalDevice->getQueueFamilyProperties();
	uint32_t numQueueFamilies = static_cast<uint32_t>(queueFamilyProperties.size());

	std::vector<VkBool32> presentationQueueFamily;
	physicalDevice->getPresentationQueueFamily(physicalDevice->getInstance()->getSurface(),
	    presentationQueueFamily);

	std::vector<VkDeviceQueueCreateInfo> vkInfos(
	  numQueueCreateInfos,
	  VkDeviceQueueCreateInfo
	{
		VkStructureType::e_DEVICE_QUEUE_CREATE_INFO,
		nullptr,
		0,
		0,
		0,
		0
	});

	const char* graphics        = "GRAPHICS ";
	const char* compute         = "COMPUTE ";
	const char* present         = "PRESENT ";
	const char* transfer        = "TRANSFER ";
	const char* sparse          = "SPARSE_BINDING ";
	const char* nothing         = "";

	// Log the supported queue families
	Log(LogLevel::Information, "Supported Queue Families:");

	for (uint32_t i = 0; i < numQueueFamilies; ++i)
	{
		queueCounts[i] = 0;

		Log(LogLevel::Information, "\tqueue family %d (#queues %d)  FLAGS: %d ( %s%s%s%s%s%s)",
		    i, queueFamilyProperties[i].numQueues, queueFamilyProperties[i].queueFlags,
		    ((queueFamilyProperties[i].queueFlags & VkQueueFlags::e_GRAPHICS_BIT) != 0) ? graphics : nothing,
		    ((queueFamilyProperties[i].queueFlags & VkQueueFlags::e_COMPUTE_BIT) != 0) ? compute : nothing,
		    (presentationQueueFamily[i] == VK_TRUE) ? present : nothing,
		    ((queueFamilyProperties[i].queueFlags & VkQueueFlags::e_TRANSFER_BIT) != 0) ? transfer : nothing,
		    ((queueFamilyProperties[i].queueFlags & VkQueueFlags::e_SPARSE_BINDING_BIT) != 0) ? sparse : nothing,
		    nothing, nothing
		   );
	}

	// FIND AND LOG REQESTED QUEUES
	Log(LogLevel::Information, "Queues Created:");
	for (uint32_t i = 0; i < numQueueCreateInfos; ++i)
	{
		vkInfos[i].queueCount = queueRequests[i].queueCount;
		vkInfos[i].queueFamilyIndex = queueRequests[i].queueFamilyIndex;
		vkInfos[i].pQueuePriorities = queueRequests[i].queuePriorities;

		Log(LogLevel::Information,
		    "\t queue Family: %d ( %s%s%s%s%s) \tqueue count: %d",
		    vkInfos[i].queueFamilyIndex,
		    ((queueFamilyProperties[vkInfos[i].queueFamilyIndex].queueFlags & VkQueueFlags::e_GRAPHICS_BIT) != 0) ? graphics : nothing,
		    ((queueFamilyProperties[vkInfos[i].queueFamilyIndex].queueFlags & VkQueueFlags::e_COMPUTE_BIT) != 0) ? compute : nothing,
		    ((queueFamilyProperties[vkInfos[i].queueFamilyIndex].queueFlags & VkQueueFlags::e_TRANSFER_BIT) != 0) ? transfer : nothing,
		    ((queueFamilyProperties[vkInfos[i].queueFamilyIndex].queueFlags & VkQueueFlags::e_SPARSE_BINDING_BIT) != 0) ? sparse : nothing,
		    (presentationQueueFamily[vkInfos[i].queueFamilyIndex] ? present : nothing),
		    vkInfos[i].queueCount);
	}
	return vkInfos;
}

bool Device_::init(const DeviceCreateInfo& createInfo)
{
	_createInfo = createInfo;

	debug_assertion(_physicalDevice->getQueueFamilyProperties().size() >= (size_t)1, "A Vulkan device must support at least 1 queue family.");

	const std::vector<VkDeviceQueueCreateInfo> vkQueueCreateInfos =
	  logQueueFamilies(getPhysicalDevice(), _createInfo.queueCreateInfos.data(),
	                   static_cast<uint32_t>(_createInfo.queueCreateInfos.size()));

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VkStructureType::e_DEVICE_CREATE_INFO;
	deviceCreateInfo.flags = _createInfo.flags;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(vkQueueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = vkQueueCreateInfos.data();

	deviceCreateInfo.pEnabledFeatures = static_cast<const VkPhysicalDeviceFeatures*>(_createInfo.enabledFeatures);

	// Extensions
	std::vector<const char*> deviceExtensionsVk;

	if (_createInfo.enabledExtensionNames.size())
	{
		uint32_t numRequestedDeviceExtensions = static_cast<uint32_t>(_createInfo.enabledExtensionNames.size());
		_createInfo.enabledExtensionNames = Extensions::filterExtensions(getPhysicalDevice()->enumerateDeviceExtensionsProperties(),
		                                    _createInfo.enabledExtensionNames.data(), static_cast<uint32_t>(_createInfo.enabledExtensionNames.size()));
		if (_createInfo.enabledExtensionNames.size() != numRequestedDeviceExtensions)
		{
			Log(LogLevel::Warning, "Device: Not all requested extensions are supported");
		}

		deviceExtensionsVk.resize(_createInfo.enabledExtensionNames.size());
		std::transform(_createInfo.enabledExtensionNames.begin(), _createInfo.enabledExtensionNames.end(),
		deviceExtensionsVk.begin(), [&](const std::string & str) { return str.c_str();});

		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionsVk.data();
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsVk.size());
	}

	std::vector<const char*> deviceLayersVk(_createInfo.enabledLayerNames.size());
	{
		std::transform(_createInfo.enabledLayerNames.begin(), _createInfo.enabledLayerNames.end(),
		deviceLayersVk.begin(), [&](const std::string & str) { return str.c_str(); });

		deviceCreateInfo.ppEnabledLayerNames = deviceLayersVk.data();
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(deviceLayersVk.size());
	}

	if (!vkIsSuccessful(vk::CreateDevice(_physicalDevice->getNativeObject(), &deviceCreateInfo, nullptr,
	                                     &_device), "Vulkan Device Creation"))
	{
		return false;
	}

	// LOG ALL
	Log(LogLevel::Information, "Logical Device Configurations");
	Log(LogLevel::Information, "\tExtensions: %d", _createInfo.enabledExtensionNames.size());
	for (uint32_t  i = 0; i < _createInfo.enabledExtensionNames.size(); ++i)
	{
		Log(LogLevel::Information, "\t\t%s", _createInfo.enabledExtensionNames[i].c_str());
	}

	Log(LogLevel::Information, "\tLayers: %d", _createInfo.enabledLayerNames.size());
	for (uint32_t  i = 0; i < _createInfo.enabledLayerNames.size(); ++i)
	{
		Log(LogLevel::Information, "\t\t%s", _createInfo.enabledLayerNames[i].c_str());
	}

	// CHECK PVRTC SUPPORT
	{
		_supportsPVRTC = isExtensionEnabled("VK_IMG_format_pvrtc");
	}


	vk::initVulkanDevice(_device);
	vk::initVk(getPhysicalDevice()->getInstance()->getNativeObject(), _device);
	const std::vector<QueueFamilyProperties>& queueFamProps = _physicalDevice->getQueueFamilyProperties();

	std::vector<VkBool32> presentationQueueFamily;
	getPhysicalDevice()->getPresentationQueueFamily(getPhysicalDevice()->getInstance()->getSurface(), presentationQueueFamily);

	uint32_t queueFamilyId;
	uint32_t queueId;
	for (uint32_t i = 0; i < _createInfo.queueCreateInfos.size(); ++i)
	{
		queueFamilyId = vkQueueCreateInfos[i].queueFamilyIndex;
		_queues.push_back(QueueFamily());
		_queues.back().queueFamily = queueFamilyId;
		_queues.back().queues.resize(vkQueueCreateInfos[i].queueCount);
		VkQueue vkQueue;
		for (queueId = 0; queueId < vkQueueCreateInfos[i].queueCount; ++queueId)
		{
			vk::GetDeviceQueue(_device, queueFamilyId, queueId, &vkQueue);
			_queues.back().queues[queueId].construct(getWeakReference(), vkQueue,
			    queueFamProps[queueFamilyId].queueFlags, queueFamilyId, presentationQueueFamily[queueFamilyId] != 0);
		}
	}

	_initialized = true;

	return true;
}
}// namespace impl
}// namespace pvrvk


//!\endcond
