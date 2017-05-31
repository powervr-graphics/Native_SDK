/*!
\brief Contains the pvr::Api::Buffer class and BufferViews classes definitions
\file PVRApi/ApiObjects/Buffer.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRCore/Base/Types.h"

namespace pvr {
namespace api {
// Forward Declarations
class DrawArrays;
class DrawIndexed;
namespace impl {

/// <summary>Class containing the necessary information for a CommandBuffer::drawIndexedIndirect command. Should be
/// filled and uploaded to a buffer (or directly written to through a shader), and used for the DrawIndexedIndirect
/// command. DrawIndexedIndirect allows to draw primitives using an IndexBuffer to select vertices.</summary>
struct DrawIndexedIndirect
{
	uint32 indexCount;  //!< Number of indexes to draw
	uint32 instanceCount; //!< Number of instances to draw
	uint32 firstIndex;   //!< First index to draw
	uint32 vertexOffset; //!< Offset into the VBO for this draw
	uint32 firstInstance; //!< First instance to draw
};

/// <summary>Class containing the necessary information for a CommandBuffer::drawIndirect command. Should be filled
/// and uploaded to a buffer (or directly written to through a shader), and used for the DrawIndirect command.
/// DrawIndirect sends the vertices directly in the order they appear, without indexing.</summary>
struct DrawIndirect
{
	uint32 vertexCount; //!< Number of vertices to draw
	uint32 instanceCount; //!< Number of instances to draw
	uint32 firstVertex;  //!< First vertex to draw
	uint32 firstInstance;//!< First instance to draw
};

/// <summary>Buffer Implementation. Access through the Refcounted Framework object Buffer. All buffer types contain
/// or extend the Buffer implementation.</summary>
class Buffer_
{
	Buffer_& operator=(Buffer_&);//deleted
public:
	/// <summary>Get the total size of the buffer.</summary>
	/// <returns>The total size of the buffer.</returns>
	uint32 getSize() const { return _size; }

	/// <summary>Get The context for the specified resource creator.</summary>
	/// <returns>The context for the specified resource creator.</returns>
	const GraphicsContext& getContext() const { return _context; }

	/// <summary>Get The context for the specified resource creator.</summary>
	/// <returns>The context for the specified resource creator.</returns>
	GraphicsContext& getContext() { return _context; }

	/// <summary>Get the allowed BufferBindingUse flags for the specified buffer</summary>
	/// <returns>The allowed BufferBindingUse flags for the specified buffer</returns>
	types::BufferBindingUse getBufferUsage()const { return _usage; }

	/// <summary>dtor</summary>
	virtual ~Buffer_() { }

	/// <summary>Return true if this buffer is mappable</summary>
	/// <returns>True if this buffer is already mapped</returns>
	bool isMappable() const { return _isMappable; }

	/// <summary>Return true if this buffer is already mapped</summary>
	/// <returns>True if this buffer is already mapped</returns>
	bool isMapped() const
	{
		return _mappedRange != 0;
	}

	bool isAllocated() const { return isAllocated_(); }

	/// <summary>Map this buffer. The buffer must have been defined as Mappable on creation.</summary>
	/// <param name="flags">The mapping modes allowed (Read, Write). It is undefined to read(resp. write) from a
	/// buffer mapped without the MapBufferFlags::Read(resp. Write) flag set here.</param>
	/// <param name="offset">The offset from the beginning of the buffer from which to start the mapped region.
	/// </param>
	/// <param name="length">The length from <paramref name="offset"/>of the region to be mapped. Default -1 (entire
	/// buffer)</param>
	void* map(types::MapBufferFlags flags, uint32 offset, uint32 length = -1)
	{
		return map_(flags, offset, length);
	}

	/// <summary>Unmap the buffer (flush and make visible). \see map.</summary>
	void unmap()
	{
		unmap_();
	}

	/// <summary>Perform an update to the buffer via using map and unmap. (Performs Map, memcpy, Unmap).</summary>
	/// <param name="data">A pointer to the data to copy to the buffer</param>
	/// <param name="offset">Offset (bytes) in buffer to copy start copying the data to</param>
	/// <param name="length">Length (bytes) of the data to be copied from <paramref name="data"/></param>
	void update(const void* data, uint32 offset, uint32 length)
	{
		update_(data, offset, length);
	}

	/// <summary>Allocate a new buffer on the <paramref name="context"/>GraphicsContext</summary>
	/// <param name="size">The size of the buffer, in bytes</param>
	/// <param name="bufferUsage">A bitfield of all allowed uses of this buffer. A buffer must not be used in a way
	/// that has not been defined.</param>
	/// <param name="allocateMappable">Set to true to allow the buffer to be mapped to host-visible memory. Set to
	/// false if mapping function is not required and the buffer will be populated through a transfer (either a buffer
	/// copy or commandBuffer->updateBuffer()). In any case, it makes sense to add the TransferDst flag to the
	/// <paramref name="bufferUsage."/></param>
	bool allocate(uint32 size, types::BufferBindingUse bufferUsage, bool allocateMappable)
	{ return allocate_(size, bufferUsage, allocateMappable); }

protected:
	uint32 _size;
	types::BufferBindingUse _usage;
	GraphicsContext _context;
	Buffer_(const GraphicsContext& context) :
		_context(context), _mappedFlags(types::MapBufferFlags(0)), _mappedRange(0), _mappedOffset(0), _isMappable(0), _size(0)
	{}
	bool _isMappable;
	uint32 _mappedRange;
	uint32 _mappedOffset;
	types::MapBufferFlags _mappedFlags;

private:
	//Template methods.

	//API implementation of map
	virtual void* map_(types::MapBufferFlags flags, uint32 offset, uint32 length) = 0;
	//API implementation of unmap
	virtual void unmap_() = 0;
	virtual void update_(const void* data, uint32 offset, uint32 range) = 0;

	//API implementation of allocate
	virtual bool isAllocated_() const = 0;
	virtual bool allocate_(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable) = 0;
};
}//namespace impl

namespace impl {
/// <summary>See class pvr::api::Buffer. The BufferView is the object that will be used with DescriptorSets.
/// </summary>
class BufferView_
{
public:
	/// <summary>Set the underlying storage buffer.</summary>
	/// <param name="buffer">The buffer</param>
	void setResource(Buffer buffer) { _buffer = buffer; }

	/// <summary>Get the underlying storage buffer (const).</summary>
	/// <returns>The underlying storage buffer</returns>
	const Buffer& getResource() const { return _buffer;}

	/// <summary>Get the underlying storage buffer.</summary>
	/// <returns>The underlying storage buffer</returns>
	Buffer getResource() {  return _buffer; }

	/// <summary>Releases all resources.</summary>
	virtual ~BufferView_() { }

	/// <summary>Update the buffer.</summary>
	/// <param name="data">Pointer to the data to update with</param>
	/// <param name="offset">in buffer to update</param>
	/// <param name="length">range in the buffer to be updated</param>
	/// <remarks>No need for explicit function calls for map and unmap. Update buffer takes care of mapping and
	/// mapping,</remarks>
	void update(const void* data, uint32 offset, uint32 length)
	{
		assertion(length + offset <= _buffer->getSize(), "BufferView_::update - offset and length exceeds the buffer size");
		_buffer->update(data, offset, length);
	}

	/// <summary>Map the buffer.</summary>
	/// <param name="flags">Mapping flags</param>
	/// <param name="offset">Offset in the buffer to map</param>
	/// <param name="length">Range of the buffer to map</param>
	/// <returns>A pointer to the region of memory where the buffer is mapped.</returns>
	void* map(types::MapBufferFlags flags, uint32 offset, uint32 length)
	{
		return _buffer->map(flags, offset, length);
	}

	/// <summary>Unmap the buffer. Flushes all operations performed after mapping it.</summary>
	void unmap() { _buffer->unmap(); }

	/// <summary>Return true if the buffer is already mapped.</summary>
	bool isMapped() const { return _buffer->isMapped();  }

	/// <summary>Return the offset this buffer view pointing in to the buffer</summary>
	uint32 getOffset() const { return _offset; }

	/// <summary>Return the range of this buffer view</summary>
	uint32 getRange() const { return _range; }

	/// <summary>Return the graphics context who owns this resource</summary>
	GraphicsContext& getContext() { return getResource()->getContext(); }


	/// <summary>Return the graphics context who owns this resource (const)</summary>
	const GraphicsContext& getContext()const { return getResource()->getContext(); }
protected:
	Buffer _buffer;
	uint32 _offset;
	uint32 _range;

	BufferView_(const Buffer& buffer, uint32 offset, uint32 range) : _buffer(buffer), _offset(offset), _range(range) { }
};
}// namespace impl
}// namespace api
}// namespace pvr