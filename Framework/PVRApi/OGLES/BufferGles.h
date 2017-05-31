<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi/OGLES/BufferGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific implementation of the Buffer class. Use only if directly using OpenGL ES calls.
              Provides the definitions allowing to move from the Framework object Buffer to the underlying OpenGL ES Buffer.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Contains OpenGL ES specific implementation of the Buffer class. Use only if directly using OpenGL ES calls.
Provides the definitions allowing to move from the Framework object Buffer to the underlying OpenGL ES Buffer.
\file PVRApi/OGLES/BufferGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvr {
/// <summary>Main PVRApi Namespace</summary>
namespace api {

/// <summary>Contains internal objects and wrapped versions of the PVRApi module</summary>
namespace gles {
/// <summary>OpenGL ES implementation of the Buffer.</summary>
class BufferGles_ : public native::HBuffer_ , public impl::Buffer_
{
<<<<<<< HEAD

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
=======

	struct ES2MemoryMapping
	{
		std::vector<byte> mem;
	};
>>>>>>> 1776432f... 4.3

public:
	GLenum _lastUse;
	GLenum _hint;
	ES2MemoryMapping _es2MemoryMapping;
	// INTERNAL. Use GraphicsContext->createBuffer() instead.
	BufferGles_(const GraphicsContext& context) : Buffer_(context) {}

	void destroy();

	/*!*********************************************************************************************************************
	\brief Destructor. Releases the resources held by this object
	***********************************************************************************************************************/
	~BufferGles_()
	{
		if (_context.isValid())
		{
			destroy();
		}
		else
		{
			Log(Log.Warning, "Buffer object was not released before context destruction");
		}
	}
<<<<<<< HEAD

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
=======
private:
	bool allocate_(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);
	void update_(const void* data, uint32 offset, uint32 length);
	void* map_(types::MapBufferFlags flags, uint32 offset, uint32 length);
	void unmap_();
	bool isAllocated_() const { return _size != 0; }
>>>>>>> 1776432f... 4.3
};

class BufferViewGles_ : public impl::BufferView_, public native::HBufferView_
{
public:
	/// <summary>ctor, Create a buffer View</summary>
	/// <param name="buffer">The buffer of this view</param>
	/// <param name="offset">Offset in to the buffer of this view</param>
	/// <param name="range">Buffer range of this view</param>
	BufferViewGles_(const Buffer& buffer, uint32 offset, uint32 range);
};

/// <summary>OpenGL ES implementation of the Buffer.</summary>
typedef RefCountedResource<BufferGles_> BufferGles;
typedef RefCountedResource<BufferViewGles_> BufferViewGles;
}
}
}
<<<<<<< HEAD
//!\endcond
=======
PVR_DECLARE_NATIVE_CAST(Buffer);
PVR_DECLARE_NATIVE_CAST(BufferView);
>>>>>>> 1776432f... 4.3
