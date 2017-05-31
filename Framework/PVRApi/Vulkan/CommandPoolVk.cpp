/*!
\brief Vulkan Implementation details CommandPool class.
\file PVRApi/Vulkan/CommandPoolVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/Vulkan/CommandPoolVk.h"
#include "PVRApi/Vulkan/CommandBufferVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
namespace pvr {
namespace api {
namespace vulkan {
bool CommandPoolVk_::init()
{
	platform::ContextVk& contextVk = native_cast(*getContext());
	VkCommandPoolCreateInfo poolCreateInfo;
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.pNext = NULL;
	poolCreateInfo.queueFamilyIndex = contextVk.getQueueFamilyId();
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	return (vk::CreateCommandPool(contextVk.getDevice(), &poolCreateInfo, NULL, &native_cast(this)->handle) == VK_SUCCESS);
}
CommandPoolVk_::~CommandPoolVk_()
{
	if (_context.isValid())
	{
		destroy();
		Log(Log.Warning, "Command pool was still active after context destruction");
	}
}

void CommandPoolVk_::destroy()
{
	if (_context.isValid() && handle != VK_NULL_HANDLE)
	{
		VkDevice dev = native_cast(*getContext()).getDevice();
		vk::DestroyCommandPool(dev, handle, NULL);
		handle = VK_NULL_HANDLE;
		_context.reset();
	}
}
}

namespace impl {
CommandBuffer CommandPool_::allocateCommandBuffer()
{
	api::CommandBuffer commandBuffer;
	CommandPool this_ref = static_cast<vulkan::CommandPoolVk_*>(this)->getReference();

	native::HCommandBuffer_ cbuff;

	VkCommandBufferAllocateInfo nfo;
	nfo.commandBufferCount = 1;
	nfo.commandPool = static_cast<vulkan::CommandPoolVk_*>(this)->handle;
	nfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	nfo.pNext = 0;
	nfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	VkResult res;
	if ((res = vk::AllocateCommandBuffers(native_cast(*_context).getDevice(), &nfo, &cbuff.handle)) != VK_SUCCESS)
	{
		Log(Log.Error, "CommandBuffer Allocation Failure with error %s. Use another command pool.", nativeVk::vkErrorToStr(res));
	}
	else
	{
		std::auto_ptr<ICommandBufferImpl_> impl(new CommandBufferImplVk_(_context, this_ref, cbuff));
		commandBuffer.construct(impl);
	}
	return commandBuffer;
}
SecondaryCommandBuffer CommandPool_::allocateSecondaryCommandBuffer()
{
	api::SecondaryCommandBuffer commandBuffer;
	CommandPool this_ref = static_cast<vulkan::CommandPoolVk_*>(this)->getReference();
	native::HCommandBuffer_ cbuff;

	VkCommandBufferAllocateInfo nfo;
	nfo.commandBufferCount = 1;
	nfo.commandPool = static_cast<vulkan::CommandPoolVk_*>(this)->handle;
	nfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	nfo.pNext = 0;
	nfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	VkResult res;
	if ((res = vk::AllocateCommandBuffers(native_cast(*_context).getDevice(), &nfo, &cbuff.handle)) != VK_SUCCESS)
	{
		Log(Log.Critical, "CommandBuffer Allocation Failure with error %s. Use another command pool.", nativeVk::vkErrorToStr(res));
	}
	else
	{
		std::auto_ptr<ICommandBufferImpl_> impl(new CommandBufferImplVk_(_context, this_ref, cbuff));
		commandBuffer.construct(impl);
	}
	return commandBuffer;
}

}
}
}
