#pragma once
#include "PVRCore/Base/Types.h"
namespace pvr {
namespace math {

/// <summary>A class representing a sphere containing at least all points of an object</summary>
class BoundingSphere
{
public:
	/// <summary>Constructor. Non-initializing.</summary>
	BoundingSphere() : _isValid(false) {}
	/// <summary>Constructor. Creates a bounding sphere that contains an Axis Aligned Bounding Box.</summary>
	/// <param name="aabbMin">The minimum (bottom-left-back) corner of the Bounding Box</param>
	/// <param name="aabbMax">The maximum (top-right-forward) corner of the Bounding Box</param>
	BoundingSphere(const glm::vec3& aabbMin, const glm::vec3 aabbMax)
	{
		expandRadius(aabbMin, aabbMax);
	}

	/// <summary>Get the center of the sphere</summary>
	/// <returns>The center of the sphere</returns>
	const glm::vec3& getCenter()const { return _center; }

	/// <summary>Get the radius of the sphere</summary>
	/// <returns>The radius of the sphere</returns>
	float getRadius()const { return _radius; }

	/// <summary>Set the sphere from center and radius</summary>
	/// <param name="center">The new center of the sphere</param>
	/// <param name="radius">The new radius of the sphere</param>
	void set(const glm::vec3& center, float radius)
	{
		assert(radius > 0);
		_center = center;
		_radius = radius;
	}

	/// <summary>Ensure that the sphere contains a point, expanding it as needed. If the
	/// sphere already contains the point, do nothing. If the point is outside the sphere,
	/// increase the radius of the sphere to exactly contain it.</summary>
	/// <param name="point">The new point to include in the sphere</param>
	void expandRadius(const glm::vec3& point)
	{
		float len = glm::length(point - _center);
		_radius = (len > _radius ? len : _radius);
	}

	/// <summary>Test if a point is inside the sphere's radius</summary>
	/// <param name="point">The point to test</param>
	/// <returns>True if the point is inside of or on the sphere, otherwise false</returns>
	bool isInside(const glm::vec3& point)const
	{
		const glm::vec3 v0(_center - point);
		return (glm::dot(v0, v0) <= _radius * _radius);
	}

	/// <summary>Ensure that the sphere contains a number of points, expanding it as needed.
	/// If any point is outside the sphere, increase the radius of the sphere to exactly contain
	/// it, while preserving the center of the sphere.</summary>
	/// <param name="points">C-style array of the new points to include in the sphere</param>
	/// <param name="numPoints">Size of the array (number of points)</param>
	void expandRadius(const glm::vec3* points, uint32_t numPoints)
	{
		for (uint32_t i = 0; i < numPoints; ++i) { expandRadius(points[i]); }
	}

	/// <summary>Ensure that a bounding sphere is large enough to contain another sphere. Does not move
	/// center, only expands radius. Will increase the radius (if needed) enough to exactly contain the
	/// other sphere.</summary>
	/// <param name="sphere">A bounding sphere to add to this sphere.</param>
	void expandRadius(const BoundingSphere& sphere)
	{
		float r = glm::length(sphere.getCenter() - getCenter()) + sphere.getRadius();
		_radius = (r > _radius ? r : _radius);
	}

	/// <summary>Enure an AABB is completely enclosed in the sphere. If it isn't, expand the sphere to include it.</summary>
	/// <param name="aabbMin">The minimum point (bottom-left-back corner) of the axis aligned box</param>
	/// <param name="aabbMax">The maximum point (top-right-forward corner) of the axis aligned box</param>
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

	/// <summary>Expand the bounding sphere to includes the given point. If the point is not already
	/// inside the sphere, increase the radius and move the center so that the new sphere contains both
	/// the old one and the new point (the new sphere exactly contains the new point as is tangent to
	/// the old sphere point at the point opposite to the new point in relation to the center)</summary>
	/// <param name="point">The new point to include</param>
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
	float _radius;
	bool _isValid;
};

}
}