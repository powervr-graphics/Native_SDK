/*!
\brief PVRVk Barrier, Fence, Event, Semaphore classes.
\file PVRVk/SyncVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"
namespace pvrvk {
#if defined(_WIN32)
#undef MemoryBarrier
#endif
/// <summary>A Global memory barrier used for memory accesses for all memory objects.</summary>
struct MemoryBarrier
{
	VkAccessFlags srcMask;//!< Bitmask of VkAccessFlagBits specifying a source access mask.
	VkAccessFlags dstMask;//!< Bitmask of VkAccessFlagBits specifying a destination access mask.
	/// <summary>Constructor, zero initialization</summary>
	MemoryBarrier(): srcMask(VkAccessFlags(0)), dstMask(VkAccessFlags(0)) {}

	/// <summary>Constructor, setting all members</summary>
	/// <param name="srcMask">Bitmask of VkAccessFlagBits specifying a source access mask.</param>
	/// <param name="dstMask">Bitmask of VkAccessFlagBits specifying a destination access mask.</param>
	MemoryBarrier(VkAccessFlags srcMask, VkAccessFlags dstMask):
		srcMask(srcMask), dstMask(dstMask) {}
};

/// <summary>A Buffer memory barrier used only for memory accesses involving a specific range of the specified
/// buffer object. It is also used to transfer ownership of an buffer range from one queue family to another.
/// </summary>
struct BufferMemoryBarrier
{
	VkAccessFlags srcMask;//!< Bitmask of VkAccessFlagBits specifying a source access mask.
	VkAccessFlags dstMask;//!< Bitmask of VkAccessFlagBits specifying a destination access mask.
	Buffer buffer;//!< Handle to the buffer whose backing memory is affected by the barrier.
	uint32_t offset;//!< Offset in bytes into the backing memory for buffer. This is relative to the base offset as bound to the buffer
	uint32_t size;//!< Size in bytes of the affected area of backing memory for buffer, or VK_WHOLE_SIZE to use the range from offset to the end of the buffer.

	/// <summary>Constructor, zero initialization</summary>
	BufferMemoryBarrier() : srcMask(VkAccessFlags(0)), dstMask(VkAccessFlags(0)) {}

	/// <summary>Constructor, individual elementssummary>
	/// <param name="srcMask">Bitmask of VkAccessFlagBits specifying a source access mask.</param>
	/// <param name="dstMask">Bitmask of VkAccessFlagBits specifying a destination access mask.</param>
	/// <param name="buffer">Handle to the buffer whose backing memory is affected by the barrier.</param>
	/// <param name="offset">Offset in bytes into the backing memory for buffer. This is relative to the base offset as bound to the buffer</param>
	/// <param name="size">Size in bytes of the affected area of backing memory for buffer, or VK_WHOLE_SIZE to use the range from offset to the end of the buffer.</param>
	BufferMemoryBarrier(VkAccessFlags srcMask, VkAccessFlags dstMask, Buffer buffer, uint32_t offset, uint32_t size) :
		srcMask(srcMask), dstMask(dstMask), buffer(buffer), offset(offset), size(size) {}
};

/// <summary>A Image memory barrier used only for memory accesses involving a specific subresource range of the
/// specified image object. It is also used to perform a layout transition for an image subresource range, or to
/// transfer ownership of an image subresource range from one queue family to another.</summary>
struct ImageMemoryBarrier
{
	VkAccessFlags srcAccessMask;//!< Bitmask of VkAccessFlagBits specifying a source access mask.
	VkAccessFlags dstAccessMask;//!< Bitmask of VkAccessFlagBits specifying a destination access mask.
	VkImageLayout oldLayout;//!< Old layout in an image layout transition.
	VkImageLayout newLayout;//!< New layout in an image layout transition.
	uint32_t srcQueueFamilyIndex;//!< Source queue family for a queue family ownership transfer.
	uint32_t dstQueueFamilyIndex;//!< Destination queue family for a queue family ownership transfer
	Image image;//!< Handle to the image affected by this barrier
	ImageSubresourceRange subresourceRange;//!< Describes the image subresource range within image that is affected by this barrier

	/// <summary>Constructor. All flags are zero initialized, and family indexes set to -1.</summary>
	ImageMemoryBarrier(): srcAccessMask(VkAccessFlags(0)), dstAccessMask(VkAccessFlags(0)), oldLayout(VkImageLayout::e_UNDEFINED),
		newLayout(VkImageLayout::e_UNDEFINED), srcQueueFamilyIndex((uint32_t) - 1), dstQueueFamilyIndex((uint32_t) - 1) {}

	/// <summary>Constructor. Set all individual elements.</summary>
	/// <param name="srcMask">Bitmask of VkAccessFlagBits specifying a source access mask.</param>
	/// <param name="dstMask">Bitmask of VkAccessFlagBits specifying a destination access mask.</param>
	/// <param name="image">Handle to the image affected by this barrier</param>
	/// <param name="subresourceRange">Describes the image subresource range within image that is affected by this barrier</param>
	/// <param name="oldLayout">Old layout in an image layout transition.</param>
	/// <param name="newLayout">New layout in an image layout transition.</param>
	/// <param name="srcQueueFamilyIndex">Source queue family for a queue family ownership transfer.</param>
	/// <param name="dstQueueFamilyIndex">Destination queue family for a queue family ownership transfer</param>
	ImageMemoryBarrier(VkAccessFlags srcMask, VkAccessFlags dstMask, const Image& image,
	                   const ImageSubresourceRange& subresourceRange, VkImageLayout oldLayout, VkImageLayout newLayout,
	                   uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
		:
		srcAccessMask(srcMask), dstAccessMask(dstMask), image(image),
		oldLayout(oldLayout), newLayout(newLayout),
		srcQueueFamilyIndex(srcQueueFamilyIndex),
		dstQueueFamilyIndex(dstQueueFamilyIndex),
		subresourceRange(subresourceRange)
	{}
};

namespace impl {
/// <summary>Vulkan implementation of the Fence class
/// Fence can be used by the host to determine completion of execution of subimmisions to queues. The host
/// can be polled for the fence signal
/// .</summary>
class Fence_
{
public:
	DECLARE_NO_COPY_SEMANTICS(Fence_)

	/// <summary>Host to wait for this fence to be signaled</summary>
	/// <param name="timeoutNanos">Time out period in nanoseconds</param>
	/// <returns>VkResult</returns>
	VkResult wait(uint64_t timeoutNanos = static_cast<uint64_t>(-1));

	/// <summary>Return true if this fence is signaled</summary>
	/// <returns>VkResult</returns>
	VkResult getStatus();

	/// <summary>Reset this fence</summary>
	void reset();

	/// <summary>Get Device which owns this resource</summary>
	/// <returns>DeviceWeakPtr</returns>
	Device getDevice() { return Device(_device); }

	/// <summary>Get Vulkan handle</summary>
	/// <returns>const VkFence&</returns>
	const VkFence& getNativeObject()const { return _vkFence; }
private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	Fence_(const DeviceWeakPtr& device) : _device(device), _vkFence(VK_NULL_HANDLE) {}
	bool init(VkFenceCreateFlags fenceCreateFlags);
	~Fence_() { destroy(); }
	void destroy();
	VkFence _vkFence;
	DeviceWeakPtr _device;
};

/// <summary>Vulkan implementation of the Semaphore class.
/// Use to "serialize" access between CommandBuffer submissions and /Queues</summary>
class Semaphore_
{
public:
	DECLARE_NO_COPY_SEMANTICS(Semaphore_)

	/// <summary>Get vulkan object</summary>
	/// <returns>const VkSemaphore&</returns>
	const VkSemaphore& getNativeObject()const { return _vkSemaphore; }
private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
	Semaphore_(const DeviceWeakPtr& device) :
		_vkSemaphore(VK_NULL_HANDLE), _device(device) { }

	~Semaphore_() { destroy(); }
	bool init();
	void destroy();
	VkSemaphore _vkSemaphore;
	DeviceWeakPtr _device;
};

/// <summary>Vulkan implementation of the Event class.
/// Event can be used by the host to do fine-grained synchronization of commands, and it
/// can be signalled either from the host (calling set()) or the device (submitting a setEvent() command).</summary>
class Event_
{
public:
	DECLARE_NO_COPY_SEMANTICS(Event_)

	/// <summary>Set this event</summary>
	void set();

	/// <summary>Reset this event</summary>
	void reset();

	/// <summary>Return true if this event is set</summary>
	/// <returns>Return true if this event is set</returns>
	bool isSet();

	/// <summary>Get Vulkan handle</summary>
	/// <returns>const VkEvent&</returns>
	const VkEvent& getNativeObject()const { return _vkEvent; }
private:
	friend class ::pvrvk::impl::Device_;

	Event_(const DeviceWeakPtr& device) :
		_vkEvent(VK_NULL_HANDLE), _device(device)
	{ }
	~Event_() { destroy(); }

	bool init();
	void destroy();

	VkEvent _vkEvent;
	DeviceWeakPtr _device;
};
}//namespace impl

/// <summary>A memory barrier into the command stream. Used to signify that some types of pending operations
/// from before the barrier must have finished before the commands after the barrier start executing.</summary>
class MemoryBarrierSet
{
	MemoryBarrierSet(const MemoryBarrierSet&); //deleted
	MemoryBarrierSet& operator=(const MemoryBarrierSet&); //deleted

	typedef std::vector<MemoryBarrier> MemBarrierContainer;
	MemBarrierContainer memBarriers;
	std::vector<ImageMemoryBarrier> imageBarriers;
	std::vector<BufferMemoryBarrier> bufferBarriers;
public:
	/// <summary>Constructor. Empty barrier</summary>
	MemoryBarrierSet() {}

	/// <summary>Clear this object of all barriers</summary>
	/// <returns>MemoryBarrierSet&</returns>
	MemoryBarrierSet& clearAllBarriers()
	{
		memBarriers.clear();
		imageBarriers.clear();
		bufferBarriers.clear();
		return *this;
	}

	/// <summary>Clear this object of all Memory barriers</summary>
	/// <returns>MemoryBarrierSet&</returns>
	MemoryBarrierSet& clearAllMemoryBarriers()
	{
		memBarriers.clear(); return *this;
	}

	/// <summary>Clear this object of all Buffer barriers</summary>
	/// <returns>MemoryBarrierSet&</returns>
	MemoryBarrierSet& clearAllBufferRangeBarriers()
	{
		bufferBarriers.clear(); return *this;
	}

	/// <summary>Clear this object of all Image barriers</summary>
	/// <returns>MemoryBarrierSet&</returns>
	MemoryBarrierSet& clearAllImageAreaBarriers()
	{
		imageBarriers.clear(); return *this;
	}

	/// <summary>Add a generic Memory barrier.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSet& addBarrier(MemoryBarrier barrier)
	{
		memBarriers.push_back(barrier);
		return *this;
	}

	/// <summary>Add a Buffer Range barrier, signifying that operations on a part of a buffer
	/// must complete before other operations on that part of the buffer execute.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSet& addBarrier(const BufferMemoryBarrier& barrier)
	{
		bufferBarriers.push_back(barrier);
		return *this;
	}

	/// <summary>Add a Buffer Range barrier, signifying that operations on a part of an Image
	/// must complete before other operations on that part of the Image execute.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSet& addBarrier(const ImageMemoryBarrier& barrier)
	{
		imageBarriers.push_back(barrier);
		return *this;
	}

	/// <summary>Get an array of the MemoryBarrier object of this set.</summary>
	/// <returns>All MemoryBarrier objects that this object contains</returns>
	const MemBarrierContainer& getMemoryBarriers() const { return this->memBarriers; }

	/// <summary>Get an array of the Image Barriers of this set.</summary>
	/// <returns>All MemoryBarrier objects that this object contains</returns>
	const std::vector<ImageMemoryBarrier>& getImageBarriers() const { return this->imageBarriers; }

	/// <summary>Get an array of the Buffer Barriers of this set.</summary>
	/// <returns>All MemoryBarrier objects that this object contains</returns>
	const std::vector<BufferMemoryBarrier>& getBufferBarriers() const { return this->bufferBarriers; }
};
}//namespace pvrvk


