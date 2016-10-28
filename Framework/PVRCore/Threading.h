#include "CoreIncludes.h"
#include "../External/concurrent_queue/blockingconcurrentqueue.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <sstream>
#include <deque>


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
	std::mutex m_mutex;
	std::condition_variable m_cond;
	volatile uint32 m_count;
public:
	Semaphore() : m_count(0) {}
	Semaphore(uint32 initialCount) : m_count(initialCount) {}
	void wait()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		while (m_count == 0)
		{
			m_cond.wait(lock);
		}
		--m_count;
	}

	uint32 count()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_count;
	}

	void signal()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		++m_count;
		m_cond.notify_one();
	}

	void reinit(uint32 count)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_count = count;
	}
};

template<typename T>
class LockedQueue
{
	Semaphore m_sf;
	std::mutex m_mutex;
	std::deque<T> m_store;
public:
	LockedQueue() : m_sf(0) {}
	void produce(const T& item)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_store.push_back(item);
		m_sf.signal();
	}
	bool consume(T& item)
	{
		m_sf.wait();
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_store.empty()) { return false; }
		item = m_store.front();
		m_store.pop_front();
		return true;
	}
	bool isEmpty()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_store.empty();
	}
	size_t itemsRemaining()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_store.size();
	}
	void drain()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		for (int i = 0; i < 1000; ++i)
		{
			m_sf.signal();
		}
	}
};

}
#endif
