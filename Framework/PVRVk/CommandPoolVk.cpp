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
bool CommandPool_::init(uint32_t queueFamilyId, VkCommandPoolCreateFlags createFlags)
{
	_queueFamilyId = queueFamilyId;
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VkStructureType::e_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex = queueFamilyId;
	poolCreateInfo.flags = createFlags;
	return (vk::CreateCommandPool(_device->getNativeObject(), &poolCreateInfo, NULL, &_vkCmdPool) == VkResult::e_SUCCESS);
}
}

namespace impl {
CommandBuffer CommandPool_::allocateCommandBuffer()
{
	pvrvk::CommandBuffer commandBuffer;
	CommandPool this_ref = static_cast<impl::CommandPool_*>(this)->getReference();

	VkCommandBuffer cbuff;
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = 1;
	nfo.commandPool = _vkCmdPool;
	nfo.level = VkCommandBufferLevel::e_PRIMARY;
	nfo.sType = VkStructureType::e_COMMAND_BUFFER_ALLOCATE_INFO;
	VkResult res;
	if ((res = vk::AllocateCommandBuffers(_device->getNativeObject(), &nfo, &cbuff)) != VkResult::e_SUCCESS)
	{
		Log(LogLevel::Error, "CommandBuffer Allocation Failure with error %s. Use another command pool.", vkErrorToStr(res));
	}
	else
	{
		commandBuffer.construct(_device, this_ref, cbuff);
	}
	return commandBuffer;
}

bool CommandPool_::allocateCommandBuffers(uint32_t numCommandbuffers, CommandBuffer* outCommandBuffers)
{
	CommandPool this_ref = static_cast<impl::CommandPool_*>(this)->getReference();
	std::vector<VkCommandBuffer> cbuff(numCommandbuffers);
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = numCommandbuffers;
	nfo.commandPool = _vkCmdPool;
	nfo.level = VkCommandBufferLevel::e_PRIMARY;
	nfo.sType = VkStructureType::e_COMMAND_BUFFER_ALLOCATE_INFO;
	VkResult res;
	if ((res = vk::AllocateCommandBuffers(_device->getNativeObject(), &nfo, cbuff.data())) != VkResult::e_SUCCESS)
	{
		Log(LogLevel::Error, "CommandBuffer Allocation Failure with error %s. Use another command pool.", vkErrorToStr(res));
		return false;
	}
	for (uint32_t  i = 0; i < numCommandbuffers; ++i)
	{
		outCommandBuffers[i].construct(_device, this_ref, cbuff[i]);
	}
	return true;
}
SecondaryCommandBuffer CommandPool_::allocateSecondaryCommandBuffer()
{
	pvrvk::SecondaryCommandBuffer commandBuffer;
	CommandPool this_ref = getReference();
	VkCommandBuffer cbuff;

	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = 1;
	nfo.commandPool = getNativeObject();
	nfo.level = VkCommandBufferLevel::e_SECONDARY;
	nfo.sType = VkStructureType::e_COMMAND_BUFFER_ALLOCATE_INFO;
	VkResult res;
	if ((res = vk::AllocateCommandBuffers(_device->getNativeObject(), &nfo, &cbuff)) != VkResult::e_SUCCESS)
	{
		Log(LogLevel::Critical, "CommandBuffer Allocation Failure with error %s."
		    " Use another command pool.", vkErrorToStr(res));
	}
	else
	{
		commandBuffer.construct(_device, this_ref, cbuff);
	}
	return commandBuffer;
}

bool CommandPool_::allocateSecondaryCommandBuffers(uint32_t numCommandbuffers,
    SecondaryCommandBuffer* outCommandBuffers)
{
	CommandPool this_ref = static_cast<impl::CommandPool_*>(this)->getReference();
	std::vector<VkCommandBuffer> cbuff(numCommandbuffers);
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = numCommandbuffers;
	nfo.commandPool = _vkCmdPool;
	nfo.level = VkCommandBufferLevel::e_SECONDARY;
	nfo.sType = VkStructureType::e_COMMAND_BUFFER_ALLOCATE_INFO;
	VkResult res;
	if ((res = vk::AllocateCommandBuffers(_device->getNativeObject(), &nfo, cbuff.data())) != VkResult::e_SUCCESS)
	{
		Log(LogLevel::Error, "CommandBuffer Allocation Failure with error %s. Use another command pool.", vkErrorToStr(res));
		return false;
	}
	for (uint32_t  i = 0; i < numCommandbuffers; ++i)
	{
		outCommandBuffers[i].construct(_device, this_ref, cbuff[i]);
	}
	return true;
}
}
}// namespace pvrvk
