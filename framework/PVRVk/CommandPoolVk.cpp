/*!
\brief Function implementations for the Command Pool.
\file PVRVk/CommandPoolVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
CommandPool_::CommandPool_(const DeviceWeakPtr& device, const CommandPoolCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_COMMAND_POOL_EXT), _createInfo(createInfo)
{
	VkCommandPoolCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_POOL_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkCommandPoolCreateFlags>(_createInfo.getFlags());
	vkCreateInfo.queueFamilyIndex = _createInfo.getQueueFamilyIndex();
	vkThrowIfFailed(_device->getVkBindings().vkCreateCommandPool(_device->getVkHandle(), &vkCreateInfo, NULL, &_vkHandle), "Failed to create CommandPool");
}
} // namespace impl

namespace impl {
CommandBuffer CommandPool_::allocateCommandBuffer()
{
	pvrvk::CommandBuffer commandBuffer;
	CommandPool this_ref = static_cast<impl::CommandPool_*>(this)->getReference();

	VkCommandBuffer cbuff;
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = 1;
	nfo.commandPool = getVkHandle();
	nfo.level = static_cast<VkCommandBufferLevel>(CommandBufferLevel::e_PRIMARY);
	nfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_ALLOCATE_INFO);
	vkThrowIfFailed(_device->getVkBindings().vkAllocateCommandBuffers(_device->getVkHandle(), &nfo, &cbuff), "CommandBuffer Allocation Failure.");
	commandBuffer.construct(_device, this_ref, cbuff);
	return commandBuffer;
}

void CommandPool_::allocateCommandBuffers(uint32_t numCommandbuffers, CommandBuffer* outCommandBuffers)
{
	CommandPool this_ref = static_cast<impl::CommandPool_*>(this)->getReference();
	ArrayOrVector<VkCommandBuffer, 8> cbuff(numCommandbuffers);
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = numCommandbuffers;
	nfo.commandPool = getVkHandle();
	nfo.level = static_cast<VkCommandBufferLevel>(CommandBufferLevel::e_PRIMARY);
	nfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_ALLOCATE_INFO);
	vkThrowIfFailed(_device->getVkBindings().vkAllocateCommandBuffers(_device->getVkHandle(), &nfo, cbuff.get()), "CommandBuffer Allocation Failed.");
	for (uint32_t i = 0; i < numCommandbuffers; ++i)
	{
		outCommandBuffers[i].construct(_device, this_ref, cbuff[i]);
	}
}

SecondaryCommandBuffer CommandPool_::allocateSecondaryCommandBuffer()
{
	pvrvk::SecondaryCommandBuffer commandBuffer;
	CommandPool this_ref = getReference();
	VkCommandBuffer cbuff;

	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = 1;
	nfo.commandPool = getVkHandle();
	nfo.level = static_cast<VkCommandBufferLevel>(CommandBufferLevel::e_SECONDARY);
	nfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_ALLOCATE_INFO);
	vkThrowIfFailed(_device->getVkBindings().vkAllocateCommandBuffers(_device->getVkHandle(), &nfo, &cbuff), "CommandBuffer Allocation Failed.");
	commandBuffer.construct(_device, this_ref, cbuff);
	return commandBuffer;
}

void CommandPool_::allocateSecondaryCommandBuffers(uint32_t numCommandbuffers, SecondaryCommandBuffer* outCommandBuffers)
{
	CommandPool this_ref = static_cast<impl::CommandPool_*>(this)->getReference();
	ArrayOrVector<VkCommandBuffer, 8> cbuff(numCommandbuffers);
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = numCommandbuffers;
	nfo.commandPool = getVkHandle();
	nfo.level = static_cast<VkCommandBufferLevel>(CommandBufferLevel::e_SECONDARY);
	nfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_ALLOCATE_INFO);
	vkThrowIfFailed(_device->getVkBindings().vkAllocateCommandBuffers(_device->getVkHandle(), &nfo, cbuff.get()), "CommandBuffer Allocation Failure.");
	for (uint32_t i = 0; i < numCommandbuffers; ++i)
	{
		outCommandBuffers[i].construct(_device, this_ref, cbuff[i]);
	}
}

void CommandPool_::reset(CommandPoolResetFlags flags)
{
	vkThrowIfFailed(_device->getVkBindings().vkResetCommandPool(_device->getVkHandle(), getVkHandle(), static_cast<VkCommandPoolResetFlags>(flags)), "CommandBuffer Reset Failure.");
}
} // namespace impl
} // namespace pvrvk
