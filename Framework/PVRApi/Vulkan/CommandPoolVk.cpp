#include "PVRApi/Vulkan/CommandPoolVk.h"
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
	if (m_context.isValid())
	{
		destroy();
		Log(Log.Warning, "Command pool was still active after context destruction");
	}
}

void CommandPoolVk_::destroy()
{
	if (m_context.isValid() && handle != VK_NULL_HANDLE)
	{
		VkDevice dev = native_cast(*getContext()).getDevice();
		vk::DestroyCommandPool(dev, handle, NULL);
		handle = VK_NULL_HANDLE;
		m_context.reset();
	}
}
}

namespace impl {
CommandBuffer CommandPool_::allocateCommandBuffer()
{
	CommandPool this_ref = static_cast<vulkan::CommandPoolVk_*>(this)->getReference();
	api::CommandBuffer commandBuffer;
	commandBuffer.construct(m_context, this_ref);
	return commandBuffer;
}
SecondaryCommandBuffer CommandPool_::allocateSecondaryCommandBuffer()
{
	CommandPool this_ref = static_cast<vulkan::CommandPoolVk_*>(this)->getReference();
	api::SecondaryCommandBuffer commandBuffer;
	commandBuffer.construct(m_context, this_ref);
	return commandBuffer;
}

const native::HCommandPool_& impl::CommandPool_::getNativeObject() const
{
	return native_cast(*this);
}

native::HCommandPool_& impl::CommandPool_::getNativeObject()
{
	return native_cast(*this);
}
}
}
}
