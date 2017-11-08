/*!
\brief Contains the MemoryAllocator, a class for suballocating Vulkan Device memory.
\file PVRUtils/Vulkan/MemoryAllocator.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/PVRVk.h"

namespace pvr {
namespace utils {
namespace impl {
class MemorySuballocation_;
}
typedef pvrvk::RefCountedResource<impl::MemorySuballocation_> MemorySuballocation;

namespace impl {
class MemorySuballocator_ : public pvrvk::EmbeddedRefCount<MemorySuballocator_>
{
	MemorySuballocator_() : _flags(VkMemoryPropertyFlags(0)), _memType(0), _alignment(0), _totalSize(0) {}
	MemorySuballocator_(const MemorySuballocator_&);
	MemorySuballocator_& operator=(const MemorySuballocator_&);
	friend class MemorySuballocation_;
	template<typename> friend class pvrvk::EmbeddedRefCount;
	struct AllocationPair
	{
		uint64_t start_offset;
		uint64_t size;
		uint64_t end() { return start_offset + size; }
		AllocationPair() : start_offset(0), size(0) {}
		AllocationPair(uint64_t start_offset, uint64_t size) : start_offset(start_offset), size(size) {}
	};
	pvrvk::DeviceMemory _memory;
	VkMemoryPropertyFlags _flags;
	uint32_t _memType;
	uint32_t _alignment;

	VkDeviceSize _totalSize;

	std::vector<AllocationPair> _freeMemory;
	void destroyObject()
	{
		this->_freeMemory.clear();
		this->_totalSize = 0;
		this->_memory.reset();
		this->_flags = VkMemoryPropertyFlags(0);
		this->_memType = 0;
	}
public:
	void init(pvrvk::Device device, VkDeviceSize chunkSize, VkMemoryRequirements requirements, VkMemoryPropertyFlags flags)
	{
		assertion(chunkSize >= 4096/*Completely unreasonable to allocate less than 4k, ideally should be a few megs.*/, "The chunk allocation size is less than a single page of memory."
		          " Use a larger chunk size, or do not use a suballocator");
		if (chunkSize * device->getPhysicalDevice()->getProperties().limits.maxMemoryAllocationCount < 32 * 1024 * 1024) // Less than 32 megs of total ram?!
		{
			Log(LogLevel::Warning, "Memory Chunk allocation of %ull is less than one megabyte. Consider using larger chunks, as there is a limit of %d total allocations, hence with an"
			    " average size of %ull the total useable memory will be only %ull MB", chunkSize, device->getPhysicalDevice()->getProperties().limits.maxMemoryAllocationCount,
			    chunkSize, device->getPhysicalDevice()->getProperties().limits.maxMemoryAllocationCount * chunkSize);
		}

		_memory = device->allocateMemory(chunkSize, requirements.memoryTypeBits, flags);
		_freeMemory.emplace_back(0, chunkSize);
		_totalSize = chunkSize;
		_flags = flags;
		_memType = requirements.memoryTypeBits;
		_alignment = (uint32_t)requirements.alignment;
	}
	MemorySuballocation suballocate(VkDeviceSize size);

	static pvrvk::EmbeddedRefCountedResource<impl::MemorySuballocator_> createNew()
	{
		return pvrvk::EmbeddedRefCount<impl::MemorySuballocator_>::createNew();
	}
};
}
typedef pvrvk::EmbeddedRefCountedResource<impl::MemorySuballocator_> MemorySuballocator;

inline MemorySuballocator createMemorySuballocator()
{
	return impl::MemorySuballocator_::createNew();
}
namespace impl {
class MemorySuballocation_ : public pvrvk::impl::IDeviceMemory_
{
public:
	/// <summary>Get vulkan handle</summary>
	/// <returns>const VkDeviceMemory&</returns>
	virtual const VkDeviceMemory& getNativeObject()const { return _suballocator->_memory->getNativeObject(); }

	/// <summary>Return true if this memory block is mappable by the host (const).</summary>
	/// <returns>bool</returns>
	virtual bool isMappable()const { return _suballocator->_memory->isMappable(); }

	/// <summary>Return the memory flags(const)</summary>
	/// <returns>VkMemoryPropertyFlags</returns>
	virtual VkMemoryPropertyFlags getMemoryFlags()const { return _suballocator->_memory->getMemoryFlags(); }

	/// <summary>Return this mapped memory offset (const)</summary>
	/// <returns>VkDeviceSize</returns
	virtual VkDeviceSize getMappedOffset()const { return _suballocator->_memory->getMappedOffset(); }

	/// <summary>Return this mapped memory size (const)</summary>
	/// <returns>VkDeviceSize</returns
	virtual VkDeviceSize getMappedSize()const { return _suballocator->_memory->getMappedSize(); }

	/// <summary>Return this memory size (const)</summary>
	/// <returns>uint64_t</returns>
	virtual VkDeviceSize getSize()const { return _suballocator->_memory->getSize(); }

	/// <summary>Return true if this memory is being mapped by the host (const)</summary>
	/// <returns>VkDeviceSize</returns
	virtual bool isMapped()const { return _suballocator->_memory->isMapped(); }

	/// <summary>
	/// map this memory. NOTE: Only memory created with HostVisible flag can be mapped and unmapped
	/// </summary>
	/// <param name="mappedMemory">Out mapped memory</param>
	/// <param name="offset">Zero-based byte offset from the beginning of the memory object.</param>
	/// <param name="size">Size of the memory range to map, or VK_WHOLE_SIZE to map from offset to the end of the allocation</param>
	/// <returns>VkResult</returns>
	virtual VkResult map(void** mappedMemory, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
	{
		return _suballocator->_memory->map(mappedMemory, offset + _offset);
	}

	/// <summary>Unmap this memory block</summary>
	virtual void unmap()
	{
		return _suballocator->_memory->unmap();
	}

	/// <summary>Flush ranges of non-coherent memory from the host caches:</summary>
	/// <param name="offset">Zero-based byte offset from the beginning of the memory object</param>
	/// <param name="size">Either the size of range, or VK_WHOLE_SIZE to affect the range from offset to the end of the current mapping of the allocation.</param>
	/// <returns>VkResult</returns>
	virtual VkResult flushRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) { return _suballocator->_memory->flushRange(offset + _offset, size); }

	/// <summary>To invalidate ranges of non-coherent memory from the host caches</summary>
	/// <param name="offset">Zero-based byte offset from the beginning of the memory object.</param>
	/// <param name="size">Either the size of range, or VK_WHOLE_SIZE to affect the range from offset to the end of the current mapping of the allocation.</param>
	/// <returns>VkResult</returns>
	virtual VkResult invalidateRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) { return _suballocator->_memory->invalidateRange(offset + _offset, size); }

	friend class MemorySuballocator_;
	template<typename> friend class EmbeddedRefCount;

	bool isValid() { return _suballocator.isValid(); }
	pvrvk::DeviceMemory memory()
	{
		return _suballocator.isValid() ? _suballocator->_memory : pvrvk::DeviceMemory();
	}
	VkDeviceSize offset() { return _offset; }
	VkDeviceSize size() { return _size; }
	void recycle();
private:
	friend struct pvrvk::IRefCountEntry;
	MemorySuballocation_(MemorySuballocator suballocator, VkDeviceSize size, VkDeviceSize offset):
		_suballocator(suballocator), _size(size), _offset(offset)
	{

	}
	template<typename> friend class pvrvk::RefCountedResource;
	template<typename> friend class pvrvk::EmbeddedRefCountedResource;
	template<typename> friend struct pvrvk::RefCountEntryIntrusive;

	MemorySuballocator _suballocator;
	VkDeviceSize _size;
	VkDeviceSize _offset;
	virtual ~MemorySuballocation_()
	{
		recycle();
	}
};
}
typedef pvrvk::RefCountedResource<impl::MemorySuballocation_> SuballocatedMemory;
namespace impl {
inline void MemorySuballocation_::recycle()
{
	if (_suballocator.isNull())
	{
		return;
	}
	auto& freemem = _suballocator->_freeMemory;
	if (freemem.empty()) // Allocator completely full. Insert this as the only free chunk.
	{
		freemem.push_back(MemorySuballocator_::AllocationPair(offset(), size()));
	}
	else // Allocator not completely full
	{
		for (size_t i = 0; i < freemem.size(); ++i) //Try to find where the chunk must be placed, handling it being contiguous to other chunks.
		{
			auto&& current_chunk = freemem[i];
			if (_offset < current_chunk.start_offset) // This is the place to put it: it must be placed before the chunk under consideration.
			{
				if (_offset + _size == current_chunk.start_offset) // It is actually contiguous to its next chunk - we'll use this!
				{
					current_chunk.start_offset = _offset;
					// Now we must additionally check that we did not introduce extra contiguousness: the new chunk exactly filling a gap with the previous chunk
					if (i > 0 && current_chunk.start_offset == freemem[i - 1].end())
					{
						// It does - so merge them!
						current_chunk.start_offset = freemem[i - 1].start_offset;
						freemem.erase(freemem.begin() + i - 1);
					}
				}
				else // ... No contiguousness with the next chunk
				{
					if (i > 0 && offset() + size() == freemem[i - 1].end()) //Check for contiguousness with the previous chunk
					{
						// Contiguous with the previous chunk:
						freemem[i - 1].size += size();
					}
					else // No contiguousness...
					{
						freemem.insert(freemem.begin(), MemorySuballocator_::AllocationPair(offset(), size()));
					}
				}
				this->_suballocator.reset();
				this->_size = 0;
				this->_offset = 0;
				return;
			}
		}

		if (_offset + _size == freemem.back().end())
		{
			freemem.back().size += _size;
		}
		else
		{
			freemem.push_back(MemorySuballocator_::AllocationPair(offset(), size()));
		}
	}
	this->_suballocator.reset();
	this->_size = 0;
	this->_offset = 0;
}

inline MemorySuballocation MemorySuballocator_::suballocate(VkDeviceSize size)
{
	MemorySuballocation retval;
	auto aligned_size = align(size, _alignment);
	for (size_t i = 0; i < _freeMemory.size(); ++i)
	{
		auto&& chunk = _freeMemory[i];
		if (chunk.size >= aligned_size)
		{
			if (aligned_size)
			{ retval.construct(getReference(), aligned_size, chunk.start_offset); }

			chunk.start_offset += aligned_size;
			chunk.size -= aligned_size;
			if (chunk.size == 0)
			{
				_freeMemory.erase(_freeMemory.begin() + i);
			}
			return retval; //SUCCESSFUL RETURN HERE
		}
	}
	Log("MemorySuballocation: Could not find enough contiguous space for allocation of size %ull (aligned from %ull)", aligned_size, size);
	return retval;
}
}
}
}