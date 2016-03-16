//!\cond NO_DOXYGEN
#pragma once
#include <vector>
#include <functional>

namespace pvr {
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
size_t insertSorted(container& cont, typename container::iterator begin, typename container::iterator end, const val& item, const cmp& compare)
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