#pragma once
#include <vector>

namespace pvr {
/*!*
	\brief A small statically allocated array
	This class represents a small array of items. The array is statically allocated, and has at most 255 items, usually 8.
	It is not meant to (and cannot) be used to store large numbers of items (use a std::vector instead), rather it is
	meant to hold small tuples of items. The PowerVR framework utilizes this class to store tuples of one-per-swap-image
	items*/
template<typename T_, uint8 MAX_ITEMS = 4> class Multi
{
public: typedef T_ ElementType;
typedef ElementType ContainerType[MAX_ITEMS];
private:
	ContainerType container;
	uint32 numItems;
public:
	ElementType& operator[](uint32 idx) { return container[idx]; }
	const ElementType& operator[](uint32 idx) const { return container[idx]; }
	ContainerType& getContainer1() { return container; }
	ElementType& back() { return container[numItems - 1]; }
	const ElementType& back() const { return container[numItems - 1]; }

	size_t size() { return numItems; }
	Multi(): numItems(0) {}
	Multi(const ElementType* elements, const uint32 count): numItems(count)
	{
		assertion(count < MAX_ITEMS, "Multi<T>: Index out of range");
		container.assign(elements, elements + count);
	}
	void add(const ElementType& element)
	{
		assertion(numItems < MAX_ITEMS, "Multi<T>: Index out of range");
		container[numItems++] = element;
	}
	void add(const ElementType* element, const uint32 count)
	{
		assertion(numItems + count < MAX_ITEMS, "Multi<T>: Index out of range");
		for (uint32 i = 0; i < count; ++i)
		{
			container[numItems++] = element[i];
		}
	}
};
}