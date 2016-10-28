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
		a buffer (or directly written to through a shader), and used for the DrawIndexedIndirect command. DrawIndexedIndirect
		allows to draw primitives using an IndexBuffer to select vertices.
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
		a buffer (or directly written to through a shader), and used for the DrawIndirect command. DrawIndirect sends the
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
	types::BufferBindingUse m_usage;
	GraphicsContext m_context;
	Buffer_(GraphicsContext& context) : m_context(context) {}
public:
	/*!*********************************************************************************************************************
	\return The size of the buffer.
	***********************************************************************************************************************/
	uint32 getSize() const { return m_size; }

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
	void* map(types::MapBufferFlags flags, uint32 offset, uint32 length)
	{
		return buffer->map(flags, offset, length);
	}

	/*!*********************************************************************************************************************
	\brief Unmap the buffer. Flushes all operations performed after mapping it.
	***********************************************************************************************************************/
	void unmap() { buffer->unmap(); }

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
};
}// namespace impl
}// namespace api
}// namespace pvr
