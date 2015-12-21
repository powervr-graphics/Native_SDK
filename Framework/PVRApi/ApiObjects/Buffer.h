/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\Buffer.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Contains the pvr::Api::Buffer class and BufferViews classes definitions
***********************************************************************************************************************/
#pragma once
#include "PVRCore/ForwardDecApiObjects.h"
#include "PVRApi/Bindables.h"
#include "PVRCore/IGraphicsContext.h"


namespace pvr {
/*!*********************************************************************************************************************
\brief Buffer mapping flags.
***********************************************************************************************************************/
namespace MapBufferFlags {
enum Enum { Read = 1, Write = 2, Unsynchronised = 4 };
}

namespace api {
// Forward Declarations
class DrawArrays;
class DrawIndexed;
struct ImageDataFormat;
class BindDescriptorSets;
namespace impl {
class DescriptorSetGlesImpl;

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
class BufferImpl
{
	friend class UboViewImpl;
	friend class AtomicBufferViewImpl;
	friend class SsboViewImpl;
	BufferImpl& operator=(BufferImpl&);//deleted
protected:
	uint32 m_size;
	BufferBindingUse::Bits m_usage;
	uint32 m_hint;
	uint32 m_lastUse;
	bool m_allocated;
	GraphicsContext m_context;
	bool m_memMapped;
	BufferImpl(GraphicsContext& context, uint32 size, api::BufferBindingUse::Bits bufferUsage,
	           BufferUse::Flags hints) : m_usage(bufferUsage), m_lastUse(0), m_allocated(false), m_context(context), m_memMapped(false) {}
public:
	/*!*********************************************************************************************************************
	\return The size of the buffer.
	***********************************************************************************************************************/
	uint32 getSize() const { return m_size; }

	/*!*********************************************************************************************************************
	\return The context for the specified resource creator.
	***********************************************************************************************************************/
	const IGraphicsContext& getContext() const { return *m_context; }

	/*!*********************************************************************************************************************
	\return Get buffer usage.
	***********************************************************************************************************************/
	BufferBindingUse::Bits getBufferUsage()const { return m_usage; }

	virtual ~BufferImpl() { }

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
	void* map(MapBufferFlags::Enum flags, uint32 offset, uint32 length);

	/*!*********************************************************************************************************************
	\brief Unmap the buffer, @see map.
	***********************************************************************************************************************/
	void unmap();
};
}//namespace impl

namespace impl {
/*!****************************************************************************************************************
\brief BufferView.
\brief See class pvr::api::Buffer. Base class to interpret how the buffer will be used.
******************************************************************************************************************/
class BufferViewImpl
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
	\brief Releases all held resources.
	***********************************************************************************************************************/
	void destroy()	{ buffer.reset(); }

	/*!*********************************************************************************************************************
	\brief Releases all resources.
	***********************************************************************************************************************/
	~BufferViewImpl() { destroy(); }

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
	void* map(MapBufferFlags::Enum flags, uint32 offset, uint32 length)
	{
		return buffer->map(flags, offset, length);
	}

	/*!*********************************************************************************************************************
	\brief Unmap the buffer. Flushes all operations performed after mapping it.
	***********************************************************************************************************************/
	void unmap() {	buffer->unmap(); }
protected:
	Buffer buffer;

	uint32 offset;
	uint32 range;
	BufferViewImpl(const Buffer& buffer, uint32 offset, uint32 range) : buffer(buffer),
		offset(offset), range(range) { }
};

/*!****************************************************************************************************************
\brief See class pvr::api::UboView. Create with pvr::api::IGraphicsContext::createUbo
******************************************************************************************************************/
class UboViewImpl : public BufferViewImpl
{
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
	friend class ::pvr::api::BindDescriptorSets;
	friend class ::pvr::api::impl::DescriptorSetGlesImpl;
	friend class IGraphicsContext;
	void bind(IGraphicsContext& context, uint16 index) const;
	void bind(IGraphicsContext& context, uint16 index, uint32 offset)const;
	void bind(IGraphicsContext& context, uint16 index, uint32 offset, uint32 range)const;

	UboViewImpl(const Buffer& buffer, uint32 offset, uint32 range) :
		BufferViewImpl(buffer, offset, range) { }
};

/*!****************************************************************************************************************
\brief See class pvr::api::SsboView.
******************************************************************************************************************/
class SsboViewImpl : public BufferViewImpl
{
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
	template<typename, typename> friend class ::pvr::api::impl::PackagedBindableWithParam;
	friend class ::pvr::api::BindDescriptorSets;
	friend class ::pvr::api::impl::DescriptorSetGlesImpl;
	friend class IGraphicsContext;

	void bind(IGraphicsContext& context, uint16 index) const;
	void bind(IGraphicsContext& context, uint16 index, uint32 offset)const;
	void bind(IGraphicsContext& context, uint16 index, uint32 offset, uint32 range)const;

	SsboViewImpl(const Buffer& buffer, uint32 offset, uint32 range) : BufferViewImpl(buffer, offset, range) {}
};

/*!****************************************************************************************************************
\brief See class pvr::api::AtomicBufferView.
******************************************************************************************************************/
class AtomicBufferViewImpl : public BufferViewImpl
{
	template<typename, typename> friend class ::pvr::api::impl::PackagedBindableWithParam;
	friend class ::pvr::api::BindDescriptorSets;
	friend class ::pvr::api::impl::DescriptorSetImpl;
	void bind(IGraphicsContext& context, uint16 index) const;
	void bind(IGraphicsContext& context, uint16 index, uint32 offset)const;
	void bind(IGraphicsContext& context, uint16 index, uint32 offset, uint32 range)const;
	AtomicBufferViewImpl(const Buffer& buffer) : BufferViewImpl(buffer, 0, 0) {}
};

}//namespace impl

}// namespace api
}// namespace pvr
