/*!*********************************************************************************************************************
\file         PVRCore\Plane.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  A Plane3d class containing functionality for representing and working with 3D Planes.
***********************************************************************************************************************/
#pragma once
#include "../External/glm/glm.hpp"
#include "PVRCore/Types.h"
namespace pvr {
/*!****************************************************************************************************************
\brief Uses the plane equation Ax + By + Cz + D = 0, where A B C are plane normal, xyz are position on the plane 
and D is distance to the plane.
*******************************************************************************************************************/
class Plane3d
{
public:
	/*!****************************************************************************************************************
	\brief  Constructs a plane from normal and distance. Distance is the scalar number that is the (unsigned) distance 
	        of this plane from (0,0,0).
	\param	normal The normal of this plane. MUST BE NORMALISED. If it not normalized, unexpected results may occur.
	\param	dist The signed distance, along the plane's normal direction, between the coordinate start and (0,0,0). This
	        number is defined as the number that the normal must be multiplied with so that the normal's coordinates
			define a point on the plane.
	*******************************************************************************************************************/
	Plane3d(const glm::vec3& normal, float32 dist) : m_norm(normal), m_dist(dist) {}

	/*!****************************************************************************************************************
	\brief  Constructs a plane from normal and a point on this plane.
	\param	normal The normal of this plane. If it is not normalized, unexpected results may occur.
	\param	pointOnPlane Any point belonging to this plane
	*******************************************************************************************************************/
	Plane3d(const glm::vec3& normal, const glm::vec3& pointOnPlane) : m_norm(normal) {	m_dist = glm::length(pointOnPlane);	}

	/*!****************************************************************************************************************
	\brief  Constructs a plane from three points.
	\param	point0 A point belonging to the plane
	\param	point1 A point belonging to the plane
	\param	point2 A point belonging to the plane
	*******************************************************************************************************************/
	Plane3d(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2) {set(point0, point1, point2);}

	/*!****************************************************************************************************************
	\brief  Sets a plane from normal and distance. Distance is the scalar number that is the distance of this
	        plane from (0,0,0).
	\param	normal The normal of this plane. MUST BE NORMALISED. If it not normalized, unexpected results may occur.
	\param	dist The signed distance, along the plane's normal direction, between the coordinate start and (0,0,0). This
	        number is defined as the number that the normal must be multiplied with so that the normal's coordinates
			define a point on the plane.
	*******************************************************************************************************************/
	void set(const glm::vec3& normal, float32 dist) { set(normal, dist); }

	/*!****************************************************************************************************************
	\brief  Sets a plane from normal and a point on this plane.
	\param	normal The normal of this plane. If it is not normalized, unexpected results may occur.
	\param	pointOnPlane Any point belonging to this plane
	*******************************************************************************************************************/
	void set(const glm::vec3& normal, const glm::vec3& pointOnPlane)
	{
		m_dist = glm::length(pointOnPlane);
		m_norm = normal;
	}

	/*!****************************************************************************************************************
	\brief  Sets a plane from three points.
	\param	point0 A point belonging to the plane
	\param	point1 A point belonging to the plane
	\param	point2 A point belonging to the plane
	*******************************************************************************************************************/
	void set(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2)
	{
		glm::vec3 edge0 = point0 - point1;
		glm::vec3 edge1 = point2 - point1;

		m_norm = glm::normalize(glm::cross(edge0, edge1));
		m_dist = -glm::dot(m_norm, point0);
	}

	/*!****************************************************************************************************************
	\brief	Find the signed distance between a point and the plane. Positive means distance along the normal, negative
	        means distance opposite to the normal's direction.
	\param	point The point
	\return	The signed distance between the point and this plane.
	*******************************************************************************************************************/
	float32 distantTo(const glm::vec3& point)	{		glm::dot(m_norm, point) - m_dist;	}

	/*!****************************************************************************************************************
	\brief	Get the distance of this plane to the coordinate start (0,0,0).
	\return	The distance of this plane to the coordinate start (0,0,0)
	*******************************************************************************************************************/
	float32 getDistance()const { return m_dist; }

	/*!****************************************************************************************************************
	\brief	Get the normal of this plane.
	\return	The normal of this plane.
	*******************************************************************************************************************/
	glm::vec3 getNormal()const { return m_norm; }

	/*!****************************************************************************************************************
	\brief	Transform the plane with a transformation matrix.
	\param	transMtx The matrix to transform this plane with.
	*******************************************************************************************************************/
	void transform(glm::mat4& transMtx) {	glm::transpose(glm::inverse(transMtx)) * glm::vec4(m_norm, m_dist);	}
private:
	glm::vec3	m_norm;
	float32		m_dist;
};
}// namespace pvr