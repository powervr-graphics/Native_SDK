/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\Buffer.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Contains the pvr::Api::Buffer class and BufferViews classes definitions
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRCore/Types.h"

namespace pvr {
/*!*********************************************************************************************************************
\brief Buffer mapping flags.
***********************************************************************************************************************/

namespace api {
// Forward Declarations
class DrawArrays;
class DrawIndexed;
struct ImageDataFormat;
class BindDescriptorSets;
namespace impl {

/*!*********************************************************************************************************************
\brief Class containing the necessary information for a CommandBuffer::drawIndexedIndirect command. Should be filled and uploaded to
		a buffer (or directly written to through a shader) and used for the DrawIndexedIndirect command. DrawIndexedIndirect
		allows to draw primitives using an IndexBuffer to select vertices/
***********************************************************************************************************************/
struct DrawIndexedIndirect
{
	pvr::uint32 indexCount;  //!< Number of indexes to draw
	pvr::uint32 instanceCount; //!< Number of instances to draw
	pvr::uint32 firstIndex;   //!< First index to draw
	pvr::uint32 vertexOffset; //!< Offset into the VBO for this draw
	pvr::uint32 firstInstance; //!< First instance to draw
};

/*!*********************************************************************************************************************
\brief Class containing the necessary information for a CommandBuffer::drawIndirect command. Should be filled and uploaded to
		a buffer (or directly written to through a shader) and used for the DrawIndirect command. DrawIndirect sends the
		vertices directly in the order they appear, without indexing.
***********************************************************************************************************************/
struct DrawIndirect
{
	pvr::uint32 vertexCount; //!< Number of vertices to draw
	pvr::uint32 instanceCount; //!< Number of instances to draw
	pvr::uint32 firstVertex;  //!< First vertex to draw
	pvr::uint32 firstInstance;//!< First instance to draw
};

/*!*********************************************************************************************************************
\brief Buffer Implementation. Access through the Refcounted Framework object Buffer. All buffer types contain or extend the
       Buffer implementation.
***********************************************************************************************************************/
class Buffer_
{
	Buffer_& operator=(Buffer_&);//deleted
protected:
	uint32 m_size;
	types::BufferBindingUse::Bits m_usage;
	GraphicsContext m_context;
	Buffer_(GraphicsContext& context) : m_context(context) {}
public:
	/*!*********************************************************************************************************************
	\return The size of the buffer.
	***********************************************************************************************************************/
	uint32 getSize() const { return m_size; }

	/*!*********************************************************************************************************************
	\return The context for the specified resource creator.
	***********************************************************************************************************************/
	const GraphicsContext& getContext() const { return m_context; }

	GraphicsContext& getContext() { return m_context; }

	/*!*********************************************************************************************************************
	\return Get buffer usage.
	***********************************************************************************************************************/
	types::BufferBindingUse::Bits getBufferUsage()const { return m_usage; }

	virtual ~Buffer_() { }

	/*!*********************************************************************************************************************
	\brief Update the buffer.
	\param[in] data
	\param[in] offset in buffer
	\param[in] length range to be updated in the buffer
	***********************************************************************************************************************/
	void update(const void* data, uint32 offset, uint32 length);

	/*!*********************************************************************************************************************
	\brief Map this buffer.
	\param[in] flags
	\param[in] offset in buffer
	\param[in] length range to be mapped in the buffer
	***********************************************************************************************************************/
	void* map(types::MapBufferFlags::Enum flags, uint32 offset, uint32 length);

	/*!*********************************************************************************************************************
	\brief Unmap the buffer, @see map.
	***********************************************************************************************************************/
	void unmap();

	/*!*********************************************************************************************************************
	\brief Allocate a new buffer on the \p context GraphicsContext
	\param context The graphics context
	\param size buffer size, in bytes
	\param hint The expected use of the buffer (CPU Read, GPU write etc)
	***********************************************************************************************************************/
	bool allocate(uint32 size, types::BufferBindingUse::Bits bufferUsage, types::BufferUse::Flags hint = types::BufferUse::DEFAULT);

	const native::HBuffer_& getNativeObject()const;
	native::HBuffer_& getNativeObject();
};
}//namespace impl

namespace impl {
/*!****************************************************************************************************************
\brief BufferView.
\brief See class pvr::api::Buffer. Base class to interpret how the buffer will be used.
******************************************************************************************************************/
class BufferView_
{
public:
	/*!*********************************************************************************************************************
	\brief Set the underlying storage buffer.
	\param buffer The buffer
	***********************************************************************************************************************/
	void setResource(Buffer buffer) { this->buffer = buffer;	}

	/*!*********************************************************************************************************************
	\brief Get the underlying storage buffer (const).
	\return The underlying storage buffer
	***********************************************************************************************************************/
	const Buffer& getResource() const { return buffer;}

	/*!*********************************************************************************************************************
	\brief Get the underlying storage buffer.
	\return The underlying storage buffer
	***********************************************************************************************************************/
	Buffer getResource() {	return buffer;	}

	/*!*********************************************************************************************************************
	\brief Releases all resources.
	***********************************************************************************************************************/
	virtual ~BufferView_() { }

	/*!*********************************************************************************************************************
	\brief Update the buffer.
	\param[in] data Pointer to the data to update with
	\param[in] offset in buffer to update
	\param[in] length range in the buffer to be updated
	***********************************************************************************************************************/
	void update(const void* data, uint32 offset, uint32 length)
	{
		buffer->update(data, offset, length);
	}

	/*!*********************************************************************************************************************
	\brief Map the buffer.
	\param[in] flags Mapping flags
	\param[in] offset Offset in the buffer to map
	\param[in] length Range of the buffer to map
	\return		A pointer to the region of memory where the buffer is mapped.
	***********************************************************************************************************************/
	void* map(types::MapBufferFlags::Enum flags, uint32 offset, uint32 length)
	{
		return buffer->map(flags, offset, length);
	}

	/*!*********************************************************************************************************************
	\brief Unmap the buffer. Flushes all operations performed after mapping it.
	***********************************************************************************************************************/
	void unmap() { buffer->unmap(); }

	uint32 getOffset() const { return offset; }
	uint32 getRange() const { return range; }
	const pvr::native::HBufferView_& getNativeObject()const;
	pvr::native::HBufferView_& getNativeObject();
	GraphicsContext& getContext() { return getResource()->getContext(); }
	const GraphicsContext& getContext()const { return getResource()->getContext(); }
protected:
	Buffer buffer;
	uint32 offset;
	uint32 range;
	BufferView_(const Buffer& buffer, uint32 offset, uint32 range) : buffer(buffer),
		offset(offset), range(range) { }
};
}// namespace impl
}// namespace api
}// namespace pvr
