//!\cond NO_DOXYGEN
#pragma once
#include "PVRCore/Types.h"
#include "PVRCore/Assert_.h"

namespace pvr {

/*!**************************************************************************************************************
\brief A very lightweight dynamically sized array. Total size: sizeof(pointer) + 4 bytes. 32 bit integers. The
array is similar to a vector in that it is dynamically sized, but does not support any kind of push_back(), pop_back()
and similar operations as it does not have a capacity that is different to its size - instead it is completely
similar to a simple new ITEMTYPE[] command.
It is conceptually equal to a vector whose size is always exactly the same as its capacity. It is, in general,
intended to be used in optimizing the space of classes who require dynamic components but not frequent resizing.
The important operations are operator[] and resize().
\tparam T the types of elements the DynamicArray supports
****************************************************************************************************************/
template<typename T> class DynamicArray
{
	T* mydata;
	uint32 mysize;
public:
	typedef T* iterator;
	typedef const T* const_iterator;

	template<typename myiterator>
	void assign(myiterator beginIt, myiterator endIt)
	{
		resize((uint32)(endIt - beginIt));
		uint32 count = 0;
		for (auto it = beginIt; it != endIt; ++it)
		{
			mydata[count++] = *it;
		}
	}

    DynamicArray(uint32 initialsize = 0) :mydata(0), mysize(0) { if (initialsize) { resize(initialsize); } }
    DynamicArray(const DynamicArray& rhs) : mydata(0), mysize(0)
	{
		resize(rhs.size());
		for (uint32 i = 0; i < rhs.size(); ++i)
		{
			mydata[i] = rhs[i];
		}
	}
    DynamicArray(DynamicArray&& rhs): mydata(rhs.mydata), mysize(rhs.mysize)
	{
		rhs.mysize = 0;
		rhs.mydata = 0;
	}

	~DynamicArray() { resize(0); }
	T& operator[](uint32 index)
	{
		PVR_ASSERTION(index < mysize);
		return mydata[index];
	}
	const T& operator[](uint32 index) const
	{
		PVR_ASSERTION(index < mysize);
		return mydata[index];
	}

	DynamicArray& operator=(const DynamicArray& rhs)
	{
		DynamicArray temp(rhs);
		swap(temp);
		return *this;
	}

	DynamicArray& operator=(DynamicArray&& rhs)
	{
		resize(0);
		swap(rhs);
		return *this;
	}

	void swap(DynamicArray& rhs)
	{
		std::swap(mysize, rhs.mysize);
		std::swap(mydata, rhs.mydata);
	}

	void clear() { resize(0);}

	void resize(uint32 newSize)
	{
		if (newSize != mysize)
		{
			T* t = 0;
			if (newSize)
			{
				t = new T[newSize];
			}
			for (uint32 idx = 0; idx < newSize && idx < mysize; ++idx)
			{
				t[idx] = std::move(mydata[idx]);
			}
			delete[] mydata;
			mydata = t;
			mysize = newSize;
		}
	}

	T* data() { return mydata; }
	const T* data() const { return mydata; }
	iterator begin() { return mydata; }
	const_iterator begin() const { return mydata; }
	iterator end() { return mydata + mysize; }
	const_iterator end() const { return mydata + mysize; }

	uint32 size() const { return mysize; }
};


namespace utils {

/*!**************************************************************************************************************
\brief Insert sorted element in to the container
\param cont Container to insert the element into.
\param begin Container range begin
\param end Container range end
\param item Item to insert in to the container
\param compare Comparison operator used for sorting
****************************************************************************************************************/
template<typename container, typename val, typename cmp>
size_t insertSorted(container& cont, typename container::iterator begin, typename container::iterator end,
                    const val& item, const cmp& compare)
{
	typename container::iterator it = std::upper_bound(begin, end, item, compare);
	int64 offset = int64(it - begin);
	cont.insert(it, item);
	return (size_t)offset;
}

/*!**************************************************************************************************************
\brief Insert sorted element in to the container
\param cont Container to insert the element into.
\param begin Container range begin
\param end Container range end
\param item Item to insert in to the container
****************************************************************************************************************/
template<typename container, typename val>
size_t insertSorted(container& cont, typename container::iterator begin, typename container::iterator end, const val& item)
{
	return insertSorted(cont, begin, end, item, std::less<val>());
}

/*!**************************************************************************************************************
\brief Insert sorted element in to the container
\param cont Container to insert the element into.
\param item Item to insert in to the container
****************************************************************************************************************/
template<typename container, typename val>
size_t insertSorted(container& cont, const val& item)
{
	return insertSorted(cont, cont.begin(), cont.end(), item);
}

/*!**************************************************************************************************************
\brief Insert sorted element in to the container
\param item Item to insert in to the container
\param compare Comparison operator used for sorting
****************************************************************************************************************/
template<typename container, typename val, typename cmp>
size_t insertSorted(container& cont, const val& item, const cmp& compare)
{
	return insertSorted(cont, cont.begin(), cont.end(), item, compare);
}

/*!**************************************************************************************************************
\brief Insert sorted element, Overwrite if element exist in the container
\param cont Container to insert the element into.
\param begin Container range begin
\param end Container range end
\param item Item to insert in to the container
\param compare Comparison operator used for sorting
****************************************************************************************************************/
template<typename container, typename val, typename cmp>
size_t insertSorted_overwrite(container& cont, typename container::iterator begin, typename container::iterator end, const val& item, const cmp& compare)
{
	typename container::iterator it = std::lower_bound(begin, end, item, compare);
	int64 offset = int64(it - begin);
	if (it != end && !(compare(*it, item) || compare(item, *it)))
	{
		*it = item;
	}
	else
	{
		cont.insert(it, item);
	}
	return (size_t)offset;
}

/*!**************************************************************************************************************
\brief Insert sorted element, Overwrite if element exist in the container
\param cont Container to insert the element into.
\param begin Container range begin
\param end Container range end
\param item Item to insert in to the container
****************************************************************************************************************/
template<typename container, typename val>
size_t insertSorted_overwrite(container& cont, typename container::iterator begin, typename container::iterator end, const val& item)
{
	return insertSorted_overwrite(cont, begin, end, item, std::less<val>());
}

/*!**************************************************************************************************************
\brief Insert sorted element, Overwrite if element exist in the container
\param cont Container to insert the element into.
\param item Item to insert in to the container
****************************************************************************************************************/
template<typename container, typename val>
size_t insertSorted_overwrite(container& cont, const val& item)
{
	return insertSorted_overwrite(cont, cont.begin(), cont.end(), item);
}

/*!**************************************************************************************************************
\brief Insert sorted element, Overwrite if element exist in the container
\param cont Container to insert the element into.
\param item Item to insert in to the container
\param compare Comparison operator used for sorting
****************************************************************************************************************/
template<typename container, typename val, typename cmp>
size_t insertSorted_overwrite(container& cont, const val& item, const cmp& compare)
{
	return insertSorted_overwrite(cont, cont.begin(), cont.end(), item, compare);
}
}
}

//!\endcond
