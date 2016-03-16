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
GLbitfield mapBufferFlags(types::MapBufferFlags::Enum flags)
{
#ifdef GL_MAP_READ_BIT
	static const GLenum mapFlags[] =
	{
		0, GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT,
		GL_MAP_UNSYNCHRONIZED_BIT, GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_READ_BIT,
		GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_WRITE_BIT, GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT,
	};
	return mapFlags[flags];
#else
	return 0;
#endif
}
}
namespace {
inline GLenum getGlenumFromBufferUsage(types::BufferBindingUse::Bits usage)
{
	if (usage & types::BufferBindingUse::VertexBuffer) { return GL_ARRAY_BUFFER; }
	if (usage & types::BufferBindingUse::IndexBuffer) { return GL_ELEMENT_ARRAY_BUFFER; }
#if defined (GL_DRAW_INDIRECT_BUFFER)
	if (usage & types::BufferBindingUse::IndirectBuffer) { return GL_DRAW_INDIRECT_BUFFER; }
#endif
#if defined(GL_SHADER_STORAGE_BUFFER)
	if (usage & types::BufferBindingUse::StorageBuffer) { return GL_SHADER_STORAGE_BUFFER; }
#endif
	if (usage & types::BufferBindingUse::UniformBuffer) { return GL_UNIFORM_BUFFER; }
	return types::BufferBindingUse::VertexBuffer;
}
}
namespace impl {
void* Buffer_::map(types::MapBufferFlags::Enum flags, uint32 offset, uint32 length)
{
	//Safe downcast. We already KNOW that this is our class.
	return static_cast<gles::BufferGles_*>(this)->map_(flags, offset, length);
}

void Buffer_::unmap()
{
	//Safe downcast. We already KNOW that this is our actual type.
	static_cast<gles::BufferGles_*>(this)->unmap_();
}

void Buffer_::update(const void* data, uint32 offset, uint32 length)
{
	//Safe downcast. We already KNOW that this is our class.
	return static_cast<gles::BufferGles_*>(this)->update_(data, offset, length);
}

bool Buffer_::allocate(uint32 size, types::BufferBindingUse::Bits bufferUsage, types::BufferUse::Flags hint)
{
	//Safe downcast. We already KNOW that this is our class.
	return static_cast<gles::BufferGles_*>(this)->allocate_(size, bufferUsage, hint);
}

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

void impl::Buffer_::destroy()
{
	if (m_context.isValid())
	{
		gl::DeleteBuffers(1, &getNativeObject().handle);
		debugLogApiError("Buffer_::destroy exit");
	}
	else
	{
		Log(Log.Warning, "Buffer object was not released before context destruction");
	}
	m_context.release();
}

void impl::BufferView_::destroy() {	buffer.reset(); }

const pvr::native::HBufferView_& impl::BufferView_::getNativeObject()const
{
	return static_cast<const gles::BufferViewGles_&>(*this);
}
pvr::native::HBufferView_& impl::BufferView_::getNativeObject()
{
	return static_cast<gles::BufferViewGles_&>(*this);
}

namespace gles {
inline void BufferGles_::update_(const void* data, uint32 offset, uint32 length)
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

inline void* BufferGles_::map_(types::MapBufferFlags::Enum flags, uint32 offset, uint32 length)
{
	if (m_memMapped)
	{
		debugLogApiError("Buffer_::map trying to map memory twice");
		return NULL;
	}
	m_memMapped = true;
	if (m_context->hasApiCapability(ApiCapabilities::MapBuffer))
	{
		gl::BindBuffer(m_lastUse, handle);
		void* retval = gl::MapBufferRange(m_lastUse, offset, length, convert_api_type::mapBufferFlags(flags));
		debugLogApiError("Buffer_::map exit");
		return retval;
	}
	else
	{
		Log(Log.Error, "OGL ES2 context does not Support buffer mapping");
		assertion(false, "OGL ES2 Does not Support buffer mapping");
		return NULL;
	}
}

inline void BufferGles_::unmap_()
{
	if (!m_memMapped)
	{
		debugLogApiError("Buffer_::unmap trying to un-map un-mapped memory");
		return;
	}
	m_memMapped = false;
	if (m_context->hasApiCapability(ApiCapabilities::MapBuffer))
	{
		gl::BindBuffer(m_lastUse, handle);
		gl::UnmapBuffer(m_lastUse);
		debugLogApiError("Buffer_::unmap exit");
	}
	else
	{
		Log(Log.Error, "ES2 Does not Support buffer mapping");
		assertion(false, "ES2 Does not Support buffer mapping");
	}
}

bool BufferGles_::allocate_(uint32 size, types::BufferBindingUse::Bits bufferUsage, types::BufferUse::Flags hint)
{
	m_size = size;
	m_usage = bufferUsage;
#ifdef GL_STATIC_DRAW
	static const GLenum hint_es3[] =
	{
		0,
		GL_DYNAMIC_READ,//CPU_READ
		GL_STREAM_DRAW,	//CPU_WRITE
		GL_DYNAMIC_READ,//CPU_READ|CPU_WRITE
		GL_STATIC_DRAW,	//GPU_READ
		GL_STATIC_DRAW,	//GPU_READ|CPU_READ
		GL_STREAM_DRAW,	//GPU_READ|CPU_WRITE
		GL_DYNAMIC_DRAW,//GPU_READ|CPU_WRITE|CPU_READ
		GL_STATIC_READ,	//GPU_WRITE
		GL_STATIC_READ,	//GPU_WRITE|CPU_READ
		GL_DYNAMIC_COPY,//GPU_WRITE|CPU_WRITE
		GL_STATIC_DRAW,	//GPU_WRITE|CPU_READ|CPU_WRITE
		GL_STATIC_DRAW,	//GPU_WRITE|GPU_READ
		GL_STATIC_COPY, //GPU_WRITE|GPU_READ|CPU_READ
		GL_DYNAMIC_DRAW,//GPU_WRITE|GPU_READ|CPU_WRITE
		GL_DYNAMIC_DRAW,//GPU_WRITE|GPU_READ|CPU_WRITE|CPU_READ
	};
	static const GLenum hint_es2[] =
	{
		0,
		GL_DYNAMIC_DRAW, 	//CPU_READ
		GL_DYNAMIC_DRAW, 	//CPU_WRITE
		GL_DYNAMIC_DRAW, 	//CPU_READ|CPU_WRITE
		GL_STATIC_DRAW,		//GPU_READ
		GL_STATIC_DRAW,		//GPU_READ|CPU_READ
		GL_DYNAMIC_DRAW,	//GPU_READ|CPU_WRITE
		GL_DYNAMIC_DRAW,	//GPU_READ|CPU_WRITE|CPU_READ
		GL_STATIC_DRAW,		//GPU_WRITE
		GL_STATIC_DRAW,		//GPU_WRITE|CPU_READ
		GL_STATIC_DRAW,		//GPU_WRITE|CPU_WRITE
		GL_STATIC_DRAW,		//GPU_WRITE|CPU_READ|CPU_WRITE
		GL_STATIC_DRAW,		//GPU_WRITE|GPU_READ
		GL_DYNAMIC_DRAW,	//GPU_WRITE|GPU_READ|CPU_READ
		GL_DYNAMIC_DRAW,	//GPU_WRITE|GPU_READ|CPU_WRITE
		GL_DYNAMIC_DRAW,	//GPU_WRITE|GPU_READ|CPU_WRITE|CPU_READ
	};
	const GLenum* apiHints = m_context->getApiType() < Api::OpenGLES3 ? hint_es2 : hint_es3;
#else
	static const GLenum hint[] =
	{
		0,
		GL_DYNAMIC_DRAW, 	//CPU_READ
		GL_DYNAMIC_DRAW, 	//CPU_WRITE
		GL_DYNAMIC_DRAW, 	//CPU_READ|CPU_WRITE
		GL_STATIC_DRAW,		//GPU_READ
		GL_STATIC_DRAW,		//GPU_READ|CPU_READ
		GL_DYNAMIC_DRAW,	//GPU_READ|CPU_WRITE
		GL_DYNAMIC_DRAW,	//GPU_READ|CPU_WRITE|CPU_READ
		GL_STATIC_DRAW,		//GPU_WRITE
		GL_STATIC_DRAW,		//GPU_WRITE|CPU_READ
		GL_STATIC_DRAW,		//GPU_WRITE|CPU_WRITE
		GL_STATIC_DRAW,		//GPU_WRITE|CPU_READ|CPU_WRITE
		GL_STATIC_DRAW,		//GPU_WRITE|GPU_READ
		GL_DYNAMIC_DRAW,		//GPU_WRITE|GPU_READ|CPU_READ
		GL_DYNAMIC_DRAW,		//GPU_WRITE|GPU_READ|CPU_WRITE
		GL_DYNAMIC_DRAW,		//GPU_WRITE|GPU_READ|CPU_WRITE|CPU_READ
	};
#endif

	//IMPLEMENT glBufferStorage
	m_lastUse = getGlenumFromBufferUsage(m_usage);
	gl::GenBuffers(1, &handle);
	gl::BindBuffer(m_lastUse, handle);
	gl::BufferData(m_lastUse, size, NULL, apiHints[hint]);
	gl::BindBuffer(m_lastUse, 0);
	m_size = size;
	m_hint = (uint32)apiHints[hint];
	debugLogApiError("Buffer_::allocate exit");
	return true;
}
}// impl
}// api
}// pvr
//!\endcond
