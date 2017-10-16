/*!
\brief Utility and helper functions
\file PVRApi/ApiUtils.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Maths.h"
#include "PVRCore/Interfaces/IGraphicsContext.h"
namespace pvr {
namespace utils {
/// <summary>Return a perspective projection matrix based on the window screen dimension & rotation and api. 
/// The context is being used to query the API-type, so generate the matrix for the specific API 
/// Framebuffer-coordinate convention.</summary>
/// <param name="context">The graphics context for it</param>
/// <param name="fov">The field of view</param>
/// <param name="nearPlane">The near clipping plane</param>
/// <param name="farPlane">The far clipping plane</param>
/// <returns>A perspective projection matrix.</returns>
inline glm::mat4 getPerspectiveMatrix(const IGraphicsContext& context, float32 fov, float32 nearPlane, float32 farPlane)
{
	const auto& displayAttrib = context.getDisplayAttributes();
	if (displayAttrib.isFullScreen() && displayAttrib.isScreenRotated())
	{
		return pvr::math::perspective(context.getApiType(), fov, (float32)displayAttrib.height / (float32)displayAttrib.width,
		                              nearPlane, farPlane, glm::pi<float32>() * .5f);
	}
	else
	{
		return pvr::math::perspective(context.getApiType(), fov, (float32)displayAttrib.width / (float32)displayAttrib.height, nearPlane, farPlane);
	}
}

/// <summary> Convert a Framebuffer-coordinate rectangle (scissors, viewport, etc) from 
/// Framework convention (0,0 is Bottom-Left) to Vulkan(0,0 is Top-Left).
/// NOTE: Framework viewport x and y are lower left where Vulkan is upper left.
/// The application should not use this function. The framework takes care of the conversion
/// </summary>
/// <tparam name="T">The datatype of the rectangle. Can normally be inferred</param>
/// <param name="rect">A Framework conversion rectangle ((0,0) is bottom-left)</param>
/// <param name="renderSurfaceDimensions">The size (in pixels) of the entire surface.</param>
/// <returns>A Vulkan-convention(0,0 is top-left) rectangle representing <paramref>rect</paramref>.</param>
template<typename T>
inline Rectangle<T> framebufferRectangleToVk(const Rectangle<T>& rect, const glm::ivec2& renderSurfaceDimensions)
{
	// convert y
	const T y = renderSurfaceDimensions.y - rect.y;
	// the extent cannot be negative so the y is recalculated by y - rect.height
	return Rectangle<T>(rect.x, y - rect.height, rect.width, rect.height);
}
}
}
