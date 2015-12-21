/*!*********************************************************************************************************************
\file         PVRApi\OGLES\Buffer.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the Buffer class. See BufferGles.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/BufferGles.h"
#include <PVRApi/ApiIncludes.h>
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiErrors.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ContextGles.h"

namespace pvr {
namespace api {
namespace convert_api_type {
GLbitfield mapBufferFlags(MapBufferFlags::Enum flags)
{
#if BUILD_API_MAX>=30
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
namespace impl {

// AT THIS POINT, IT IS ASSUMED THAT OUR BufferImpl is, in fact, a BufferGlesImpl. In this sense, to avoid
// virtual function calls, we are de-virtualising BufferImpl into BufferGles. So, for each call, our "this"
// pointer which is a BufferImpl*, gets cast into a BufferGlesImpl* and the calls are done direct (in fact,
// inline) through this pointer.

BufferGlesImpl::~BufferGlesImpl()
{
	if (m_allocated)
	{
		if (m_context.isValid())
		{
			gl::DeleteBuffers(1, &handle); m_allocated = false;
			debugLogApiError("BufferImpl::dtor exit");
		}
		else
		{
			Log(Log.Warning, "Buffer object was not released before context destruction");
		}
	}
	m_context.release();
}

inline GLenum getGlenumFromBufferUsage(BufferBindingUse::Bits usage)
{
	if (usage & BufferBindingUse::VertexBuffer) { return GL_ARRAY_BUFFER; }
	if (usage & BufferBindingUse::IndexBuffer) { return GL_ELEMENT_ARRAY_BUFFER; }
#if defined (GL_DRAW_INDIRECT_BUFFER)
	if (usage & BufferBindingUse::IndirectBuffer) { return GL_DRAW_INDIRECT_BUFFER; }
#endif
#if defined(GL_SHADER_STORAGE_BUFFER)
	if (usage & BufferBindingUse::StorageBuffer) { return GL_SHADER_STORAGE_BUFFER; }
#endif
	if (usage & BufferBindingUse::UniformBuffer) { return GL_UNIFORM_BUFFER; }
	return BufferBindingUse::VertexBuffer;
}


void BufferGlesImpl::allocate(GraphicsContext& context, uint32 size, BufferUse::Flags use)
{
#if BUILD_API_MAX>=30
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
	const GLenum* hint = context->getApiType() < Api::OpenGLES3 ? hint_es2 : hint_es3;
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

	if (m_allocated)
	{
		Log(Log.Debug,
		    "BufferGlesImpl::allocate: OpenGL ES buffer %d was already allocated, deleting it. This should normally NOT happen - allocate is private.",
		    handle);
		gl::DeleteBuffers(1, &handle);
	}
	//IMPLEMENT glBufferStorage
	{
		m_lastUse = getGlenumFromBufferUsage(m_usage);//Random...
		gl::GenBuffers(1, &handle);
		gl::BindBuffer(m_lastUse, handle);
		gl::BufferData(m_lastUse, size, NULL, hint[use]);
		gl::BindBuffer(m_lastUse, 0);
		m_size = size;
		m_allocated = true;
		m_hint = (uint32)hint[use];
	}
	debugLogApiError("BufferImpl::allocate exit");
}

void BufferImpl::update(const void* data, uint32 offset, uint32 length)
{
	//Safe downcast. We already KNOW that this is our class.
	static_cast<BufferGlesImpl*>(this)->update(data, offset, length);
}

void* BufferImpl::map(MapBufferFlags::Enum flags, uint32 offset, uint32 length)
{
	//Safe downcast. We already KNOW that this is our class.
	return static_cast<BufferGlesImpl*>(this)->map(flags, offset, length);
}

void BufferImpl::unmap()
{
	//Safe downcast. We already KNOW that this is our actual type.
	static_cast<BufferGlesImpl*>(this)->unmap();
}

inline void BufferGlesImpl::update(const void* data, uint32 offset, uint32 length)
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
	debugLogApiError("BufferImpl::update exit");
}

inline void* BufferGlesImpl::map(MapBufferFlags::Enum flags, uint32 offset, uint32 length)
{
	if (m_memMapped)
	{
		debugLogApiError("BufferImpl::map trying to map memory twice");
		return NULL;
	}
	m_memMapped = true;
	if (m_context->hasApiCapability(ApiCapabilities::MapBuffer))
	{
		gl::BindBuffer(m_lastUse, handle);
		void* retval = gl::MapBufferRange(m_lastUse, offset, length, convert_api_type::mapBufferFlags(flags));
		debugLogApiError("BufferImpl::map exit");
		return retval;
	}
	else
	{
		Log(Log.Error, "OGL ES2 context does not Support buffer mapping");
		PVR_ASSERT(false && "OGL ES2 Does not Support buffer mapping");
		return NULL;
	}
}

inline void BufferGlesImpl::unmap()
{
	if (!m_memMapped)
	{
		debugLogApiError("BufferImpl::unmap trying to un-map un-mapped memory");
		return;
	}
	m_memMapped = false;
	if (m_context->hasApiCapability(ApiCapabilities::MapBuffer))
	{
		gl::BindBuffer(m_lastUse, handle);
		gl::UnmapBuffer(m_lastUse);
		debugLogApiError("BufferImpl::unmap exit");
	}
	else
	{
		Log(Log.Error, "ES2 Does not Support buffer mapping");
		PVR_ASSERT(false && "ES2 Does not Support buffer mapping");
	}
}

void UboViewImpl::bind(IGraphicsContext& context, uint16 index) const
{
	bind(context, index, 0);
}

void UboViewImpl::bind(IGraphicsContext& context, uint16 index, pvr::uint32 offset) const
{
#ifdef GL_UNIFORM_BUFFER
	if (this->buffer->m_context->hasApiCapability(ApiCapabilities::Ubo))
	{
		const platform::ContextGles::BufferRange& lastBound = static_cast<platform::ContextGles&>(context).getBoundProgramBufferUbo(index);
		if (lastBound.buffer.isValid() && lastBound.offset == this->offset + offset && lastBound.range == range - offset && lastBound.buffer == getResource()) { return; }
		static_cast<pvr::platform::ContextGles&>(context).onBindUbo(index, getResource(), this->offset + offset, range - offset);

		gl::BindBufferRange(GL_UNIFORM_BUFFER, index, native::useNativeHandle(buffer), this->offset + offset, range - offset);
		debugLogApiError("UboViewImpl::bind exit");
	}
	else
#endif
	{
		PVR_ASSERT(0 && "UBO not supported from underlying API. No effect from SSBO::bind");
		Log(Log.Warning, "UBO not supported from underlying API. No effect from SSBO::bind");
	}
}

void SsboViewImpl::bind(IGraphicsContext& context, uint16 index) const
{
	bind(context, index, 0);
}

void SsboViewImpl::bind(IGraphicsContext& context, uint16 index, pvr::uint32 offset)const
{
#ifdef GL_SHADER_STORAGE_BUFFER
	if (this->buffer->m_context->hasApiCapability(ApiCapabilities::Ssbo))
	{
#ifdef DEBUG
		if (range > this->range - offset)
		{
			Log(Log.Error, "SsboView: Attempted to bind out-of-bounds buffer range");
			PVR_ASSERT(0 && "ATTEMPT TO BIND BUFFER RANGE OUT OF BOUNDS");
		}
#endif
		const platform::ContextGles::BufferRange& lastBound = static_cast<platform::ContextGles&>(context).getBoundProgramBufferSsbo(index);
		if (lastBound.buffer.isValid() && lastBound.offset == this->offset + offset && lastBound.range == this->range - offset && lastBound.buffer == getResource()) { return; }
		static_cast<pvr::platform::ContextGles&>(context).onBindSsbo(index, getResource(), this->offset + offset, range - offset);

		gl::BindBufferRange(GL_SHADER_STORAGE_BUFFER, index, native::useNativeHandle(buffer), this->offset + offset, range - offset);
		debugLogApiError("SsboViewImpl::bind exit");
	}
	else
#endif
	{
		PVR_ASSERT(0 && "SSBO not supported from underlying API. No effect from SSBO::bind");
		Log(Log.Warning, "SSBO not built into PVRApi. (BUILD_API_MAX is defined and <=3)");
	}
}

void AtomicBufferViewImpl::bind(IGraphicsContext& context, uint16 index, pvr::uint32 offset) const
{
#ifdef GL_ATOMIC_COUNTER_BUFFER
	if (this->buffer->m_context->hasApiCapability(ApiCapabilities::AtomicBuffer))
	{
#ifdef DEBUG
		if (range > this->range - offset)
		{
			Log(Log.Error, "AtomicBufferView: Attempted to bind out-of-bounds buffer");
			PVR_ASSERT(0 && "ATTEMPT TO BIND BUFFER RANGE OUT OF BOUNDS");
		}
#endif
		const platform::ContextGles::BufferRange& lastBound = static_cast<platform::ContextGles&>(context).getBoundProgramBufferAtomicBuffer(index);
		if (lastBound.buffer.isValid() && lastBound.offset == this->offset - offset && lastBound.range == range - offset && lastBound.buffer == getResource()) { return; }
		static_cast<pvr::platform::ContextGles&>(context).onBindAtomicBuffer(index, getResource(), this->offset + offset, range - offset);

		gl::BindBufferRange(GL_ATOMIC_COUNTER_BUFFER, index, native::useNativeHandle(buffer), this->offset + offset, range - offset);
		debugLogApiError("AtomicBufferViewImpl::bind exit");
	}
	else
#endif
	{
		PVR_ASSERT(0 && "AtomicBuffer not supported from underlying API. No effect from AtomicBufferViewImpl::bind");
		Log(Log.Warning, "AtomicBuffer not built into PVRApi. (BUILD_API_MAX is defined and <=3)");
	}
}

void AtomicBufferViewImpl::bind(IGraphicsContext& context, uint16 index) const
{
	bind(context, index, 0);
}
}
}
}
//!\endcond