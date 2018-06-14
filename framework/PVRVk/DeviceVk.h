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
#include "PVRVk/ObjectHandleVk.h"
#include "PVRVk/DebugMarkerVk.h"
#include <map>
#include <set>
#include <stdlib.h>
#include <bitset>

namespace pvrvk {

/// <summary>Contains functions and methods related to the wiring of the PVRVk library to the underlying platform,
/// including extensions and the Context classes.</summary>
struct SamplerCreateInfo;
namespace impl {
//\cond NO_DOXYGEN
inline void reportDestroyedAfterDevice(const char* objectName)
{
	Log(LogLevel::Warning, "Attempted to destroy object of type [%s] after its corresponding device", objectName);
}
//\endcond

/// <summary>GpuDevice implementation that supports Vulkan</summary>
class Device_ : public PhysicalDeviceObjectHandle<VkDevice>, public EmbeddedRefCount<Device_>
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
	/// <param name="pipelineCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that
	/// cache is enabled for the duration of the command.</param>
	ComputePipeline createComputePipeline(const ComputePipelineCreateInfo& createInfo, const PipelineCache& pipelineCache = PipelineCache());

	/// <summary>create array of compute pipelines</summary>
	/// <param name="createInfo">Compute pipeline create Infos</param>
	/// <param name="numCreateInfos">Number of compute pipleine to create</param>
	/// <param name="outPipelines">Out pipelines</param>
	/// <param name="pipelineCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that
	/// cache is enabled for the duration of the command.</param>
	void createComputePipelines(const ComputePipelineCreateInfo* createInfo, uint32_t numCreateInfos, const PipelineCache& pipelineCache, ComputePipeline* outPipelines);

	/// <summary>create graphicsPipeline</summary>
	/// <param name="createInfo">Pipeline create info</param>
	/// <param name="pipelineCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that
	/// cache is enabled for the duration of the command.</param> <returns>Return a valid pipeline on success</returns>.
	GraphicsPipeline createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo, const PipelineCache& pipelineCache = PipelineCache());

	/// <summary>create array of graphics pipelines</summary>
	/// <param name="createInfos">Pipeline create infos</param>
	/// <param name="numCreateInfos">Number of pipeline to create</param>
	/// <param name="outPipelines">Out pipeline</param>
	/// <param name="pipelineCache">Either null handle, indicating that pipeline caching is disabled; or the handle of a valid pipeline cache object, in which case use of that
	/// cache is enabled for the duration of the command.</param> <returns>return true if success</returns>.
	void createGraphicsPipelines(const GraphicsPipelineCreateInfo* createInfos, uint32_t numCreateInfos, const PipelineCache& pipelineCache, GraphicsPipeline* outPipelines);

	/// <summary>Create sampler object</summary>
	/// <param name="createInfo">Sampler Create info</param>
	/// <returns> Return a valid sampler object on success</returns>.
	Sampler createSampler(const SamplerCreateInfo& createInfo);

	/// <summary>create an image using this device.</summary>
	/// <param name="createInfo">The image creation descriptor</param>
	/// <returns> The created Image object on success, null Image on failure</returns>
	Image createImage(const ImageCreateInfo& createInfo);

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
	ImageView createImageView(
		const Image& image, pvrvk::ImageViewType viewType, pvrvk::Format format, const ImageSubresourceRange& range, const ComponentMapping& swizzleChannels = ComponentMapping());

	/// <summary>Create buffer view</summary>
	/// <param name="buffer">buffer object</param>
	/// <param name="format">buffer format</param>
	/// <param name="offset">view offset</param>
	/// <param name="range"> view range</param>
	/// <returns> Return valid object if success</returns>
	BufferView createBufferView(const Buffer& buffer, pvrvk::Format format, VkDeviceSize offset, VkDeviceSize range);

	/// <summary>Create a new buffer object and (optionally) allocate and bind memory for it</summary>
	/// <param name="createInfo">The buffer creation descriptor</param>
	/// <returns>Return a valid object if success</returns>.
	Buffer createBuffer(const BufferCreateInfo& createInfo);

	/// <summary>Create device memory block</summary>
	/// <param name="allocationInfo">memory allocation info</param>
	/// <returns>Return a valid object if success</returns>.
	DeviceMemory allocateMemory(const MemoryAllocationInfo& allocationInfo);

	/// <summary>Create Shader Object</summary>
	/// <param name="shaderSrc">Shader source data</param>
	/// <returns> Return a valid shader if success</returns>
	ShaderModule createShader(const std::vector<uint32_t>& shaderSrc);

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
	/// <param name="initialDataSize"> initialDataSize is the number of bytes in pInitialData. If initialDataSize is zero, the pipeline cache will initially be empty.</param>
	/// <param name="initialData">initialData is a pointer to previously retrieved pipeline cache data.</param>
	/// <param name="flags"> flags is reserved for future use</param>
	/// <returns>Return a valid Pipeline cache object which will be initially empty. If initialDataSize is zero, initialData is ignored.</returns>
	PipelineCache createPipelineCache(size_t initialDataSize = 0, const void* initialData = nullptr, pvrvk::PipelineCacheCreateFlags flags = pvrvk::PipelineCacheCreateFlags::e_NONE);

	/// <summary>Merge PipelineCache objects</summary>
	/// <param name="srcPipeCaches">Pipeline caches, which will be merged into destPipeCache</param>
	/// <param name="numSrcPipeCaches">Number of source pipeline caches to be merged</param>
	/// <param name="destPipeCache">Pipeline cache to merge results into. The previous contents of destPipeCache are included after the merge</param>
	/// <returns>Return result</returns>.
	void mergePipelineCache(const PipelineCache* srcPipeCaches, uint32_t numSrcPipeCaches, PipelineCache destPipeCache);

	/// <summary>Create pipeline layout</summary>
	/// <param name="createInfo">Pipeline layout create info</param>
	/// <returns>Return a valid object if success</returns>.
	PipelineLayout createPipelineLayout(const PipelineLayoutCreateInfo& createInfo);

	/// <summary>Wait this device for an array of fences </summary>
	/// <param name="numFences">Number of fence to wait</param>
	/// <param name="fences">Fences to wait for</param>
	/// <param name="waitAll">Wait for all fence if flags set to true</param>
	/// <param name="timeout">Wait timeout</param>
	/// <returns>Return true if fence waits successfull, false if timed out.</returns>.
	bool waitForFences(uint32_t numFences, const Fence* fences, const bool waitAll, const uint64_t timeout);

	/// <summary>Reset an array of fences</summary>
	/// <param name="numFences"> Number of fence to reset</param>
	/// <param name="fences">Fence to reset</param>
	void resetFences(uint32_t numFences, const Fence* fences);

	/// <summary> Create commandpool</summary>
	/// <param name="queueFamilyId">All commandbuffer created from this commandpool must be submitted ti the queue </param>
	///  with the same queue family id.
	/// <param name="createFlags">Create flags</param>
	/// <returns>Return a valid object if success</returns>.
	CommandPool createCommandPool(uint32_t queueFamilyId, pvrvk::CommandPoolCreateFlags createFlags);

	/// <summary>Create Fence</summary>
	/// <param name="fenceCreateFlags">Fence create flags</param>
	/// <returns>Return a valid object if success</returns>.
	Fence createFence(pvrvk::FenceCreateFlags fenceCreateFlags = pvrvk::FenceCreateFlags(0));

	/// <summary>Create Event</summary>
	/// <returns>Return a valid object if success</returns>.
	Event createEvent();

	/// <summary>Create semaphore</summary>
	/// <returns>Return a valid object if success</returns>.
	Semaphore createSemaphore();

	/// <summary>Create QueryPool</summary>
	/// <param name="queryType">Specifies the type of queries managed by the pool</param>
	/// <param name="queryCount">The number of queries managed by the pool</param>
	/// <param name="statisticsFlags">Specifies which counters will be returned in queries on the new pool</param>
	/// <returns>return a valid object if success</returns>.
	QueryPool createQueryPool(pvrvk::QueryType queryType, uint32_t queryCount, pvrvk::QueryPipelineStatisticFlags statisticsFlags = pvrvk::QueryPipelineStatisticFlags(0));

	/// <summary>Return true if this device support PVRTC image</summary>
	/// <returns>Return true if supported</returns>
	bool supportsPVRTC() const
	{
		return _supportsPVRTC;
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
		for (uint32_t i = 0; i < _queueFamilies.size(); ++i)
		{
			if (_queueFamilies[i].queueFamily == queueFamily)
			{
				return _queueFamilies[i].queues[queueId];
			}
		}
		throw ErrorValidationFailedEXT("Request for queue from family id that did not exist.");
	}

	/// <summary> Return true if the given extension is enabled (const).</summary>
	/// <param name="extension"></param>
	/// <returns></returns>
	bool isExtensionEnabled(const char* extension) const
	{
		const auto& it = std::find_if(_createInfo.getEnabledExtensionNames().begin(), _createInfo.getEnabledExtensionNames().end(),
			[&](const std::string& str) { return strcmp(str.c_str(), extension) == 0; });
		return it != _createInfo.getEnabledExtensionNames().end();
	}

	/// <summary>Get all the enabled extensions</summary>
	/// <returns>Extension strings</returns>
	const std::vector<std::string>& getEnabledExtensions() const
	{
		return _createInfo.getEnabledExtensionNames();
	}

	/// <summary>Update Descriptorsets</summary>
	/// <param name="writeDescSets">Write descriptor sets</param>
	/// <param name="numWriteDescSets">Number of write Descriptor sets</param>
	/// <param name="copyDescSets">Copy operation happens after the Write operation.</param>
	/// <param name="numCopyDescSets">Number of copy descriptor sets</param>
	void updateDescriptorSets(const WriteDescriptorSet* writeDescSets, uint32_t numWriteDescSets, const CopyDescriptorSet* copyDescSets, uint32_t numCopyDescSets);

	/// <summary>Gets the device dispatch table</summary>
	/// <returns>The device dispatch table</returns>
	inline const VkDeviceBindings& getVkBindings() const
	{
		return _vkBindings;
	}

private:
	friend class Swapchain_;
	friend class ::pvrvk::impl::PhysicalDevice_;
	friend class ::pvrvk::EmbeddedRefCount<Device_>;

	void destroyObject()
	{
		_queueFamilies.clear();
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			_vkBindings.vkDestroyDevice(getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
	}

	Device_(PhysicalDeviceWeakPtr physicalDevice, const DeviceCreateInfo& createInfo);

	struct QueueFamily
	{
		uint32_t queueFamily;
		std::vector<Queue> queues;
	};

	std::vector<QueueFamily> _queueFamilies;
	bool _supportsPVRTC;
	DeviceCreateInfo _createInfo;
	VkDeviceBindings _vkBindings;
};
} // namespace impl
} // namespace pvrvk
