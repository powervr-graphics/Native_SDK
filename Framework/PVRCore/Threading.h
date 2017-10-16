/*!
\brief MultiThreading tools. Mainly an adaptation of MoodyCamel's BlockingConcurrentQueue.
\file PVRCore/Threading.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "CoreIncludes.h"
#include "PVRCore/DataStructures/RingBuffer.h"
#include "../External/concurrent_queue/blockingconcurrentqueue.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <sstream>
#include <deque>

//  ASYNCHRONOUS FRAMEWORK: Framework async loader base etc //
namespace pvr {
namespace async {
typedef moodycamel::details::mpmc_sema::LightweightSemaphore Semaphore;
typedef RefCountedResource<Semaphore> SemaphorePtr;

template<typename T>
class IFrameworkCleanupObject
{
	bool _destroyed;
protected:
	IFrameworkCleanupObject() : _destroyed(false) {}
public:
	virtual ~IFrameworkCleanupObject() { }
	void cleanup()
	{
		if (!_destroyed) { cleanup_(); _destroyed = true; }
	}
private:
	virtual void cleanup_() = 0;
};


template<typename T>
class IFrameworkAsyncResult : public IFrameworkCleanupObject<T>
{
public:
	typedef T ValueType;
	typedef EmbeddedRefCountedResource<IFrameworkAsyncResult<T>> PointerType;
	typedef void(*Callback)(PointerType);
	std::atomic_bool _inCallback;

	bool isComplete() const
	{
		return _isComplete ? true : (_isComplete = isComplete_());
	}
	ValueType get()
	{
		return get_();
	}
	bool isSuccessful() const { return _successful; }
	Callback _completionCallback;
protected:
	void setTheCallback(Callback completionCallback)
	{
		_completionCallback = completionCallback;
	}
	virtual void executeCallBack(PointerType thisPtr)
	{
		if (_completionCallback) { _inCallback = true; _completionCallback(thisPtr); _inCallback = false; }
	}
	bool _successful;
	IFrameworkAsyncResult() : _successful(false), _isComplete(false), _completionCallback(NULL), _inCallback({ false }) { }
private:
	mutable bool _isComplete;
	virtual bool isComplete_() const = 0;
	virtual T get_() const = 0;
};


template<typename ValueType, typename FutureType, void(*worker)(FutureType) >
class AsyncScheduler
{
public:
	typedef EmbeddedRefCountedResource<IFrameworkAsyncResult<ValueType>> AsyncResult;

	uint32 getNumQueuedItemsApprox()
	{
		return (uint32)_queue.size();
	}

	uint32 getNumQueuedItems()
	{
		_queueSema.wait();
		uint32 retval = (uint32)_queue.size();
		_queueSema.signal();
		return retval;
	}

	virtual ~AsyncScheduler()
	{
		bool didit = false;
		_queueSema.wait(); //protect the _done variable
		if (!_done)
		{
			didit = true;
			_done = true;
			_workSema.signal();
		}
		_queueSema.signal();
		if (didit) // Only join if it was actually running. Duh!
		{
			_thread.join();
		}
	}

protected:
	AsyncScheduler()
	{
		_done = false;
		_thread = std::thread(&AsyncScheduler::run, this);
	}

	Semaphore _workSema;
	Semaphore _queueSema;
	std::deque<FutureType> _queue;
private:
	std::thread _thread;
	std::atomic_bool _done;
	void run()
	{
		Log(Log.Information, "Asynchronous Scheduler (::pvr::async::AsyncScheduler interface) starting. "
		    "1 worker thread spawned. The worker thread will be sleeping as long as no work is being performed, "
		    "and will be released when the async sheduler is destroyed.");
		
		//Are we done? 
		//a) The queue will be empty on the first iteration.
		//b) Even if the queue is empty, consumers may be blocked on results.
		//c) Even if done, consumers may be blocked on results.
		while (!_done || _queue.size()) 
		{
			_queueSema.signal();// First iteration: Signal that we are actually waiting on the queue, essentially priming it,
			// consumers to start adding items. 
			// Subsequent iterations: Release the check we are doing on the queue.
			_workSema.wait();   //Wait for work to arrive
			_queueSema.wait();  //Work has arrived! Or has it? Lock the queue, as we need to check that it is not empty,
			// and to unqueue items.
			if (!_queue.size()) //Are we done? Was this a signal for work or for us to finish?
			{
				// Someone signalled the queue without putting items in. This means we are finished.
				assertion(_done); //duh?
				break;
			}
			else //Yay! Work to do!
			{
				FutureType future = std::move(_queue.front()); //Get work.
				_queue.pop_front();
				_queueSema.signal(); //Release the krak... QUEUE.
				worker(future); //Load a texture! Yay!
			}
			_queueSema.wait(); //Continue. Lock the queue to check it out.
		}
		_queueSema.signal(); //Finish. Release the queue.
		Log(Log.Information, "Asynchronous asset loader closing down. Freeing workers.");
	}
};
}
}

#ifndef PVR_USE_CUSTOM_QUEUE

namespace pvr {
template<typename T>
class LockedQueue
{
	moodycamel::BlockingConcurrentQueue<T> queue;
public:
	typedef moodycamel::ProducerToken ProducerToken;
	typedef moodycamel::ConsumerToken ConsumerToken;
	LockedQueue() {}
	ConsumerToken getConsumerToken()
	{
		return ConsumerToken(queue);
	}
	ProducerToken getProducerToken()
	{
		return ProducerToken(queue);
	}
	void produce(const T& item)
	{
		queue.enqueue(item);
	}
	void produce(ProducerToken& token, const T& item)
	{
		queue.enqueue(token, item);
	}
	template<typename iterator>
	void produceMultiple(iterator items, uint32 numItems)
	{
		queue.enqueue_bulk(items, numItems);
	}
	template<typename iterator>
	void produceMultiple(ProducerToken& token, iterator items, uint32 numItems)
	{
		queue.enqueue_bulk(token, items, numItems);
	}
	bool consume(T& item)
	{
		return queue.wait_dequeue(item);
	}
	bool consume(ConsumerToken& token, T& item)
	{
		return queue.wait_dequeue(token, item);
	}

	template<typename iterator>
	size_t consumeMultiple(iterator firstItem, uint32 maxItems)
	{
		return queue.wait_dequeue_bulk(firstItem, maxItems);
	}

	template<typename iterator>
	size_t consumeMultiple(ConsumerToken& token, iterator firstItem, uint32 maxItems)
	{
		return queue.wait_dequeue_bulk(token, firstItem, maxItems);
	}

	void unblockOne()
	{
		queue.unblock(1);
	}
	void unblockMultiple(uint32 numUnblocks)
	{
		queue.unblock(numUnblocks);
	}
	void reset()
	{
		queue.reset();
	}

	bool isEmpty()
	{
		return itemsRemainingApprox() == 0;
	}
	size_t itemsRemainingApprox()
	{
		return queue.size_approx();
	}
	void drainEmpty()
	{
		queue.drainEmpty();
	}
	void done()
	{
		queue.finishImmediate();
	}
};
}

#else
namespace pvr {
class Semaphore
{
private:
	std::mutex _mutex;
	std::condition_variable _cond;
	volatile uint32 _count;
public:
	Semaphore() : _count(0) {}
	Semaphore(uint32 initialCount) : _count(initialCount) {}
	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		while (_count == 0)
		{
			_cond.wait(lock);
		}
		--_count;
	}

	uint32 count()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		return _count;
	}

	void signal()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		++_count;
		_cond.notify_one();
	}

	void reinit(uint32 count)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_count = count;
	}
};

template<typename T>
class LockedQueue
{
	Semaphore _sf;
	std::mutex _mutex;
	std::deque<T> _store;
public:
	LockedQueue() : _sf(0) {}
	void produce(const T& item)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_store.push_back(item);
		_sf.signal();
	}
	bool consume(T& item)
	{
		_sf.wait();
		std::unique_lock<std::mutex> lock(_mutex);
		if (_store.empty()) { return false; }
		item = _store.front();
		_store.pop_front();
		return true;
	}
	bool isEmpty()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		return _store.empty();
	}
	size_t itemsRemaining()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		return _store.size();
	}
	void drain()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		for (int i = 0; i < 1000; ++i)
		{
			_sf.signal();
		}
	}
};

}
#endif