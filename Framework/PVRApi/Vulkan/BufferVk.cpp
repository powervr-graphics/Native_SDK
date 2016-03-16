/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\BufferVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the Buffer class. See BufferVulkan.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/BufferVk.h"
#include <PVRApi/ApiIncludes.h>
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRNativeApi/Vulkan/VkErrors.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRNativeApi/Vulkan/BufferUtilsVk.h"
namespace pvr {
namespace api {
namespace impl {
//Future consideration: If it becomes allowed to make this a run-time choice (As opposed to a link-time choice),
//this will need to be re-virtualized and pass through an interface.

void Buffer_::update(const void* data, uint32 offset, uint32 length)
{
	//Safe downcast. We already KNOW that this is our class.
	static_cast<vulkan::BufferVk_*>(this)->update(data, offset, length);
}

void* Buffer_::map(types::MapBufferFlags::Enum flags, uint32 offset, uint32 length)
{
	//Safe downcast. We already KNOW that this is our class.
	return static_cast<vulkan::BufferVk_*>(this)->map(flags, offset, length);
}

void Buffer_::unmap()
{
	//Safe downcast. We already KNOW that this is our actual type.
	static_cast<vulkan::BufferVk_*>(this)->unmap();
}

bool Buffer_::allocate(uint32 size, types::BufferBindingUse::Bits bufferUsage, types::BufferUse::Flags hint)
{
	//Safe downcast. We already KNOW that this is our actual type.
	return native_cast(this)->allocate(size, types::BufferBindingUse::Enum(bufferUsage), hint);
}

void Buffer_::destroy() { static_cast<vulkan::BufferVk_*>(this)->destroy(); }

void BufferView_::destroy() {}

const native::HBuffer_& Buffer_::getNativeObject()const { return native_cast(*this); }

native::HBuffer_& Buffer_::getNativeObject() {	return native_cast(*this); }
}// namespace impl

// AT THIS POINT, IT IS ASSUMED THAT OUR Buffer_ is, in fact, a BufferVulkanImpl. In this sense, to avoid
// virtual function calls, we are de-virtualising Buffer_ into BufferVulkan. So, for each call, our "this"
// pointer (declared type Buffer_*), gets cast into a BufferVulkanImpl* and the calls are done direct (in fact,
// inline) through this pointer.
namespace vulkan {
BufferVk_::BufferVk_(GraphicsContext& context) : Buffer_(context), m_mappedOffset(0), m_mappedRange(0)
{
	buffer = VK_NULL_HANDLE; memory = VK_NULL_HANDLE;
}

void BufferVk_::destroy()
{
	if (m_context.isValid())
	{
		VkDevice& deviceVk =  native_cast(*m_context).getDevice();
		if (buffer != VK_NULL_HANDLE)
		{
			vk::DestroyBuffer(deviceVk, buffer, NULL);
		}
		else
		{
			Log(Log.Warning, "Buffer Double deletion?");
		}
		if (memory != VK_NULL_HANDLE)
		{
			vk::FreeMemory(deviceVk, memory, NULL);
		}
		else
		{
			Log(Log.Warning, "Buffer Double deletion?");
		}
		memory = VK_NULL_HANDLE;
		buffer = VK_NULL_HANDLE;
	}
}

BufferVk_::~BufferVk_()
{
	if (isAllocated())
	{
		if (m_context.isValid())
		{
			destroy();
		}
		else
		{
			Log(Log.Warning, "Buffer object was not released before context destruction");
		}
	}
	m_context.reset();
}

void BufferVk_::update(const void* data, uint32 offset, uint32 length)
{
	assertion(length + offset <= m_size);
	void* mapData  = map(types::MapBufferFlags::Write, offset, length);
	if (mapData)
	{
		memcpy(mapData, data, length);
		unmap();
	}
	else
	{
		assertion(false, "Failed to map memory");
	}
}

void* BufferVk_::map(types::MapBufferFlags::Enum flags, uint32 offset, uint32 length)
{
	void* mapped;
	if (m_mappedRange)
	{
		assertion(false, "BufferVk_::map trying to map memory twice");
		return NULL;
	}
	m_mappedRange = length;
	m_mappedOffset = offset;
	m_mappedFlags = flags;
	platform::ContextVk& contextVk = native_cast(*m_context);

	VkResult rslt = vk::MapMemory(contextVk.getDevice(), memory, offset, length, 0, &mapped);
	if (rslt != VK_SUCCESS)
	{
		Log("BufferVk_::map Failed to to map buffer");
		return NULL;
	}
	return mapped;
}


void BufferVk_::unmap()
{
	if (!m_mappedRange)
	{
		assertion(false, "Buffer_::unmap trying to un-map un-mapped memory");
		return;
	}
	platform::ContextVk& contextVk = native_cast(*m_context);
	VkMappedMemoryRange range = {};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = memory;
	range.offset = m_mappedOffset;
	range.size = m_mappedRange;
	if (m_mappedFlags & types::MapBufferFlags::Write)
	{
		vk::FlushMappedMemoryRanges(contextVk.getDevice(), 1, &range);
	}
	if (m_mappedFlags & types::MapBufferFlags::Read)
	{
		vk::InvalidateMappedMemoryRanges(contextVk.getDevice(), 1, &range);
	}
	vk::UnmapMemory(contextVk.getDevice(), memory);
	m_mappedRange = 0;
	m_mappedOffset = 0;
	m_mappedFlags = types::MapBufferFlags::Enum(0);
}


bool BufferVk_::allocate(uint32 size, types::BufferBindingUse::Enum usage, types::BufferUse::Flags hint)
{
	platform::ContextVk& contextVk = native_cast(*m_context);
	if (isAllocated())// re-allocate if neccessary
	{
		Log(Log.Debug, "BufferVulkanImpl::allocate: Vulkan buffer %d was already allocated, deleting it. This should normally NOT happen - allocate is private.", buffer);
		destroy();
	}
	m_size = size;
	m_usage = usage;
	if (!pvr::apiUtils::vulkan::createBuffer(contextVk.getDevice(),
	    contextVk.getPlatformContext().getNativePlatformHandles().deviceMemProperties,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, usage, size, getNativeObject(), NULL))
	{
		return false;
	}
	return true;
}

BufferViewVk_::BufferViewVk_(const Buffer& buffer, uint32 offset, uint32 range) :
	impl::BufferView_(buffer, offset, range) {}

}// namespace vulkan
}
}
//!\endcond
