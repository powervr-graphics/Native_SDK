/*!
\brief Includes required GLM library components and defines the rest of the information necessary for PowerVR Math
needs.
\file PVRCore/Maths.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include <cmath>
#include "PVRCore/Base/Types.h"

namespace pvr {
//!\cond NO_DOXYGEN
#if (0)
namespace internal {
typedef glm::simdMat4 optimizedMat4;
typedef glm::simdVec4 optimizedVec4;
inline glm::mat4x4 toMat4(const optimizedMat4& mat)
{
	return glm::mat4_cast(mat);
}
}
#else

namespace internal {
typedef glm::mat4 optimizedMat4;
typedef glm::vec4 optimizedVec4;
inline glm::mat4x4 toMat4(const optimizedMat4& mat)
{
	return mat;
}

}
#endif
//!\endcond

namespace math {

/// <summary>Calculate the Greatest Common Divisor of two numbers (the larger number that,
/// if used to divide either value, has a remainder of zero. Order is irrelevant</summary>
/// <typeparam name="T">The type of the values. Must have equality, assignment and modulo
/// defined</typeparam>
/// <param name="lhs">One of the input values</param>
/// <param name="rhs">The other input values</param>
/// <returns>The GCD. If the numbers are "coprime" (have no common divisor exept 1),
/// the GCD is 1. </returns>
template<typename T>
inline T gcd(T lhs, T rhs)
{
	T tmprhs;
	while (true)
	{
		if (rhs == 0) { return lhs; }
		tmprhs = rhs;
		rhs = lhs % rhs;
		lhs = tmprhs;
	}
}

/// <summary>Calculate the Least Common Multiple of two numbers (the smaller integer that
/// is a factor of both numbers). Order is irrelevant. If either of the numbers is 0, will
/// return 0</summary>
/// <typeparam name="T">The type of the values. Must have equality, assignment multiplication
/// and either modulo or a gcd function defined</typeparam>
/// <param name="lhs">One of the input values</param>
/// <param name="rhs">The other input values</param>
/// <returns>The LCM. If the inputs don't have any common factors (except 1), the LCM is
/// equal to lhs * rhs. If either input is 0, returns 0.</returns>
template<typename T>
inline T lcm(T lhs, T rhs)
{
	return (lhs / gcd(lhs, rhs)) * rhs;
}

/// <summary>Calculate the Least Common Multiple of two numbers (the smaller integer that
/// is a multiple of both numbers), but discards 0: If either number is 0, will return the
/// other number</summary>
/// <typeparam name="T">The type of the values. Must have equality, assignment multiplication
/// and either modulo or a gcd function defined</typeparam>
/// <param name="lhs">One of the input values</param>
/// <param name="rhs">The other input values</param>
/// <returns>The LCM. If the numbers don't have any common factors (except 1), the LCM is
/// equal to lhs * rhs. If either input is 0, returns the other</returns>
template<typename T>
inline T lcm_with_max(T lhs, T rhs)
{
	T strict = (lhs / gcd(lhs, rhs)) * rhs;
	if (strict == 0)
	{
		strict = std::max(lhs, rhs);
	}
	return strict;
}

/// <summary>Pack 4 values (red, green, blue, alpha) in the range of 0-255 into a single 32 bit unsigned Integer
/// unsigned.</summary>
/// <param name="r">Red channel (8 bit)</param>
/// <param name="g">Blue channel (8 bit)</param>
/// <param name="b">Red channel (8 bit)</param>
/// <param name="a">Red channel (8 bit)</param>
/// <returns>32 bit RGBA value</returns>
inline uint32_t packRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return (static_cast<uint32_t>(((a) << 24) | ((b) << 16) | ((g) << 8) | (r)));
}

/// <summary>Pack 4 values (red, green, blue, alpha) in the range of 0.0-1.0 into a single 32 bit unsigned Integer
/// unsigned.</summary>
/// <param name="r">Red channel (normalized 0.0-1.0)</param>
/// <param name="g">Blue channel (normalized 0.0-1.0)</param>
/// <param name="b">Red channel (normalized 0.0-1.0)</param>
/// <param name="a">Red channel (normalized 0.0-1.0)</param>
/// <returns>32 bit RGBA value</returns>
inline uint32_t packRGBA(float r, float g, float b, float a)
{
	return packRGBA(static_cast<uint8_t>(r * 255), static_cast<uint8_t>(g * 255), static_cast<uint8_t>(b * 255), static_cast<uint8_t>(a * 255));
}

/// <summary>Return the smallest power of two that is greater than or equal to the provided value.</summary>
/// <param name="iVal">An integer value.</param>
/// <returns>The smallest PoT that is greater or equal to iVal</returns>
inline int32_t makePowerOfTwoHigh(int32_t iVal)
{
	int iTmp = 1;
	do
	{
		iTmp <<= 1;
	}
	while (iTmp < iVal);
	return iTmp;
}

/// <summary>Return the smallest power of two that is less than or equal to the provided value.</summary>
/// <param name="iVal">An integer value.</param>
/// <returns>The smallest PoT that is less or equal to iVal</returns>
inline int32_t makePowerOfTwoLow(int32_t iVal)
{
	int iTmp = 1;
	do
	{
		iTmp <<= 1;
	}
	while (iTmp < iVal);
	return iTmp;
	iTmp >>= 1;
}

/// <summary>Convert a normalized device coordinate (-1..1) to a number of pixels from the start (left or top)
/// </summary>
/// <param name="ndc">The normalised coordinate along the direction in question (same direction as screenSize)
/// </param>
/// <param name="screenSize">The size of the screen along the direction in question (same as ndc)</param>
/// <returns>Pixel coordinates from normalized device coordinates</returns>
inline int32_t ndcToPixel(float ndc, int32_t screenSize)
{
	return static_cast<int32_t>(ndc  * screenSize * .5f + screenSize * .5f);
}


/// <summary>Convert a number of pixels (left or top) to a normalized device coordinate (-1..1)</summary>
/// <param name="pixelCoord">The pixel coordinate (number of pixels) along the direction in question (same
/// direction as screenSize)</param>
/// <param name="screenSize">The size of the screen along the direction in question (same as pixelCoord)</param>
/// <returns>Normalized device coordinates (number in the 0..1 range)</returns>
inline float pixelToNdc(int32_t pixelCoord, int32_t screenSize)
{
	return (2.f / screenSize) * (pixelCoord - screenSize * .5f);
}

/// <summary>Performs quadratic interpolation between two points, beginning with a faster rate and slowing down.
/// </summary>
/// <param name="start">The starting point.</param>
/// <param name="end">The end point</param>
/// <param name="factor">Current LINEAR interpolation factor, from 0..1</param>0
/// <returns> For <paramRef name="factor"/>=0, returns <paramRef name="start"/>. For <paramRef name="factor"/>=1, returns <paramRef name="end"/>.
/// Closer to 0, the rate of change is faster, closer to 1 slower.</param>
inline float quadraticEaseOut(float start, float end, float factor)
{
	float fTInv = 1.0f - factor;
	return ((start - end) * fTInv * fTInv) + end;
}

/// <summary>Performs quadratic interpolation between two points, beginning with a slow rate and speeding up.
/// </summary>
/// <param name="start">The starting point.</param>
/// <param name="end">The end point</param>
/// <param name="factor">Interpolation factor. At 0, returns start. At 1, returns end. Closer to 0, the rate of change is
/// slower, closer to 1 faster.</param>
/// <returns>The modified value to use, quadratically interpolated between start and end with factor factor.</returns>
inline float quadraticEaseIn(float start, float end, float factor)
{
	return ((end - start) * factor * factor) + start;
}

/// <summary>Performs line -to - plane intersection </summary>
/// <typeparam name="genType">A glm:: vector type. Otherwise, a type with the following
/// operations defined: A typename member value_type (type of scalar), +/- (vector add/mul), / (divide
/// by scalar), and a dot() function in either the global or glm:: namespace</typeparam>
/// <param name="origin">The start point of the line</param>
/// <param name="dir">The (positive) direction of the line</param>
/// <param name="planeOrigin">Any point on the plane</param>
/// <param name="planeNormal">The normal of the plane</param>
/// <param name="intersectionDistance">Output parameter: If an intersection happens, this parameter
/// will contain the signed distance from <paramRef name="origin"> towards <paramRef name="dir"> of
/// the intersection point.</param>
/// <param name="epsilon">For any comparison calculations, any value smaller than that will be considered
/// zero (otherwise, if two numbers difference is smaller than this, they are considered equal) </param>
/// <returns>True if the line and plane intersect, otherwise false</returns>
template <typename genType> bool intersectLinePlane
(
  genType const& origin, genType const& dir,
  genType const& planeOrigin, genType const& planeNormal,
  typename genType::value_type& intersectionDistance,
  typename genType::value_type epsilon = std::numeric_limits<typename genType::value_type>::epsilon()
)
{
	using namespace glm;
	typename genType::value_type d = dot(dir, planeNormal);

	if (glm::abs(d) > epsilon)
	{
		intersectionDistance = dot(planeOrigin - origin, planeNormal) / d;
		return true;
	}
	return false;
}

/// <summary>Get a vector that is perpendicular to another vector</summary>
/// <typeparam name="Vec2">A vector with two components that can be accessed through .x and .y</typeparam>
/// <param name="aVector">A vector</param>
/// <returns>A vector that is perpendicular to <paramRef name="aVector"/></returns>
template <typename Vec2> Vec2 getPerpendicular
(Vec2 const& aVector)
{
	return Vec2(aVector.y, -aVector.x);
}

/// <summary>Calculated a tilted perspective projection matrix</summary>
/// <param name="api">The graphics API for which this matrix will be created. It is used for the
/// Framebuffer coordinate convention.</param>
/// <param name="fovy">The field of vision in the y axis</param>
/// <param name="aspect">The aspect of the viewport</param>
/// <param name="near1">The near clipping plane distance (trailing 1 to avoid win32 keyword)</param>
/// <param name="far1">The far clipping plane distance (trailing 1 to avoid win32 keyword)</param>
/// <param name="rotate">Angle of tilt (rotation around the z axis), in radians</param>
/// <returns>A projection matrix for the specified parameters, tilted by rotate</returns>
inline glm::mat4 perspective(Api api, float fovy, float aspect, float near1,
                             float far1, float rotate = .0f)
{
	glm::mat4 mat = glm::perspective(fovy, aspect, near1, far1);
	if (api == Api::Vulkan)
	{
		mat[1][1] *= -1.f;// negate the y axis's y component, because vulkan coordinate system is +y down
	}
	return (rotate == 0.f ? mat : glm::rotate(rotate, glm::vec3(0.0f, 0.0f, 1.0f)) * mat);
}

/// <summary>Calculated a tilted perspective projection matrix</summary>
/// <param name="fovy">The field of vision in the y axis</param>
/// <param name="width">The width of the viewport</param>
/// <param name="height">The height of the viewport</param>
/// <param name="near1">The near clipping plane distance</param>
/// <param name="far1">The far clipping plane distance</param>
/// <param name="rotate">Angle of tilt (rotation around the z axis), in radians</param>
/// <param name="api">The graphics API for which this matrix will be created. It is used for things such as the
/// Framebuffer coordinate conventions.</param>
/// <returns>A projection matrix for the specified parameters, tilted by rotate</returns>
inline glm::mat4 perspectiveFov(Api api, float fovy, float width, float height,
                                float near1, float far1, float rotate = .0f)
{
	return perspective(api, fovy, width / height, near1, far1, rotate);
}

/// <summary>Calculated an orthographic projection tilted projection matrix</summary>
/// <param name="left">The x coordinate of the left clipping plane</param>
/// <param name="right">The x coordinate of the right clipping plane</param>
/// <param name="bottom">The y coordinate of the bottom clipping plane</param>
/// <param name="top">The y coordinate of the bottom clipping plane</param>
/// <param name="rotate">Angle of tilt (rotation around the z axis), in radians</param>
/// <param name="api">The graphics API for which this matrix will be created. It is used for things such as the
/// Framebuffer coordinate conventions.</param>
/// <returns>An orthographic projection matrix for the specified parameters, tilted by rotate</returns>
inline glm::mat4 ortho(Api api, float left, float right, float bottom,
                       float top, float rotate = 0.0f)
{
	if (api == pvr::Api::Vulkan)
	{
		std::swap(bottom, top);// Vulkan origin y is top
	}
	glm::mat4 proj = glm::ortho<float>(left, right, bottom, top);
	return (rotate == 0.0f ? proj : glm::rotate(rotate, glm::vec3(0.0f, 0.0f, 1.0f)) * proj);
}

}
}