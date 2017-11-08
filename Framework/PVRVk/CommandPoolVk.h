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
class CommandPool_: public EmbeddedRefCount<CommandPool_>
{
	// Implementing EmbeddedRefCount
	template <typename> friend class ::pvrvk::EmbeddedRefCount;
public:
	DECLARE_NO_COPY_SEMANTICS(CommandPool_)

	/// <summary>Allocate a primary commandBuffer</summary>
	/// <returns>Return a valid commandbuffer if allocation is successful, otherwise a null CommandBuffer</returns>
	CommandBuffer allocateCommandBuffer();

	/// <summary>Allocate primary commandbuffers</summary>
	/// <param name="numCommandbuffers">Number of commandbuffers to allocate</param>
	/// <param name="outCommandBuffers">Allocated commandbuffers</param>
	/// <returns>Return true if success</returns>
	bool allocateCommandBuffers(uint32_t numCommandbuffers, CommandBuffer* outCommandBuffers);

	/// <summary>Allocate a secondary commandBuffer</summary>
	/// <returns>Return a valid commandbuffer if allocation success, otherwise a null CommandBuffer</returns>
	SecondaryCommandBuffer allocateSecondaryCommandBuffer();

	/// <summary>Allocate secondary commandbuffers</summary>
	/// <param name="numCommandbuffers">Number of commmandbuffers to allocate</param>
	/// <param name="outCommandBuffers">allocated commandbuffers</param>
	/// <returns>Return true if success</returns>
	bool allocateSecondaryCommandBuffers(uint32_t numCommandbuffers, SecondaryCommandBuffer* outCommandBuffers);

	/// <summary>Get vulkan object (const)</summary>
	/// <returns>VkCommandPool</returns>
	const VkCommandPool& getNativeObject()const { return _vkCmdPool; }

	/// <summary>Get commandpool queue family id.</summary>
	/// <returns>uint32_t</returns>
	uint32_t getQueueFamilyId()const { return _queueFamilyId; }
private:
	friend class ::pvrvk::impl::Device_;

	bool init(uint32_t queueFamilyId, VkCommandPoolCreateFlags createFlags);

	/// <summary>Construct a CommandPool</summary>
	/// <param name="device">The GpuDevice this command pool will be constructed from.</param>
	CommandPool_(const DeviceWeakPtr& device) : _device(device), _vkCmdPool(VK_NULL_HANDLE) {}

	/* IMPLEMENTING EmbeddedResource */
	void destroyObject()
	{
		if (_device.isValid() && _vkCmdPool != VK_NULL_HANDLE)
		{
			vk::DestroyCommandPool(_device->getNativeObject(), _vkCmdPool, NULL);
			_vkCmdPool = VK_NULL_HANDLE;
		}
	}
	DeviceWeakPtr _device;
	uint32_t _queueFamilyId;
	VkCommandPool _vkCmdPool;
};
}// namespace impl
}// namespace pvrvk
