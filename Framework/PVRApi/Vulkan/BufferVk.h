/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/BufferVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the Buffer class. Use only if directly using Vulkan calls.
              Provides the definitions allowing to move from the Framework object Buffer to the underlying Vulkan Buffer.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN

#pragma once
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"

/*!*********************************************************************************************************************
\brief Main PowerVR Framework Namespace
***********************************************************************************************************************/
namespace pvr {
/*!*********************************************************************************************************************
\brief Main PVRApi Namespace
***********************************************************************************************************************/
namespace api {

/*!*********************************************************************************************************************
\brief Contains internal objects and wrapped versions of the PVRApi module
***********************************************************************************************************************/
namespace vulkan {
/*!*********************************************************************************************************************
\brief Vulkan implementation of the Buffer.
***********************************************************************************************************************/
class BufferVk_ : public native::HBuffer_ , public impl::Buffer_
{
public:

	/*!*********************************************************************************************************************
	\brief ctor, create buffer on device.
	\param context The graphics context
	\param size buffer size in bytes.
	\param bufferUsage how this buffer will be used for. e.g VertexBuffer, IndexBuffer.
	\param hints What kind of access will be done (GPU Read, GPU Write, CPU Write, Copy etc)
	***********************************************************************************************************************/
	BufferVk_(GraphicsContext& context);

	/*!*********************************************************************************************************************
	\brief dtor, release all resources
	***********************************************************************************************************************/
	virtual ~BufferVk_();

	/*!*********************************************************************************************************************
	\brief Map the buffer.
	\param flags
	\param offset offset in the buffer to map
	\param length length of the buffer to map
	\description Only buffer created for host access can be mapped/ un-mapped. A buffer that created on device local memory cannot be mapped/ un-mapped.
	***********************************************************************************************************************/
	void* map(types::MapBufferFlags flags, uint32 offset, uint32 length);

	/*!*********************************************************************************************************************
	\brief Unmap the buffer
	\description Only buffer created for host access can be mapped/ un-mapped. A buffer that created on device local memory cannot be mapped/ un-mapped.
	***********************************************************************************************************************/
	void unmap();

	/*!*********************************************************************************************************************
	\brief Allocate a new buffer on the \p context GraphicsContext
	\param context The graphics context
	\param size buffer size, in bytes
	\param hint The expected use of the buffer (CPU Read, GPU write etc)
	***********************************************************************************************************************/
	bool allocate(uint32 size, types::BufferBindingUse usage, bool isMappable);

	/*!*********************************************************************************************************************
	\brief Check whether the buffer has been allocated.
	\return Return true if the buffer is allocated.
	***********************************************************************************************************************/
	bool isAllocated() { return buffer != VK_NULL_HANDLE; }

    /*!
       \brief Return true if this buffer object is mappable by the host.
     */
	bool isMappable() const { return m_isMappable; }

    /*!
       \brief Return true if this buffer object memory is mapped.
     */
	bool isMapped() const { return m_mappedRange != 0; }

	/*!*********************************************************************************************************************
	\brief Destroy this buffer.
	***********************************************************************************************************************/
	void destroy();

    uint32 m_mappedRange;
	uint32 m_mappedOffset;
	types::MapBufferFlags m_mappedFlags;
	bool m_isMappable;
};


/*!*********************************************************************************************************************
\brief Vulkan implementation of the Buffer.
***********************************************************************************************************************/
class BufferViewVk_ : public impl::BufferView_
{
public:
	/*!*********************************************************************************************************************
	\brief ctor, Create a buffer View
	\param buffer The buffer of this view
	\param offset Offset in to the buffer of this view
	\param range Buffer range of this view
	***********************************************************************************************************************/
	BufferViewVk_(const Buffer& buffer, uint32 offset, uint32 range);
};

typedef RefCountedResource<BufferViewVk_> BufferViewVk;
typedef RefCountedResource<BufferVk_> BufferVk;
}// namespace vulkan
}

/*!*********************************************************************************************************************
\brief Contains functions and classes for manipulating the underlying API and getting to the underlying API objects
***********************************************************************************************************************/
namespace native {
/*!*********************************************************************************************************************
\brief Get the Vulkan object underlying a PVRApi Buffer object.
\return A smart pointer wrapper containing the Vulkan Buffer
\description If the smart pointer returned by this function is kept alive, it will keep alive the underlying Vulkan
object even if all other references to the buffer (including the one that was passed to this function)
are released.
***********************************************************************************************************************/
inline const HBuffer_& native_cast(const api::impl::Buffer_& buffer)
{
	return static_cast<const api::vulkan::BufferVk_&>(buffer);
}

/*!*********************************************************************************************************************
\brief Get the Vulkan object underlying a PVRApi Buffer object.
\return A smart pointer wrapper containing the Vulkan Buffer
\description If the smart pointer returned by this function is kept alive, it will keep alive the underlying Vulkan
object even if all other references to the buffer (including the one that was passed to this function)
are released.
***********************************************************************************************************************/
inline HBuffer_& native_cast(api::impl::Buffer_& buffer)
{
	return static_cast<api::vulkan::BufferVk_&>(buffer);
}

/*!*********************************************************************************************************************
\brief Get the Vulkan object underlying a PVRApi Buffer object.
\return A smart pointer wrapper containing the Vulkan Buffer
\description If the smart pointer returned by this function is kept alive, it will keep alive the underlying Vulkan
object even if all other references to the buffer (including the one that was passed to this function)
are released.
***********************************************************************************************************************/
inline const HBuffer_& native_cast(const api::Buffer& buffer)
{
	return static_cast<const api::vulkan::BufferVk_&>(*buffer);
}

/*!*********************************************************************************************************************
\brief Get the Vulkan object underlying a PVRApi Buffer object.
\return A smart pointer wrapper containing the Vulkan Buffer
\description If the smart pointer returned by this function is kept alive, it will keep alive the underlying Vulkan
object even if all other references to the buffer (including the one that was passed to this function)
are released.
***********************************************************************************************************************/
inline HBuffer_& native_cast(api::Buffer& buffer)
{
	return static_cast<api::vulkan::BufferVk_&>(*buffer);
}

}
}

PVR_DECLARE_NATIVE_CAST(Buffer)
PVR_DECLARE_NATIVE_CAST(BufferView)

//!\endcond