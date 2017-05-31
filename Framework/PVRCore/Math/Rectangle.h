/*!
\brief Contains A rectangle class.
\file PVRCore/Math/Rectangle.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Types.h"
#include "PVRCore/Base/ComplexTypes.h"
#include "../External/glm/glm.hpp"
namespace pvr {
/// <summary>A class representing an axis-aligned rectangle. Internal representation TopLeft and size.
/// </summary>
/// <typeparam name="TYPE">The datatype of the units of the rectangle (int, float etc.)</typeparam>
template <typename TYPE>
struct Rectangle
{
	TYPE x; //!<The x-coordinate of the left side of the rectangle
	TYPE y; //!<The y-coordinate of the bottom side of the rectangle
	TYPE width; //!<The width of the rectangle
	TYPE height; //!<The height of the rectangle

	glm::detail::tvec2<TYPE, glm::highp> offset()const { return glm::detail::tvec2<TYPE, glm::highp>(x, y); }
	glm::detail::tvec2<TYPE, glm::highp> extent()const { return glm::detail::tvec2<TYPE, glm::highp>(width, height); }
	glm::detail::tvec2<TYPE, glm::highp> center()const { return offset() + extent() / TYPE(2); }
	/// <summary>Create a rectangle with uninitialized values.</summary>
	Rectangle() {}

	Rectangle(const types::GenericOffset2D<TYPE>& offset0, const types::GenericOffset2D<TYPE>& offset1)
	{
		x = offset0.offsetX; y = offset0.offsetY;
		width = offset1.offsetX - x;
		height = offset1.offsetY - y;
	}

	/// <summary>Create a rectangle with initial values.</summary>
	/// <param name="TX">The x-coordinate of the left of the rectangle</param>
	/// <param name="TY">The y-coordinate of the bottom of the rectangle</param>
	/// <param name="TWidth">The width of the rectangle</param>
	/// <param name="THeight">The height of the rectangle</param>
	Rectangle(TYPE TX, TYPE TY, TYPE TWidth, TYPE THeight) :
		x(TX), y(TY), width(TWidth), height(THeight)
	{
	}

	/// <summary>Create a rectangle with initial values.</summary>
	/// <param name="bottomLeft">The bottom-left corner of the rectangle (bottom, left)</param>
	/// <param name="dimensions">The dimensions(width, height)</param>
	Rectangle(glm::detail::tvec2<TYPE, glm::precision::defaultp> bottomLeft,
	    glm::detail::tvec2<TYPE, glm::precision::defaultp> dimensions) :
		x(bottomLeft.x), y(bottomLeft.y), width(dimensions.x), height(dimensions.y)
	{
	}

	bool operator==(const Rectangle& rhs)const
	{
		return (x == rhs.x) && (y == rhs.y) && (width == rhs.width) && (height == rhs.height);
	}

	bool operator!=(const Rectangle& rhs)const { return !(*this == rhs); }

	/// <summary>Expand this rectangle which will also contains the given rectangle.</summary>
	void expand(const Rectangle& rect)
	{
		x = glm::min(x, rect.x);
		y = glm::min(y, rect.y);
		width = glm::max(width, rect.width);
		height = glm::max(height, rect.height);
	}

};

template<typename TYPE>
Rectangle<TYPE> operator *(const glm::mat4& xform, const Rectangle<TYPE>& rect)
{
	// transform the rectangle
	glm::detail::tvec4<TYPE, glm::precision::defaultp> xform0(xform *
	    glm::detail::tvec4<TYPE, glm::precision::defaultp>(rect.x, rect.y, 0.0f, 1.0f));

	glm::detail::tvec4<TYPE, glm::precision::defaultp> xform1 =
	  glm::detail::tvec2<TYPE, glm::precision::defaultp>(xform * glm::vec4(rect.x + rect.width, rect.height, 0.0f, 1.0f));

	return Rectangle<TYPE>(glm::detail::tvec2<TYPE, glm::precision::defaultp>(xform0, xform1 - xform0));
}

typedef Rectangle<int32> Rectanglei;
typedef Rectangle<float32> Rectanglef;
}
