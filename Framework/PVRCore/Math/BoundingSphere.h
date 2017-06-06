#pragma once
#include "../External/glm/glm.hpp"
#include "PVRCore/Base/Types.h"
#include "PVRCore/Base/Assert_.h"
namespace pvr {
namespace math {

class BoundingSphere
{
public:
	BoundingSphere() : _isValid(false) {}
	BoundingSphere(const glm::vec3& aabbMin, const glm::vec3 aabbMax)
	{
		expandRadius(aabbMin, aabbMax);
	}


	const glm::vec3& getCenter()const { return _center; }

	float32 getRadius()const { return _radius; }



	void set(const glm::vec3& center, float32 radius)
	{
		PVR_ASSERTION(radius > 0);
		_center = center;
		_radius = radius;
	}

	void expandRadius(const glm::vec3& point)
	{
		float32 len = glm::length(point - _center);
		_radius = (len > _radius ? len : _radius);
	}

	bool isInside(const glm::vec3& point)const
	{
		const glm::vec3 v0(_center - point);
		return (glm::dot(v0, v0) <= _radius * _radius);
	}

	void expandRadius(const glm::vec3* points, uint32 numPoints)
	{
		for (uint32 i = 0; i < numPoints; ++i) { expandRadius(points[i]); }
	}

	void expandRadius(const BoundingSphere& sphere)
	{
		float32 r = glm::length(sphere.getCenter() - getCenter()) + sphere.getRadius();
		_radius = (r > _radius ? r : _radius);
	}

	void expandRadius(const glm::vec3& aabbMin, const glm::vec3 aabbMax)
	{
		//expand the 8 possible points
		expandRadius(aabbMin);
		expandRadius(glm::vec3(aabbMin.x, aabbMax.y, aabbMin.z));
		expandRadius(glm::vec3(aabbMin.x, aabbMax.y, aabbMax.z));
		expandRadius(glm::vec3(aabbMin.x, aabbMin.y, aabbMax.z));
		expandRadius(aabbMax);
		expandRadius(glm::vec3(aabbMax.x, aabbMax.y, aabbMin.z));
		expandRadius(glm::vec3(aabbMax.x, aabbMin.y, aabbMax.z));
		expandRadius(glm::vec3(aabbMax.x, aabbMin.y, aabbMin.z));
	}

	/*!
	\brief  expand the bounding sphere which includes the given point.
			The center will be recalculated to include this new point and the previous point
	*/
	void expand(const glm::vec3& point)
	{
		if (_isValid)
		{
			const glm::vec3 dir = point - _center;
			if (glm::length2(dir) > _radius * _radius)
			{
				glm::vec3 g =  _center - _radius * glm::normalize(dir);
				// calculate the new center
				_center = (g + point) * .5f;
				_radius = glm::length(point - _center);
			}
		}
		else
		{
			_center = point; _radius = 1;
		}
	}

private:
	glm::vec3 _center;
	float32 _radius;
	bool _isValid;
};

}
}