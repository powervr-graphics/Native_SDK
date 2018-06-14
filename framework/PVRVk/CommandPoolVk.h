/*!
\brief The PVRVk CommandPool, a pool that can Allocate and Free Command Buffers
\file PVRVk/CommandPoolVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Vulkan implementation of the Command Pool class.
/// Destroying the commandpool will also destroys the commandbuffers allocated from this pool
/// </summary>
class CommandPool_ : public EmbeddedRefCount<CommandPool_>, public DeviceObjectHandle<VkCommandPool>, public DeviceObjectDebugMarker<CommandPool_>
{
	// Implementing EmbeddedRefCount
	template<typename>
	friend class ::pvrvk::EmbeddedRefCount;

public:
	DECLARE_NO_COPY_SEMANTICS(CommandPool_)

	/// <summary>Allocate a primary commandBuffer</summary>
	/// <returns>Return a valid commandbuffer if allocation is successful, otherwise a null CommandBuffer</returns>
	CommandBuffer allocateCommandBuffer();

	/// <summary>Allocate primary commandbuffers</summary>
	/// <param name="numCommandbuffers">Number of commandbuffers to allocate</param>
	/// <param name="outCommandBuffers">Allocated commandbuffers</param>
	void allocateCommandBuffers(uint32_t numCommandbuffers, CommandBuffer* outCommandBuffers);

	/// <summary>Allocate a secondary commandBuffer</summary>
	/// <returns>Return a valid commandbuffer if allocation success, otherwise a null CommandBuffer</returns>
	SecondaryCommandBuffer allocateSecondaryCommandBuffer();

	/// <summary>Allocate secondary commandbuffers</summary>
	/// <param name="numCommandbuffers">Number of commmandbuffers to allocate</param>
	/// <param name="outCommandBuffers">allocated commandbuffers</param>
	void allocateSecondaryCommandBuffers(uint32_t numCommandbuffers, SecondaryCommandBuffer* outCommandBuffers);

	/// <summary>Get commandpool queue family id.</summary>
	/// <returns>uint32_t</returns>
	uint32_t getQueueFamilyId() const
	{
		return _queueFamilyId;
	}

	/// <summary>Resets the command pool and also optionally recycles all of the resoources of all of the command buffers allocated from the command pool.</summary>
	/// <param name="flags">VkCommandPoolResetFlags controls the reset operation</param>
	/// <returns>Return true if success</returns>
	void reset(pvrvk::CommandPoolResetFlags flags);

private:
	friend class ::pvrvk::impl::Device_;

	/// <summary>Construct a CommandPool</summary>
	/// <param name="device">The GpuDevice this command pool will be constructed from.</param>
	CommandPool_(const DeviceWeakPtr& device, uint32_t queueFamilyId, pvrvk::CommandPoolCreateFlags createFlags);

	/* IMPLEMENTING EmbeddedResource */
	void destroyObject()
	{
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			if (_device.isValid())
			{
				_device->getVkBindings().vkDestroyCommandPool(_device->getVkHandle(), getVkHandle(), nullptr);
				_vkHandle = VK_NULL_HANDLE;
				_device.reset();
			}
			else
			{
				reportDestroyedAfterDevice("CommandPool");
			}
		}
	}

	uint32_t _queueFamilyId;
};
} // namespace impl
} // namespace pvrvk
