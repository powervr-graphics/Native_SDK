/*!*********************************************************************************************************************
\file         PVRCore\ReinterpretBits.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Functions for viewing bits as different types.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr{
	/*!****************************************************************************************************************
	\brief     Take a value's bit representation and reinterpret them as another type. Second template parameter should
	           normally be implicitly declared. First template parameter is mandatory.
	\tparam     Toutput_ Output value type. Must be explicitly defined.
	\tparam     Tinput_ Input value type. Should not need to be explicitly defined, can be inferred.
	\param     value  The value to reinterpret
	\return    The reinterpreted value
	*******************************************************************************************************************/
	template <typename Toutput_, typename Tinput_>
	Toutput_ reinterpretBits(const Tinput_& value)
	{
		assertion(sizeof(Tinput_) <= sizeof(Toutput_));
		Toutput_ ret = static_cast<Toutput_>(0);
		memcpy(&ret, &value, sizeof(Tinput_));
		return ret;
	}


	/*!****************************************************************************************************************
	\brief     Store the bits of a value in a static array of char.
	\tparam     T1_ Input value type. Should not need to be explicitly defined, can be inferred.
	\param     value  The value to reinterpret
	\return    A StaticArray<T1_> with a size exactly equal to the size of T1_ in characters, containing the bit
	           representation of value.
	*******************************************************************************************************************/
	template<typename T1_>
	std::array<T1_, sizeof(T1_)> readBits(const T1_& value)
	{
		std::array<char, sizeof(T1_)> retval;
		memcpy(&retval[0], &value, sizeof(T1_));
		return retval;
	}
}