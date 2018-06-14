/*!
\brief The PVRVk class, an object wrapping memory that is directly(non-image) accessible to the shaders.
\file PVRVk/BufferVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/DeviceMemoryVk.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvrvk {
/// <summary>Buffer creation descriptor.</summary>
struct BufferCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	BufferCreateInfo()
		: _flags(BufferCreateFlags::e_NONE), _size(0), _sharingMode(SharingMode::e_EXCLUSIVE), _usageFlags(BufferUsageFlags::e_NONE), _numQueueFamilyIndices(0),
		  _queueFamilyIndices(nullptr)
	{}

	/// <summary>Constructor</summary>
	/// <param name="size">The buffer creation size</param>
	/// <param name="usageFlags">The buffer creation usage flags</param>
	/// <param name="flags">The buffer creation flags</param>
	/// <param name="sharingMode">The buffer creation sharing mode</param>
	/// <param name="queueFamilyIndices">A pointer to a list of supported queue families</param>
	/// <param name="numQueueFamilyIndices">The number of supported queue family indices</param>
	BufferCreateInfo(pvrvk::DeviceSize size, pvrvk::BufferUsageFlags usageFlags, pvrvk::BufferCreateFlags flags = pvrvk::BufferCreateFlags::e_NONE,
		SharingMode sharingMode = SharingMode::e_EXCLUSIVE, const uint32_t* queueFamilyIndices = nullptr, uint32_t numQueueFamilyIndices = 0)
		: _flags(flags), _size(size), _sharingMode(sharingMode), _usageFlags(usageFlags), _numQueueFamilyIndices(numQueueFamilyIndices), _queueFamilyIndices(queueFamilyIndices)
	{}

	/// <summary>Get the buffer creation flags</summary>
	/// <returns>The set of buffer creation flags</returns>
	inline BufferCreateFlags getFlags() const
	{
		return _flags;
	}
	/// <summary>Set the buffer creation flags</summary>
	/// <param name="flags">The buffer creation flags</param>
	inline void setFlags(BufferCreateFlags flags)
	{
		this->_flags = flags;
	}

	/// <summary>Get the buffer creation size</summary>
	/// <returns>The set of buffer creation size</returns>
	inline DeviceSize getSize() const
	{
		return _size;
	}
	/// <summary>Set the buffer creation size</summary>
	/// <param name="size">The buffer creation size</param>
	inline void setSize(DeviceSize size)
	{
		this->_size = size;
	}

	/// <summary>Get the buffer creation sharing mode</summary>
	/// <returns>The set of buffer creation sharing mode</returns>
	inline SharingMode getSharingMode() const
	{
		return _sharingMode;
	}
	/// <summary>Set the buffer creation sharing mode</summary>
	/// <param name="sharingMode">The buffer creation sharing mode</param>
	inline void setSharingMode(SharingMode sharingMode)
	{
		this->_sharingMode = sharingMode;
	}

	/// <summary>Get the buffer creation usage flags</summary>
	/// <returns>The set of buffer creation usage flags</returns>
	inline BufferUsageFlags getUsageFlags() const
	{
		return _usageFlags;
	}
	/// <summary>Set the buffer creation usage flags</summary>
	/// <param name="usageFlags">The buffer creation usage flags</param>
	inline void setUsageFlags(BufferUsageFlags usageFlags)
	{
		this->_usageFlags = usageFlags;
	}

	/// <summary>Get the number of queue family indices pointed to</summary>
	/// <returns>The number of queue family indices pointed to</returns>
	inline uint32_t getNumQueueFamilyIndices() const
	{
		return _numQueueFamilyIndices;
	}

	/// <summary>Set the number of queue family indices pointed to</summary>
	/// <param name="numQueueFamilyIndices">The number of queue family indices pointed to</param>
	inline void setNumQueueFamilyIndices(uint32_t numQueueFamilyIndices)
	{
		this->_numQueueFamilyIndices = numQueueFamilyIndices;
	}

	/// <summary>Get this buffer creation infos pointer to a list of supported queue family indices</summary>
	/// <returns>A pointer to a list of supported queue family indices</returns>
	inline const uint32_t* getQueueFamilyIndices() const
	{
		return _queueFamilyIndices;
	}

private:
	/// <summary>Flags to use for creating the image</summary>
	BufferCreateFlags _flags;
	/// <summary>The size of the buffer in bytes</summary>
	DeviceSize _size;
	/// <summary>Specifies the buffer sharing mode specifying how the image can be used by multiple queue families</summary>
	SharingMode _sharingMode;
	/// <summary>describes the buffer's intended usage</summary>
	BufferUsageFlags _usageFlags;
	/// <summary>The number of queue families in the _queueFamilyIndices array</summary>
	uint32_t _numQueueFamilyIndices;
	/// <summary>The list of queue families that will access this image</summary>
	const uint32_t* _queueFamilyIndices;
};

/// <summary>Contains internal objects and wrapped versions of the PVRVk module</summary>
namespace impl {
/// <summary>Vulkan implementation of the Buffer.</summary>
class Buffer_ : public DeviceObjectHandle<VkBuffer>, public DeviceObjectDebugMarker<Buffer_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(Buffer_)

	/// <summary>Return the DeviceMemory bound to this buffer. Note</summary>: only nonsparse buffer can have a bound memory block
	/// <returns>MemoryBlock</returns>
	DeviceMemory getDeviceMemory()
	{
		return _deviceMemory;
	}

	/// <summary>Get this buffer's creation flags</summary>
	/// <returns>The set of buffer creation flags</returns>
	inline BufferCreateFlags getFlags() const
	{
		return _createInfo.getFlags();
	}

	/// <summary>Indicates whether the buffers creation flags includes the given flag</summary>
	/// <param name="flags">A buffer creation flag</param>
	/// <returns>True if the buffers creation flags includes the given flag</returns>
	inline bool hasCreateFlag(pvrvk::BufferCreateFlags flags) const
	{
		return (_createInfo.getFlags() & flags) == flags;
	}

	/// <summary>Indicates whether the buffers usage flags includes the given flag</summary>
	/// <param name="flags">A buffer usage flag</param>
	/// <returns>True if the buffers usage flags includes the given flag</returns>
	inline bool hasUsageFlag(pvrvk::BufferUsageFlags flags) const
	{
		return (_createInfo.getUsageFlags() & flags) == flags;
	}

	/// <summary>Get this buffer's size</summary>
	/// <returns>The size of this buffer</returns>
	inline DeviceSize getSize() const
	{
		return _createInfo.getSize();
	}

	/// <summary>Get this buffer's supported sharing mode</summary>
	/// <returns>A SharingMode structure specifying this buffer's supported sharing mode</returns>
	inline SharingMode getSharingMode() const
	{
		return _createInfo.getSharingMode();
	}

	/// <summary>Get this buffer's supported usage flags</summary>
	/// <returns>A BufferUsageFlags structure specifying this buffer's supported usage flags</returns>
	inline BufferUsageFlags getUsageFlags() const
	{
		return _createInfo.getUsageFlags();
	}

	/// <summary>Get the number of queue families supported by this buffer</summary>
	/// <returns>The size of the list of supported queue family indices</returns>
	inline uint32_t getNumQueueFamilyIndices() const
	{
		return _createInfo.getNumQueueFamilyIndices();
	}

	/// <summary>Get this buffer's pointer to supported queue families</summary>
	/// <returns>A pointer to a list of supported queue family indices</returns>
	inline const uint32_t* getQueueFamilyIndices() const
	{
		return _createInfo.getQueueFamilyIndices();
	}

	/// <summary> Call only on Non-sparse buffer.
	/// Binds a non-sparse memory block. This function must be called once
	/// after this buffer creation. Calling second time don't do anything</summary>
	/// <param name="deviceMemory">Device memory block to bind</param>
	/// <param name="offset"> begin offset in the memory block</param>
	void bindMemory(DeviceMemory deviceMemory, VkDeviceSize offset)
	{
		if (isSparseBuffer())
		{
			throw ErrorValidationFailedEXT("Cannot call bindMemory on a sparse buffer");
		}

		if (_deviceMemory.isValid())
		{
			throw ErrorValidationFailedEXT("Cannot bind a memory block as Buffer already has a memory block bound");
		}
		_memoryOffset = offset;
		_deviceMemory = deviceMemory;

		vkThrowIfFailed(static_cast<Result>(_device->getVkBindings().vkBindBufferMemory(_device->getVkHandle(), getVkHandle(), _deviceMemory->getVkHandle(), offset)),
			"Failed to bind memory to buffer");
	}

	/// <summary>Get this buffer's create flags</summary>
	/// <returns>VkBufferCreateFlags</returns>
	BufferCreateInfo getCreateInfo() const
	{
		return _createInfo;
	}

	/// <summary>Return true if this is a sparse buffer</summary>
	/// <returns>bool</returns>
	bool isSparseBuffer() const
	{
		return (_createInfo.getFlags() & (BufferCreateFlags::e_SPARSE_ALIASED_BIT | BufferCreateFlags::e_SPARSE_BINDING_BIT | BufferCreateFlags::e_SPARSE_RESIDENCY_BIT)) != 0;
	}

	/// <summary>Get thus buffer memory requirements</summary>
	/// <returns>VkMemoryRequirements</returns>
	const MemoryRequirements& getMemoryRequirement() const
	{
		return _memRequirements;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	Buffer_(DeviceWeakPtr device, const BufferCreateInfo& createInfo);

	/// <summary>Destructor. Checks if the device is valid</summary>
	~Buffer_();

	BufferCreateInfo _createInfo;
	MemoryRequirements _memRequirements;
	VkDeviceSize _memoryOffset;
	DeviceMemory _deviceMemory;
};

/// <summary>Vulkan implementation of the Buffer.</summary>
class BufferView_ : public DeviceObjectHandle<VkBufferView>, public DeviceObjectDebugMarker<BufferView_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(BufferView_)

	/// <summary>Return the offset this buffer view pointing in to the buffer</summary>
	/// <returns>VkDeviceSize</returns>
	VkDeviceSize getOffset() const
	{
		return _offset;
	}

	/// <summary>Return the range of this buffer view</summary>
	/// <returns>VkDeviceSize</returns>
	VkDeviceSize getSize() const
	{
		return _size;
	}

	/// <summary>Get the pointing buffer.</summary>
	/// <returns>Buffer</returns>
	Buffer getBuffer()
	{
		return _buffer;
	}

	/// <summary>Get the pointing buffer </summary>(const).
	/// <returns>Buffer</returns>
	const Buffer getBuffer() const
	{
		return _buffer;
	}

	/// <summary>Get buffer view format</summary>
	/// <returns>VkFormat</returns>
	Format getFormat() const
	{
		return _format;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
	BufferView_(const DeviceWeakPtr& device, const Buffer& buffer, Format format, VkDeviceSize offset, VkDeviceSize size);

	void destroy();
	VkDeviceSize _offset;
	VkDeviceSize _size;
	Format _format;
	Buffer _buffer;
};
} // namespace impl
} // namespace pvrvk
