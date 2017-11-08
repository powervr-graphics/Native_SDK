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
/// <summary>Contains internal objects and wrapped versions of the PVRVk module</summary>
namespace impl {
/// <summary>Vulkan implementation of the Buffer.</summary>
class Buffer_
{
public:
	DECLARE_NO_COPY_SEMANTICS(Buffer_)

	/// <summary>Return the DeviceMemory bound to this buffer. Note</summary>: only nonsparse buffer can have a bound memory block
	/// <returns>MemoryBlock</returns>
	DeviceMemory getDeviceMemory() { return _deviceMemory; }

	/// <summary> Call only on Non-sparse buffer.
	/// Binds a non-sparse memory block. This function must be called once
	/// after this buffer creation. Calling second time don't do anything</summary>
	/// <param name="deviceMemory">Device memory block to bind</param>
	/// <param name="offset"> begin offset in the memory block</param>
	/// <returns>VkResult::e_SUCCESS on success, otherwise according to spec.</returns>
	VkResult bindMemory(DeviceMemory deviceMemory, VkDeviceSize offset)
	{
		if (isSparseBuffer())
		{
			Log(LogLevel::Error, "Cannot bind non-sparse memory block: 0x%ullx to sparse buffer 0x%ullx", deviceMemory->getNativeObject(), _vkBuffer);
			return VkResult::e_ERROR_VALIDATION_FAILED_EXT;
		}

		if (_deviceMemory.isValid())
		{
			Log("memory block is already bound to this Buffer object");
			Log(LogLevel::Error, "Cannot bind memory block: 0x%ullx as buffer: 0x%ullx already has device memory backing", deviceMemory->getNativeObject(), _vkBuffer);
			return VkResult::e_ERROR_VALIDATION_FAILED_EXT;
		}
		_memoryOffset = offset;
		_deviceMemory = deviceMemory;
		return vk::BindBufferMemory(_device->getNativeObject(), _vkBuffer,
		                            _deviceMemory->getNativeObject(), offset);
	}

	/// <summary>Get the allowed BufferBindingUse flags for the specified buffer</summary>
	/// <returns>The allowed BufferBindingUse flags for the specified buffer</returns>
	VkBufferUsageFlags getBufferUsage()const { return _usage; }

	/// <summary>Get the total size of the buffer.</summary>
	/// <returns>VkDeviceSize</returns>
	VkDeviceSize getSize() const { return _size; }

	/// <summary>Get vulkan object (const)</summary>
	/// <returns>VkBuffer</returns>
	const VkBuffer& getNativeObject()const { return _vkBuffer; }

	/// <summary>Get this buffer's create flags</summary>
	/// <returns>VkBufferCreateFlags</returns>
	VkBufferCreateFlags getCreateFlags()const
	{
		return _createFlags;
	}

	/// <summary>Return true if this is a sparse buffer</summary>
	/// <returns>bool</returns>
	bool isSparseBuffer()const
	{
		return (_createFlags & (VkBufferCreateFlags::e_SPARSE_ALIASED_BIT | VkBufferCreateFlags::e_SPARSE_BINDING_BIT | VkBufferCreateFlags::e_SPARSE_RESIDENCY_BIT)) != 0;
	}

	/// <summary>Get thus buffer memory requirements</summary>
	/// <returns>VkMemoryRequirements</returns>
	const VkMemoryRequirements& getMemoryRequirement()const { return _memRequirements; }

	/// <summary>Get Device</summary>
	/// <returns>DeviceWeakPtr</returns>
	DeviceWeakPtr getDevice()const
	{
		return _device;
	}

private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	bool init(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags bufferCreateFlags,
	          bool sharingExclusive, const uint32_t* queueFamilyIndices, uint32_t numQueueFamilyIndices);

	Buffer_(DeviceWeakPtr device) :
		_size(0), _usage(VkBufferUsageFlags(0)), _device(device),
		_createFlags(VkBufferCreateFlags(0)), _vkBuffer(VK_NULL_HANDLE)
	{}

	/// <summary>Destructor. Checks if the device is valid</summary>
	~Buffer_()
	{
		destroy();
	}

	/// <summary>Release Vulkan objects of this buffer</summary>
	void destroy();

	VkDeviceSize _size;
	VkBufferUsageFlags _usage;
	DeviceWeakPtr    _device;
	VkMemoryRequirements _memRequirements;
	VkDeviceSize            _memoryOffset;
	VkBufferCreateFlags     _createFlags;
	DeviceMemory _deviceMemory;
	VkBuffer                _vkBuffer;
};

/// <summary>Vulkan implementation of the Buffer.</summary>
class BufferView_
{
public:
	DECLARE_NO_COPY_SEMANTICS(BufferView_)

	~BufferView_() { release(); }

	/// <summary>Return the offset this buffer view pointing in to the buffer</summary>
	/// <returns>VkDeviceSize</returns>
	VkDeviceSize getOffset() const { return _offset; }

	/// <summary>Return the range of this buffer view</summary>
	/// <returns>VkDeviceSize</returns>
	VkDeviceSize getSize() const { return _size; }

	/// <summary>Get Vulkan object </summary>
	/// <returns>VkBufferView</returns>
	VkBufferView& getNativeObject() { return _vkBufferView; }

	/// <summary>Get Vulkan object </summary>(const)
	/// <returns>VkBufferView</returns>
	const VkBufferView& getNativeObject()const { return _vkBufferView; }

	/// <summary>Get the pointing buffer.</summary>
	/// <returns>Buffer</returns>
	Buffer getBuffer() { return _buffer; }

	/// <summary>Get the pointing buffer </summary>(const).
	/// <returns>Buffer</returns>
	const Buffer getBuffer()const { return _buffer; }

	/// <summary>Get buffer view format</summary>
	/// <returns>VkFormat</returns>
	VkFormat getFormat()const { return _format; }

private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
	BufferView_(const DeviceWeakPtr& device, const Buffer& buffer,
	            VkFormat format, VkDeviceSize offset, VkDeviceSize size) :
		_offset(offset), _size(size), _format(format),  _buffer(buffer),
		_vkBufferView(VK_NULL_HANDLE), _device(device) {}

	bool init();

	void release();
	VkDeviceSize _offset;
	VkDeviceSize _size;
	VkFormat      _format;
	Buffer          _buffer;
	VkBufferView    _vkBufferView;
	DeviceWeakPtr _device;
};
}// namespace impl
}// namespace pvrvk
