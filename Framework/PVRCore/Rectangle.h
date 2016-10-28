/*!*********************************************************************************************************************
\file         PVRCore\Rectangle.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Contains A rectangle class.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Types.h"
#include "../External/glm/glm.hpp"
namespace pvr {
/*!*********************************************************************************************************************
\brief  A class representing an axis-aligned rectangle. Internal representation TopLeft and size.
\tparam TYPE The datatype of the units of the rectangle (int, float etc.)
***********************************************************************************************************************/
template <typename TYPE>
struct Rectangle
{
	TYPE x; //!<The x-coordinate of the left side of the rectangle
	TYPE y; //!<The y-coordinate of the bottom side of the rectangle
	TYPE width; //!<The width of the rectangle
	TYPE height; //!<The height of the rectangle
	/*!*********************************************************************************************************************
	\brief  Create a rectangle with uninitialized values.
	***********************************************************************************************************************/
	Rectangle() {}

	/*!*********************************************************************************************************************
	\brief  Create a rectangle with initial values.
	\param  TX The x-coordinate of the left of the rectangle
	\param  TY The y-coordinate of the bottom of the rectangle
	\param  TWidth The width of the rectangle
	\param  THeight The height of the rectangle
	***********************************************************************************************************************/
	Rectangle(TYPE TX, TYPE TY, TYPE TWidth, TYPE THeight) :
		x(TX), y(TY), width(TWidth), height(THeight)
	{
	}

	/*!*********************************************************************************************************************
	\brief  Create a rectangle with initial values.
	\param  bottomLeft The bottom-left corner of the rectangle (bottom, left)
	\param  dimensions The dimensions(width, height)
	***********************************************************************************************************************/
	Rectangle(glm::detail::tvec2<TYPE, glm::precision::defaultp> bottomLeft, glm::detail::tvec2<TYPE, glm::precision::defaultp> dimensions) :
		x(bottomLeft.x), y(bottomLeft.y), width(dimensions.x), height(dimensions.y)
	{
	}
	bool operator==(const Rectangle& rhs)const
	{
		return (x == rhs.x) && (y == rhs.y) && (width == rhs.width) && (height == rhs.height);
	}

	bool operator!=(const Rectangle& rhs)const{ return !(*this == rhs); }

	/*!*********************************************************************************************************************
	\brief  Get the center of a rectangle.
	***********************************************************************************************************************/
	glm::detail::tvec2<TYPE, glm::precision::defaultp> getCenter()const
	{
		glm::detail::tvec2<TYPE, glm::precision::defaultp> out;
		out.x = (x + width) * .5f;
		out.y = (y + height) * .5f;
		return out;
	}

	/*!*********************************************************************************************************************
	\brief  Get the Dimension of a rectangle.
	***********************************************************************************************************************/
	glm::detail::tvec2<TYPE, glm::precision::defaultp> getDimension()const
	{
		return glm::detail::tvec2<TYPE, glm::precision::defaultp>(width - x, height - y);
	}
};
typedef Rectangle<int32> Rectanglei;
typedef Rectangle<float32> Rectanglef;
}