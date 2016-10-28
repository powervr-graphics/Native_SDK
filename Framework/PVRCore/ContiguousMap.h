#pragma once
#include <vector>
#include <algorithm>

namespace pvr {

template<typename T_>
class Deque
{
public:
	typedef T_ ElementType;
private:
	enum
	{
		ChunkTargetSize = 512,
		MapChunkSize = 16,
		NumItemsChunk = sizeof(T_) > 256 ? 1 : sizeof(T_) > 128 ? 2 : sizeof(T_) > 64 ? 4 : sizeof(T_) > 32 ? 8 : sizeof(T_) > 16 ? 16 : sizeof(T_) > 8 ? 32 : sizeof(T_) > 4 ? 64 : sizeof(T_) > 2 ? 128 : sizeof(T_) > 1 ? 256 : 512,
		ShiftNumItemsChunk = sizeof(T_) > 256 ? 0 : sizeof(T_) > 128 ? 1 : sizeof(T_) > 64 ? 2 : sizeof(T_) > 32 ? 3 : sizeof(T_) > 16 ? 4 : sizeof(T_) > 8 ? 5 : sizeof(T_) > 4 ? 6 : sizeof(T_) > 2 ? 7 : sizeof(T_) > 1 ? 8 : 9,
		ModNumItemsChunk = sizeof(T_) > 256 ? 1 : sizeof(T_) > 128 ? 3 : sizeof(T_) > 64 ? 7 : sizeof(T_) > 32 ? 15 : sizeof(T_) > 16 ? 31 : sizeof(T_) > 8 ? 63 : sizeof(T_) > 4 ? 127 : sizeof(T_) > 2 ? 255 : sizeof(T_) > 1 ? 511 : 1023,
		ChunkSize = NumItemsChunk * sizeof(T_)
	};
	ElementType** _map;
	ElementType* _static_map[MapChunkSize];
	struct Position
	{
		size_t block;
		size_t offset;
		Position() {}
		Position(size_t block, size_t offset) : block(block), offset(offset) {}
		Position& operator +=(size_t rhs)
		{
			size_t tmp = flatten() + rhs;
			block = tmp / NumItemsChunk;
			offset = tmp % NumItemsChunk;
			return *this;
		}

		Position& operator +=(const Position& rhs)
		{
			size_t tmp = (block + rhs.block) * NumItemsChunk + offset + rhs.offset;
			block = tmp / NumItemsChunk;
			offset = tmp % NumItemsChunk;
			return *this;
		}
		Position& operator ++()
		{
			if (++offset == NumItemsChunk)
			{
				offset = 0;
				++block;
			}
			return *this;
		}
		Position operator ++(int)
		{
			Position pos(*this);
			++(*this);
			return pos;
		}
		Position& operator --()
		{
			if (offset == 0) //wrap
			{
				offset = NumItemsChunk - 1;
				--block;
			}
			return *this;
		}
		Position operator --(int)
		{
			Position pos(*this);
			--(*this);
			return pos;
		}
		Position operator +(const Position& rhs) const
		{
			return Position(*this) += rhs;
		}
		Position& operator -=(size_t rhs)
		{
			size_t tmp = flatten() - rhs;
			block = tmp / NumItemsChunk;
			offset = tmp % NumItemsChunk;
			return *this;
		}
		Position& operator -=(const Position& rhs)
		{
			size_t tmp = (block - rhs.block) * NumItemsChunk + offset - rhs.offset;
			block = tmp / NumItemsChunk;
			offset = tmp % NumItemsChunk;
			return *this;
		}
		Position operator -(const Position& rhs) const
		{
			return Position(*this) -= rhs;
		}
		ptrdiff_t diff(const Position& rhs) const
		{
			return (block - rhs.block) * NumItemsChunk + offset - rhs.offset;
		}
		ptrdiff_t diff(size_t rhs) const
		{
			return flatten() - rhs;
		}
		ptrdiff_t sum(const Position& rhs) const
		{
			return (block + rhs.block) * NumItemsChunk + offset + rhs.offset;
		}
		ptrdiff_t sum(const size_t rhs) const
		{
			return flatten() + rhs;
		}

		size_t flatten()
		{
			return block * NumItemsChunk + offset;
		}
	};
	size_t _map_size;
	Position _first_item;
	Position _first_empty;

public:
	Deque() : _map(_static_map), _map_size(MapChunkSize), _first_item(7, NumItemsChunk / 2), _first_empty(7, NumItemsChunk / 2) {}

	size_t has_space_back()
	{
		return _first_empty.offset < MapChunkSize;
	}
	size_t has_space_front()
	{
		return _first_item.offset > 0;
	}
	size_t has_map_space_back()
	{
		return _first_empty.block < _map_size;
	}
	size_t has_map_space_front()
	{
		return _first_item.block > 0;
	}

	size_t size() const
	{
		return _first_empty.diff(_first_item);
	}

	void grow_map()
	{
		size_t newsize = _map_size + (_map_size >> 1); //50% grow
		ElementType** newmap = new ElementType*[newsize];
		memcpy(newmap + (_map_size >> 2), _map, _map_size * sizeof(ElementType*));
		if (_map_size != MapChunkSize)
		{
			delete[] _map;
		}
		_map = newmap;
		++_map_size;
	}

	void move_map(int offset)
	{
		ptrdiff_t step = offset > 0 ? 1 : -1;
		ptrdiff_t first = offset >= 0 ? _first_item.block : _first_empty.block;
		ptrdiff_t last = offset >= 0 ? _first_empty.block : _first_item.block;
		for (ptrdiff_t i = first; i < last; i += step)
		{
			_map[i + offset] = _map[i];
		}
		_first_item.block += offset;
		_first_empty.block += offset;
	}

	void reserve_map_space()
	{
		size_t offset1 = _first_item.block;
		size_t offset2 = _map_size - _first_empty.block;
		ptrdiff_t offset = (ptrdiff_t)offset2 - (ptrdiff_t)offset1;
		offset >>= 1;
		if (offset) //can gain something...
		{
			move_map(offset);
			assertion(_first_item.block > 0);
			//
		}
		else
		{
			grow_map();
		}
	}

	void reserve_back()
	{
		if (has_space_back()) { return; }

		reserve_map_space();

	}

	void reserve_front()
	{
		if (has_space_back()) { return; }

		reserve_map_space();
	}


	ElementType* get(size_t chunk, size_t offset)
	{
		return _map[chunk] + offset;
	}

	ElementType* find_item(size_t position)
	{
		size_t chunk = position / NumItemsChunk;
		size_t offset = position % NumItemsChunk;
		return get(chunk, offset);
	}

	void push_back()
	{
		reserve_back();
	}
};


template<typename Key_, typename Value_, typename Comparator = std::less<Key_>/**/>
class ContiguousMap
{
public:
	typedef Key_ KeyType;
	typedef Value_ ValueType;
	typedef std::pair<KeyType, ValueType> EntryType;
	typedef std::vector<EntryType> StorageType;
	typedef typename StorageType::iterator iterator;
	typedef typename StorageType::const_iterator const_iterator;
	typedef typename StorageType::reverse_iterator reverse_iterator;
	typedef typename StorageType::const_reverse_iterator const_reverse_iterator;
private:
	StorageType _storage;

public:
	/*!
	   \brief operator =
	   \param initialmap
	   \return Return this object
	 */
	ContiguousMap& operator=(const std::map<Key_, Value_>& initialmap)
	{
		assign(initialmap.begin(), initialmap.end());
	}

	/*!
	   \brief Assign elements in this container
	   \param beginIt Begin iterator
	   \param endIt End iterator
	   \return Return this object
	 */
	template<typename new_iterator>
	ContiguousMap& assign(new_iterator beginIt, new_iterator endIt)
	{
		size_t count = 0;
		//Take a copy to reserve the correct amount of data in the vector
		for (auto tmp = beginIt; tmp != endIt; ++tmp)
		{
			++count;
		}
		_storage.clear();
		if (_storage.capacity() != count)
		{
			_storage.swap(StorageType());
			_storage.reserve(count);
		}

		for (; beginIt != endIt; ++beginIt)
		{
		    operator[](beginIt.first) = beginIt.second;
		}
	}

	/*!
	   \brief Find an element
	   \param key Element's key to find
	   \return Return iterator to the element if found, else return iterator to the end of the container.
	 */
	iterator find(const KeyType& key)
	{
		iterator it = myBinarySearch(begin(), end(), key);
		return it == end() ? end() : it->first == key ? it : end();
	}

	/*!
	   \brief Find an element (const).
	   \param key Element's key to find
	   \return Return iterator to the element if found, else return iterator to the end of the container.
	 */
	const_iterator find(const KeyType& key) const
	{
		const_iterator it = myBinarySearch(begin(), end(), key);
		return it == end() ? end() : it->first == key ? it : end();
	}

	/*!
	   \brief Return a iterator to tbe begining of the container
	 */
	iterator begin(){ return _storage.begin(); }

	/*!
	   \brief Return a iterator to tbe begining of the container (const).
	 */
	const_iterator begin() const{ return _storage.begin(); }

	/*!
	   \brief Returns a reverse iterator to the beginning
	 */
	reverse_iterator rbegin() { return _storage.rbegin(); }

	/*!
	   \brief Returns a reverse iterator to the beginning (const)
	 */
	const_reverse_iterator rbegin() const { return _storage.rbegin(); }

	/*!
	   \brief Returns a iterator to the end of the container.
	 */
	iterator end() { return _storage.end(); }

	/*!
	   \brief Returns a const iterator to the end of the container.
	 */
	const_iterator end() const { return _storage.end(); }

	/*!
	   \brief Returns a reverse iterator to the end
	 */
	reverse_iterator rend() { return _storage.rend(); }

	/*!
	   \brief Returns a const reverse iterator to the end
	   \return
	 */
	const_reverse_iterator rend() const	{ return _storage.rend(); }

	/*!
	   \brief operator []. Reference to the mapped value of the new element if no element with key existed.
			  Otherwise a reference to the mapped value of the existing element whose key is equivalent to key.
	   \param key
	   \return Reference to the value
	 */
	ValueType& operator[](const KeyType& key)
	{
		auto it = find_the_spot(key);
		if (it != _storage.end() && it->first == key)
		{
			return it->second;
		}
		else
		{
			auto it2 = _storage.insert(it, std::make_pair(key, ValueType()));
			return it2->second;
		}
	}

	/*!
	   \brief Return number of entries in the container
	   \return Number of elements
	 */
	size_t size() const { return _storage.size(); }

	/*!
	   \brief Removes specified elements from the container.
	   \param key key value of the elements to remove
	 */
	void erase(const KeyType& key)
	{
		_storage.erase(find_the_spot(key));
	}

	/*!
	   \brief Removes specified elements from the container.
	   \param pos Removes the element at pos
	   \return Return iterator following the last removed element
	 */
	iterator erase(iterator& pos)
	{
		return  _storage.erase(pos);
	}

	/*!
	   \brief Removes specified elements from the container.
	   \param pos Removes the element at pos
	   \return Return iterator following the last removed element
	 */
	iterator erase(const_iterator& pos)
	{
		return  _storage.erase(pos);
	}

	/*!
	   \brief Clear all entries in the container
	 */
	void clear(){ _storage.clear(); }
private:
	struct KeyEntryComparator
	{
		bool operator()(const KeyType& key, const EntryType& entry) const
		{
			return Comparator()(key, entry.first);
		}
		bool operator()(const EntryType& entry, const KeyType& key) const
		{
			return Comparator()(entry.first, key);
		}
	};

	iterator myBinarySearch(iterator begin, iterator end, KeyType value)
	{
		Comparator compare;
		pvr::int32 diff = (int32)(end - begin);
		while (0 < diff)
		{
			pvr::int32 halfdiff = diff >> 1;
			iterator mid = begin;
			mid += halfdiff;

			if (compare(mid->first, value))
			{
				begin = ++mid;
				diff -= halfdiff + 1;
			}
			else
			{
				diff = halfdiff;
			}
		}
		return begin;
	}
	const_iterator myBinarySearch(const_iterator begin, const_iterator end, KeyType value)const
	{
		Comparator compare;
		pvr::int32 diff = int32(end - begin);
		while (0 < diff)
		{
			pvr::int32 halfdiff = diff >> 1;
			const_iterator mid = begin;
			mid += halfdiff;

			if (compare(mid->first, value))
			{
				begin = ++mid;
				diff -= halfdiff + 1;
			}
			else
			{
				diff = halfdiff;
			}
		}
		return begin;
	}

	iterator find_the_spot(const KeyType& key)
	{
		return myBinarySearch(_storage.begin(), _storage.end(), key);
	}

	const_iterator find_the_spot(const KeyType& key) const
	{
		const_iterator it = myBinarySearch(_storage.begin(), _storage.end(), key);
	}

};
}
