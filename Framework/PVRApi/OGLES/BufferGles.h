/*!*********************************************************************************************************************
\file         PVRApi/OGLES/BufferGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific implementation of the Buffer class. Use only if directly using OpenGL ES calls.
              Provides the definitions allowing to move from the Framework object Buffer to the underlying OpenGL ES Buffer.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
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

	struct ES2MemoryMapping
	{
		std::vector<byte> mem;
		uint32 offset;
		uint32 length;
		ES2MemoryMapping() : offset(0), length(0){}
	};

public:
	GLenum m_lastUse;
	GLenum m_hint;
	ES2MemoryMapping m_es2MemoryMapping;
	bool m_memMapped;
	bool m_isMappable;
	//!\cond NO_DOXYGEN
	// INTERNAL. Use GraphicsContext->createBuffer() instead.
	BufferGles_(GraphicsContext& context) : Buffer_(context), m_memMapped(false) {}
	//!\endcond

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
	void* map_(types::MapBufferFlags flags, uint32 offset, uint32 length);

	/*!*********************************************************************************************************************
	\brief Unmap the buffer
	***********************************************************************************************************************/
	void unmap_();

	/*!*********************************************************************************************************************
	\brief Release the resources held by this object
	***********************************************************************************************************************/
	void destroy();

	/*!*********************************************************************************************************************
	\brief Destructor. Releases the resources held by this object
	***********************************************************************************************************************/
	~BufferGles_()
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

	/*!*********************************************************************************************************************
	\brief Allocate a new buffer
	\param size buffer size, in bytes
	\param bufferUsage A bitfield describing all allowed uses of the buffer
	\param isMappable If set to true, the buffer will be mappable to host-visible memmory. Otherwise, using the map() or
	the update() operation is undefined.
	\details It is reasonable to say that the buffer must be either mappable (to be written to by the host with map/update),
	have the use flag  TransferDst (so its data is written to by a copy buffer or cmdUpdateBuffer), or StorageBuffer so that
	its data is written to by a shader. If none of these three conditions is met there is no way to populate it.
	***********************************************************************************************************************/
	bool allocate_(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);
};

class BufferViewGles_ : public impl::BufferView_, public native::HBufferView_
{
public:
	BufferViewGles_(const Buffer& buffer, uint32 offset, uint32 range) :
		impl::BufferView_(buffer, offset, range) {}

	void destroy();
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
//!\endcond
