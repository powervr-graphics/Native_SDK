/*!*********************************************************************************************************************
\file         PVRCore\RingBuffer.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains a RingBuffer data structure implementation.
***********************************************************************************************************************/
#pragma once
#pragma warning(push)
#pragma warning(disable:4127)
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace pvr {

template<typename> class RingBuffer;
template<typename Item>
void swap(RingBuffer<Item>& left, RingBuffer<Item>& right);

/*!*********************************************************************************************************************
\brief   A classic, templated, efficient RingBuffer implementation. Supports operations at both ends (front, back) and dynamic
         resizing when full. Does not at current implementations support iterators.
\tparam ItemType The type of the items stored in the RingBuffer
***********************************************************************************************************************/
template<typename ItemType> class RingBuffer
{
	template<typename> friend void swap(RingBuffer<ItemType>, RingBuffer<ItemType>);
	size_t getIndex(size_t item)const
	{
		size_t idx = m_first + item;
		if (idx >= m_capacity) { idx -= m_capacity; }
		return idx;
	}
public:
	/*!*********************************************************************************************************************
	\return The item at the head of the ringbuffer.
	***********************************************************************************************************************/
	ItemType& front(){	return m_store[m_first];	}

	/*!*********************************************************************************************************************
	\return The item at the head of the ringbuffer. const
	***********************************************************************************************************************/
	const ItemType& front() const{	return m_store[m_first];	}
	
	/*!*********************************************************************************************************************
	\return The item at the tail of the ringbuffer.
	***********************************************************************************************************************/
	ItemType& back(){	return m_store[getIndex(m_size - 1)];	}
	
	/*!*********************************************************************************************************************
	\return The item at the tail of the ringbuffer. const
	***********************************************************************************************************************/
	const ItemType& back() const{	return m_store[getIndex(m_size - 1)];	}

	/*!*********************************************************************************************************************
	\brief Indexing. 0 is the head, size()-1 is the tail. Wraps automatically. const.
	\return const Element
	***********************************************************************************************************************/
	const ItemType& operator[](size_t idx)const
	{
		size_t tmp = m_first + idx;
		if (tmp >= m_capacity) { tmp -= m_capacity; }
		return m_store[tmp];
	}

	/*!*********************************************************************************************************************
	\brief Indexing. 0 is the head, size()-1 is the tail. Wraps automatically.
	\return Element
	***********************************************************************************************************************/
	ItemType& operator[](size_t idx)
	{
		size_t tmp = m_first + idx;
		if (tmp >= m_capacity) { tmp -= m_capacity; }
		return m_store[tmp];
	}
	/*!*********************************************************************************************************************
	\brief ctor.
	***********************************************************************************************************************/
	RingBuffer() : m_store(NULL), m_first(0), m_size(0), m_capacity(0) {}
	
	/*!*********************************************************************************************************************
	\brief Copy constructor
	***********************************************************************************************************************/
	RingBuffer(const RingBuffer& rhs) : m_store(static_cast<ItemType*>(malloc(rhs.m_capacity* sizeof(ItemType)))), m_first(0), m_size(rhs.m_size),
		m_capacity(rhs.m_capacity)
	{
		copy_items<false>(rhs.m_store, m_store, rhs.m_size, rhs.m_first, rhs.m_capacity);
	}

	/*!*********************************************************************************************************************
	\brief Copy constructor
	\param rhs R-Value reference to the object to copy. Called in move operator
	***********************************************************************************************************************/
	RingBuffer(RingBuffer&& rhs) : m_store(NULL), m_first(0), m_size(0), m_capacity(0)
	{
		swap(*this, rhs);
	}

	/*!*********************************************************************************************************************
	\brief Assignment operator
	\return L-Value reference of this object.
	***********************************************************************************************************************/
	RingBuffer& operator=(RingBuffer rhs){	swap(*this, rhs); return *this;	}
	
	/*!*********************************************************************************************************************
	\brief Assignment operator
	\return R-Value reference of this object. Used for move operator
	***********************************************************************************************************************/
	RingBuffer&& operator=(RingBuffer && rhs)
	{
		clear(); free(m_store);
		m_store = rhs.m_store; rhs.m_store = NULL;
		m_first = rhs.m_first; rhs.m_first = 0;
		m_size = rhs.m_size; rhs.m_size = 0;
		m_capacity = rhs.m_capacity; rhs.m_capacity = 0; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Empty the buffer. Does not deallocate the backing store.
	***********************************************************************************************************************/
	void clear()
	{
		for (size_t i = 0; i < m_size; ++i)
		{
			m_store[getIndex(i)].~ItemType();
		}
		m_size = 0;
		m_first = 0;
	}

	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	~RingBuffer(){	clear();	free(m_store);	}

	/*!*********************************************************************************************************************
	\brief Add an item to the back of the buffer. Auto grows.
	***********************************************************************************************************************/
	void push_back(const ItemType& item)
	{
		if (size() >= capacity())
		{
			reserve(std::max<size_t>(1, capacity() * 2));
		}
		construct(item, getIndex(m_size++));
	}

	/*!*********************************************************************************************************************
	\brief Add an item to the front of the buffer. Auto grows.
	***********************************************************************************************************************/
	void push_front(const ItemType& item)
	{
		if (size() >= capacity())
		{
			reserve(std::max<size_t>(1, capacity() * 2));
		}
		m_size++;
		if (!m_first)
		{
			m_first += m_capacity;
		}
		-- m_first;
		construct(item, m_first);
	}

	/*!*********************************************************************************************************************
	\brief Delete the item at the back of the buffer. Undefined if the buffer is empty.
	***********************************************************************************************************************/
	void pop_back()
	{
		size_t idx = getIndex(--m_size);
		m_store[idx].~ItemType();
	}

	/*!*********************************************************************************************************************
	\brief Delete the item at the front of the buffer. Undefined if the buffer is empty.
	***********************************************************************************************************************/
	void pop_front()
	{
		m_store[m_first].~ItemType();
		++m_first;
		if (m_first == m_capacity)
		{
			m_first = 0;
		}
		--m_size;
	}

	/*!*********************************************************************************************************************
	\brief Reserve at least size items of internal space for the ring buffer. Useful if the number of items that the buffer needs to 
	       accomodate is known in advance.
	***********************************************************************************************************************/
	void reserve(size_t size)
	{
		if (size > m_capacity)
		{
			ItemType* old = m_store;
			m_store = (ItemType*)malloc(size * sizeof(ItemType));
			copy_items<true>(old, m_store, m_capacity, m_first, m_capacity);
			m_first = 0;
			free(old);
			m_capacity = size;
		}
	}

	/*!*********************************************************************************************************************
	\return The number of items in the RingBuffer. It is no indication of the actual amount of memory allocated.
	***********************************************************************************************************************/
	size_t size() const	{	return m_size;	}
private:
	ItemType* m_store;
	size_t m_first;
	size_t m_size;
	size_t m_capacity;


	size_t capacity() const	{	return m_capacity;	}

	void construct(const ItemType& item, size_t position)
	{
		new(m_store + position) ItemType(item);
	}

	void destroy(size_t position){	m_store[position].~ItemType();	}

	template<bool move>
	static void copy_items(ItemType* from, ItemType* to, size_t number, size_t from_start, size_t from_capacity)
	{
		for (size_t i = 0; i < number; ++i)
		{
			new(to + i)ItemType(from[from_start]);

			if (move)
			{
				from[from_start].~ItemType();
			}
			++from_start;
			if (from_start >= from_capacity) { from_start -= from_capacity; }
		}
	}
};

/*!*********************************************************************************************************************
\brief Swap the items of two ringbuffers.
***********************************************************************************************************************/
template<typename Item>
void swap(RingBuffer<Item>& left, RingBuffer<Item>& right)
{
	using namespace std;
	swap(left.m_store, right.m_store);
	swap(left.m_first, right.m_first);
	swap(left.m_size, right.m_size);
	swap(left.m_capacity, right.m_capacity);
}
}
#pragma warning(pop)