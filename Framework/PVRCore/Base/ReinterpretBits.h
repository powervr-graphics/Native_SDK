/*!
\brief Functions for viewing bits as different types.
\file PVRCore/Base/ReinterpretBits.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr{
	/// <summary>Take a value's bit representation and reinterpret them as another type. Second template parameter
	/// should normally be implicitly declared. First template parameter is mandatory.</summary>
	/// <param name="value">The value to reinterpret</param>
	/// <typeparam name="Toutput_">Output value type. Must be explicitly defined.</typeparam>
	/// <typeparam name="Tinput_">Input value type. Should not need to be explicitly defined, can be inferred.
	/// </typeparam>
	/// <returns>The reinterpreted value</returns>
	template <typename Toutput_, typename Tinput_>
	Toutput_ reinterpretBits(const Tinput_& value)
	{
		assertion(sizeof(Tinput_) <= sizeof(Toutput_));
		Toutput_ ret = static_cast<Toutput_>(0);
		memcpy(&ret, &value, sizeof(Tinput_));
		return ret;
	}


	/// <summary>Store the bits of a value in a static array of char.</summary>
	/// <param name="value">The value to reinterpret</param>
	/// <typeparam name="T1_">Input value type. Should not need to be explicitly defined, can be inferred.</typeparam>
	/// <returns>A StaticArray<T1_> with a size exactly equal to the size of T1_ in characters, containing the bit
	/// representation of value.</returns>
	template<typename T1_>
	std::array<T1_, sizeof(T1_)> readBits(const T1_& value)
	{
		std::array<char, sizeof(T1_)> retval;
		memcpy(&retval[0], &value, sizeof(T1_));
		return retval;
	}
}