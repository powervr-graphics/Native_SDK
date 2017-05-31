/*!
\brief OpenGL ES Implementation of the Buffer class. See BufferGles.h.
\file PVRApi/OGLES/BufferGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/BufferGles.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ContextGles.h"


namespace pvr {
namespace api {
namespace convert_api_type {
GLbitfield mapBufferFlags(types::MapBufferFlags flags)
{
#ifdef GL_MAP_READ_BIT
	static const GLenum mapFlags[] =
	{
		0, GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT,
		GL_MAP_UNSYNCHRONIZED_BIT, GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_READ_BIT,
		GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_WRITE_BIT, GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT,
	};
	return mapFlags[uint32(flags)];
#else
	return 0;
#endif
}
}
namespace {
inline GLenum getGlenumFromBufferUsage(types::BufferBindingUse usage)
{
	if (uint32(usage & types::BufferBindingUse::VertexBuffer) != 0) { return GL_ARRAY_BUFFER; }
	if (uint32(usage & types::BufferBindingUse::IndexBuffer) != 0) { return GL_ELEMENT_ARRAY_BUFFER; }
#if defined (GL_DRAW_INDIRECT_BUFFER)
	if (uint32(usage & types::BufferBindingUse::IndirectBuffer) != 0) { return GL_DRAW_INDIRECT_BUFFER; }
#endif
#if defined(GL_SHADER_STORAGE_BUFFER)
	if (uint32(usage & types::BufferBindingUse::StorageBuffer) != 0) { return GL_SHADER_STORAGE_BUFFER; }
#endif
	if (uint32(usage & types::BufferBindingUse::UniformBuffer) != 0) { return GL_UNIFORM_BUFFER; }
	return GL_ARRAY_BUFFER;
}
}
<<<<<<< HEAD
namespace impl {
void* Buffer_::map(types::MapBufferFlags flags, uint32 offset, uint32 length)
{
	//Safe downcast. We already KNOW that this is our class.
	return static_cast<gles::BufferGles_*>(this)->map_(flags, offset, length == uint32(-1) ? m_size : length);
}

void Buffer_::unmap()
{
	//Safe downcast. We already KNOW that this is our actual type.
	static_cast<gles::BufferGles_*>(this)->unmap_();
}

bool Buffer_::allocate(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable)
{
	//Safe downcast. We already KNOW that this is our class.
	return static_cast<gles::BufferGles_*>(this)->allocate_(size, bufferUsage, isMappable);
}

bool Buffer_::isMappable()const { return static_cast<const gles::BufferGles_*>(this)->m_isMappable; }
bool Buffer_::isMapped()const { return static_cast<const gles::BufferGles_*>(this)->m_memMapped; }

/*!*********************************************************************************************************************
\brief Get the OpenGL ES object underlying a PVRApi Buffer object.
\return A OpenGL ES buffer. For immediate use only.
\description The object returned by this function will only be kept alive as long as there are other references to it.
***********************************************************************************************************************/
native::HBuffer_& impl::Buffer_::getNativeObject()
{
	return static_cast<gles::BufferGles_&>(*this);
}

/*!*********************************************************************************************************************
\brief Get the OpenGL ES object underlying a PVRApi Buffer object.
\return A OpenGL ES buffer. For immediate use only.
\description The object returned by this function will only be kept alive as long as there are other references to it.
***********************************************************************************************************************/
const native::HBuffer_& impl::Buffer_::getNativeObject() const
{
	return static_cast<const gles::BufferGles_&>(*this);
}
=======
>>>>>>> 1776432f... 4.3


namespace gles {
void BufferGles_::destroy()
{
	if (_context.isValid())
	{
		gl::DeleteBuffers(1, &handle);
		debugLogApiError("Buffer_::destroy exit");
	}
	_context.reset();
}

<<<<<<< HEAD
void BufferViewGles_::destroy() { buffer.reset(); }

void BufferGles_::update(const void* data, uint32 offset, uint32 length)
=======
void BufferGles_::update_(const void* data, uint32 offset, uint32 length)
>>>>>>> 1776432f... 4.3
{
	gl::BindBuffer(_lastUse, handle);
	if (offset == 0 && length == _size)
	{
		//Orphan the buffer, if possible...
		//gl::BufferData(_lastUse, length, NULL, _hint);
		gl::BufferData(_lastUse, length, data, _hint);
	}
	else
	{
		gl::BufferSubData(_lastUse, offset, length, data);
	}
	debugLogApiError("Buffer_::update exit");
}

<<<<<<< HEAD
inline void* BufferGles_::map_(types::MapBufferFlags flags, uint32 offset, uint32 length)
=======
void* BufferGles_::map_(types::MapBufferFlags flags, uint32 offset, uint32 length)
>>>>>>> 1776432f... 4.3
{
	if (_mappedRange)
	{
<<<<<<< HEAD
		Log("Buffer_::map trying to map memory twice");
		return NULL;
	}
	if (m_context->hasApiCapability(ApiCapabilities::MapBufferRange))
=======
		assertion(false, "BufferGles_::map trying to map memory twice");
		return NULL;
	}
	if (_context->hasApiCapability(ApiCapabilities::MapBufferRange))
>>>>>>> 1776432f... 4.3
	{
		gl::BindBuffer(_lastUse, handle);
		void* retval = gl::MapBufferRange(_lastUse, offset, length, convert_api_type::mapBufferFlags(flags));
		debugLogApiError("Buffer_::map exit");
<<<<<<< HEAD
		m_memMapped = true;
=======
		if (retval != NULL)
		{
			_mappedRange = length;
			_mappedOffset = offset;
			_mappedFlags = flags;
		}
>>>>>>> 1776432f... 4.3
		return retval;
	}
	else if (flags == types::MapBufferFlags::Write)
	{
<<<<<<< HEAD
		m_es2MemoryMapping.mem.resize(length);
		m_memMapped = true;
		m_es2MemoryMapping.offset = offset;
		m_es2MemoryMapping.length = length;
		return (void*)m_es2MemoryMapping.mem.data();
	}
	else
	{
		Log(Log.Error, "BufferGles_::map_ - OGLES2 context does not Support buffer mapping");
=======
		_es2MemoryMapping.mem.resize(length);
		_mappedRange = length;
		_mappedOffset = offset;
		_mappedFlags = flags;
		return (void*)_es2MemoryMapping.mem.data();
	}
	else
	{
		Log(Log.Error, "BufferGles_::map_ - OGLES2 context does not Support buffer mapping. You can still map for write only.");
>>>>>>> 1776432f... 4.3
		assertion(false, "BufferGles_::map_ - OGLES2 Does not Support buffer mapping");
		return NULL;
	}
}

void BufferGles_::unmap_()
{
	if (!_mappedRange)
	{
<<<<<<< HEAD
		Log("Buffer_::unmap trying to un-map un-mapped memory");
		return;
	}
	if (m_context->hasApiCapability(ApiCapabilities::MapBufferRange))
=======
		assertion(false, "Buffer_::unmap trying to un-map un-mapped memory");
		return;
	}
	_mappedRange = 0;
	_mappedOffset = 0;
	_mappedFlags = types::MapBufferFlags(0);

	if (_context->hasApiCapability(ApiCapabilities::MapBufferRange))
>>>>>>> 1776432f... 4.3
	{
		gl::BindBuffer(_lastUse, handle);
		gl::UnmapBuffer(_lastUse);
		debugLogApiError("Buffer_::unmap exit");
		m_memMapped = false;
	}
	else if (m_es2MemoryMapping.length > 0)/* have we have given cpu memory for write operation ?*/
	{
		update(m_es2MemoryMapping.mem.data(), m_es2MemoryMapping.offset, m_es2MemoryMapping.length);
		m_es2MemoryMapping.offset  = m_es2MemoryMapping.length = 0;// reset it
        m_memMapped = false;
	}
	else if (_es2MemoryMapping.mem.size() > 0)/* have we have given cpu memory for write operation ?*/
	{
		debug_assertion(_es2MemoryMapping.mem.size() == _mappedRange, "ES2 MEMORY MAPPING FAILED");
		update_(_es2MemoryMapping.mem.data(), _mappedOffset, _mappedRange);
		_es2MemoryMapping.mem.resize(0);
	}
	else
	{
		Log(Log.Error, "ES2 Does not Support buffer mapping");
		assertion(false, "ES2 Does not Support buffer mapping");
	}
}

bool BufferGles_::allocate_(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable)
{
<<<<<<< HEAD
	m_size = size;
	m_usage = bufferUsage;
	m_hint = isMappable ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	m_isMappable = isMappable;
=======
	if(size == 0)
	{
		assertion(size != 0, "Failed to allocate buffer. Allocation size should not be 0");
		return false;
	}
	_size = size;
	_usage = bufferUsage;
	_hint = isMappable ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	_isMappable = isMappable;
>>>>>>> 1776432f... 4.3

	//IMPLEMENT glBufferStorage
	_lastUse = getGlenumFromBufferUsage(_usage);
	gl::GenBuffers(1, &handle);
<<<<<<< HEAD
	gl::BindBuffer(m_lastUse, handle);
	gl::BufferData(m_lastUse, size, NULL, m_hint);
	gl::BindBuffer(m_lastUse, 0);
	m_size = size;
=======
	gl::BindBuffer(_lastUse, handle);
	gl::BufferData(_lastUse, size, NULL, _hint);
	gl::BindBuffer(_lastUse, 0);
	_size = size;
>>>>>>> 1776432f... 4.3
	debugLogApiError("Buffer_::allocate exit");
	return true;
}

BufferViewGles_::BufferViewGles_(const Buffer& buffer, uint32 offset, uint32 range) :
	impl::BufferView_(buffer, offset, range) {}

}// impl
}// api
}// pvr
