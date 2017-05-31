/*!
\brief A pointer with an additional size field.
\file PVRCore/Base/SizedPointer.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
namespace pvr {
template <typename TYPE>

/// <summary>A pointer with an additional size field.</summary>
class SizedPointer
{
public:
	SizedPointer() : _pointer(NULL), _size(0) {}
	SizedPointer(TYPE* pointer, const size_t size) : _pointer(pointer), _size(size) {}

	/// <summary>return pointer to the object</summary>
	operator TYPE* (){	return getData();	}

	/// <summary>return const pointer to the object</summary>
	operator const TYPE* () const{	return getData();	}

	/// <summary>return the pointer to the object</summary>
	TYPE* getData(){ return _pointer; }

	/// <summary>return const pointer to the object</summary>
	const TYPE* getData() const{ return _pointer;	}
	
	/// <summary>return the size of the object</summary>
    size_t getSize() const{	return _size;	}

private:
	TYPE* _pointer;
	size_t _size;
};
}