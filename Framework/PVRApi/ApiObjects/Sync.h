/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\Sync.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         The PVRApi basic Texture implementation.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"

namespace pvr {
namespace api {
namespace impl {
/*!*********************************************************************************************************************
\brief Fence can be used by the host to determine completion of execution of subimmisions to queues.
	   The host can be polled for the fence signal
***********************************************************************************************************************/
class Fence_
{
protected:
	GraphicsContext context;
	Fence_(const GraphicsContext& context) : context(context) {}
public:
	/*!*********************************************************************************************************************
	\brief Returns const reference to the graphics context who own this resource
	***********************************************************************************************************************/
	const GraphicsContext& getContext() const { return context; }

	/*!*********************************************************************************************************************
	\brief Returns reference to the graphics context who own this resource
	***********************************************************************************************************************/
	GraphicsContext& getContext() { return context; }

	/*!*********************************************************************************************************************
	\brief Host to wait for this fence to be signaled
	\param timeoutNanos Time out period in nanoseconds
	***********************************************************************************************************************/
	bool wait(uint64 timeoutNanos = uint64(-1));

	/*!*********************************************************************************************************************
	\brief Reset this fence
	***********************************************************************************************************************/
	void reset();

	/*!*********************************************************************************************************************
	\brief Return true if this fence is signaled
	***********************************************************************************************************************/
	bool isSignalled();

	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	virtual ~Fence_() {}
};

/*!*********************************************************************************************************************
\brief Use to "serialize" access between CommandBuffer submissions and /Queues
***********************************************************************************************************************/
class Semaphore_
{
protected:
	GraphicsContext context;
	Semaphore_(const GraphicsContext& context) : context(context) {}
public:
	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	virtual ~Semaphore_() {}
};

class Event_
{
protected:
	GraphicsContext context;
	Event_(const GraphicsContext& context) : context(context) {}
public:
	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	virtual ~Event_() {}

	/*!*********************************************************************************************************************
	\brief Set this event
	***********************************************************************************************************************/
	void set();

	/*!*********************************************************************************************************************
	\brief Reset this event
	***********************************************************************************************************************/
	void reset();

	/*!*********************************************************************************************************************
	\brief Return true if this event is set
	***********************************************************************************************************************/
	bool isSet();
};
}

/*!*********************************************************************************************************************
\brief  A Global memory barrier used for memory accesses for all memory objects.
***********************************************************************************************************************/
struct MemoryBarrier
{
	types::AccessFlags srcMask;
	types::AccessFlags dstMask;
	MemoryBarrier(): srcMask(types::AccessFlags(0)), dstMask(types::AccessFlags(0)) {}
	MemoryBarrier(types::AccessFlags srcMask, types::AccessFlags dstMask): srcMask(srcMask), dstMask(dstMask) {}
};

/*!*********************************************************************************************************************
\brief  A Buffer memory barrier used only for memory accesses involving a specific range
		of the specified buffer object. It is also used to transfer ownership of an buffer range from one queue family to another.
***********************************************************************************************************************/
struct BufferRangeBarrier
{
	types::AccessFlags srcMask;
	types::AccessFlags dstMask;
	Buffer buffer;
	uint32 offset;
	uint32 range;
	BufferRangeBarrier() : srcMask(types::AccessFlags(0)), dstMask(types::AccessFlags(0)) {}
	BufferRangeBarrier(types::AccessFlags srcMask, types::AccessFlags dstMask, Buffer buffer, uint32 offset, uint32 range) :
		srcMask(srcMask), dstMask(dstMask), buffer(buffer), offset(offset), range(range) {}
};

/*!*********************************************************************************************************************
\brief  A Image memory barrier used only for memory accesses involving a specific subresource range
		of the specified image object. It is also used to perform a layout transition for an image subresource range,
		or to transfer ownership of an image subresource range from one queue family to another.
***********************************************************************************************************************/
struct ImageAreaBarrier
{
	types::AccessFlags srcMask;
	types::AccessFlags dstMask;
	TextureStore texture;
	types::ImageSubresourceRange area;
	types::ImageLayout oldLayout;
	types::ImageLayout newLayout;
	ImageAreaBarrier() {}
	ImageAreaBarrier(types::AccessFlags srcMask, types::AccessFlags dstMask,
	                 const TextureStore& texture, const types::ImageSubresourceRange& area,
	                 types::ImageLayout oldLayout, types::ImageLayout newLayout) :
		srcMask(srcMask), dstMask(dstMask), texture(texture), area(area), oldLayout(oldLayout), newLayout(newLayout)
	{}
};

namespace impl {
class MemoryBarrierSetImpl;
}
/*!*********************************************************************************************************************
\brief  A memory barrier into the command stream. Used to signify that some types of pending operations from
before the barrier must have finished before the commands after the barrier start executing.
***********************************************************************************************************************/
class MemoryBarrierSet
{
	std::auto_ptr<impl::MemoryBarrierSetImpl> pimpl;
	MemoryBarrierSet(const MemoryBarrierSet&); //deleted
	MemoryBarrierSet& operator=(const MemoryBarrierSet&); //deleted
public:
	/*!*********************************************************************************************************************
	\brief ctor
	***********************************************************************************************************************/
	MemoryBarrierSet();

	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	~MemoryBarrierSet();

	MemoryBarrierSet& clearAllBarriers();

	MemoryBarrierSet& clearAllMemoryBarriers();

	MemoryBarrierSet& clearAllBufferRangeBarriers();

	MemoryBarrierSet& clearAllImageAreaBarriers();

	/*!*********************************************************************************************************************
	\brief  A memory barrier into the command stream. Used to signify that some types of pending operations from
	before the barrier must have finished before the commands after the barrier start executing.
	***********************************************************************************************************************/
	MemoryBarrierSet& addBarrier(MemoryBarrier barrier);

	/*!*********************************************************************************************************************
	\brief  A buffer range barrier into the command stream. Used to signify that some types of pending operations from
	before the barrier must have finished before the commands after the barrier start executing.
	***********************************************************************************************************************/
	MemoryBarrierSet& addBarrier(const BufferRangeBarrier& barrier);

	/*!*********************************************************************************************************************
	\brief  A image area barrier into the command stream. Used to signify that some types of pending operations from
	before the barrier must have finished before the commands after the barrier start executing.
	***********************************************************************************************************************/
	MemoryBarrierSet& addBarrier(const ImageAreaBarrier& barrier);

	/*!*********************************************************************************************************************
	\brief  Return the native memory barriers
	***********************************************************************************************************************/
	const void* getNativeMemoryBarriers()const;

	/*!*********************************************************************************************************************
	\brief  Return the native image barriers
	***********************************************************************************************************************/
	const void* getNativeImageBarriers()const;

	/*!*********************************************************************************************************************
	\brief  Return the native buffer barriers
	***********************************************************************************************************************/
	const void* getNativeBufferBarriers()const;

	/*!*********************************************************************************************************************
	\brief  Return the number of memory barriers
	***********************************************************************************************************************/
	uint32 getNativeMemoryBarriersCount() const;

	/*!*********************************************************************************************************************
	\brief  Return the number of image barriers
	***********************************************************************************************************************/
	uint32 getNativeImageBarriersCount() const;

	/*!*********************************************************************************************************************
	\brief  Return the number of buffer barriers
	***********************************************************************************************************************/
	uint32 getNativeBufferBarriersCount() const;
};


typedef RefCountedResource<impl::Fence_> Fence;
typedef RefCountedResource<impl::Semaphore_> Semaphore;
typedef RefCountedResource<impl::Event_> Event;

namespace impl {
class FenceSetImpl_;
class FenceSet_
{
	std::auto_ptr<FenceSetImpl_> pimpl;
	friend class CommandBufferBase_;
	friend class CommandBuffer_;
public:
	FenceSet_();
	FenceSet_(Fence* fences, uint32 numFences);
	~FenceSet_();
	void add(const Fence& fence);
	void add(Fence* fences, uint32 numFences);
	void assign(Fence* fences, uint32 numFences);
	void clear();
	const Fence& operator[](uint32 index)const;
	Fence& operator[](uint32 index);
	const Fence& get(uint32 index)const { return operator[](index); }
	Fence& get(uint32 index) { return operator[](index); }

	bool waitOne(uint64 timeoutNanos = uint64(-1));
	bool waitAll(uint64 timeoutNanos = uint64(-1));
	void resetAll();

	const void* getNativeFences()const;
	uint32 getNativeFencesCount() const;
};

class SemaphoreSetImpl_;
class SemaphoreSet_
{
	std::auto_ptr<SemaphoreSetImpl_> pimpl;
	friend class CommandBufferBase_;
	friend class CommandBuffer_;
public:
	SemaphoreSet_();
	SemaphoreSet_(Semaphore* semaphores, uint32 numSemaphores);
	~SemaphoreSet_();
	const Semaphore& operator[](uint32 index)const;
	Semaphore& operator[](uint32 index);
	const Semaphore& get(uint32 index)const { return operator[](index); }
	Semaphore& get(uint32 index) { return operator[](index); }

	void add(const Semaphore& semaphore);
	void add(Semaphore* semaphores, uint32 numSemaphores);
	void assign(Semaphore* semaphores, uint32 numSemaphores);
	void clear();

	const void* getNativeSemaphores()const;
	uint32 getNativeSemaphoresCount() const;
};

class EventSetImpl_;

class EventSet_
{
	std::auto_ptr<EventSetImpl_> pimpl;
	friend class CommandBufferBase_;
	friend class CommandBuffer_;
public:
	EventSet_();
	EventSet_(Event* events, uint32 numEvents);
	~EventSet_();
	const Event& operator[](uint32 index)const;
	Event& operator[](uint32 index);
	const Event& get(uint32 index)const { return operator[](index); }
	Event& get(uint32 index) { return operator[](index); }

	void add(const Event& fence);
	void add(Event* events, uint32 numEvents);
	void assign(Event* events, uint32 numEvents);
	void clear();

	void setAll();
	void resetAll();
	bool any();
	bool all();
	bool anyNotSet() { return !all(); }
	bool allUnset() { return !any(); }

	const void* getNativeEvents()const;
	uint32 getNativeEventsCount() const;
};
}

typedef RefCountedResource<impl::EventSet_> EventSet;
typedef RefCountedResource<impl::FenceSet_> FenceSet;
typedef RefCountedResource<impl::SemaphoreSet_> SemaphoreSet;
}//namespace api
}//namespace pvr


