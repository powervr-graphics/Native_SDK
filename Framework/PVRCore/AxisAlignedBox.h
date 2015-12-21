/*!*********************************************************************************************************************
\file         PVRCore\AxisAlignedBox.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Class used to handle common AABB operations.
***********************************************************************************************************************/
#pragma once
#include "../External/glm/glm.hpp"
#include "PVRCore/Types.h"
#include <cfloat>

namespace pvr {
/*!********************************************************************************************************************
\brief Contains mathematical functionality and classes, such as bounding box calculations, intersections etc.
**********************************************************************************************************************/
namespace math {
/*!********************************************************************************************************************
\brief This class provides functionality to handle 3 dimensional Axis Aligned Boxes. Center-halfextent representation.
**********************************************************************************************************************/
class AxisAlignedBox
{
	glm::vec3	m_center;
	glm::vec3	m_halfExtent;
public:
	/*!********************************************************************************************************************
	\brief Constructor center/halfextent.
	**********************************************************************************************************************/
	AxisAlignedBox(const glm::vec3& center = glm::vec3(0.0f), const glm::vec3& halfExtent = glm::vec3(0.f))
	{
		set(center, halfExtent);
	}

	/*!********************************************************************************************************************
	\brief Sets center and extents to zero.
	**********************************************************************************************************************/
	void clear()
	{
		m_center.x = m_center.y = m_center.z = 0.0f;
		m_halfExtent = glm::vec3(0.f);
	}

	/*!********************************************************************************************************************
	\brief Sets from min and max. All components of min must be than all components in max.
	**********************************************************************************************************************/
	void setMinMax(const glm::vec3& min, const glm::vec3& max)
	{
		// compute the center.
		m_center = (max + min) * .5f;
		m_halfExtent = (max - min) * .5f;
	}

	/*!********************************************************************************************************************
	\brief Sets from center and half extent.
	**********************************************************************************************************************/
	void set(const glm::vec3& centerPoint, const glm::vec3& halfExtent)
	{
		m_center = centerPoint;
		m_halfExtent = halfExtent;
	}

    void remove(float32 x, float32 y, float32 z)
    {
        remove(glm::vec3(x,y,z));
    }

    void remove(const glm::vec3& point)
    {
        setMinMax(glm::max(point, getMin()), glm::min(point, getMax()));
    }

    void remove(const AxisAlignedBox& aabb)
    {
        remove(aabb.getMin());
        remove(aabb.getMax());
    }

	/*!****************************************************************************************************************
	\brief	Add a new point to the box. The new box will be the minimum box containing the old box and the new point.
	*******************************************************************************************************************/
	void add(const glm::vec3& point)
	{
		setMinMax(glm::min(point, getMin()), glm::max(point, getMax()));
	}

	/*!****************************************************************************************************************
	\brief	Merge two axis aligned boxes. The new box will be the minimum box containing both the old and the new box.
	*******************************************************************************************************************/
    void add(const AxisAlignedBox& aabb)
    {
        add(aabb.getMin());
        add(aabb.getMax());
    }
    
	/*!****************************************************************************************************************
	\brief	Add a new point to the box. The new box will be the minimum box containing the old box and the new point.
	*******************************************************************************************************************/
	void add(float32 x, float32 y, float32 z)
	{
		add(glm::vec3(x, y, z));
	}

	/*!****************************************************************************************************************
	\brief	Return a point consisting of the smallest (minimum) coordinate in each axis.
	*******************************************************************************************************************/
	glm::vec3 getMin() const { return m_center - m_halfExtent; }

	/*!****************************************************************************************************************
	\brief	Return a point consisting of the largest (maximum) coordinate in each axis.
	*******************************************************************************************************************/
	glm::vec3 getMax() const { return m_center + m_halfExtent; }

	/*!****************************************************************************************************************
	\brief	Return the min and the max.
	\param[out] outMin Output: The min point will be stored here.
	\param[out] outMax Output: The max point will be stored here.
	*******************************************************************************************************************/
	void getMinMax(glm::vec3& outMin, glm::vec3& outMax) const
	{
		outMin = m_center - m_halfExtent; outMax = m_center + m_halfExtent;
	}

	/*!****************************************************************************************************************
	\brief	Get the local bounding box transformed by a provided matrix.
	\param[in] m An affine transformation matrix. Skew will be ignored.
	\param[out] outAABB The transformed AABB will be stored here. Previous contents ignored.
	*******************************************************************************************************************/
	void transform(const glm::mat4& m, AxisAlignedBox& outAABB)
	{
		transform(glm::mat3(m), glm::vec3(m[3][0], m[3][1], m[3][2]), outAABB);
	}

	/*!****************************************************************************************************************
	\brief	Get the local bounding box transformed by a provided 3x3 matrix and translation vector.
	\param	m Rotation and scale matrix.
	\param	t Translation vector
	\param[out] outAABB The transformed AABB will be stored here. Previous contents ignored.
	*******************************************************************************************************************/
	void transform(const glm::mat3& m, const glm::vec3& t, AxisAlignedBox& outAABB)
	{
		for (int i = 0; i < 3; i++)
		{
			outAABB.m_center[i] = t[i];
			outAABB.m_halfExtent[i] = 0.0f;

			outAABB.m_center[i] += m[i][0] * m_center[0];
			outAABB.m_halfExtent[i] += fabs(m[i][0]) * m_halfExtent[0];

			outAABB.m_center[i] += m[i][1] * m_center[1];
			outAABB.m_halfExtent[i] += fabs(m[i][1]) * m_halfExtent[1];

			outAABB.m_center[i] += m[i][2] * m_center[2];
			outAABB.m_halfExtent[i] += fabs(m[i][2]) * m_halfExtent[2];
		}
	}

	/*!****************************************************************************************************************
	\brief	Get the size (width, height, depth) of the AABB.
	*******************************************************************************************************************/
	glm::vec3 getSize()const { return m_halfExtent + m_halfExtent;}

	/*!****************************************************************************************************************
	\brief	Get the half-size (half-width, half-height, half-depth) of the AABB.
	*******************************************************************************************************************/
	glm::vec3 getHalfExtent()const{ return m_halfExtent; }

	/*!****************************************************************************************************************
	\brief	Get the -x +y +z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 topLeftFar()const
	{
		return m_center + glm::vec3(-m_halfExtent.x, m_halfExtent.y, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the center-x +y +z  point of the box.
	*******************************************************************************************************************/
	glm::vec3 topCenterFar()const
	{
		return m_center + glm::vec3(0, m_halfExtent.y, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the +x +y +z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 topRightFar()const
	{
		return m_center + m_halfExtent;
	}

	/*!****************************************************************************************************************
	\brief	Get the -x +y -z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 topLeftNear()const
	{
		return m_center + glm::vec3(-m_halfExtent.x, m_halfExtent.y, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the center-x +y -z  point of the box.
	*******************************************************************************************************************/
	glm::vec3 topCenterNear()const
	{
		return m_center + glm::vec3(0., m_halfExtent.y, -m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the +x +y -z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 topRightNear()const
	{
		return m_center + glm::vec3(m_halfExtent.x, m_halfExtent.y, -m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the center-x center-y center-z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 center()const
	{
		return m_center;
	}

	/*!****************************************************************************************************************
	\brief	Get the -x center-y -z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 centerLeftNear()const
	{
		return m_center + glm::vec3(-m_halfExtent.x, 0, -m_halfExtent.z);
	}
	/*!****************************************************************************************************************
	\brief	Get the center-x center-y -z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 centerNear()const
	{
		return m_center + glm::vec3(0, 0, -m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the +x center-y -z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 centerRightNear()const
	{
		return m_center + glm::vec3(m_halfExtent.x, 0, -m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the -x center-y +z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 centerLeftFar()const
	{
		return m_center + glm::vec3(-m_halfExtent.x, 0, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the center-x center-y +z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 centerFar()const
	{
		return m_center + glm::vec3(0, 0, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the +x center-y +z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 centerRightFar()const
	{
		return m_center + glm::vec3(m_halfExtent.x, 0, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the -x -y -z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 bottomLeftNear()const
	{
		return m_center + glm::vec3(-m_halfExtent.x, -m_halfExtent.y, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the center-x -y -z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 bottomCenterNear()const
	{
		return m_center + glm::vec3(0, -m_halfExtent.y, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the +x -y -z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 bottomRightNear()const
	{
		return m_center + glm::vec3(m_halfExtent.x, -m_halfExtent.y, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the -x -y +z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 bottomLeftFar()const
	{
		return m_center + glm::vec3(-m_halfExtent.x, -m_halfExtent.y, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the center-x -y +z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 bottomCenterFar()const
	{
		return m_center + glm::vec3(0, -m_halfExtent.y, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Get the +x -y +z  corner of the box.
	*******************************************************************************************************************/
	glm::vec3 bottomRightFar()const
	{
		return m_center + glm::vec3(m_halfExtent.x, -m_halfExtent.y, m_halfExtent.z);
	}

	/*!****************************************************************************************************************
	\brief	Set this AABB as the minimum AABB that contains itself and the AABB provided.
	*******************************************************************************************************************/
	void mergeBox(const AxisAlignedBox& rhs)
	{
		setMinMax(glm::min(getMin(), rhs.getMin()), glm::max(getMax(), rhs.getMax()));
	}
};


/*!****************************************************************************************************************
\brief	An AABB with a min-max representation.
*******************************************************************************************************************/
class AxisAlignedBoxMinMax
{
	glm::vec3	m_min;
	glm::vec3	m_max;
public:
	void setMin(const glm::vec3& min) { m_min = min; }
	void setMax(const glm::vec3& max) { m_max = max; }
	const glm::vec3& getMin() { return m_min; }
	const glm::vec3& getMax() { return m_max; }

	void add(const glm::vec3& point)
	{
		m_min = glm::min(m_min, point);
		m_max = glm::max(m_max, point);
	}
};
}
}
