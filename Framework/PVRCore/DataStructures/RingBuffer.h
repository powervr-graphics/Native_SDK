/*!
\brief Contains a RingBuffer data structure implementation.
\file PVRCore/DataStructures/RingBuffer.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#pragma warning(push)
#pragma warning(disable:4127)
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace pvr {

template<typename> class RingBuffer;

/// <summary>A swap function for the RingBuffer. Efficiently exchanges the items between two ring buffers. No
/// actual item copies happen.</summary>
/// <param name="left">The first RingBuffer to swap</param>
/// <param name="right">The second RingBuffer to swap</param>
/// <typeparam name="Item">The type of the items stored in the RingBuffer</typeparam>
template<typename Item>
void swap(RingBuffer<Item>& left, RingBuffer<Item>& right);

/// <summary>A classic, templated, efficient RingBuffer implementation. Supports operations at both ends (front,
/// back) and dynamic resizing when full. Does not at current implementations support iterators.</summary>
/// <typeparam name="ItemType">The type of the items stored in the RingBuffer</typeparam>
template<typename ItemType> class RingBuffer
{
	template<typename> friend void swap(RingBuffer<ItemType>, RingBuffer<ItemType>);
	size_t getIndex(size_t item)const
	{
		size_t idx = _first + item;
		if (idx >= _capacity) { idx -= _capacity; }
		return idx;
	}
public:
	/// <summary>Get the item at the head of the ringbuffer.</summary>
	/// <returns>The item at the head of the ringbuffer.</returns>
	ItemType& front(){	return _store[_first];	}

	/// <summary>Get the item at the head of the ringbuffer. (const)</summary>
	/// <returns>The item at the head of the ringbuffer. (const)</returns>
	const ItemType& front() const{	return _store[_first];	}
	
	/// <summary>Get the item at the tail of the ringbuffer.</summary>
	/// <returns>The item at the tail of the ringbuffer.</returns>
	ItemType& back(){	return _store[getIndex(_size - 1)];	}
	
	/// <summary>Get the item at the tail of the ringbuffer. const</summary>
	/// <returns>The item at the tail of the ringbuffer. const</returns>
	const ItemType& back() const{	return _store[getIndex(_size - 1)];	}

	/// <summary>Logical indexing. 0 is the head, size()-1 is the tail (const)</summary>
	/// <param name="idx">The index of the item to get. Index is always the logical index in the buffer (0:head,
	/// size-1:tail), not the physical index in the underlying storage.</param>
	/// <returns>const The element at index</returns>
	const ItemType& operator[](size_t idx)const
	{
		size_t tmp = _first + idx;
		if (tmp >= _capacity) { tmp -= _capacity; }
		return _store[tmp];
	}

	/// <summary>Logical indexing. 0 is the head, size()-1 is the tail</summary>
	/// <param name="idx">The index of the item to get. Index is always the logical index in the buffer (0:head,
	/// size-1:tail), not the physical index in the underlying storage.</param>
	/// <returns>const The element at index</returns>
	ItemType& operator[](size_t idx)
	{
		size_t tmp = _first + idx;
		if (tmp >= _capacity) { tmp -= _capacity; }
		return _store[tmp];
	}
	/// <summary>Constructor</summary>
	RingBuffer() : _store(NULL), _first(0), _size(0), _capacity(0) {}
	
	/// <summary>Copy constructor. Does a deep copy of the contents.</summary>
	/// <param name="rhs">The item to copy</param>
	RingBuffer(const RingBuffer& rhs) : _store(static_cast<ItemType*>(malloc(rhs._capacity* sizeof(ItemType)))), _first(0), _size(rhs._size),
		_capacity(rhs._capacity)
	{
		copy_items<false>(rhs._store, _store, rhs._size, rhs._first, rhs._capacity);
	}

	/// <summary>Move constructor. Will take the resources of the right hand side.</summary>
	/// <param name="rhs">The object to move. Will be left empty.</param>
	RingBuffer(RingBuffer&& rhs) : _store(NULL), _first(0), _size(0), _capacity(0)
	{
		swap(*this, rhs);
	}

	/// <summary>Copy ssignment operator. Does a deep copy of the contents.</summary>
	/// <param name="rhs">The object to copy.</param>
	/// <returns>Reference of this object.</returns>
	/// <remarks>Utilizes copy-and-swap. Self-assignment safe.</remarks>
	RingBuffer& operator=(RingBuffer rhs){	swap(*this, rhs); return *this;	}
	
	/// <summary>Move ssignment operator. Will take the resources of the right hand side.</summary>
	/// <param name="rhs">The object to move.</param>
	/// <returns>Reference of this object.</returns>
	/// <remarks>Utilizes copy-and-swap. Self-assignment safe.</remarks>
	RingBuffer&& operator=(RingBuffer && rhs)
	{
		clear(); free(_store);
		_store = rhs._store; rhs._store = NULL;
		_first = rhs._first; rhs._first = 0;
		_size = rhs._size; rhs._size = 0;
		_capacity = rhs._capacity; rhs._capacity = 0; return *this;
	}

	/// <summary>Empty the buffer. Does not deallocate the backing store.</summary>
	void clear()
	{
		for (size_t i = 0; i < _size; ++i)
		{
			_store[getIndex(i)].~ItemType();
		}
		_size = 0;
		_first = 0;
	}

	/// <summary>dtor</summary>
	~RingBuffer(){ clear(); free(_store); }

	/// <summary>Add an item to the back of the buffer. Auto grows.</summary>
	/// <param name="item">The item to add to the buffer.</param>
	void push_back(const ItemType& item)
	{
		if (size() >= capacity())
		{
			reserve(std::max<size_t>(1, capacity() * 2));
		}
		construct(item, getIndex(_size++));
	}

	/// <summary>Add an item to the front of the buffer. Auto grows.</summary>
	/// <param name="item">The item to add to the buffer.</param>
	void push_front(const ItemType& item)
	{
		if (size() >= capacity())
		{
			reserve(std::max<size_t>(1, capacity() * 2));
		}
		_size++;
		if (!_first)
		{
			_first += _capacity;
		}
		-- _first;
		construct(item, _first);
	}

	/// <summary>Delete the item at the back of the buffer. Undefined if the buffer is empty.</summary>
	void pop_back()
	{
		size_t idx = getIndex(--_size);
		_store[idx].~ItemType();
	}

	/// <summary>Delete the item at the front of the buffer. Undefined if the buffer is empty.</summary>
	void pop_front()
	{
		_store[_first].~ItemType();
		++_first;
		if (_first == _capacity)
		{
			_first = 0;
		}
		--_size;
	}

	/// <summary>Reserve at least size items of internal space for the ring buffer. Efficient if the number of items that
	/// the buffer needs to accomodate is known in advance.</summary>
	void reserve(size_t size)
	{
		if (size > _capacity)
		{
			ItemType* old = _store;
			_store = (ItemType*)malloc(size * sizeof(ItemType));
			copy_items<true>(old, _store, _capacity, _first, _capacity);
			_first = 0;
			free(old);
			_capacity = size;
		}
	}

	/// <summary>Get the number of items in the RingBuffer. It is no indication of the actual amount of memory
	/// allocated.</summary>
	/// <returns>The number of items in the RingBuffer. It is no indication of the actual amount of memory allocated.
	/// </returns>
	size_t size() const { return _size; }

	/// <summary>Get the number of items that the backing store of the ring buffer may accomodate before needing to
	/// reallocate.</summary>
	/// <returns>The number of items that the backing store of the ring buffer may accomodate before needing to
	/// reallocate.</returns>
	size_t capacity() const { return _capacity; }
private:
	ItemType* _store;
	size_t _first;
	size_t _size;
	size_t _capacity;

	void construct(const ItemType& item, size_t position)
	{
		new(_store + position) ItemType(item);
	}

	void destroy(size_t position){	_store[position].~ItemType();	}

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

/// <summary>Swap the items of two ringbuffers.</summary>
/// <param name="left">The first item to swap</param>
/// <param name="left">The right item to swap</param>
template<typename Item>
void swap(RingBuffer<Item>& left, RingBuffer<Item>& right)
{
	using namespace std;
	swap(left._store, right._store);
	swap(left._first, right._first);
	swap(left._size, right._size);
	swap(left._capacity, right._capacity);
}
}
#pragma warning(pop)