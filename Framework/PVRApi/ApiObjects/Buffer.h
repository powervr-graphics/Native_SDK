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

<<<<<<< HEAD
/*!*********************************************************************************************************************
\brief Class containing the necessary information for a CommandBuffer::drawIndexedIndirect command. Should be filled and uploaded to
		a buffer (or directly written to through a shader), and used for the DrawIndexedIndirect command. DrawIndexedIndirect
		allows to draw primitives using an IndexBuffer to select vertices.
***********************************************************************************************************************/
=======
/// <summary>Class containing the necessary information for a CommandBuffer::drawIndexedIndirect command. Should be
/// filled and uploaded to a buffer (or directly written to through a shader), and used for the DrawIndexedIndirect
/// command. DrawIndexedIndirect allows to draw primitives using an IndexBuffer to select vertices.</summary>
>>>>>>> 1776432f... 4.3
struct DrawIndexedIndirect
{
	uint32 indexCount;  //!< Number of indexes to draw
	uint32 instanceCount; //!< Number of instances to draw
	uint32 firstIndex;   //!< First index to draw
	uint32 vertexOffset; //!< Offset into the VBO for this draw
	uint32 firstInstance; //!< First instance to draw
};

<<<<<<< HEAD
/*!*********************************************************************************************************************
\brief Class containing the necessary information for a CommandBuffer::drawIndirect command. Should be filled and uploaded to
		a buffer (or directly written to through a shader), and used for the DrawIndirect command. DrawIndirect sends the
		vertices directly in the order they appear, without indexing.
***********************************************************************************************************************/
=======
/// <summary>Class containing the necessary information for a CommandBuffer::drawIndirect command. Should be filled
/// and uploaded to a buffer (or directly written to through a shader), and used for the DrawIndirect command.
/// DrawIndirect sends the vertices directly in the order they appear, without indexing.</summary>
>>>>>>> 1776432f... 4.3
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
<<<<<<< HEAD
protected:
	uint32 m_size;
	types::BufferBindingUse m_usage;
	GraphicsContext m_context;
	Buffer_(GraphicsContext& context) : m_context(context) {}
=======
>>>>>>> 1776432f... 4.3
public:
	/// <summary>Get the total size of the buffer.</summary>
	/// <returns>The total size of the buffer.</returns>
	uint32 getSize() const { return _size; }

<<<<<<< HEAD
	/*!*********************************************************************************************************************
    \return Return The context for the specified resource creator.
	***********************************************************************************************************************/
	const GraphicsContext& getContext() const { return m_context; }

    /*!
       \brief getContext
       \return
     */
	GraphicsContext& getContext() { return m_context; }

	/*!*********************************************************************************************************************
    \return Return The context for the specified resource creator (const).
	***********************************************************************************************************************/
	types::BufferBindingUse getBufferUsage()const { return m_usage; }

    /*!
       \brief dtor
     */
	virtual ~Buffer_() { }

    /*!
       \brief Return true if this buffer is mappable
       \return
     */
	bool isMappable() const;

    /*!
       \brief Return true if this buffer is already mapped
       \return
     */
	bool isMapped() const;

	/*!*********************************************************************************************************************
	\brief Map this buffer.
	\param[in] flags
	\param[in] offset in buffer
	\param[in] length range to be mapped in the buffer
	***********************************************************************************************************************/
	void* map(types::MapBufferFlags flags, uint32 offset, uint32 length = -1);

	/*!*********************************************************************************************************************
	\brief Unmap the buffer, @see map.
	***********************************************************************************************************************/
	void unmap();

    /*!*********************************************************************************************************************
    \brief Perform an update to the buffer via using map and unmap. (Performs Map, memcpy, Unmap).
    \param[in] data A pointer to the data to copy to the buffer
    \param[in] offset Offset (bytes) in buffer to copy start copying the data to
    \param[in] length Length (bytes) of the data to be copied from \p data
    ***********************************************************************************************************************/
    virtual void update(const void* data, uint32 offset, uint32 length)
    {
        assertion(length + offset <= m_size);
        void* mapData  = map(types::MapBufferFlags::Write, offset, length);
        if (mapData)
        {
            memcpy(mapData, data, length);
            unmap();
        }
        else
        {
            assertion(false, "Failed to map memory");
        }
    }

	/*!*********************************************************************************************************************
	\brief Allocate a new buffer on the \p context GraphicsContext
	\param size The size of the buffer, in bytes
	\param bufferUsage A bitfield of all allowed uses of this buffer. A buffer must not be used in a way that has not been defined.
	\param isMappable Set to true to allow the buffer to be mapped to host-visible memory. Set to false is this function is
	not required and the buffer will be populated through a buffer copy (or commandBuffer->updateBuffer()). In both of these
	cases, add the TransferDst flag to the \p bufferUsage.
	***********************************************************************************************************************/
	bool allocate(uint32 size, types::BufferBindingUse bufferUsage, bool isMappable);

    /*!
       \brief Return a handle to the api-specifc native object (GLenum, VkBuffer etc.) (const)
     */
	const native::HBuffer_& getNativeObject()const;


    /*!
       \brief Return a handle to the native api object (GLenum, VkBuffer etc.)
       \return
     */
	native::HBuffer_& getNativeObject();
=======
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
>>>>>>> 1776432f... 4.3
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

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief Update the buffer.
	\param[in] data Pointer to the data to update with
	\param[in] offset in buffer to update
	\param[in] length range in the buffer to be updated
	\description No need for explicit function calls for map and unmap. Update buffer takes care of mapping and mapping,
	***********************************************************************************************************************/
	void update(const void* data, uint32 offset, uint32 length)
	{
        assertion(length + offset <= buffer->getSize(), "BufferView_::update - offset and length exceeds the buffer size");
        buffer->update(data, offset, length);
	}

	/*!*********************************************************************************************************************
	\brief Map the buffer.
	\param[in] flags Mapping flags
	\param[in] offset Offset in the buffer to map
	\param[in] length Range of the buffer to map
	\return		A pointer to the region of memory where the buffer is mapped.
	***********************************************************************************************************************/
=======
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
>>>>>>> 1776432f... 4.3
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

<<<<<<< HEAD
    /*!
       \brief Return true if the buffer is already mapped.
       \return
     */
	bool isMapped() const { return buffer->isMapped();  }

    /*!
       \brief Return the offset this buffer view pointing in to the buffer
     */
	uint32 getOffset() const { return offset; }

    /*!
       \brief Return the range of this buffer view
     */
	uint32 getRange() const { return range; }

    /*!
       \brief Return a handle to the native api object
     */
	const pvr::native::HBufferView_& getNativeObject()const;

    /*!
       \brief Return  a handle to the native object
     */
	pvr::native::HBufferView_& getNativeObject();

    /*!
       \brief Return the graphics context who owns this resource
     */
	GraphicsContext& getContext() { return getResource()->getContext(); }


    /*!
       \brief Return the graphics context who owns this resource (const)
     */
	const GraphicsContext& getContext()const { return getResource()->getContext(); }
protected:
	Buffer buffer;
	uint32 offset;
	uint32 range;

	BufferView_(const Buffer& buffer, uint32 offset, uint32 range) : buffer(buffer),
		offset(offset), range(range) { }
=======
	/// <summary>Return the graphics context who owns this resource</summary>
	GraphicsContext& getContext() { return getResource()->getContext(); }


	/// <summary>Return the graphics context who owns this resource (const)</summary>
	const GraphicsContext& getContext()const { return getResource()->getContext(); }
protected:
	Buffer _buffer;
	uint32 _offset;
	uint32 _range;

	BufferView_(const Buffer& buffer, uint32 offset, uint32 range) : _buffer(buffer), _offset(offset), _range(range) { }
>>>>>>> 1776432f... 4.3
};
}// namespace impl
}// namespace api
}// namespace pvr