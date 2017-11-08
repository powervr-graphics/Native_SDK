/*!
\brief The Prank Queue class
\file PVRVk/QueueVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {

/// <summary>Containes memory bind data for SparseImageMemoryBind and SparseBufferMemoryBindInfo.</summary>
struct SparseMemoryBind
{
	uint64_t                  resourceOffset;//!< offset in to the resource
	uint64_t                  size; //!< Size of the memory region to be bound
	DeviceMemory               memory;//!< is the DeviceMemory object that the range of the resource is bound to. If memory is null handle, the range is unbound
	uint64_t                  memoryOffset; //!< is the offset into the DeviceMemory object to bind the resource range to. If memory is null handle, this value is ignored
	VkSparseMemoryBindFlags   flags; //!< is a bitmask of VkSparseMemoryBindFlagBits specifying usage of the binding operation
};

/// <summary>Bind memory to sparse image which has been created with VkImageCreateFlags::e_SPARSE_BINDING_BIT flag</summary>
struct SparseImageMemoryBind
{
	ImageSubresource         subresource;//!< The aspectMask and region of interest in the image.
	pvrvk::Offset3D          offset;//!< Coordinates of the first texel within the image subresource to bind.
	pvrvk::Extent3D          extent;//!< the size in texels of the region within the image subresource to bind. The extent must be a multiple of the sparse image block dimensions, except when binding sparse image blocks along the edge of an image subresource it can instead be such that any coordinate of offset + extent equals the corresponding dimensions of the image subresource
	DeviceMemory              memory;//!< DeviceMemory object that the sparse image blocks of the image are bound to. If memory is null handle, the sparse image blocks are unbound.
	uint64_t                 memoryOffset;//!< Offset into VkDeviceMemory object. If memory is null handle, this value is ignored.
	VkSparseMemoryBindFlags  flags;//!< Sparse memory binding flags
};

/// <summary>Bind memory to sparse buffer object which has been created with VK_BUFFER_CREATE_SPARSE_BINDING_BIT flag</summary>
struct SparseBufferMemoryBindInfo
{
	Buffer buffer;//!< Buffer object to be bound
	std::vector<SparseMemoryBind> binds;//!< array of sparse memory binds
};

/// <summary>Bind memory to opaque regions of image objects created with the VK_IMAGE_CREATE_SPARSE_BINDING_BIT flag</summary>
struct SparseImageOpaqueMemoryBindInfo
{
	Image image;//!< Image object to be bound
	std::vector<SparseMemoryBind> binds;//!< array of sparse memory binds
};

/// <summary>Bind memory sparse image blocks of image objects created with the VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT flag</summary>
struct SparseImageMemoryBindInfo
{
	Image image;//!< Buffer object to be bound
	std::vector<SparseImageMemoryBind> binds;//!< array of sparse memory binds
};

/// <summary>BindSparseInfo specifying a sparse binding submission batch.</summary>
struct BindSparseInfo
{
	std::vector<SparseBufferMemoryBindInfo>         bufferBinds;//!< Array of SparseBufferMemoryBindInfo structures, indicating sparse buffer bindings to perform.
	std::vector<SparseImageOpaqueMemoryBindInfo>    imageOpaqueBinds;//!< Array of SparseImageOpaqueMemoryBindInfo structures, indicating opaque sparse image bindings to perform
	std::vector<SparseImageMemoryBindInfo>          imageBinds;//!< Array of SparseImageMemoryBindInfo structures, indicating sparse image bindings to perform
	std::vector<Semaphore>                          waitSemaphores;//!< Array of semaphores upon which to wait on before the sparse binding operations for this batch begin execution
	std::vector<Semaphore>                          signalSemaphore;//!< Array of semaphores which will be signaled when the sparse binding operations for this batch have completed execution
};

/// <summary>Queue submit info. Contains the commandbuffers to be summited to the queue.</summary>
struct SubmitInfo
{
	const VkPipelineStageFlags* waitDestStages;//!< pointer to an array of pipeline stages at which each corresponding semaphore wait will occur
	const CommandBuffer* commandBuffers; //!< Pointer to an array of command buffers to execute in the batch
	uint32_t numCommandBuffers;//!< The number of command buffers to execute in the batch
	const Semaphore* waitSemaphores; //!<  Pointer to an array of semaphores upon which to wait before the command buffers for this batch begin execution
	uint32_t numWaitSemaphores; //!<  The number of semaphores upon which to wait before executing the command buffers for the batch
	const Semaphore* signalSemaphores; //!< Pointer to an array of semaphores which will be signaled when the command buffers for this batch have completed execution
	uint32_t numSignalSemaphores;//!< Number of semaphores to be signaled once the commands specified in pCommandBuffers have completed execution

	/// <summary>Constructor. Default initialised to 0.</summary>
	SubmitInfo() : waitDestStages(nullptr), commandBuffers(nullptr),
		numCommandBuffers(0), waitSemaphores(nullptr), numWaitSemaphores(0),
		signalSemaphores(0), numSignalSemaphores(0)
	{}

	/// <summary>Constructor. Initialise with inidividual values</summary>
	/// <param name="waitDestStages">pointer to an array of pipeline stages at which each corresponding semaphore wait will occur</param>
	/// <param name="commandBuffers">Pointer to an array of command buffers to execute in the batch</param>
	/// <param name="numCommandBuffers">The number of command buffers to execute in the batch</param>
	/// <param name="waitSemaphores">ointer to an array of semaphores upon which to wait before the command buffers for this batch begin execution</param>
	/// <param name="numWaitSemaphores">The number of semaphores upon which to wait before executing the command buffers for the batch</param>
	/// <param name="signalSemaphores">Pointer to an array of semaphores which will be signaled when the command buffers for this batch have completed execution</param>
	/// <param name="numSignalSemaphores">Number of semaphores to be signaled once the commands specified in pCommandBuffers have completed execution</param>
	SubmitInfo(const VkPipelineStageFlags* waitDestStages,
	           const CommandBuffer* commandBuffers, uint32_t numCommandBuffers,
	           const Semaphore* waitSemaphores, uint32_t numWaitSemaphores,
	           const Semaphore* signalSemaphores, uint32_t numSignalSemaphores) :
		waitDestStages(waitDestStages), commandBuffers(commandBuffers),
		numCommandBuffers(numCommandBuffers), waitSemaphores(waitSemaphores),
		numWaitSemaphores(numWaitSemaphores), signalSemaphores(signalSemaphores),
		numSignalSemaphores(numSignalSemaphores)
	{}
};

/// <summary>Swapchain present info. Containes information for presentation such as swapchains, swapchain indices and wait semaphores</summary>
struct PresentInfo
{
	uint32_t numWaitSemaphores; //!<  The number of wait semaphores
	Semaphore*  waitSemaphores;//!<  Pointer to an array of semaphores
	uint32_t numSwapchains; //!< The number of swapchains being presented by this command
	const pvrvk::Swapchain* swapchains;//!< Pointer to an array of swapchains to use for presentation
	const uint32_t* imageIndices; //!< Pointer to an array of indices into the array of swapchain's presentatble images

	/// <summary>Constructor. Default initialised to 0</summary>
	PresentInfo() :
		numWaitSemaphores(0), waitSemaphores(nullptr), numSwapchains(0),
		swapchains(nullptr), imageIndices(0)
	{}

	/// <summary>Constructor. Initialise with inidividual values.</summary>
	/// <param name="numWaitSemaphores">The number of wait semaphores</param>
	/// <param name="waitSemaphores">Pointer to an array of semaphores</param>
	/// <param name="numSwapchains">The number of swapchains being presented by this command</param>
	/// <param name="swapchains">Pointer to an array of swapchains to use for presentation</param>
	/// <param name="imageIndices">Pointer to an array of indices into the array of swapchain's presentatble images</param>
	PresentInfo(uint32_t numWaitSemaphores, Semaphore*  waitSemaphores,
	            uint32_t numSwapchains, const pvrvk::Swapchain* swapchains, const uint32_t* imageIndices)
		: numWaitSemaphores(numWaitSemaphores), waitSemaphores(waitSemaphores), numSwapchains(numSwapchains),
		  swapchains(swapchains), imageIndices(imageIndices) {}
};

namespace impl {
/// <summary>Wraps vulkan queue object.</summary>
class Queue_
{
public:
	DECLARE_NO_COPY_SEMANTICS(Queue_)

	/// <summary>Submit</summary>
	/// <param name="queueSubmitInfo">Queue submit info</param>
	/// <param name="signalFence">Optional handle to a fence to be signaled. If fence is not null handle, it defines a fence signal</param>
	/// <returns>VkResult</returns>
	VkResult submit(const SubmitInfo& queueSubmitInfo, Fence signalFence = Fence())
	{
		return submit(&queueSubmitInfo, 1, signalFence);
	}

	/// <summary>Submit</summary>
	/// <param name="queueSubmitInfo">Pointer to submit info</param>
	/// <param name="numSubmitInfos">number of submit info</param>
	/// <param name="signalFence">Optional handle to a fence to be signaled. If fence is not null handle, it defines a fence signal</param></param>
	/// <returns>VkResult</returns>
	VkResult submit(const SubmitInfo* queueSubmitInfo,
	                uint32_t numSubmitInfos, Fence signalFence = Fence());

	/// <summary>Present</summary>
	/// <param name="presentInfo">Present info</param>
	/// <returns>VkResult</returns>
	VkResult present(PresentInfo& presentInfo);

	/// <summary>To wait on the host for the completion of outstanding queue operations for a given queue. This is equivalent to submitting a fence to a queue and waiting with an infinite timeout for that fence to signal.</summary>
	/// <returns>VkResult</returns>
	VkResult waitIdle();

	/// <summary>Check if this queue supports presentation</summary>
	/// <returns>Return true if supported</returns>
	bool isSupportPresentation()const { return _supportPresentation; }

	/// <summary>Get vulkan object</summary>
	/// <returns>VkQueue</returns>
	const VkQueue& getNativeObject()const { return _vkQueue; }

	/// <summary>Return supported queue flags</summary>
	/// <returns>VkQueueFlags</returns>
	VkQueueFlags getQueueFlags()const
	{
		return _queueFlags;
	}

	/// <summary>Submit sparse binding operations.</summary>
	/// <param name="bindInfo">Pointer to bind infos</param>
	/// <param name="numBindInfos">Number of bind infos</param>
	/// <param name="fenceSignal"> Optional handle to a fence to be signaled. If fence is not null handle, it defines a fence signal operation.</param>
	/// <returns>VkResult</returns>
	VkResult bindSparse(const BindSparseInfo* bindInfo, uint32_t numBindInfos, Fence& fenceSignal);

	/// <summary>Get queue family id</summary>
	/// <returns>uint</returns>32_t
	uint32_t getQueueFamilyId()const { return  _queueFamilyIndex; }

	/// <summary>Get the devioce which own this resource </summary>(const)
	/// <returns>DeviceWeakPtr</returns>
	const DeviceWeakPtr& getDevice()const
	{
		return _device;
	}

	/// <summary>Get the devioce which own this resource</summary>
	/// <returns>DeviceWeakPtr</returns>
	DeviceWeakPtr getDevice()
	{
		return _device;
	}
private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	Queue_(DeviceWeakPtr device, VkQueue queue,
	       VkQueueFlags flags, uint32_t queueFamilyIndex, bool supportPresentation) :
		_device(device), _vkQueue(queue), _queueFlags(flags), _queueFamilyIndex(queueFamilyIndex),
		_supportPresentation(supportPresentation)
	{
	}

	VkQueue                 _vkQueue;
	DeviceWeakPtr           _device;
	VkQueueFlags            _queueFlags;
	uint32_t                _queueFamilyIndex;
	bool                    _supportPresentation;
};
}
}// namespace pvrvk
