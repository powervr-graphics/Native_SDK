/*!
\brief Helper for math computations
\file PVRSuperResolution/MathUtil.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include <cstddef>
#include "MathUtil.h"

namespace pvr {

uint32_t as_uint(const float x) { return *(uint32_t*)&x; }

uint16_t float32ToFloat16(const float x)
{
	// IEEE-754 16-bit format:
	// 1 bit sign
	// 5 bits exponent
	// 10 bits mantissa
	const uint32_t b = as_uint(x) + 0x00001000;
	const uint32_t e = (b & 0x7F800000) >> 23;
	const uint32_t m = b & 0x007FFFFF;
	return static_cast<uint16_t>(
		(b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF);
}

void convertByteArrayFloat32ToFloat16(const std::vector<float>& vectorData, std::vector<uint8_t>& vectorByteResult)
{
	for (size_t i = 0; i < vectorData.size(); ++i)
	{
		uint16_t result = float32ToFloat16(vectorData[i]);
		uint8_t partA = static_cast<uint8_t>(result & 0x00FF);
		uint8_t partB = static_cast<uint8_t>((result & 0xFF00) >> 8);
		vectorByteResult.push_back(partA);
		vectorByteResult.push_back(partB);
	}
}

} // namespace pvr
