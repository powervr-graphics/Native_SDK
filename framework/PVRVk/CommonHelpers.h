/*!
\brief Contains generic functions used in multiple places through PVRVk
\file PVRVk/TypesVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include <vector>

namespace pvrvk {

/// <summary>Sets an element in a vector at a specified index. If the index is outside the allocated size of the vector,
/// create new elements using their default constructor to fill the vector with up to the specified index.</summary>
/// <typeparam name="T">Type of element to be inserted to the vector.</typeparam>
/// <param name="index">Index to insert newElement into.</param>
/// <param name="newElement">The new value/element to be inserted to the vector.</param>
/// <param name="elements">The vector you want to insert the new element into.</param>
template<typename T>
static void setElementAtIndex(const uint32_t index, const T& newElement, std::vector<T>& elements)
{
	size_t numElements = elements.size();
	if (index > numElements)
	{
		elements.reserve(index + 1u); // so we don't do 2 dynamic allocations
		elements.resize(index); // don't need to initialize elements[index], assumes you want default initializer
	}
	if (index >= numElements)
	{ elements.emplace_back(newElement); }
	else
	{ elements[index] = newElement; }
}

} // namespace pvrvk