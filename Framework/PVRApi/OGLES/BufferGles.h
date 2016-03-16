/*!*********************************************************************************************************************
\file         PVRApi/OGLES/BufferGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific implementation of the Buffer class. Use only if directly using OpenGL ES calls.
              Provides the definitions allowing to move from the Framework object Buffer to the underlying OpenGL ES Buffer.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"

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
namespace gles {
/*!*********************************************************************************************************************
\brief OpenGL ES implementation of the Buffer.
***********************************************************************************************************************/
class BufferGles_ : public native::HBuffer_ , public impl::Buffer_
{
public:
	GLenum m_lastUse;
	GLenum m_hint;
	bool m_memMapped;
	/*!*********************************************************************************************************************
	\brief ctor, create buffer on device.
	\param context The graphics context
	\param size buffer size in bytes.
	\param bufferUsage how this buffer will be used for. e.g VertexBuffer, IndexBuffer.
	\param hints What kind of access will be done (GPU Read, GPU Write, CPU Write, Copy etc)
	***********************************************************************************************************************/
	BufferGles_(GraphicsContext& context) : Buffer_(context), m_memMapped(false){}

	/*!*********************************************************************************************************************
	\brief Update the buffer.
	\param data Pointer to the data that will be copied to the buffer
	\param offset offset in the buffer to update
	\param length length of the buffer to update
	***********************************************************************************************************************/
	void update_(const void* data, uint32 offset, uint32 length);

	/*!*********************************************************************************************************************
	\brief Map the buffer.
	\param flags
	\param offset offset in the buffer to map
	\param length length of the buffer to map
	***********************************************************************************************************************/
	void* map_(types::MapBufferFlags::Enum flags, uint32 offset, uint32 length);

	/*!*********************************************************************************************************************
	\brief Unmap the buffer
	***********************************************************************************************************************/
	void unmap_();

	/*!*********************************************************************************************************************
	\brief Allocate a new buffer on the \p context GraphicsContext
	\param context The graphics context
	\param size buffer size, in bytes
	\param hint The expected use of the buffer (CPU Read, GPU write etc)
	***********************************************************************************************************************/
	bool allocate_(uint32 size,types::BufferBindingUse::Bits bufferUsage,
				  types::BufferUse::Flags hint = types::BufferUse::DEFAULT);
};

class BufferViewGles_ : public impl::BufferView_, public native::HBufferView_
{
public:
	BufferViewGles_(const Buffer &buffer, uint32 offset, uint32 range) :
		impl::BufferView_(buffer,offset,range){}
};

/*!*********************************************************************************************************************
\brief OpenGL ES implementation of the Buffer.
***********************************************************************************************************************/
typedef RefCountedResource<BufferGles_> BufferGles;
typedef RefCountedResource<BufferViewGles_> BufferViewGles;
}
}

/*!*********************************************************************************************************************
\brief Contains functions and classes for manipulating the underlying API and getting to the underlying API objects
***********************************************************************************************************************/
namespace native {
/*!*********************************************************************************************************************
\brief Get the OpenGL ES object underlying a PVRApi Buffer object.
\return A smart pointer wrapper containing the OpenGL ES Buffer
\description If the smart pointer returned by this function is kept alive, it will keep alive the underlying OpenGL ES
			object even if all other references to the buffer (including the one that was passed to this function)
			are released.
***********************************************************************************************************************/
inline RefCountedResource<HBuffer_> createNativeHandle(const RefCountedResource<api::impl::Buffer_>& buffer)
{
	return static_cast<RefCountedResource<native::HBuffer_>/**/>(static_cast<RefCountedResource<api::gles::BufferGles_>/**/>(buffer));
}

}
}
