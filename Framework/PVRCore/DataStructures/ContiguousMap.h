/*!
\brief Contains a map implementation based on contiguous storage, optimised for retrieval and iteration, and allowing
array access of the elements. Based on std::vector with binary search sorting (O(logn) retrieval, O(N)
insertion& removal)
\file PVRCore/DataStructures/ContiguousMap.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <map>
#include <vector>
#include <algorithm>

namespace pvr {

/// <summary> A map that uses a std::vector as an underlying sparse storage and uses
/// binary search for logarithmic time key lookup or indexing for constant time lookup.
/// a) it can be indexed either by index in constant time, or by key in logarithmic time.
/// b) Guaranteed to have contiguous storage
/// c) If items are removed, iterators are invalidated.</summary>
template<typename Key_, typename Value_, typename Comparator = std::less<Key_>/**/>
class ContiguousMap
{
public:
	typedef Key_ KeyType; //!< The type of the Key stored in the map
	typedef Value_ ValueType; //!< The type of the Value stored in the map
	typedef std::pair<KeyType, ValueType> EntryType; //!< The type of the map entry (key, value) used in the map
	typedef std::vector<EntryType> StorageType; //!< The type of the contiguous backing store of the map
	typedef typename StorageType::iterator iterator; //!< The type of the (linear) iterator of the map
	typedef typename StorageType::const_iterator const_iterator; //!< The type of the (linear) const iterator of the map
	typedef typename StorageType::reverse_iterator reverse_iterator; //!< The type of the (linear) reverse iterator of the map
	typedef typename StorageType::const_reverse_iterator const_reverse_iterator; //!< The type of the (linear) reverse const iterator of the map
private:
	StorageType _storage;

public:
	/// <summary>Copy assignment operator from std::map</summary>
	/// <param name="initialmap">std::map whose items will be used to initialize this map</param>
	/// <returns>This object</returns>
	ContiguousMap& operator=(const std::map<Key_, Value_>& initialmap)
	{
		assign(initialmap.begin(), initialmap.end());
	}

	/// <summary>Assign elements in this container</summary>
	/// <param name="beginIt">Begin iterator</param>
	/// <param name="endIt">End iterator</param>
	/// <returns>This object</returns>
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

	/// <summary>Find an element</summary>
	/// <param name="key">Element's key to find</param>
	/// <returns>Return iterator to the element if found, else return iterator to the end of the container.</returns>
	iterator find(const KeyType& key)
	{
		iterator it = myBinarySearch(begin(), end(), key);
		return it == end() ? end() : it->first == key ? it : end();
	}

	/// <summary>Find an element (const).</summary>
	/// <param name="key">Element's key to find</param>
	/// <returns>Return iterator to the element if found, else return iterator to the end of the container.</returns>
	const_iterator find(const KeyType& key) const
	{
		const_iterator it = myBinarySearch(begin(), end(), key);
		return it == end() ? end() : it->first == key ? it : end();
	}

	/// <summary>Return an iterator to tbe begining of the container</summary>
	/// <returns>An iterator to the beginning of the backing store</returns>
	iterator begin() { return _storage.begin(); }

	/// <summary>Return a iterator to tbe begining of the container (const).</summary>
	/// <returns>A const iterator to the beginning of the backing store</returns>
	const_iterator begin() const { return _storage.begin(); }

	/// <summary>Returns a reverse iterator to the beginning</summary>
	/// <returns>A reverse iterator to the beginning of the backing store</returns>
	reverse_iterator rbegin() { return _storage.rbegin(); }

	/// <summary>Returns a reverse iterator to the beginning (const)</summary>
	/// <returns>A const reverse iterator to the beginning of the backing store</returns>
	const_reverse_iterator rbegin() const { return _storage.rbegin(); }

	/// <summary>Returns a iterator to the end of the container.</summary>
	/// <returns>An iterator to the end of the backing store</returns>
	iterator end() { return _storage.end(); }

	/// <summary>Returns a const iterator to the end of the container.</summary>
	/// <returns>A const iterator to the end of the backing store</returns>
	const_iterator end() const { return _storage.end(); }

	/// <summary>Returns a reverse iterator to the end</summary>
	/// <returns>A reverse iterator to the end of the backing store</returns>
	reverse_iterator rend() { return _storage.rend(); }

	/// <summary>Returns a const reverse iterator to the end</summary>
	/// <returns>A const reverse iterator to the end of the backing store</returns>
	const_reverse_iterator rend() const { return _storage.rend(); }

	/// <summary>operator []. Reference to the mapped value of the new element if no element with key existed. Otherwise a
	/// reference to the mapped value of the existing element whose key is equivalent to key.</summary>
	/// <param name="key">The key the value that corresponds to which will be retrieved</param>
	/// <returns>The value that is stored associated to <paramref name="key"/></returns>
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

	/// <summary>Return number of entries in the container</summary>
	/// <returns>Number of elements</returns>
	size_t size() const { return _storage.size(); }

	/// <summary>Removes specified elements from the container.</summary>
	/// <param name="key">key value of the elements to remove</param>
	void erase(const KeyType& key)
	{
		_storage.erase(find_the_spot(key));
	}

	/// <summary>Removes specified elements from the container.</summary>
	/// <param name="pos">Removes the element at pos</param>
	/// <returns>Return iterator following the last removed element</returns>
	iterator erase(iterator& pos)
	{
		return  _storage.erase(pos);
	}

	/// <summary>Removes specified elements from the container.</summary>
	/// <param name="pos">Removes the element at pos</param>
	/// <returns>Return iterator following the last removed element</returns>
	iterator erase(const_iterator& pos)
	{
		return  _storage.erase(pos);
	}

	/// <summary>Clear all entries in the container</summary>
	void clear() { _storage.clear(); }
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
		int32_t diff = static_cast<int32_t>(end - begin);
		while (0 < diff)
		{
			int32_t halfdiff = diff >> 1;
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
		int32_t diff = static_cast<int32_t>(end - begin);
		while (0 < diff)
		{
			int32_t halfdiff = diff >> 1;
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