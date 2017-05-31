/*!
\brief Contains OpenGL ES specific implementation of the Buffer class. Use only if directly using OpenGL ES calls.
Provides the definitions allowing to move from the Framework object Buffer to the underlying OpenGL ES Buffer.
\file PVRApi/OGLES/BufferGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
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

	struct ES2MemoryMapping
	{
		std::vector<byte> mem;
	};

public:
	GLenum _lastUse;
	GLenum _hint;
	ES2MemoryMapping _es2MemoryMapping;
	// INTERNAL. Use GraphicsContext->createBuffer() instead.
	BufferGles_(const GraphicsContext& context) : Buffer_(context) {}

	void destroy();

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
private:
	bool allocate_(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);
	void update_(const void* data, uint32 offset, uint32 length);
	void* map_(types::MapBufferFlags flags, uint32 offset, uint32 length);
	void unmap_();
	bool isAllocated_() const { return _size != 0; }
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
PVR_DECLARE_NATIVE_CAST(Buffer);
PVR_DECLARE_NATIVE_CAST(BufferView);
