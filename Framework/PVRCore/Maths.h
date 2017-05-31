/*!
\brief Includes required GLM library components and defines the rest of the information necessary for PowerVR Math
needs.
\file PVRCore/Maths.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#define GLM_FORCE_RADIANS

#ifdef _WIN32
#define GLM_FORCE_SSE2
#endif

#include <cmath>
#include "../External/glm/glm.hpp"
#include "../External/glm/gtc/type_ptr.hpp"
#include "../External/glm/gtc/matrix_inverse.hpp"
#include "../External/glm/gtc/matrix_access.hpp"
#include "../External/glm/gtx/quaternion.hpp"
#include "../External/glm/gtx/transform.hpp"
#include "../External/glm/gtx/transform.hpp"
#include "../External/glm/gtx/simd_vec4.hpp"
#include "../External/glm/gtx/simd_mat4.hpp"
#include "../External/glm/gtx/fast_trigonometry.hpp"
#include "Base/Defines.h"
#include "Math/AxisAlignedBox.h"
#include "Math/BoundingSphere.h"
#include "Math/Plane.h"
#include "Math/Rectangle.h"



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

template<typename T>
inline T lcm(T lhs, T rhs)
{
	return (lhs / gcd(lhs, rhs)) * rhs;
}

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


template<typename T>
inline T roundAwayFromZero(T roundThis, T roundTo)
{
	if (roundTo == 0) { return roundThis; }
	T positive = T(roundThis >= 0);
	return ((roundThis + (positive * (roundTo - 1))) / roundTo) * roundTo;
}

/// <summary>Pack 4 values (red, green, blue, alpha) in the range of 0-255 into a single 32 bit unsigned integer
/// unsigned.</summary>
inline uint32 packRGBA(uint8 r, uint8 g, uint8 b, uint8 a)
{
	return ((uint32)(((a) << 24) | ((b) << 16) | ((g) << 8) | (r)));
}

/// <summary>Pack 4 values (red, green, blue, alpha) in the range of 0.0-1.0 into a single 32 bit unsigned integer
/// unsigned.</summary>
inline uint32 packRGBA(float32 r, float32 g, float32 b, float32 a)
{
	return packRGBA(uint8(r * 255), uint8(g * 255), uint8(b * 255), uint8(a * 255));
}

/// <summary>Return the smallest power of two that is greater than or equal to the provided value.</summary>
inline int32 makePowerOfTwoHigh(int32 iVal)
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
inline int32 makePowerOfTwoLow(int32 iVal)
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
inline pvr::int32 ndcToPixel(pvr::float32 ndc, pvr::int32 screenSize)
{
	return (int32)(ndc  * screenSize * .5f + screenSize * .5f);
}


/// <summary>Convert a number of pixels (left or top) to a normalized device coordinate (-1..1)</summary>
/// <param name="pixelCoord">The pixel coordinate (number of pixels) along the direction in question (same
/// direction as screenSize)</param>
/// <param name="screenSize">The size of the screen along the direction in question (same as pixelCoord)</param>
/// <returns>Normalized device coordinates (number in the 0..1 range)</returns>
inline pvr::float32 pixelToNdc(pvr::int32 pixelCoord, pvr::int32 screenSize)
{
	return (2.f / screenSize) * (pixelCoord - screenSize * .5f);
}

/// <summary>Performs quadratic interpolation between two points, beginning with a faster rate and slowing down.
/// </summary>
/// <param name="start">The starting point.</param>
/// <param name="end">The end point</param>
/// <param name="factor">Interpolation factor. At 0, returns start. At 1, returns end. Closer to 0, the rate of change is
/// faster, closer to 1 slower.</param>
inline float quadraticEaseOut(pvr::float32 start, pvr::float32 end, pvr::float32 factor)
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
inline float quadraticEaseIn(pvr::float32 start, pvr::float32 end, pvr::float32 factor)
{
	return ((end - start) * factor * factor) + start;
}


/*!\cond NO_DOXYGEN
//Bug fix for glm
template <typename T, glm::precision P>
GLM_FUNC_QUALIFIER glm::mat3 translate(glm::detail::tmat3x3<T, P> const& m, glm::detail::tvec2<T, P> const& v)
{
  glm::detail::tmat3x3<T, P> Result(m);
  Result[2] = m[0] * v[0] + m[1] * v[1] + m[2];
  return Result;
}

template <typename T, glm::precision P>
GLM_FUNC_QUALIFIER glm::detail::tmat3x3<T, P> translate(glm::detail::tvec2<T, P> const& v)
{
  return translate(glm::detail::tmat3x3<T, P>(1.0f), v);
}

template <typename T, glm::precision P>
GLM_FUNC_QUALIFIER glm::detail::tmat3x3<T, P> scale(glm::detail::tmat3x3<T, P> const& m, glm::detail::tvec2<T, P> const& v)
{
  glm::detail::tmat3x3<T, P> Result(glm::detail::tmat3x3<T, P>::_null);
  Result[0] = m[0] * v[0];
  Result[1] = m[1] * v[1];
  Result[2] = m[2];
  return Result;
}

template <typename T, glm::precision P>
GLM_FUNC_QUALIFIER glm::detail::tmat3x3<T, P> scale(glm::detail::tvec2<T, P> const& v)
{
  return scale(glm::detail::tmat3x3<T, P>(1.0f), v);
}
\endcond
*/

<<<<<<< HEAD
/*!********************************************************************************************
\brief Calculated a tilted projection matrix
\param fovy The field of vision in the y axis
\param aspect The aspect  of the viewport
\param near1 The near clipping plane distance (name is not near to avoid clash with platform keywords)
\param far1 The far clipping plane distance (name is not near to avoid clash with platform keywords)
\param rotate Angle of tilt (rotation around the z axis), in radians
\param api The graphics API for which this matrix will be created. It is used for things such
as the Framebuffer coordinate conventions.
\return	A projection matrix for the specified parameters, tilted by rotate
***********************************************************************************************/
=======
/// <summary>Calculated a tilted projection matrix</summary>
/// <param name="fovy">The field of vision in the y axis</param>
/// <param name="aspect">The aspect of the viewport</param>
/// <param name="near1">The near clipping plane distance (name is not near to avoid clash with platform keywords)
/// </param>
/// <param name="far1">The far clipping plane distance (name is not near to avoid clash with platform keywords)
/// </param>
/// <param name="rotate">Angle of tilt (rotation around the z axis), in radians</param>
/// <param name="api">The graphics API for which this matrix will be created. It is used for things such as the
/// Framebuffer coordinate conventions.</param>
/// <returns>A projection matrix for the specified parameters, tilted by rotate</returns>
>>>>>>> 1776432f... 4.3
inline glm::mat4 perspective(pvr::Api api, float32 fovy, float32 aspect, float32 near1,
                             float32 far1, pvr::float32 rotate = .0f)
{
	glm::mat4 mat = glm::perspective(fovy, aspect, near1, far1);
	if (api == Api::Vulkan)
	{
		mat[1][1] *= -1.f;// negate the y axis's y component, because vulkan coordinate system is +y down
	}
	return (rotate == 0.f ? mat : glm::rotate(rotate, glm::vec3(0.0f, 0.0f, 1.0f)) * mat);
}

<<<<<<< HEAD
/*!********************************************************************************************
\brief	Calculated a tilted projection matrix
\param	fovy The field of vision in the y axis
\param	width The width  of the viewport
\param	height The height of the viewport
\param	near1 The near clipping plane distance
\param	far1 The far clipping plane distance
\param	rotate Angle of tilt (rotation around the z axis), in radians
\param api The graphics API for which this matrix will be created. It is used for things such
as the Framebuffer coordinate conventions.
\return	A projection matrix for the specified parameters, tilted by rotate
***********************************************************************************************/
=======
/// <summary>Calculated a tilted projection matrix</summary>
/// <param name="fovy">The field of vision in the y axis</param>
/// <param name="width">The width of the viewport</param>
/// <param name="height">The height of the viewport</param>
/// <param name="near1">The near clipping plane distance</param>
/// <param name="far1">The far clipping plane distance</param>
/// <param name="rotate">Angle of tilt (rotation around the z axis), in radians</param>
/// <param name="api">The graphics API for which this matrix will be created. It is used for things such as the
/// Framebuffer coordinate conventions.</param>
/// <returns>A projection matrix for the specified parameters, tilted by rotate</returns>
>>>>>>> 1776432f... 4.3
inline glm::mat4 perspectiveFov(pvr::Api api, float32 fovy, float32 width, float32 height,
                                float32 near1, float32 far1, pvr::float32 rotate = .0f)
{
	return perspective(api, fovy, width / height, near1, far1, rotate);
}

inline glm::mat4 ortho(pvr::Api api, float32 left, float32 right, float32 bottom,
                       float32 top, float32 rotate = 0.0f)
{
	if (api == pvr::Api::Vulkan)
	{
		std::swap(bottom, top);// Vulkan origin y is top
	}
	glm::mat4 proj = glm::ortho<pvr::float32>(left, right, bottom, top);
	return (rotate == 0.0f ? proj : glm::rotate(rotate, glm::vec3(0.0f, 0.0f, 1.0f)) * proj);
}




}
}