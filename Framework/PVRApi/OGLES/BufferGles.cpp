/*!*********************************************************************************************************************
\file         PVRApi\OGLES\BufferGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the Buffer class. See BufferGles.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/BufferGles.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRNativeApi/ApiErrors.h"
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

// AT THIS POINT, IT IS ASSUMED THAT OUR Buffer_ is, in fact, a BufferGles_. In this sense, to avoid
// virtual function calls, we are de-virtualising Buffer_ into BufferGles. So, for each call, our "this"
// pointer which is a Buffer_*, gets cast into a BufferGles_* and the calls are done direct (in fact,
// inline) through this pointer.
}

const pvr::native::HBufferView_& impl::BufferView_::getNativeObject()const
{
	return static_cast<const gles::BufferViewGles_&>(*this);
}
pvr::native::HBufferView_& impl::BufferView_::getNativeObject()
{
	return static_cast<gles::BufferViewGles_&>(*this);
}

namespace gles {
void BufferGles_::destroy()
{
	if (m_context.isValid())
	{
		gl::DeleteBuffers(1, &getNativeObject().handle);
		debugLogApiError("Buffer_::destroy exit");
	}
	m_context.reset();
}

void BufferViewGles_::destroy() { buffer.reset(); }

void BufferGles_::update(const void* data, uint32 offset, uint32 length)
{
	gl::BindBuffer(m_lastUse, handle);
	if (offset == 0 && length == m_size)
	{
		//Orphan the buffer, if possible...
		//gl::BufferData(m_lastUse, length, NULL, m_hint);
		gl::BufferData(m_lastUse, length, data, m_hint);
	}
	else
	{
		gl::BufferSubData(m_lastUse, offset, length, data);
	}
	debugLogApiError("Buffer_::update exit");
}

inline void* BufferGles_::map_(types::MapBufferFlags flags, uint32 offset, uint32 length)
{
	if (m_memMapped)
	{
		Log("Buffer_::map trying to map memory twice");
		return NULL;
	}
	if (m_context->hasApiCapability(ApiCapabilities::MapBufferRange))
	{
		gl::BindBuffer(m_lastUse, handle);
		void* retval = gl::MapBufferRange(m_lastUse, offset, length, convert_api_type::mapBufferFlags(flags));
		debugLogApiError("Buffer_::map exit");
		m_memMapped = true;
		return retval;
	}
	else if (flags == types::MapBufferFlags::Write)
	{
		m_es2MemoryMapping.mem.resize(length);
		m_memMapped = true;
		m_es2MemoryMapping.offset = offset;
		m_es2MemoryMapping.length = length;
		return (void*)m_es2MemoryMapping.mem.data();
	}
	else
	{
		Log(Log.Error, "BufferGles_::map_ - OGLES2 context does not Support buffer mapping");
		assertion(false, "BufferGles_::map_ - OGLES2 Does not Support buffer mapping");
		return NULL;
	}
}

inline void BufferGles_::unmap_()
{
	if (!m_memMapped)
	{
		Log("Buffer_::unmap trying to un-map un-mapped memory");
		return;
	}
	if (m_context->hasApiCapability(ApiCapabilities::MapBufferRange))
	{
		gl::BindBuffer(m_lastUse, handle);
		gl::UnmapBuffer(m_lastUse);
		debugLogApiError("Buffer_::unmap exit");
		m_memMapped = false;
	}
	else if (m_es2MemoryMapping.length > 0)/* have we have given cpu memory for write operation ?*/
	{
		update(m_es2MemoryMapping.mem.data(), m_es2MemoryMapping.offset, m_es2MemoryMapping.length);
		m_es2MemoryMapping.offset  = m_es2MemoryMapping.length = 0;// reset it
        m_memMapped = false;
	}
	else
	{
		Log(Log.Error, "ES2 Does not Support buffer mapping");
		assertion(false, "ES2 Does not Support buffer mapping");
	}
}

bool BufferGles_::allocate_(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable)
{
	m_size = size;
	m_usage = bufferUsage;
	m_hint = isMappable ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	m_isMappable = isMappable;

	//IMPLEMENT glBufferStorage
	m_lastUse = getGlenumFromBufferUsage(m_usage);
	gl::GenBuffers(1, &handle);
	gl::BindBuffer(m_lastUse, handle);
	gl::BufferData(m_lastUse, size, NULL, m_hint);
	gl::BindBuffer(m_lastUse, 0);
	m_size = size;
	debugLogApiError("Buffer_::allocate exit");
	return true;
}
}// impl
}// api
}// pvr
//!\endcond
