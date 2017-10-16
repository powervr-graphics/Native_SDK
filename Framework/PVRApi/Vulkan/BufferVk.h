/*!
\brief Contains Vulkan specific implementation of the Buffer class. Use only if directly using Vulkan calls. Provides
the definitions allowing to move from the Framework object Buffer to the underlying Vulkan Buffer.
\file PVRApi/Vulkan/BufferVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvr {
/// <summary>Main PVRApi Namespace</summary>
namespace api {

/// <summary>Contains internal objects and wrapped versions of the PVRApi module</summary>
namespace vulkan {
/// <summary>Vulkan implementation of the Buffer.</summary>
class BufferVk_ : public native::HBuffer_, public impl::Buffer_
{
public:
	BufferVk_(const GraphicsContext& context);

	virtual ~BufferVk_();
	void destroy();

private:
	void* map_(types::MapBufferFlags flags, uint32 offset, uint32 length);
	void unmap_();
	void update_(const void* data, uint32 offset, uint32 length)
	{
		assertion(length + offset <= _size);
		void* mapData = map(types::MapBufferFlags::Write, offset, length);
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

	bool allocate_(uint32 size, types::BufferBindingUse usage, bool isMappable);
	bool isAllocated_() const { return buffer != VK_NULL_HANDLE; }

};


/// <summary>Vulkan implementation of the Buffer.</summary>
class BufferViewVk_ : public impl::BufferView_
{
public:
	/// <summary>ctor, Create a buffer View</summary>
	/// <param name="buffer">The buffer of this view</param>
	/// <param name="offset">Offset in to the buffer of this view</param>
	/// <param name="range">Buffer range of this view</param>
	BufferViewVk_(const Buffer& buffer, uint32 offset, uint32 range);
};

typedef RefCountedResource<BufferViewVk_> BufferViewVk;
typedef RefCountedResource<BufferVk_> BufferVk;
}// namespace vulkan
}

/// <summary>Contains functions and classes for manipulating the underlying API and getting to the underlying API
/// objects</summary>
namespace native {
/// <summary>Get the Vulkan object underlying a PVRApi Buffer object.</summary>
/// <returns>A smart pointer wrapper containing the Vulkan Buffer</returns>
/// <remarks>If the smart pointer returned by this function is kept alive, it will keep alive the underlying
/// Vulkan object even if all other references to the buffer (including the one that was passed to this function)
/// are released.</remarks>
inline const HBuffer_& native_cast(const api::impl::Buffer_& buffer)
{
	return static_cast<const api::vulkan::BufferVk_&>(buffer);
}

/// <summary>Get the Vulkan object underlying a PVRApi Buffer object.</summary>
/// <returns>A smart pointer wrapper containing the Vulkan Buffer</returns>
/// <remarks>If the smart pointer returned by this function is kept alive, it will keep alive the underlying
/// Vulkan object even if all other references to the buffer (including the one that was passed to this function)
/// are released.</remarks>
inline HBuffer_& native_cast(api::impl::Buffer_& buffer)
{
	return static_cast<api::vulkan::BufferVk_&>(buffer);
}

/// <summary>Get the Vulkan object underlying a PVRApi Buffer object.</summary>
/// <returns>A smart pointer wrapper containing the Vulkan Buffer</returns>
/// <remarks>If the smart pointer returned by this function is kept alive, it will keep alive the underlying
/// Vulkan object even if all other references to the buffer (including the one that was passed to this function)
/// are released.</remarks>
inline const HBuffer_& native_cast(const api::Buffer& buffer)
{
	return static_cast<const api::vulkan::BufferVk_&>(*buffer);
}

/// <summary>Get the Vulkan object underlying a PVRApi Buffer object.</summary>
/// <returns>A smart pointer wrapper containing the Vulkan Buffer</returns>
/// <remarks>If the smart pointer returned by this function is kept alive, it will keep alive the underlying
/// Vulkan object even if all other references to the buffer (including the one that was passed to this function)
/// are released.</remarks>
inline HBuffer_& native_cast(api::Buffer& buffer)
{
	return static_cast<api::vulkan::BufferVk_&>(*buffer);
}

}
}

PVR_DECLARE_NATIVE_CAST(Buffer)
PVR_DECLARE_NATIVE_CAST(BufferView)


