/*!*********************************************************************************************************************
\file         PVRApi/OGLES/BufferGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific implementation of the Buffer class. Use only if directly using OpenGL ES calls.
              Provides the definitions allowing to move from the Framework object Buffer to the underlying OpenGL ES Buffer.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/ApiObjects/GraphicsStateCreateParam.h"
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
namespace impl {
/*!*********************************************************************************************************************
\brief OpenGL ES implementation of the Buffer.
***********************************************************************************************************************/
class BufferGlesImpl : public native::HBuffer_ , public BufferImpl
{
public:

	/*!*********************************************************************************************************************
	\brief ctor, create buffer on device.
	\param context The graphics context
	\param size buffer size in bytes.
	\param bufferUsage how this buffer will be used for. e.g VertexBuffer, IndexBuffer.
	\param hints What kind of access will be done (GPU Read, GPU Write, CPU Write, Copy etc)
	***********************************************************************************************************************/
	BufferGlesImpl(GraphicsContext& context, uint32 size, BufferBindingUse::Bits bufferUsage,
                   BufferUse::Flags hints) : BufferImpl(context, size, bufferUsage, hints)
	{
		allocate(m_context, size, hints);
	}

	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	~BufferGlesImpl();

	/*!*********************************************************************************************************************
	\brief Update the buffer.
	\param data Pointer to the data that will be copied to the buffer
	\param offset offset in the buffer to update
	\param length length of the buffer to update
	***********************************************************************************************************************/
	void update(const void* data, uint32 offset, uint32 length);

	/*!*********************************************************************************************************************
	\brief Map the buffer.
	\param flags
	\param offset offset in the buffer to map
	\param length length of the buffer to map
	***********************************************************************************************************************/
	void* map(MapBufferFlags::Enum flags, uint32 offset, uint32 length);

	/*!*********************************************************************************************************************
	\brief Unmap the buffer
	***********************************************************************************************************************/
	void unmap();

	/*!*********************************************************************************************************************
	\brief Allocate a new buffer on the \p context GraphicsContext
	\param context The graphics context
	\param size buffer size, in bytes
	\param hint The expected use of the buffer (CPU Read, GPU write etc)
	***********************************************************************************************************************/
	virtual void allocate(GraphicsContext& context, uint32 size, BufferUse::Flags hint = BufferUse::DEFAULT);
};
}

/*!*********************************************************************************************************************
\brief OpenGL ES implementation of the Buffer.
***********************************************************************************************************************/
typedef RefCountedResource<impl::BufferGlesImpl> BufferGles;
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
inline RefCountedResource<HBuffer_> getNativeHandle(const RefCountedResource<api::impl::BufferImpl>& buffer)
{
	return static_cast<RefCountedResource<native::HBuffer_>/**/>(static_cast<RefCountedResource<api::impl::BufferGlesImpl>/**/>(buffer));
}

/*!*********************************************************************************************************************
\brief Get the OpenGL ES object underlying a PVRApi Buffer object.
\return A OpenGL ES buffer. For immediate use only.
\description The object returned by this function will only be kept alive as long as there are other references to it.
***********************************************************************************************************************/
inline HBuffer_::NativeType useNativeHandle(const RefCountedResource<api::impl::BufferImpl>& buffer)
{
	return static_cast<const api::impl::BufferGlesImpl&>(*buffer).handle;
}

}
}
