/*!
\brief Contains a very lightweight but not automatically-growing resizeable array, and helpers to insert items sorted
into containers.
\file PVRCore/DataStructures/SortedArray.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#pragma once
#include "PVRCore/Base/Types.h"

namespace pvr {

/// <summary>A very lightweight dynamically sized array. Total size: sizeof(pointer) + 4 bytes. 32 bit integers.
/// The array is similar to a vector in that it is dynamically sized, but does not support any kind of push_back(),
/// pop_back() and similar operations as it does not have a capacity that is different to its size - instead it is
/// completely similar to a simple new T[mycapacity] command. It is intended as a vector replacement to optimize
/// the space of classes who require dynamic components but not frequent resizing.</summary>
/// <typeparam name="T">the types of elements the DynamicArray contains</typeparam>
template<typename T> class DynamicArray
{
	T* mydata;
	uint32_t mysize;
public:
	typedef T* iterator;//!< An iterator to an item in the array
	typedef const T* const_iterator;//!< A const iterator to an item in the array

	/// <summary>Copy all elements between two iterators into the array. Previous contents overwritten.</summary>
	/// <typeparam name="myiterator">Type of the iterators.</summary>
	/// <param name="beginIt">Start iterator.</param>
	/// <param name="endIt">End iterator.</param>
	template<typename myiterator>
	void assign(myiterator beginIt, myiterator endIt)
	{
		resize(static_cast<uint32_t>(endIt - beginIt));
		uint32_t count = 0;
		for (auto it = beginIt; it != endIt; ++it)
		{
			mydata[count++] = *it;
		}
	}

	/// <summary>Constructor.</summary>
	DynamicArray(uint32_t initialsize = 0) : mydata(0), mysize(0) { if (initialsize) { resize(initialsize); } }
	/// <summary>Copy constructor.</summary>
	/// <param name="rhs">Item to copy.</rhs>
	DynamicArray(const DynamicArray& rhs) : mydata(0), mysize(0)
	{
		resize(rhs.size());
		for (uint32_t i = 0; i < rhs.size(); ++i)
		{
			mydata[i] = rhs[i];
		}
	}
	/// <summary>Move constructor.</summary>
	/// <param name="rhs">Item to move. May be in an invalid state after this call.</rhs>
	DynamicArray(DynamicArray&& rhs): mydata(rhs.mydata), mysize(rhs.mysize)
	{
		rhs.mysize = 0;
		rhs.mydata = 0;
	}

	/// <summary>Destructor.</summary>
	~DynamicArray() { resize(0); }
	T& operator[](uint32_t index)
	{
		assert(index < mysize);
		return mydata[index];
	}
	/// <summary>Indexing Operator.</summary>
	/// <param name="index">Index</param>
	/// <returns>Reference to the item</returns>
	const T& operator[](uint32_t index) const
	{
		assert(index < mysize);
		return mydata[index];
	}

	/// <summary>Copy Assignment Operator.</summary>
	/// <param name="rhs">Item to copy</param>
	/// <returns>This item</returns>
	DynamicArray& operator=(const DynamicArray& rhs)
	{
		DynamicArray temp(rhs);
		swap(temp);
		return *this;
	}

	/// <summary>Move Assignment Operator.</summary>
	/// <param name="rhs">Item to move. May be in an invalid state after this call.</param>
	/// <returns>This item</returns>
	DynamicArray& operator=(DynamicArray&& rhs)
	{
		resize(0);
		swap(rhs);
		return *this;
	}

	/// <summary>Swap the contents of this dynamic array with another one.</summary>
	/// <param name="rhs">Array with which to swap the items.
	void swap(DynamicArray& rhs)
	{
		std::swap(mysize, rhs.mysize);
		std::swap(mydata, rhs.mydata);
	}

	/// <summary>Empty this array.</summary>
	void clear() { resize(0);}

	/// <summary>Resize this array. Existing items will be copied, others will be default constructed.</summary>
	void resize(uint32_t newSize)
	{
		if (newSize != mysize)
		{
			T* t = 0;
			if (newSize)
			{
				t = new T[newSize];
			}
			for (uint32_t idx = 0; idx < newSize && idx < mysize; ++idx)
			{
				t[idx] = std::move(mydata[idx]);
			}
			delete[] mydata;
			mydata = t;
			mysize = newSize;
		}
	}

	/// <summary>Pointer to the underlying C-array.</summary>
	/// <returns>The underlying C-array</returns>
	T* data() { return mydata; }
	/// <summary>Pointer to the underlying C-array.</summary>
	/// <returns>The underlying C-array</returns>
	const T* data() const { return mydata; }
	/// <summary>Get an iterator to the first item of this array</summary>
	/// <returns>An iterator to the first item of the array</returns>
	iterator begin() { return mydata; }
	/// <summary>Get an iterator to the first item of this array</summary>
	/// <returns>An iterator to the first item of the array</returns>
	const_iterator begin() const { return mydata; }
	/// <summary>Get an iterator to after the last item of this array</summary>
	/// <returns>An iterator to to after the last item of the array</returns>
	iterator end() { return mydata + mysize; }
	/// <summary>Get an iterator to after the last item of this array</summary>
	/// <returns>An iterator to to after the last item of the array</returns>
	const_iterator end() const { return mydata + mysize; }

	/// <summary>Get the number of items in the array</summary>
	/// <returns>The number of items in the array</returns>
	uint32_t size() const { return mysize; }
};


namespace utils {

/// <summary>Insert sorted element in to the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
/// <param name="compare">Comparison operator used for sorting</param>
template<typename container, typename val, typename cmp>
size_t insertSorted(container& cont, typename container::iterator begin, typename container::iterator end,
                    const val& item, const cmp& compare)
{
	typename container::iterator it = std::upper_bound(begin, end, item, compare);
	int64_t offset = static_cast<int64_t>(it - begin);
	cont.insert(it, item);
	return static_cast<size_t>(offset);
}

/// <summary>Insert sorted element in to the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
template<typename container, typename val>
size_t insertSorted(container& cont, typename container::iterator begin, typename container::iterator end, const val& item)
{
	return insertSorted(cont, begin, end, item, std::less<val>());
}

/// <summary>Insert sorted element in to the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="item">Item to insert in to the container</param>
template<typename container, typename val>
size_t insertSorted(container& cont, const val& item)
{
	return insertSorted(cont, cont.begin(), cont.end(), item);
}

/// <summary>Insert sorted element in to the container</summary>
/// <param name="item">Item to insert in to the container</param>
/// <param name="compare">Comparison operator used for sorting</param>
template<typename container, typename val, typename cmp>
size_t insertSorted(container& cont, const val& item, const cmp& compare)
{
	return insertSorted(cont, cont.begin(), cont.end(), item, compare);
}

/// <summary>Insert sorted element, Overwrite if element exist in the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
/// <param name="compare">Comparison operator used for sorting</param>
template<typename container, typename val, typename cmp>
size_t insertSorted_overwrite(container& cont, typename container::iterator begin, typename container::iterator end, const val& item, const cmp& compare)
{
	typename container::iterator it = std::lower_bound(begin, end, item, compare);
	int64_t offset = static_cast<int64_t>(it - begin);
	if (it != end && !(compare(*it, item) || compare(item, *it)))
	{
		*it = item;
	}
	else
	{
		cont.insert(it, item);
	}
	return static_cast<size_t>(offset);
}

/// <summary>Insert sorted element, Overwrite if element exist in the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
template<typename container, typename val>
size_t insertSorted_overwrite(container& cont, typename container::iterator begin, typename container::iterator end, const val& item)
{
	return insertSorted_overwrite(cont, begin, end, item, std::less<val>());
}

/// <summary>Insert sorted element, Overwrite if element exist in the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="item">Item to insert in to the container</param>
template<typename container, typename val>
size_t insertSorted_overwrite(container& cont, const val& item)
{
	return insertSorted_overwrite(cont, cont.begin(), cont.end(), item);
}

/// <summary>Insert sorted element, Overwrite if element exist in the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="item">Item to insert in to the container</param>
/// <param name="compare">Comparison operator used for sorting</param>
template<typename container, typename val, typename cmp>
size_t insertSorted_overwrite(container& cont, const val& item, const cmp& compare)
{
	return insertSorted_overwrite(cont, cont.begin(), cont.end(), item, compare);
}
}
}

//!\endcond