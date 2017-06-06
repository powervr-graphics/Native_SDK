/*!
\brief Contains several definitions used throughout the PowerVR Framework.
\file PVRCore/Base/Defines.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#if defined(_WIN32)
#define PVR_API_FUNC __cdecl
#else
#define PVR_API_FUNC
#endif

#if defined(_MSC_VER)
#if (_MSC_VER>=1600)
#define PVR_SUPPORT_MOVE_SEMANTICS 1
#endif
#elif defined (__GNUC__)
#if defined(__GXX_EXPERIMENTAL_CXX0X) || __cplusplus >= 201103L
#define PVR_SUPPORT_MOVE_SEMANTICS 1
#endif
#elif defined __clang
#if __has_feature(cxx_rvalue_references)
#define PVR_SUPPORT_MOVE_SEMANTICS 1
#endif
#endif

#ifdef DEBUG
#define PVR_DEBUG_THROW_ON_API_ERROR
#define PVR_FRAMEWORK_OBJECT_NAMES
#endif

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(__ANDROID__)
#define PVR_PLATFORM_IS_MOBILE 1
#else
#define PVR_PLATFORM_IS_DESKTOP 1
#endif

#include "PVRCore/Base/Types.h"

#if !defined(PVR_ASSERT_SUCCESS)
#define PVR_ASSERT_SUCCESS(result) do { if ((result) != pvr::Result::Success) { assertion(false, "Assertion failed: Result was NOT pvr::Result::Success"); }} while (false)
#endif

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvr {
/// <summary>Contains assorted utility functions (test endianness, unicode conversions etc.)</summary>
namespace utils {
inline bool initLittleEndian()
{
	short int word = 0x0001;
	char* byte = (char*)&word;
	return byte[0] ? true : false;
}


/// <summary>Tests endianness of the current platform.</summary>
/// <returns>True if this platform is Little Endian, false if it is Big Endian.</returns>
inline bool isLittleEndian()
{
	static bool bLittleEndian = initLittleEndian();
	return bLittleEndian;
}

template<typename T>
inline void memSet(T& dst, int32 i)
{
	memset(&dst, i, sizeof(T));
}

template<typename T1, typename T2>
inline void memCopy(T1& dst, const T2& src)
{
	PVR_ASSERTION(sizeof(T1) == sizeof(T2));
	memcpy(&dst, &src, sizeof(T1));
}

/// <summary>Copy from volatile memory</summary>
/// <param name="dst">Copy destination</param>
/// <param name="src">Copy source</param>
template<typename T1, typename T2>
inline void memCopyFromVolatile(T1& dst, const volatile T2& src)
{
	PVR_ASSERTION(sizeof(T1) == sizeof(T2));
	memcpy(&dst, &(const T2&)src, sizeof(T1));
}

template<typename T1, typename T2>
inline void memCopyToVolatile(volatile T1& dst, const T2& src)
{
	PVR_ASSERTION(sizeof(T1) == sizeof(T2));
	memcpy(&(T1&)dst, &src, sizeof(T1));
}

}
}