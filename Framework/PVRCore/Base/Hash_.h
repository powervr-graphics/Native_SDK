/*!
\brief Hash functions inmplementations.
\file PVRCore/Hash_.h
\author PowerVR by Imagination, Developer Technology Team.
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Defines.h"
#include "PVRCore/Base/ReinterpretBits.h"
#include <string>
#include <cwchar>
namespace pvr {
/// <summary>Function object hashing to 32 bit values into a 32 bit unsigned integer.</summary>
/// <param name="t">The value to hash.</param>
/// <typeparam name="T1_">The type of the value to hash.</typeparam>
/// <returns>The hash of the value.</returns>
template<typename T1_>
inline uint32 hash32_32(const T1_& t)
{
	uint32 a = reinterpretBits<uint32>(t);
	a = (a + 0x7ed55d16) + (a << 12);
	a = (a ^ 0xc761c23c) ^ (a >> 19);
	a = (a + 0x165667b1) + (a << 5);
	a = (a + 0xd3a2646c) ^ (a << 9);
	a = (a + 0xfd7046c5) + (a << 3);
	a = (a ^ 0xb55a4f09) ^ (a >> 16);
	return a;
}

/// <summary>Function object hashing a number of bytes into a 32 bit unsigned integer.</summary>
/// <param name="bytes">Pointer to a block of memory.</param>
/// <param name="count">Number of bytes to hash.</param>
/// <returns>The hash of the value.</returns>
inline uint32 hash32_bytes(const void* bytes, size_t count)
{
	/////////////// WARNING // WARNING // WARNING // WARNING // WARNING // WARNING // /////////////
	// IF THIS ALGORITHM IS CHANGED, THE ALGORITHM IN THE BOTTOM OF THE PAGE MUST BE CHANGED
	// AS IT IS AN INDEPENDENT COMPILE TIME IMPLEMENTATION OF TTHIS ALGORITHM.
	/////////////// WARNING // WARNING // WARNING // WARNING // WARNING // WARNING // /////////////

	uint32 hashValue = 2166136261U;
	const unsigned char* current = static_cast<const unsigned char*>(bytes);
	const unsigned char* end = current + count;
	while (current < end)
	{
		hashValue = (hashValue * 16777619U) ^ *current;
		++current;
	}
	return hashValue;
}

/// <summary>Class template denoting a hash. Specializations only - no default implementation.
/// (int32/int64/uint32/uint64/string)</summary>
/// <typeparam name="The">type of the value to hash.</typeparam>
template<typename T>
struct hash
{
};

//!\cond NO_DOXYGEN
template<> struct hash<uint32> { uint32 operator()(uint32 value) { return hash32_32(value); } };

template<> struct hash<int32> { uint32 operator()(uint32 value) { return hash32_32(value); } };

template<> struct hash<uint64>
{
	uint32 operator()(uint64 value)
	{
		uint32 a = static_cast<uint32>((value >> 32) | (value & 0x00000000FFFFFFFFull));
		return hash32_32(a);
	}
};

template<> struct hash<int64>
{
	uint32 operator()(uint64 value)
	{
		uint32 a = static_cast<uint32>((value >> 32) | (value & 0x00000000FFFFFFFFull));
		return hash32_32(a);
	}
};

template<typename T>
struct hash<std::basic_string<T> >
{
	uint32 operator()(const std::string& t) const
	{
		return hash32_bytes(t.data(), sizeof(T) * t.size());
	}
};

#pragma warning(push)
#pragma warning(disable:4307)
// COMPILE TIME HASHING //
// WARNING - MUST GIVE THE SAME RESULTS AS THE HASH32BYTES ALGORITHM OTHERWISE
// MANY CLASSES AROUND THE FRAMEWORK WILL BREAK. IT IS USED TO OPTIMISE COMPILE-TIME
// SWITCH STATEMENTS.
template<uint32_t hashvalue, unsigned char... dummy> class hasher_helper;

template<uint32_t hashvalue, unsigned char first> class hasher_helper<hashvalue, first>
{
public: static const uint32_t value = hashvalue * 16777619U ^ first;
};
template<uint32_t hashvalue, unsigned char first, unsigned char... dummy> class hasher_helper<hashvalue, first, dummy...>
{
public: static const uint32_t value = hasher_helper<hashvalue * 16777619U ^ first, dummy...>::value;
};

template<unsigned char... chars> class HashCompileTime
{
public: static const uint32_t value = hasher_helper<2166136261U, chars...>::value;
};
#pragma warning(pop)

//!\endcond
}