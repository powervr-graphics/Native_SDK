/*!
\brief The Device Memory class, a class representing a memory block managed by Vulkan
\file PVRVk/DeviceMemoryVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"
namespace pvrvk {
namespace impl {

/// <summary>VkDeviceMemory wrapper</summary>
class IDeviceMemory_
{
public:
	virtual ~IDeviceMemory_() {}

	/// <summary>Get vulkan handle</summary>
	/// <returns>const VkDeviceMemory&</returns>
	virtual const VkDeviceMemory& getNativeObject()const = 0;

	/// <summary>Return true if this memory block is mappable by the host (const).</summary>
	/// <returns>bool</returns>
	virtual bool isMappable()const = 0;

	/// <summary>Return the memory flags(const)</summary>
	/// <returns>VkMemoryPropertyFlags</returns>
	virtual VkMemoryPropertyFlags getMemoryFlags()const = 0;

	/// <summary>Return this mapped memory offset (const)</summary>
	/// <returns>VkDeviceSize</returns
	virtual VkDeviceSize getMappedOffset()const = 0;

	/// <summary>Return this mapped memory size (const)</summary>
	/// <returns>VkDeviceSize</returns
	virtual VkDeviceSize getMappedSize()const = 0;

	/// <summary>Return this memory size (const)</summary>
	/// <returns>uint64_t</returns>
	virtual VkDeviceSize getSize()const = 0;

	/// <summary>Return true if this memory is being mapped by the host (const)</summary>
	/// <returns>VkDeviceSize</returns
	virtual bool isMapped()const = 0;

	/// <summary>
	/// map this memory. NOTE: Only memory created with HostVisible flag can be mapped and unmapped
	/// </summary>
	/// <param name="mappedMemory">Out mapped memory</param>
	/// <param name="offset">Zero-based byte offset from the beginning of the memory object.</param>
	/// <param name="size">Size of the memory range to map, or VK_WHOLE_SIZE to map from offset to the end of the allocation</param>
	/// <returns>VkResult</returns>
	virtual VkResult map(void** mappedMemory, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) = 0;

	/// <summary>Unmap this memory block</summary>
	virtual void unmap() = 0;

	/// <summary>Flush ranges of non-coherent memory from the host caches:</summary>
	/// <param name="offset">Zero-based byte offset from the beginning of the memory object</param>
	/// <param name="size">Either the size of range, or VK_WHOLE_SIZE to affect the range from offset to the end of the current mapping of the allocation.</param>
	/// <returns>VkResult</returns>
	virtual VkResult flushRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) = 0;

	/// <summary>To invalidate ranges of non-coherent memory from the host caches</summary>
	/// <param name="offset">Zero-based byte offset from the beginning of the memory object.</param>
	/// <param name="size">Either the size of range, or VK_WHOLE_SIZE to affect the range from offset to the end of the current mapping of the allocation.</param>
	/// <returns>VkResult</returns>
	virtual VkResult invalidateRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) = 0;
};

/// <summary>VkDeviceMemory wrapper</summary>
class DeviceMemory_ : public IDeviceMemory_
{
public:
	DECLARE_NO_COPY_SEMANTICS(DeviceMemory_)

	/// <summary>Get vulkan handle</summary>
	/// <returns>const VkDeviceMemory&</returns>
	const VkDeviceMemory& getNativeObject()const { return _vkDeviceMemory; }

	/// <summary>Return true if this memory block is mappable by the host (const).</summary>
	/// <returns>bool</returns>
	bool isMappable()const
	{
		return (static_cast<uint32_t>(_flags & VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT) != 0) ||
		       (static_cast<uint32_t>(_flags & VkMemoryPropertyFlags::e_HOST_COHERENT_BIT) != 0);
	}

	/// <summary>Return the memory flags(const)</summary>
	/// <returns>VkMemoryPropertyFlags</returns>
	VkMemoryPropertyFlags getMemoryFlags()const
	{
		return _flags;
	}

	/// <summary>Return this mapped memory offset (const)</summary>
	/// <returns>VkDeviceSize</returns
	VkDeviceSize getMappedOffset()const { return _mappedOffset; }

	/// <summary>Return this mapped memory size (const)</summary>
	/// <returns>VkDeviceSize</returns
	VkDeviceSize getMappedSize()const { return _mappedSize; }

	/// <summary>Return this memory size (const)</summary>
	/// <returns>uint64_t</returns>
	VkDeviceSize getSize()const { return _size; }

	/// <summary>Return true if this memory is being mapped by the host (const)</summary>
	/// <returns>VkDeviceSize</returns
	bool isMapped()const { return _mappedSize > 0; }

	/// <summary>
	/// map this memory. NOTE: Only memory created with HostVisible flag can be mapped and unmapped
	/// </summary>
	/// <param name="mappedMemory">Out mapped memory</param>
	/// <param name="offset">Zero-based byte offset from the beginning of the memory object.</param>
	/// <param name="size">Size of the memory range to map, or VK_WHOLE_SIZE to map from offset to the end of the allocation</param>
	/// <returns>VkResult</returns>
	VkResult map(void** mappedMemory, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
	{
		if (!isMappable())
		{
			Log(LogLevel::Warning, "Cannot map memory block 0x%ullx as the memory was created without "
			    "HOST_VISIBLE_BIT or HOST_COHERENT_BIT memory flags", _vkDeviceMemory);
			return VkResult::e_ERROR_MEMORY_MAP_FAILED;
		}
		if (_mappedSize)
		{
			Log(LogLevel::Error, "Cannot map memory block 0x%ullx as the memory is already mapped", _vkDeviceMemory);
			return VkResult::e_ERROR_MEMORY_MAP_FAILED;
		}
		if (size != VK_WHOLE_SIZE)
		{
			if (offset + size > _size)
			{
				Log(LogLevel::Error, "Cannot map map memory block 0x%ullx"
				    " - Attempting to map offset (0x%ullx) + size (0x%ullx) range greater than the memory block size", _vkDeviceMemory, offset, size);
				return VkResult::e_ERROR_MEMORY_MAP_FAILED;
			}
		}

		VkResult rslt = vk::MapMemory(_device->getNativeObject(), _vkDeviceMemory, offset, size, 0, mappedMemory);
		if (rslt != VkResult::e_SUCCESS)
		{
			Log(LogLevel::Error, "Failed to map memory block 0x%ullx", _vkDeviceMemory);
			return VkResult::e_ERROR_MEMORY_MAP_FAILED;
		}

		// store the mapped offset and mapped size
		_mappedOffset = offset;
		_mappedSize = size;

		return rslt;
	}

	/// <summary>Unmap this memory block</summary>
	void unmap()
	{
		if (!_mappedSize)
		{
			Log(LogLevel::Error, "Cannot unmap memory block 0x%ullx as the memory is not mapped", _vkDeviceMemory);
			return;
		}

		_mappedSize = 0;
		_mappedOffset = 0;

		vk::UnmapMemory(_device->getNativeObject(), _vkDeviceMemory);
	}

	/// <summary>Flush ranges of non-coherent memory from the host caches:</summary>
	/// <param name="offset">Zero-based byte offset from the beginning of the memory object</param>
	/// <param name="size">Either the size of range, or VK_WHOLE_SIZE to affect the range from offset to the end of the current mapping of the allocation.</param>
	/// <returns>VkResult</returns>
	VkResult flushRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
	{
		if (static_cast<uint32_t>(_flags & VkMemoryPropertyFlags::e_HOST_COHERENT_BIT) != 0)
		{
			Log(LogLevel::Warning, "Flushing memory block 0x%ullx"
			    " created using HOST_COHERENT_BIT memory flags - this is unnecessary.", _vkDeviceMemory);
		}

		VkMappedMemoryRange range = {};
		range.sType = VkStructureType::e_MAPPED_MEMORY_RANGE;
		range.memory = _vkDeviceMemory;
		range.offset = offset;
		range.size = size;
		VkResult rslt = vk::FlushMappedMemoryRanges(_device->getNativeObject(), 1, &range);
		if (rslt != VkResult::e_SUCCESS)
		{
			Log(LogLevel::Error, "Failed to flush range of memory block 0x%ullx", _vkDeviceMemory);
		}
		return rslt;
	}

	/// <summary>To invalidate ranges of non-coherent memory from the host caches</summary>
	/// <param name="offset">Zero-based byte offset from the beginning of the memory object.</param>
	/// <param name="size">Either the size of range, or VK_WHOLE_SIZE to affect the range from offset to the end of the current mapping of the allocation.</param>
	/// <returns>VkResult</returns>
	VkResult invalidateRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
	{
		if (static_cast<uint32_t>(_flags & VkMemoryPropertyFlags::e_HOST_COHERENT_BIT) != 0)
		{
			Log(LogLevel::Warning, "Invalidating range of memory block 0x%ullx"
			    " created using HOST_COHERENT_BIT memory flags - this is unnecessary.", _vkDeviceMemory);
		}

		VkMappedMemoryRange range = {};
		range.sType = VkStructureType::e_MAPPED_MEMORY_RANGE;
		range.memory = _vkDeviceMemory;
		range.offset = offset;
		range.size = size;
		VkResult rslt = vk::InvalidateMappedMemoryRanges(_device->getNativeObject(), 1, &range);
		if (rslt != VkResult::e_SUCCESS)
		{
			Log(LogLevel::Error, "Failed to invalidate range of memory block 0x%ullx", _vkDeviceMemory);
		}
		return rslt;
	}

private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;


	virtual ~DeviceMemory_() { release(); }

	bool init(uint64_t size, uint32_t allowedMemoryBits, VkMemoryPropertyFlags memPropFlags)
	{

		const auto& memProp = _device->getPhysicalDevice()->getMemoryProperties();
		_flags = memPropFlags;
		_size = size;
		return allocateDeviceMemory(_device->getNativeObject(), (const VkPhysicalDeviceMemoryProperties&)memProp,
		                            memPropFlags, allowedMemoryBits, size, _vkDeviceMemory);
	}

	DeviceMemory_(DeviceWeakPtr device, VkDeviceMemory memory, uint32_t size)
	{
		_vkDeviceMemory = memory;
		_size = size;
		_device = device;
	}

	void release()
	{
		if (_vkDeviceMemory != VK_NULL_HANDLE)
		{
			vk::FreeMemory(_device->getNativeObject(), _vkDeviceMemory, nullptr);
			_vkDeviceMemory = VK_NULL_HANDLE;
		}
	}

	DeviceMemory_(DeviceWeakPtr device) :
		_vkDeviceMemory(VK_NULL_HANDLE), _size(0), _mappedSize(0), _mappedOffset(0), _device(device) {}

	bool allocateDeviceMemory(
	  VkDevice device, const VkPhysicalDeviceMemoryProperties& deviceMemProperty,
	  VkMemoryPropertyFlags memFlags, uint32_t allowedMemoryBits,
	  uint64_t size, VkDeviceMemory& outMemory)
	{
		// allocate the memory
		VkMemoryAllocateInfo memAllocInfo = {};

		if (allowedMemoryBits == 0) // find the first allowed type
		{
			Log("Allowed memory Bits must not be 0");
			return false;
		}
		memAllocInfo.sType = VkStructureType::e_MEMORY_ALLOCATE_INFO;
		memAllocInfo.pNext = nullptr;
		memAllocInfo.allocationSize = size;
		if (!getMemoryTypeIndex(deviceMemProperty, allowedMemoryBits, memFlags,
		                        memAllocInfo.memoryTypeIndex))
		{
			return false;
		}
		if (vk::AllocateMemory(device, &memAllocInfo, nullptr, &outMemory) != VkResult::e_SUCCESS)
		{
			Log("Failed to allocate buffer's memory with allocation size %d", memAllocInfo.allocationSize);
			return false;
		}
		return true;
	}

	bool getMemoryTypeIndex(
	  const VkPhysicalDeviceMemoryProperties& deviceMemProps,
	  uint32_t typeBits, VkMemoryPropertyFlags properties,
	  uint32_t& outTypeIndex)
	{
		for (;;)
		{
			uint32_t typeBitsTmp = typeBits;
			for (uint32_t i = 0; i < 32; ++i)
			{
				if ((typeBitsTmp & 1) == 1)
				{
					if (VkMemoryPropertyFlags(deviceMemProps.memoryTypes[i].propertyFlags & properties) == properties)
					{
						outTypeIndex = i;
						return true;
					}
				}
				typeBitsTmp >>= 1;
			}
			if ((properties & VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT) != 0)
			{
				properties = VkMemoryPropertyFlags(properties & ~VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT);
				continue;
			}
			if ((properties & VkMemoryPropertyFlags::e_DEVICE_LOCAL_BIT) != 0)
			{
				properties = VkMemoryPropertyFlags(properties & ~VkMemoryPropertyFlags::e_DEVICE_LOCAL_BIT);
				continue;
			}
			if ((properties & VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT) != 0)
			{
				properties = VkMemoryPropertyFlags(properties & ~VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT);
				continue;
			}
			else
			{
				break;
			}
		}
		return false;
	}

	VkDeviceMemory _vkDeviceMemory;
	VkMemoryPropertyFlags _flags;
	VkDeviceSize _size;
	VkDeviceSize _mappedOffset;
	VkDeviceSize _mappedSize;
	DeviceWeakPtr _device;
};
}// namespace impl
}// namespace pvrvk
