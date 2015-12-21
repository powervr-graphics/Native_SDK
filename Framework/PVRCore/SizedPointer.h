/*!*********************************************************************************************************************
\file         PVRCore\SizedPointer.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         A pointer with an additional size field.
***********************************************************************************************************************/
#pragma once
namespace pvr {
template <typename TYPE>

/*!**************************************************************************************************************
\brief         A pointer with an additional size field.
****************************************************************************************************************/
class SizedPointer
{
public:
	SizedPointer() : m_pointer(NULL), m_size(0) {}
	SizedPointer(TYPE* pointer, const size_t size) : m_pointer(pointer), m_size(size) {}

	operator TYPE* ()
	{
		return getData();
	}

	operator const TYPE* () const
	{
		return getData();
	}

	TYPE* getData()
	{
		return m_pointer;
	}

	const TYPE* getData() const
	{
		return m_pointer;
	}

	const size_t getSize() const
	{
		return m_size;
	}

private:
	TYPE* m_pointer;
	size_t m_size;
};
}