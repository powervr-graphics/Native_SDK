/*!
\brief Function definitions for Fence, Semaphore, MemoryBarrier, Event classes
\file PVRVk/SyncVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/SyncVk.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/ImageVk.h"

namespace pvrvk {
namespace impl {
bool Fence_::init(VkFenceCreateFlags fenceCreateFlags)
{
	VkFenceCreateInfo nfo = {};
	nfo.sType = VkStructureType::e_FENCE_CREATE_INFO;
	nfo.flags = fenceCreateFlags;
	VkResult res = vk::CreateFence(_device->getNativeObject(), &nfo, NULL, &_vkFence);
	vkThrowIfFailed(res, "FenceVk_::init: Failed to create Fence object");
	return res == VkResult::e_SUCCESS;
}

void Fence_::destroy()
{
	if (_vkFence != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			vk::DestroyFence(_device->getNativeObject(), _vkFence, NULL);
			_vkFence = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterContext("Fence");
		}
	}
}

VkResult Fence_::wait(uint64_t timeoutNanos)
{
	VkResult res;
	res = vk::WaitForFences(_device->getNativeObject(), 1, &_vkFence, true, timeoutNanos);
	vkThrowIfFailed(res, "Fence::wait returned an error");
	return (VkResult)res;
}

void Fence_::reset()
{
	vkThrowIfFailed(vk::ResetFences(_device->getNativeObject(), 1, &_vkFence),
	                "Fence::reset returned an error");
}

VkResult Fence_::getStatus()
{
	return (VkResult)vk::GetFenceStatus(_device->getNativeObject(), _vkFence);
}

bool Semaphore_::init()
{
	VkSemaphoreCreateInfo nfo;
	nfo.sType = VkStructureType::e_SEMAPHORE_CREATE_INFO;
	nfo.pNext = 0;
	nfo.flags = 0;
	VkResult res = vk::CreateSemaphore(_device->getNativeObject(), &nfo, NULL, &_vkSemaphore);
	vkThrowIfFailed(res, "SemaphoreVk_::init: Failed to create Semaphore object");
	return res == VkResult::e_SUCCESS;
}

void Semaphore_::destroy()
{
	if (_vkSemaphore != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			vk::DestroySemaphore(_device->getNativeObject(), _vkSemaphore, NULL);
			_vkSemaphore = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterContext("Semaphore");
		}
	}
}

bool Event_::init()
{
	VkEventCreateInfo nfo;
	nfo.sType = VkStructureType::e_EVENT_CREATE_INFO;
	nfo.pNext = 0;
	nfo.flags = 0;
	VkResult res = vk::CreateEvent(_device->getNativeObject(), &nfo, NULL, &_vkEvent);
	vkThrowIfFailed(res, "EventVk_::init: Failed to create Semaphore object");
	return res == VkResult::e_SUCCESS;
}

void Event_::destroy()
{
	if (_vkEvent != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			vk::DestroyEvent(_device->getNativeObject(), _vkEvent, NULL);
			_vkEvent = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterContext("Event");
		}
	}
}
}

namespace impl {

void Event_::set()
{
	vkThrowIfFailed(vk::SetEvent(_device->getNativeObject(), _vkEvent), "Event::set returned an error");
}

void Event_::reset()
{
	vkThrowIfFailed(vk::ResetEvent(_device->getNativeObject(), _vkEvent), "Event::reset returned an error");
}

bool Event_::isSet()
{
	VkResult res = vk::GetEventStatus(_device->getNativeObject(), _vkEvent);
	if (res == VkResult::e_EVENT_SET)
	{
		return true;
	}
	else if (res == VkResult::e_EVENT_RESET)
	{
		return false;
	}
	vkThrowIfFailed(res, "Event::set returned an error");
	return false;
}
}
}
