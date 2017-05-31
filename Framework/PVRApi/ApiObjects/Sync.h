/*!
\brief The PVRApi basic Texture implementation.
\file PVRApi/ApiObjects/Sync.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"

namespace pvr {
namespace api {
namespace impl {
/// <summary>Fence can be used by the host to determine completion of execution of subimmisions to queues. The host
/// can be polled for the fence signal</summary>
class Fence_
{
protected:
	GraphicsContext context;
	Fence_(const GraphicsContext& context) : context(context) {}
public:
	/// <summary>Returns const reference to the graphics context who own this resource</summary>
	const GraphicsContext& getContext() const { return context; }

	/// <summary>Returns reference to the graphics context who own this resource</summary>
	GraphicsContext& getContext() { return context; }

	/// <summary>Host to wait for this fence to be signaled</summary>
	/// <param name="timeoutNanos">Time out period in nanoseconds</param>
	bool wait(uint64 timeoutNanos = uint64(-1)) { return wait_(timeoutNanos); }

	/// <summary>Reset this fence</summary>
	void reset() { return reset_(); }

	/// <summary>Return true if this fence is signaled</summary>
	bool isSignalled() { return isSignalled_(); }

	/// <summary>dtor</summary>
	virtual ~Fence_() {}
private:
	virtual bool wait_(uint64 timeoutNanos) = 0;
	virtual void reset_() = 0;
	virtual bool isSignalled_() = 0;

};

/// <summary>Use to "serialize" access between CommandBuffer submissions and /Queues</summary>
class Semaphore_
{
protected:
	GraphicsContext context;
	Semaphore_(const GraphicsContext& context) : context(context) {}
public:
	/// <summary>dtor</summary>
	virtual ~Semaphore_() {}
};

/// <summary>Event can be used by the host to do fine-grained synchronization of commands, and it
/// can be signalled either from the host (calling set()) or the device (submitting a setEvent() command).</summary>
class Event_
{
protected:
	GraphicsContext context;
	Event_(const GraphicsContext& context) : context(context) {}
public:
	/// <summary>dtor</summary>
	virtual ~Event_() {}

	/// <summary>Set this event</summary>
	void set();

	/// <summary>Reset this event</summary>
	void reset();

	/// <summary>Return true if this event is set</summary>
	bool isSet();
};
}

/// <summary>A Global memory barrier used for memory accesses for all memory objects.</summary>
struct MemoryBarrier
{
	types::AccessFlags srcMask;
	types::AccessFlags dstMask;
	MemoryBarrier(): srcMask(types::AccessFlags(0)), dstMask(types::AccessFlags(0)) {}
	MemoryBarrier(types::AccessFlags srcMask, types::AccessFlags dstMask): srcMask(srcMask), dstMask(dstMask) {}
};

/// <summary>A Buffer memory barrier used only for memory accesses involving a specific range of the specified
/// buffer object. It is also used to transfer ownership of an buffer range from one queue family to another.
/// </summary>
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

/// <summary>A Image memory barrier used only for memory accesses involving a specific subresource range of the
/// specified image object. It is also used to perform a layout transition for an image subresource range, or to
/// transfer ownership of an image subresource range from one queue family to another.</summary>
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

/// <summary>A memory barrier into the command stream. Used to signify that some types of pending operations
/// from before the barrier must have finished before the commands after the barrier start executing.</summary>
class MemoryBarrierSet
{
	MemoryBarrierSet(const MemoryBarrierSet&); //deleted
	MemoryBarrierSet& operator=(const MemoryBarrierSet&); //deleted
	typedef std::vector<MemoryBarrier> MemBarrierContainer;
	typedef std::vector<ImageAreaBarrier> ImgBarrierContainer;
	typedef std::vector<BufferRangeBarrier> BuffBarrierContainer;
	MemBarrierContainer memBarriers;
	ImgBarrierContainer imgBarriers;
	BuffBarrierContainer bufBarriers;
public:
<<<<<<< HEAD
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
=======
	MemoryBarrierSet() {}

	///<summary>Clear this object of all barriers</summary>
	MemoryBarrierSet& clearAllBarriers()
	{
		memBarriers.clear();
		imgBarriers.clear();
		bufBarriers.clear();
		return *this;
	}

	///<summary>Clear this object of all Memory barriers</summary>
	MemoryBarrierSet& clearAllMemoryBarriers()
	{
		memBarriers.clear(); return *this;
	}

	///<summary>Clear this object of all Buffer barriers</summary>
	MemoryBarrierSet& clearAllBufferRangeBarriers()
	{
		bufBarriers.clear(); return *this;
	}


	///<summary>Clear this object of all Image barriers</summary>
	MemoryBarrierSet& clearAllImageAreaBarriers()
	{
		imgBarriers.clear(); return *this;
	}


	/// <summary>Add a generic Memory barrier.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSet& addBarrier(MemoryBarrier barrier)
	{
		memBarriers.push_back(barrier);
		return *this;
	}

	/// <summary>Add a Buffer Range barrier, signifying that operations on a part of a buffer
	/// must complete before other operations on that part of the buffer execute.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSet& addBarrier(const BufferRangeBarrier& barrier)
	{
		bufBarriers.push_back(barrier);
		return *this;
	}


	/// <summary>Add a Buffer Range barrier, signifying that operations on a part of an Image
	/// must complete before other operations on that part of the Image execute.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSet& addBarrier(const ImageAreaBarrier& barrier)
	{
		imgBarriers.push_back(barrier);
		return *this;
	}


	/// <summary>Get an array of the MemoryBarrier object of this set.</summary>
	/// <returns>All MemoryBarrier objects that this object contains</returns>
	const MemBarrierContainer& getMemoryBarriers() const { return this->memBarriers; }

	/// <summary>Get an array of the Image Barriers of this set.</summary>
	/// <returns>All MemoryBarrier objects that this object contains</returns>
	const ImgBarrierContainer& getImageBarriers() const { return this->imgBarriers; }

	/// <summary>Get an array of the Buffer Barriers of this set.</summary>
	/// <returns>All MemoryBarrier objects that this object contains</returns>
	const BuffBarrierContainer& getBufferBarriers() const { return this->bufBarriers; }
>>>>>>> 1776432f... 4.3
};


/// <summary>A framework Fence object (automatic reference counted).</summary>
typedef RefCountedResource<impl::Fence_> Fence;
/// <summary>A framework Semaphore object (automatic reference counted).</summary>
typedef RefCountedResource<impl::Semaphore_> Semaphore;
/// <summary>A framework Semaphore object (automatic reference counted).</summary>
typedef RefCountedResource<impl::Event_> Event;

//!\cond NO_DOXYGEN
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
//!\endcond

}//namespace api
}//namespace pvr

