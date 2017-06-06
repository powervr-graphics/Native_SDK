/*!
\brief Specialized data structure required by the PVRApi CommandBuffer.
\file PVRCore/DataStructures/ListOfInterfaces.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/DataStructures/RingBuffer.h"
#include "PVRCore/Base/Types.h"

namespace pvr {
//!\cond NO_DOXYGEN
namespace helpers {
inline unsigned int getOffset(size_t value, uint16 alignment_pot)
{
	unsigned int offset = 0;
	if (alignment_pot)
	{
		int mask = alignment_pot - 1;
		offset = value & mask;
		offset = (alignment_pot - offset)&mask;
	}
	return offset;
}

inline void alignValue(size_t& value, uint16 alignment_pot)
{
	value += getOffset(value, alignment_pot);
}
inline void alignValue(void*& value, uint16 alignment_pot)
{
	value = reinterpret_cast<void*>(reinterpret_cast<size_t>(value) +
	                                getOffset(reinterpret_cast<size_t&>(value), alignment_pot));
}

template<typename T> T* aligned_construct(char*& buffer, int& size)
{
	unsigned int offset = getOffset(size_t(buffer), __alignof(T));
	assert(size >= sizeof(T) + offset);

	size -= offset;
	size -= sizeof(T);
	buffer = (char*)buffer + offset;
	new(buffer)T;
	T* retval = reinterpret_cast<T*>(buffer);
	buffer = (char*)buffer + sizeof(T);
	return retval;
}

template<typename T, typename A1> T* aligned_construct(void*& buffer, size_t& size, const A1& param1)
{
	unsigned int offset = getOffset(size_t(buffer), __alignof(T));
	size_t t = sizeof(T);
	assert(size >= t + offset);

	//Calculate remaining size
	size -= offset;
	size -= t;

	//Actually align the buffer and safely construct the item
	buffer = (char*)buffer + offset;
	new(buffer)T(param1);

	//Prepare the value to return
	T* retval = reinterpret_cast<T*>(buffer);
	//Finally, modify the buffer so that it is ready to accept the same item.
	buffer = (char*)buffer + t;
	return retval;
}


template<typename T, typename A1, typename A2> T* aligned_construct(void*& buffer, size_t& size, const A1& param1,
    const A2& param2)
{
	unsigned int offset = getOffset(size_t(buffer), __alignof(T));
	assert(size >= sizeof(T) + offset);

	size -= offset;
	size -= sizeof(T);
	buffer = (char*)buffer + offset;
	new(buffer)T(param1, param2);
	T* retval = reinterpret_cast<T*>(buffer);
	buffer = (char*)buffer + sizeof(T);
	return retval;
}

template<typename T, typename A1, typename A2, typename A3> T* aligned_construct(void*& buffer, size_t& size,
    const A1& param1, const A2& param2, const A3& param3)
{
	unsigned int offset = getOffset(size_t(buffer), __alignof(T));
	assert(size >= sizeof(T) + offset);

	size -= offset;
	size -= sizeof(T);
	buffer = (char*)buffer + offset;
	new(buffer)T(param1, param2, param3);
	T* retval = reinterpret_cast<T*>(buffer);
	buffer = (char*)buffer + sizeof(T);
	return retval;
}
}
//!\endcond

/// <summary>Specialized data structure required by the PVRApi. Is a list of variable type objects that are kept in a
/// contiguous block of memory and are at the same time a linked list of interfaces.</summary>
template<typename Interface_>
class ListOfInterfaces
{
	template<typename Inner_Interface_>
	class NodeOfInterfaces
	{
	public:
		Inner_Interface_* interfacePtr;
		NodeOfInterfaces* next;

		NodeOfInterfaces(Inner_Interface_* interfacePtr) : interfacePtr(interfacePtr), next(0) {}
		virtual ~NodeOfInterfaces() {}
	};
	template<typename Inner_Interface_, typename ConcreteClass>
	class NodeOfConcreteClasses : public ConcreteClass, public NodeOfInterfaces<Inner_Interface_>
	{
	public:
#pragma warning(push)
#pragma warning(disable:4355)
		NodeOfConcreteClasses() : NodeOfInterfaces<Inner_Interface_>(this) {}
		virtual ~NodeOfConcreteClasses() {}
		template<typename T1> NodeOfConcreteClasses(T1 t1) : ConcreteClass(t1) , NodeOfInterfaces<Inner_Interface_>(this) {}
		template<typename T1, typename T2> NodeOfConcreteClasses(T1 t1, T2 t2) : ConcreteClass(t1, t2), NodeOfInterfaces<Inner_Interface_>(this) {}
		template<typename T1, typename T2, typename T3> NodeOfConcreteClasses(T1 t1, T2 t2, T3 t3) : ConcreteClass(t1, t2, t3), NodeOfInterfaces<Inner_Interface_>(this) {}
#pragma warning(pop)
	};

	class ChunkList
	{
	public:
		ChunkList(size_t chunkSize) : _chunkSize(chunkSize) {}
		~ChunkList()
		{
			destroy(free_list);
			destroy(list);
		}
		static void destroy(RingBuffer<char*>& list)
		{
			while (list.size())
			{
				free(list.front());
				list.pop_front();
			}
		}
		size_t _chunkSize;
		RingBuffer<char*> list;
		RingBuffer<char*> free_list;

		size_t chunkSize()
		{
			return _chunkSize;
		}

		void reset()
		{
			while (list.size() > 1)
			{
				free_list.push_back(list.back());
				list.pop_back();
			}
		}
		void push_back()
		{
			if (free_list.size())
			{
				list.push_back(free_list.back());
				free_list.pop_back();
			}
			else
			{
				list.push_back((char*)malloc(_chunkSize));
			}
		}
		char* back()
		{
			return list.back();
		}
		void size()
		{

		}
	};

	ChunkList _chunks;
	void* _firstEmpty;
	NodeOfInterfaces<Interface_>* _last;
	//This is necessary due to our strange type system
	NodeOfInterfaces<Interface_>* _first;
	size_t _remainingSpace;
public:
	NodeOfInterfaces<Interface_>* first()
	{
		return _first;
	};
	NodeOfInterfaces<Interface_>* last()
	{
		return _last;
	};
	/// <summary>Iterator for const access</summary>
	class const_iterator
	{
	public:
		const_iterator() {}
		const_iterator(const NodeOfInterfaces<Interface_>* node) : node(node) {}
		const_iterator& operator++()
		{
			node = node->next;
			return *this;
		}
		const_iterator& operator++(int)
		{
			iterator tmp = *this;
			++(*this);
			return tmp;
		}
		const Interface_& operator*() { return *node->interfacePtr; }
		const Interface_* operator->() const { return node->interfacePtr; }
		bool operator!=(const const_iterator& rhs) { return node != rhs.node; }
		bool operator==(const const_iterator& rhs) { return node == rhs.node; }
	private:
		const NodeOfInterfaces<Interface_>* node;
	};

	/// <summary>Iterator for non-const access</summary>
	class iterator
	{
	public:
		iterator() {}
		iterator(NodeOfInterfaces<Interface_>* node) : node(node) {}
		iterator(const const_iterator& rhs) : node(rhs.node) {}
		iterator& operator++()
		{
			node = node->next;
			return *this;
		}
		iterator operator++(int)
		{
			iterator tmp = *this;
			++(*this);
			return tmp;
		}
		Interface_& operator*()
		{
			return *node->interfacePtr;
		}
		const Interface_& operator*() const
		{
			return *node->interfacePtr;
		}
		Interface_* operator->() { return node->interfacePtr; }
		const Interface_* operator->() const
		{
			return node->interfacePtr;
		}
		bool operator!=(const iterator& rhs)
		{
			return node != rhs.node;
		}
		bool operator==(const iterator& rhs)
		{
			return node == rhs.node;
		}
	private:
		NodeOfInterfaces<Interface_>* node;
	};

	ListOfInterfaces() : _firstEmpty(0), _last(0), _remainingSpace(0)
	{
		reserveUntyped();
		_first = reinterpret_cast<NodeOfInterfaces<Interface_>*>(_chunks[0].data);
	}
	ListOfInterfaces(size_t chunkSize) : _chunks(chunkSize), _firstEmpty(0), _last(0), _remainingSpace(0)
	{
		assert(chunkSize > sizeof(Interface_));
		reserveUntyped();
		_first = reinterpret_cast<NodeOfInterfaces<Interface_>*>(_chunks.list.front());
	}

	template<typename T>
	void reserve()
	{
		assert(_chunks.chunkSize() >= sizeof(T));
		unsigned int offset = helpers::getOffset(size_t(_firstEmpty), __alignof(T));

		if (_remainingSpace < sizeof(T) + offset)
		{
			reserveUntyped();
		}
	}

	void reserveUntyped()
	{
		_chunks.push_back();

		_remainingSpace = _chunks.chunkSize();
		_firstEmpty = _chunks.back();

	}

	template<typename ItemType>
	void emplace_back()
	{
		typedef NodeOfConcreteClasses<Interface_, ItemType>* NodeType;
		typedef NodeOfInterfaces<Interface_>* ErasedNodeType;

		reserve<NodeType>();

		//First, ensure correct alignment
		helpers::alignValue(_firstEmpty, __alignof(ItemType));

		NodeType theItem = helpers::aligned_construct<NodeOfConcreteClasses<Interface_, ItemType> > (_firstEmpty, _remainingSpace);

		if (_last)
		{
			_last->next = theItem;
			_last = _last->next;
		}
		else //Whooops! first item!
		{
			_last = _first = theItem;
		}

		//From now on we can safely cast it to its new, erased type!
		//Store the next pointer, for now.
		reinterpret_cast<ErasedNodeType>(_last)->next = reinterpret_cast<ErasedNodeType>(_firstEmpty);
	}
	template<typename ItemType, typename T1>
	void emplace_back(const T1& t1)
	{
		typedef NodeOfConcreteClasses<Interface_, ItemType> NodeType;
		typedef NodeOfInterfaces<Interface_> ErasedNodeType;

		reserve<NodeType>();

		//First, ensure correct alignment
		helpers::alignValue(_firstEmpty, __alignof(ItemType));
		//ErasedNodeType oldLast = _last;
		//void* oldEmpty = _firstEmpty;

		//The first argument gets incremented. That's why we kept it.
		NodeType* theItem = helpers::aligned_construct<NodeOfConcreteClasses<Interface_, ItemType>, T1>(_firstEmpty, _remainingSpace, t1);
		//! We need to adjust the pointer for correct polymorphic behaviour !
		//So, first reinterpret it to its ORIGINAL constructed type, and then cast it to the interfacePtr type.
		// :) Complete type erasure :)
		//Also, tie up loose ends: Now that we know the actual type of the item it points to, adjust the actual value pointed to by _last->next
		if (_last)
		{
			_last->next = theItem;
			_last = theItem;
		}
		else //Whooops! first item!
		{
			_last = _first = theItem;
		}

		//From now on we can safely cast it to its new, erased type!
		//Store the next pointer, for now.
		_last->next = reinterpret_cast<ErasedNodeType*>(_firstEmpty);
	}

	template<typename ItemType, typename T1, typename T2>
	void emplace_back(const T1& t1, const T2& t2)
	{
		typedef NodeOfConcreteClasses<Interface_, ItemType> NodeType;
		typedef NodeOfInterfaces<Interface_> ErasedNodeType;

		reserve<NodeType>();

		//First, ensure correct alignment
		helpers::alignValue(_firstEmpty, __alignof(ItemType));
		//ErasedNodeType oldLast = _last;

		NodeType* theItem =
		  helpers::aligned_construct<NodeOfConcreteClasses<Interface_, ItemType>, T1, T2>(_firstEmpty, _remainingSpace, t1, t2);

		//! We need to adjust the pointer for correct polymorphic behaviour !
		//So, first reinterpret it to its ORIGINAL constructed type, and then cast it to the interfacePtr type.
		// :) Complete type erasure :)
		//Also, tie up loose ends: Now that we know the actual type of the item it points to, adjust the actual value pointed to by _last->next
		if (_last)
		{
			_last->next = theItem;
			_last = theItem;
		}
		else //Whooops! first item!
		{
			_first = theItem;
			_last = _first;
		}


		//From now on we can safely cast it to its new, erased type!
		//Store the next pointer, for now.
		_last->next = reinterpret_cast<ErasedNodeType*>(_firstEmpty);
	}

	template<typename ItemType, typename T1, typename T2, typename T3>
	void emplace_back(const T1& t1, const T2& t2, const T3& t3)
	{
		typedef NodeOfConcreteClasses<Interface_, ItemType> NodeType;
		typedef NodeOfInterfaces<Interface_> ErasedNodeType;

		reserve<NodeType>();

		//First, ensure correct alignment
		helpers::alignValue(_firstEmpty, __alignof(ItemType));
		//ErasedNodeType oldLast = _last;
		NodeType* theItem =
		  helpers::aligned_construct<NodeOfConcreteClasses<Interface_, ItemType>, T1, T2, T3>(_firstEmpty, _remainingSpace, t1, t2, t3);
		//! We need to adjust the pointer for correct polymorphic behaviour !
		//So, first reinterpret it to its ORIGINAL constructed type, and then cast it to the interfacePtr type.
		// :) Complete type erasure :)
		//Also, tie up loose ends: Now that we know the actual type of the item it points to, adjust the actual value pointed to by _last->next
		if (_last)
		{
			_last->next = theItem;
			_last = theItem;
		}
		else //Whooops! first item!
		{
			_first = theItem;
			_last = _first;
		}


		//From now on we can safely cast it to its new, erased type!
		//Store the next pointer, for now.
		_last->next = reinterpret_cast<ErasedNodeType*>(_firstEmpty);
	}

	iterator begin()
	{
		return iterator(first());
	}

	const_iterator begin() const
	{
		return const_iterator(first());
	}

	iterator end()
	{
		return iterator(reinterpret_cast<NodeOfInterfaces<Interface_>*>(_firstEmpty));
	}

	const_iterator end() const
	{
		return const_iterator(reinterpret_cast<NodeOfInterfaces<Interface_>*>(_firstEmpty));
	}

	void clear()
	{
		NodeOfInterfaces<Interface_>* node = _first;
		NodeOfInterfaces<Interface_>* next;
		while (node != _firstEmpty)
		{
			next = node->next;
			node->~NodeOfInterfaces();
			node = next;
		}
		_last = 0;
		_remainingSpace = _chunks.chunkSize();
		_chunks.reset();
		_first = reinterpret_cast<NodeOfInterfaces<Interface_>*>(_chunks.list.front());
		_firstEmpty = _first;
	}

	~ListOfInterfaces() { clear(); }
};

}