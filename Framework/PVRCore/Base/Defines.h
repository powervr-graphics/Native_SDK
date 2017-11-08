/*!
\brief Contains several definitions used throughout the PowerVR Framework.
\file PVRCore/Base/Defines.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/Base/Types.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvr {
/// <summary>Contains assorted utility functions (test endianness, unicode conversions etc.)</summary>
namespace utils {
//!\cond NO_DOXYGEN
namespace details {
inline bool initLittleEndian()
{
	short int word = 0x0001;
	char* character = reinterpret_cast<char*>(&word);
	return character[0] ? true : false;
}
}
//!\endcond

/// <summary>Tests endianness of the current platform.</summary>
/// <returns>True if this platform is Little Endian, false if it is Big Endian.</returns>
inline bool isLittleEndian()
{
	static bool bLittleEndian = details::initLittleEndian();
	return bLittleEndian;
}

/// <summary>Typed memset. Sets each byte of the destination object to a source value.</summary>
/// <typeparam name="T">Type of target object.</typeparam>
/// <param name="dst">Reference to the object whose bytes we will set to the value</param>
/// <param name="i">The conversion of this object to unsigned char will be the value that is set to each byte.</param>
template<typename T>
inline void memSet(T& dst, int32_t i)
{
	memset(&dst, i, sizeof(T));
}

/// <summary>Typed memcopy. Copies the bits of an object to another object. Although T1 can be different to T2,
/// sizeof(T1) must be equal to sizeof(T2)</summary>
/// <typeparam name="T1">Type of target object.</typeparam>
/// <typeparam name="T2">Type of the source object.</typeparam>
/// <param name="dst">Reference to the destination object</param>
/// <param name="src">Reference to the source object</param>
template<typename T1, typename T2>
inline void memCopy(T1& dst, const T2& src)
{
	assert(sizeof(T1) == sizeof(T2));
	memcpy(&dst, &src, sizeof(T1));
}

/// <summary>Copy from volatile memory (facilitate from volatile
/// variables to nonvolatile)</summary>
/// <param name="dst">Copy destination</param>
/// <param name="src">Copy source</param>
template<typename T1, typename T2>
inline void memCopyFromVolatile(T1& dst, const volatile T2& src)
{
	assert(sizeof(T1) == sizeof(T2));
	memcpy(&dst, &const_cast<const T2&>(src), sizeof(T1));
}

/// <summary>Copy to volatile memory (facilitate from normal
/// variables to volatile)</summary>
/// <param name="dst">Copy destination</param>
/// <param name="src">Copy source</param>
template<typename T1, typename T2>
inline void memCopyToVolatile(volatile T1& dst, const T2& src)
{
	assert(sizeof(T1) == sizeof(T2));
	memcpy(&const_cast<T1&>(dst), &src, sizeof(T1));
}
}
}