/*!
\brief Helper for math computations
\file PVRSuperResolution/MathUtil.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <stdint.h>
#include <vector>

namespace pvr {

/// <summary>Used for the 32-bit float to 16-bit float conversion.</summary>
/// <param name="x">value to return as unsigned int</param>
/// <returns>Value converted to unsigned int.</returns>
uint32_t as_uint(const float x);

/// <summary>Convert from 32-bit float to 16-bit float.</summary>
/// https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
/// <param name="x">value to convert</param>
/// <returns>Value converted, encoded as a 16-bit uint.</returns>
uint16_t float32ToFloat16(const float x);

/// <summary>Convert a vector formed of 32-bit floats to 16-bit floats.</summary>
/// <param name="vectorByteData">Vector to convert</param>
/// <param name="vectorByteResult">Vector converted</param>
void convertByteArrayFloat32ToFloat16(const std::vector<float>& vectorData, std::vector<uint8_t>& vectorByteResult);

} // namespace pvr
