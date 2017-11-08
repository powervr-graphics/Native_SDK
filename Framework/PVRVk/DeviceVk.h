/*!
\brief The PVRVk Device class. One of the bysiest classes in Vulkan, together with the Command Buffer.
\file PVRVk/DeviceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/ExtensionsVk.h"
#include "PVRVk/LayersVk.h"
#include "PVRVk/BindingsVk.h"
#include "PVRVk/ErrorsVk.h"
#include <map>
#include <set>
#include <stdlib.h>
#include <bitset>

namespace pvrvk {
namespace platform {
class DisplayAttributes;
}

/// <summary>Contains functions and methods related to the wiring of the PVRVk library to the underlying platform,
/// including extensions and the Context classes.</summary>
struct SamplerCreateInfo;
namespace impl {
//\cond NO_DOXYGEN
inline void reportDestroyedAfterContext(const char* objectName)
{
	Log(LogLevel::Warning, "Attempted to destroy object of type [%s] after its corresponding context", objectName);
#ifdef DEBUG
#endif
}
//\endcond

/// <summary>GpuDevice implementation that supports Vulkan</summary>
class Device_ : public EmbeddedRefCount<Device_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(Device_)

	/// <summary> Wait on the host for the completion of outstanding queue operations
	/// for all queues on this device This is equivalent to calling waitIdle for all
	/// queues owned by this device.</summary>
	void waitIdle();

	/// <summary>createComputePipeline</summary>
	/// <param name="createInfo">create info</param>
	/// <returns>Return a valid compute pipeline on success</returns>
	/// <param name="pipeCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that cache is enabled for the duration of the command.</param>
	ComputePipeline createComputePipeline(const ComputePipelineCreateInfo& createInfo, const PipelineCache& pipeCache = PipelineCache());

	/// <summary>create array of compute pipelines</summary>
	/// <param name="createInfo">Compute pipeline create Infos</param>
	/// <param name="numCreateInfos">Number of compute pipleine to create</param>
	/// <param name="outPipelines">Out pipelines</param>
	/// <param name="pipeCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that cache is enabled for the duration of the command.</param>
	/// <returns>Return true on success</returns>.
	bool createComputePipelines(const ComputePipelineCreateInfo* createInfo,
	                            uint32_t numCreateInfos, const PipelineCache& pipeCache, ComputePipeline* outPipelines);

	/// <summary>create graphicsPipeline</summary>
	/// <param name="createInfo">Pipeline create info</param>
	/// <param name="pipeCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that cache is enabled for the duration of the command.</param>
	/// <returns>Return a valid pipeline on success</returns>.
	GraphicsPipeline createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo,
	                                        const PipelineCache& pipeCache = PipelineCache());

	/// <summary>create array of graphics pipelines</summary>
	/// <param name="createInfos">Pipeline create infos</param>
	/// <param name="numCreateInfos">Number of pipeline to create</param>
	/// <param name="outPipelines">Out pipeline</param>
	/// <param name="pipeCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that cache is enabled for the duration of the command.</param>
	/// <returns>return true if success</returns>.
	bool createGraphicsPipelines(const GraphicsPipelineCreateInfo* createInfos, uint32_t numCreateInfos,
	                             const PipelineCache& pipeCache, GraphicsPipeline* outPipelines);

	/// <summary>Create sampler object</summary>
	/// <param name="createInfo">Sampler Create info</param>
	/// <returns> Return a valid sampler object on success</returns>.
	Sampler createSampler(const SamplerCreateInfo& createInfo);

	/// <summary>create 3d Image(sparse or with memory backing, depending on <paramref name="flags"/>.
	/// User should not call bindMemory on the image if sparse flags are used.
	/// <paramref name="allocMemFlags"/> is ignored if <paramref name="flags"/> contains
	/// a sparse binding flag.)</summary>
	/// <param name="imageType">The type of the created image (1D/2D/3D etc)</param>
	/// <param name="format">The Image format</param>
	/// <param name="dimension">Image dimension</param>
	/// <param name="usage">Image usage flags</param>
	/// <param name="flags">Image create flags</param>
	/// <param name="layerSize">Image layer size</param>
	/// <param name="samples">NUmber of samples</param>
	/// <param name="sharingExclusive">Specifying the sharing mode of this image.
	/// Setting it true means that only a single queue can access it therefore
	/// the calle must exclusively transfer queue ownership of this image if they want separate queue family to access this image
	/// and the <paramref name="queueFamilyIndices"/>, <paramref name="numQueueFamilyIndices"/> is ignored.
	/// Setting it to false means multiple queue family can access this image at any point in time and requires
	/// <paramref name="queueFamilyIndices"/>, <paramref name="numQueueFamilyIndices"/></param>
	/// <param name="queueFamilyIndices">A c-style array containing the queue family indices that
	/// this image is exclusive to</param>
	/// <param name="numQueueFamilyIndices">The number of queues in <paramref name="queueFamilyIndices"></param>
	/// <returns> The created Imageobject on success, null Image on failure</returns>
	Image createImage(
	  VkImageType imageType, VkFormat format,
	  const Extent3D& dimension, VkImageUsageFlags usage,
	  VkImageCreateFlags flags = VkImageCreateFlags(0),
	  const ImageLayersSize& layerSize = ImageLayersSize(),
	  VkSampleCountFlags samples = VkSampleCountFlags::e_1_BIT,
	  bool sharingExclusive = true,
	  const uint32_t* queueFamilyIndices = nullptr,
	  uint32_t numQueueFamilyIndices = 0);

	/// <summary> Create image view object. NOTE: for a non sparse image, a valid memory object must
	/// have bound on the image</summary>
	/// <param name="image">The image to use for creating the image view</param>
	/// <param name="swizzleChannels">The channels to swizzle, default Identity</param>
	/// <returns> The created ImageView object on success, null ImageView on failure</returns>
	ImageView createImageView(const Image& image, const ComponentMapping& swizzleChannels = ComponentMapping());

	/// <summary>create Image view object
	/// NOTE: for non sparse image, a valid memory object must have bound on the image</summary>
	/// </summary>
	/// <param name="image">The image to use for creating the image view</param>
	/// <param name="viewType">The type of the image view</param>
	/// <param name="format">the Format must be the same as the Image format, Unless if the image is
	///   created with Mutable format flag</param>
	/// <param name="range">The sub resource image range</param>
	/// <param name="swizzleChannels">The channels to swizzle</param>
	/// <returns> The created ImageView object on success, null ImageView on failure</returns>
	ImageView createImageView(const Image& image, VkImageViewType viewType, VkFormat format,
	                          const ImageSubresourceRange& range,
	                          const ComponentMapping& swizzleChannels = ComponentMapping());

	/// <summary>Create buffer view</summary>
	/// <param name="buffer">buffer object</param>
	/// <param name="format">buffer format</param>
	/// <param name="offset">view offset</param>
	/// <param name="range"> view range</param>
	/// <returns> Return valid object if success</returns>
	BufferView createBufferView(const Buffer& buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range);

	/// <summary>Create a new buffer object and (optionally) allocate and bind memory for it</summary>
	/// <param name="size">The total size of the buffer</param>
	/// <param name="bufferUsage">All buffer usages for which this buffer will be valid</param>
	/// <param name="bufferCreateFlags">Buffer creation flags (see Vulkan spec)</param>
	/// <param name="sharingExclusive">indicates whether the buffer is exclusive for some queues or can be used simultaneously multiple queues</param>
	/// <param name="queueFamilyIndices">If not exclusive, indicates which queue families the buffer can be used by</param>
	/// <param name="numQueueFamilyIndices">If not exclusive, the number of queue families for which this buffer is valid</param>
	/// <returns>Return a valid object if success</returns>.
	Buffer createBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage,
	                    VkBufferCreateFlags bufferCreateFlags = VkBufferCreateFlags(0),
	                    bool sharingExclusive = true, const uint32_t* queueFamilyIndices = nullptr,
	                    uint32_t numQueueFamilyIndices = 0);

	/// <summary>Create device memory block</summary>
	/// <param name="size">memory size</param>
	/// <param name="allowedMemoryBits"> allowed memory bits</param>
	/// <param name="memoryProps">memory property flags</param>
	/// <returns>Return a valid object if success</returns>.
	DeviceMemory allocateMemory(VkDeviceSize size, uint32_t allowedMemoryBits, VkMemoryPropertyFlags memoryProps);


	/// <summary>Create Shader Object</summary>
	/// <param name="shaderSrc">Shader source data</param>
	/// <returns> Return a valid shader if success</returns>
	Shader createShader(const std::vector<uint32_t>& shaderSrc);

	/// <summary>createFramebuffer Create Framebuffer object</summary>
	/// <param name="createInfo">Framebuffer createInfo</param>
	/// <returns>return a valid object if success</returns>.
	Framebuffer createFramebuffer(const FramebufferCreateInfo& createInfo);

	/// <summary>create renderpass</summary>
	/// <param name="createInfo">Renderpass createInfo</param>
	/// <returns>return a valid object if success</returns>.
	RenderPass createRenderPass(const RenderPassCreateInfo& createInfo);

	/// <summary>Create DescriptorPool</summary>
	/// <param name="createInfo">DescriptorPool createInfo</param>
	/// <returns>return a valid object if success</returns>.
	DescriptorPool createDescriptorPool(const DescriptorPoolCreateInfo& createInfo);

	/// <summary>create Descriptor set layout</summary>
	/// <param name="createInfo"> Descriptor layout createInfo</param>
	/// <returns>Return a valid object if success</returns>.
	DescriptorSetLayout createDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo);

	/// <summary>Create PipelineCache object</summary>
	/// <param name="flags"> flags is reserved for future use</param>
	/// <param name="initialDataSize"> initialDataSize is the number of bytes in pInitialData. If initialDataSize is zero, the pipeline cache will initially be empty.</param>
	/// <param name="pInitialData"> initialData is a pointer to previously retrieved pipeline cache data. If the pipeline cache data is incompatible (as defined below) with the device, the pipeline cache will be initially empty. If initialDataSize is zero, pInitialData is ignored.</param>
	/// <returns>Return a valid object if success</returns>.
	PipelineCache createPipelineCache(size_t initialDataSize, const void* initialData, VkPipelineCacheCreateFlags flags);

	/// <summary>Merge PipelineCache objects</summary>
	/// <param name="srcPipeCaches">Pipeline caches, which will be merged into destPipeCache</param>
	/// <param name="numSrcPipeCaches">Number of source pipeline caches to be merged</param>
	/// <param name="destPipeCache">Pipeline cache to merge results into. The previous contents of destPipeCache are included after the merge</param>
	/// <returns>Return result</returns>.
	VkResult mergePipelineCache(const PipelineCache* srcPipeCaches, uint32_t  numSrcPipeCaches, PipelineCache destPipeCache);


	/// <summary>Create pipeline layout</summary>
	/// <param name="createInfo">Pipeline layout create info</param>
	/// <returns>Return a valid object if success</returns>.
	PipelineLayout createPipelineLayout(const PipelineLayoutCreateInfo& createInfo);

	/// <summary>Wait this device for an array of fences </summary>
	/// <param name="numFences">Number of fence to wait</param>
	/// <param name="fences">Fences to wait for</param>
	/// <param name="waitAll">Wait for all fence if flags set to true</param>
	/// <param name="timeout">Wait timeout</param>
	/// <returns>Return true if fence waits successfull</returns>.
	bool waitForFences(const uint32_t numFences, const Fence* fences, const bool waitAll, const uint64_t timeout);

	/// <summary>Reset an array of fences</summary>
	/// <param name="numFences"> Number of fence to reset</param>
	/// <param name="fences">Fence to reset</param>
	/// <returns>Return true if success</returns>.
	bool resetFences(const uint32_t numFences, const Fence* fences);

	/// <summary> Create commandpool</summary>
	/// <param name="queueFamilyId">All commandbuffer created from this commandpool must be submitted ti the queue </param>
	///  with the same queue family id.
	/// <param name="createFlags">Create flags</param>
	/// <returns>Return a valid object if success</returns>.
	CommandPool createCommandPool(uint32_t queueFamilyId, VkCommandPoolCreateFlags createFlags);

	/// <summary>brief Create Fence</summary>
	/// <param name="fenceCreateFlags">Fence create flags</param>
	/// <returns>Return a valid object if success</returns>.
	Fence createFence(VkFenceCreateFlags fenceCreateFlags = VkFenceCreateFlags(0));

	/// <summary>Create semaphore</summary>
	/// <returns>Return a valid object if success</returns>.
	Semaphore createSemaphore();

	/// <summary>Return true if this device support PVRTC image</summary>
	/// <returns>Return true if supported</returns>
	bool supportsPVRTC()const { return _supportsPVRTC; }

	/// <summary>Get the native handle to the context</summary>
	/// <returns>Return the context native handle</returns>
	const VkDevice& getNativeObject()const { return _device; }


	/// <summary>brief Get the physical device which this logical device was created from.(const)</summary>
	/// <returns>Return the physical device</returns>.
	const PhysicalDeviceWeakPtr getPhysicalDevice()const
	{
		return _physicalDevice;
	}


	/// <summary> Get the physical device which this logical device was created from.</summary>
	/// <returns>Return the physical device</returns>.
	PhysicalDeviceWeakPtr getPhysicalDevice()
	{
		return _physicalDevice;
	}

	/// <summary>Get List of enabled device extension.</summary>
	/// <returns>Device extensions</returns>
	const std::vector<std::string>& getEnabledDeviceExtensions()const
	{
		return _enabledDeviceExtensions;
	}

	/// <summary>Return true if the device extension is enabled.</summary>
	/// <param name="extensionName"></param>
	/// <returns></returns>
	bool isDeviceExtensionEnabled(const char* extensionName)
	{
		for (uint32_t i = 0; i < _enabledDeviceExtensions.size(); i++)
		{
			if (!strcmp(_enabledDeviceExtensions[i].c_str(), extensionName))
			{
				return true;
			}
		}

		return false;
	}

	/// <summary>Create Swapchain</summary>
	/// <param name="createInfo">Swapchain createInfo</param>
	/// <param name="surface">Swapchain's surface</param>
	/// <returns>Return a valid object if success</returns>.
	Swapchain createSwapchain(const SwapchainCreateInfo& createInfo, const Surface& surface);

	/// <summary>Get Queue</summary>
	/// <param name="queueFamily">Queue Family id</param>
	/// <param name="queueId">Queue Id</param>
	/// <returns>Return the queue</returns>
	Queue getQueue(uint32_t queueFamily, uint32_t queueId)
	{
		for (uint32_t i = 0; i < _queues.size(); ++i)
		{
			if (_queues[i].queueFamily == queueFamily)
			{
				return _queues[i].queues[queueId];
			}
		}
		return Queue();
	}


	/// <summary> Return true if the given extension is enabled (const).</summary>
	/// <param name="extension"></param>
	/// <returns></returns>
	bool isExtensionEnabled(const char* extension) const
	{
		const auto& it = std::find_if(_createInfo.enabledExtensionNames.begin(),
		                              _createInfo.enabledExtensionNames.end(),
		[&](const std::string & str) { return  strcmp(str.c_str(), extension) == 0;});
		return it != _createInfo.enabledExtensionNames.end();
	}

	/// <summary>Get all the enabled extensions</summary>
	/// <returns>Extension strings</returns>
	const std::vector<std::string>& getAllEnabledExtensions()const
	{
		return _createInfo.enabledExtensionNames;
	}

	/// <summary>Update Descriptorsets</summary>
	/// <param name="writeDescSets">Write descriptor sets</param>
	/// <param name="numWriteDescSets">Number of write Descriptor sets</param>
	/// <param name="copyDescSets">Copy operation happens after the Write operation.</param>
	/// <param name="numCopyDescSets">Number of copy descriptor sets</param>
	void updateDescriptorSets(const WriteDescriptorSet* writeDescSets,
	                          uint32_t numWriteDescSets, const CopyDescriptorSet* copyDescSets, uint32_t numCopyDescSets);

private:
	friend class Swapchain_;
	friend class ::pvrvk::impl::PhysicalDevice_;
	friend class ::pvrvk::EmbeddedRefCount<Device_>;
	struct QueueFamily
	{
		uint32_t queueFamily;
		std::vector<Queue> queues;
	};

	void destroyObject()
	{
		_queues.clear();
		_enabledDeviceExtensions.clear();
		_physicalDevice.reset();

		if (!_initialized) { return; }
		vk::DestroyDevice(_device, NULL);
		_initialized = false;
	}
	Device_(PhysicalDeviceWeakPtr physicalDevice) :
		_physicalDevice(physicalDevice), _initialized(false), _device(VK_NULL_HANDLE)
	{}

	bool init(const DeviceCreateInfo& createInfo);

	PhysicalDeviceWeakPtr _physicalDevice;
	bool _initialized;
	std::vector<std::string> _enabledDeviceExtensions;
	std::vector<QueueFamily> _queues;
	VkDevice _device;
	bool _supportsPVRTC;
	DeviceCreateInfo _createInfo;
};
}// namespace impl
}// namespace pvrvk
