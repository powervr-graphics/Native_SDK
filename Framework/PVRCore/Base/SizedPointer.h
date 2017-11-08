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
	/// <summary>Constructor. Initialize to empty.</summary>
	SizedPointer() : _pointer(NULL), _size(0) {}

	/// <summary>Constructor. Wrap a pointer with a known size.</summary>
	/// <param name="pointer">Pointer that will be wrapped</param>
	/// <param name="size">The size of the data that <paramRef name="pointer"/> points to</param>
	SizedPointer(TYPE* pointer, const size_t size) : _pointer(pointer), _size(size) {}

	/// <summary>Automatic conversion to the contained pointer</summary>
	/// <returns>The pointer</returns>
	operator TYPE* () {  return getData(); }

	/// <summary>Automatic conversion to the contained pointer</summary>
	/// <returns>The pointer</returns>ary>
	operator const TYPE* () const {  return getData(); }

	/// <summary>Return the pointer to the object</summary>
	/// <returns>The pointer</returns>
	TYPE* getData() { return _pointer; }

	/// <summary>Return const pointer to the object</summary>
	/// <returns>The pointer</returns>
	const TYPE* getData() const { return _pointer; }

	/// <summary>Return the size of the object</summary>
	/// <returns>The size of the object</returns>
	size_t getSize() const { return _size; }

private:
	TYPE* _pointer;
	size_t _size;
};
}