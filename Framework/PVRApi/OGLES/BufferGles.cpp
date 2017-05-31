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

void BufferGles_::update_(const void* data, uint32 offset, uint32 length)
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

void* BufferGles_::map_(types::MapBufferFlags flags, uint32 offset, uint32 length)
{
	if (_mappedRange)
	{
		assertion(false, "BufferGles_::map trying to map memory twice");
		return NULL;
	}
	if (_context->hasApiCapability(ApiCapabilities::MapBufferRange))
	{
		gl::BindBuffer(_lastUse, handle);
		void* retval = gl::MapBufferRange(_lastUse, offset, length, convert_api_type::mapBufferFlags(flags));
		debugLogApiError("Buffer_::map exit");
		if (retval != NULL)
		{
			_mappedRange = length;
			_mappedOffset = offset;
			_mappedFlags = flags;
		}
		return retval;
	}
	else if (flags == types::MapBufferFlags::Write)
	{
		_es2MemoryMapping.mem.resize(length);
		_mappedRange = length;
		_mappedOffset = offset;
		_mappedFlags = flags;
		return (void*)_es2MemoryMapping.mem.data();
	}
	else
	{
		Log(Log.Error, "BufferGles_::map_ - OGLES2 context does not Support buffer mapping. You can still map for write only.");
		assertion(false, "BufferGles_::map_ - OGLES2 Does not Support buffer mapping");
		return NULL;
	}
}

void BufferGles_::unmap_()
{
	if (!_mappedRange)
	{
		assertion(false, "Buffer_::unmap trying to un-map un-mapped memory");
		return;
	}
	_mappedRange = 0;
	_mappedOffset = 0;
	_mappedFlags = types::MapBufferFlags(0);

	if (_context->hasApiCapability(ApiCapabilities::MapBufferRange))
	{
		gl::BindBuffer(_lastUse, handle);
		gl::UnmapBuffer(_lastUse);
		debugLogApiError("Buffer_::unmap exit");
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
	if(size == 0)
	{
		assertion(size != 0, "Failed to allocate buffer. Allocation size should not be 0");
		return false;
	}
	_size = size;
	_usage = bufferUsage;
	_hint = isMappable ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	_isMappable = isMappable;

	//IMPLEMENT glBufferStorage
	_lastUse = getGlenumFromBufferUsage(_usage);
	gl::GenBuffers(1, &handle);
	gl::BindBuffer(_lastUse, handle);
	gl::BufferData(_lastUse, size, NULL, _hint);
	gl::BindBuffer(_lastUse, 0);
	_size = size;
	debugLogApiError("Buffer_::allocate exit");
	return true;
}

BufferViewGles_::BufferViewGles_(const Buffer& buffer, uint32 offset, uint32 range) :
	impl::BufferView_(buffer, offset, range) {}

}// impl
}// api
}// pvr
