/*!
\brief Class used to handle common AABB operations.
\file PVRCore/Math/AxisAlignedBox.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "../External/glm/glm.hpp"
#include "PVRCore/Base/Types.h"
#include <cfloat>

namespace pvr {
/// <summary>Contains mathematical functionality and classes, such as bounding box calculations, intersections etc.
/// </summary>
namespace math {

/// <summary>This class provides functionality to handle the volume enclosed by 6 planes, normally the viewing
/// frustum. The planes are represented in Hessian Normal Form (normal, distance) as vec4( (xyz):[normal],
/// w:[distance from 0,0,0] ) In order to optimize the frustum, the following are assumed: A "frustum part" of a
/// plane means "the quadrilateral of the plane that is enclosed by the 4 other planes not opposite to it" 1)
/// Frustum parts opposite each other do not intersect 2) Any point of a positive(negative) part of the frustum has
/// a larger(smaller) corresponding coordinate than its opposite part 3) The frustum is "opening" accross the z
/// axis accross al direction, meaning that the positive Z part of the frustum completely encloses the projection
/// of the negative z part on it. These optimizations allow us to</summary>
struct Frustum
{
	glm::vec4 minusX;
	glm::vec4 plusX;
	glm::vec4 minusY;
	glm::vec4 plusY;
	glm::vec4 minusZ;
	glm::vec4 plusZ;
};

/// <summary>This class provides specialized functionality for when a frustum is a "normal"viewing frustum, that
/// is, the following conditions hold (The conditions ARE NOT checked) Note: A "frustum side" of a plane means "the
/// quadrilateral of the plane that is enclosed by the 4 other planes not opposite to it" "Opposite" means the
/// "other" side of the frustum (minusX is opposit plusX). 1) Opposite Frustum sides do not intersect (their planes
/// may do so outside the frustum) 2) The frustum is "opening", or at least not "closing" accross the z axis
/// accross al direction, meaning that the any point of the projection of the negative Z side of the frustum on the
/// positive Z plane, is inside or on the positive Z side of the frustum. 3) Any point of a positive(negative) part
/// of the frustum has a larger(smaller) corresponding coordinate than its opposite part These optimizations allow
/// us to greatly reduce the calculations for a viewing frustum. 4) All plane normals point INTO of the frustum
/// </summary>
struct ViewingFrustum : public Frustum
{
	bool isFrustum() const
	{
		bool xopp = (glm::dot(glm::vec3(minusX), glm::vec3(plusX)) < 0);
		bool yopp = (glm::dot(glm::vec3(minusY), glm::vec3(plusY)) < 0);
		bool zopp = (glm::dot(glm::vec3(minusZ), glm::vec3(plusZ)) < 0);
		return xopp && yopp && zopp;
	};
};

inline float distancePointToPlane(const glm::vec3& point, const glm::vec4& plane)
{
	return glm::dot(point, glm::vec3(plane.x, plane.y, plane.z)) + plane.w;
}
inline bool pointOnSide(const glm::vec3& point, const glm::vec4& plane)
{
	return distancePointToPlane(point, plane) >= 0.0f;
}

inline void getFrustumPlanes(const glm::mat4& projection_from_world, ViewingFrustum& frustum_out)
{
	const glm::vec4 row0 = glm::row(projection_from_world, 0);
	const glm::vec4 row1 = glm::row(projection_from_world, 1);
	const glm::vec4 row2 = glm::row(projection_from_world, 2);
	const glm::vec4 row3 = glm::row(projection_from_world, 3);

	frustum_out.minusX = row3 + row0;
	frustum_out.plusX = row3 - row0;
	frustum_out.minusY = row3 + row1;
	frustum_out.plusY = row3 - row1;
	frustum_out.minusZ = row3 + row2;
	frustum_out.plusZ = row3 - row2;

	frustum_out.minusX = frustum_out.minusX * (1 / glm::length(glm::vec3(frustum_out.minusX)));
	frustum_out.plusX = frustum_out.plusX * (1 / glm::length(glm::vec3(frustum_out.plusX)));
	frustum_out.minusY = frustum_out.minusY * (1 / glm::length(glm::vec3(frustum_out.minusY)));
	frustum_out.plusY = frustum_out.plusY * (1 / glm::length(glm::vec3(frustum_out.plusY)));
	frustum_out.minusZ = frustum_out.minusZ * (1 / glm::length(glm::vec3(frustum_out.minusZ)));
	frustum_out.plusZ = frustum_out.plusZ * (1 / glm::length(glm::vec3(frustum_out.plusZ)));

	//frustum_out.plusY = frustum_out.plusY * (1.f - 2.f * signbit(frustum_out.plusY.w));
	//frustum_out.minusY = frustum_out.minusY * (1.f - 2.f * signbit(frustum_out.minusY.w));
	//frustum_out.minusX = frustum_out.minusX * (1.f - 2.f * signbit(frustum_out.minusX.w));
	//frustum_out.minusZ = frustum_out.minusZ * (1.f - 2.f * signbit(frustum_out.minusZ.w));
	//frustum_out.plusX = frustum_out.plusX * (1.f - 2.f * signbit(frustum_out.plusX.w));
	//frustum_out.plusZ = frustum_out.plusZ * (1.f - 2.f * signbit(frustum_out.plusZ.w));


}


/// <summary>This class provides functionality to handle 3 dimensional Axis Aligned Boxes. Center-halfextent
/// representation.</summary>
class AxisAlignedBox
{
	glm::vec3	_center;
	glm::vec3	_halfExtent;
public:
	/// <summary>Constructor center/halfextent.</summary>
	AxisAlignedBox(const glm::vec3& center = glm::vec3(0.0f), const glm::vec3& halfExtent = glm::vec3(0.f))
	{
		set(center, halfExtent);
	}

	/// <summary>Sets center and extents to zero.</summary>
	void clear()
	{
		_center.x = _center.y = _center.z = 0.0f;
		_halfExtent = glm::vec3(0.f);
	}

	/// <summary>Sets from min and max. All components of min must be than all components in max.</summary>
	void setMinMax(const glm::vec3& min, const glm::vec3& max)
	{
		// compute the center.
		_center = (max + min) * .5f;
		_halfExtent = (max - min) * .5f;
	}

	/// <summary>Sets from center and half extent.</summary>
	void set(const glm::vec3& centerPoint, const glm::vec3& halfExtent)
	{
		_center = centerPoint;
		_halfExtent = halfExtent;
	}

	void remove(float32 x, float32 y, float32 z)
	{
		remove(glm::vec3(x, y, z));
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

	/// <summary>Add a new point to the box. The new box will be the minimum box containing the old box and the new
	/// point.</summary>
	void add(const glm::vec3& point)
	{
		setMinMax(glm::min(point, getMin()), glm::max(point, getMax()));
	}

	/// <summary>Merge two axis aligned boxes. The new box will be the minimum box containing both the old and the new
	/// box.</summary>
	void add(const AxisAlignedBox& aabb)
	{
		add(aabb.getMin());
		add(aabb.getMax());
	}

	/// <summary>Add a new point to the box. The new box will be the minimum box containing the old box and the new
	/// point.</summary>
	void add(float32 x, float32 y, float32 z)
	{
		add(glm::vec3(x, y, z));
	}

	/// <summary>Return a point consisting of the smallest (minimum) coordinate in each axis.</summary>
	glm::vec3 getMin() const { return _center - _halfExtent; }

	/// <summary>Return a point consisting of the largest (maximum) coordinate in each axis.</summary>
	glm::vec3 getMax() const { return _center + _halfExtent; }

	/// <summary>Return the min and the max.</summary>
	/// <param name="outMin">Output: The min point will be stored here.</param>
	/// <param name="outMax">Output: The max point will be stored here.</param>
	void getMinMax(glm::vec3& outMin, glm::vec3& outMax) const
	{
		outMin = _center - _halfExtent; outMax = _center + _halfExtent;
	}

	/// <summary>Get the local bounding box transformed by a provided matrix.</summary>
	/// <param name="m">An affine transformation matrix. Skew will be ignored.</param>
	/// <param name="outAABB">The transformed AABB will be stored here. Previous contents ignored.</param>
	void transform(const glm::mat4& m, AxisAlignedBox& outAABB)
	{
		outAABB._center = glm::vec3(m[3]) + (glm::mat3(m) * this->center());

		glm::mat3 absModelMatrix;
		absModelMatrix[0] = glm::vec3(fabs(m[0][0]), fabs(m[0][1]), fabs(m[0][2]));
		absModelMatrix[1] = glm::vec3(fabs(m[1][0]), fabs(m[1][1]), fabs(m[1][2]));
		absModelMatrix[2] = glm::vec3(fabs(m[2][0]), fabs(m[2][1]), fabs(m[2][2]));

		
		outAABB._halfExtent = glm::vec3(absModelMatrix * this->getHalfExtent());
	}

	/// <summary>Get the size (width, height, depth) of the AABB.</summary>
	glm::vec3 getSize()const { return _halfExtent + _halfExtent;}

	/// <summary>Get the half-size (half-width, half-height, half-depth) of the AABB.</summary>
	glm::vec3 getHalfExtent()const { return _halfExtent; }

	/// <summary>Get the -x +y +z corner of the box.</summary>
	glm::vec3 topLeftFar()const
	{
		return _center + glm::vec3(-_halfExtent.x, _halfExtent.y, _halfExtent.z);
	}

	/// <summary>Get the center-x +y +z point of the box.</summary>
	glm::vec3 topCenterFar()const
	{
		return _center + glm::vec3(0, _halfExtent.y, _halfExtent.z);
	}

	/// <summary>Get the +x +y +z corner of the box.</summary>
	glm::vec3 topRightFar()const
	{
		return _center + _halfExtent;
	}

	/// <summary>Get the -x +y -z corner of the box.</summary>
	glm::vec3 topLeftNear()const
	{
		return _center + glm::vec3(-_halfExtent.x, _halfExtent.y, _halfExtent.z);
	}

	/// <summary>Get the center-x +y -z point of the box.</summary>
	glm::vec3 topCenterNear()const
	{
		return _center + glm::vec3(0., _halfExtent.y, -_halfExtent.z);
	}

	/// <summary>Get the +x +y -z corner of the box.</summary>
	glm::vec3 topRightNear()const
	{
		return _center + glm::vec3(_halfExtent.x, _halfExtent.y, -_halfExtent.z);
	}

	/// <summary>Get the center-x center-y center-z corner of the box.</summary>
	glm::vec3 center()const
	{
		return _center;
	}

	/// <summary>Get the -x center-y -z corner of the box.</summary>
	glm::vec3 centerLeftNear()const
	{
		return _center + glm::vec3(-_halfExtent.x, 0, -_halfExtent.z);
	}
	/// <summary>Get the center-x center-y -z corner of the box.</summary>
	glm::vec3 centerNear()const
	{
		return _center + glm::vec3(0, 0, -_halfExtent.z);
	}

	/// <summary>Get the +x center-y -z corner of the box.</summary>
	glm::vec3 centerRightNear()const
	{
		return _center + glm::vec3(_halfExtent.x, 0, -_halfExtent.z);
	}

	/// <summary>Get the -x center-y +z corner of the box.</summary>
	glm::vec3 centerLeftFar()const
	{
		return _center + glm::vec3(-_halfExtent.x, 0, _halfExtent.z);
	}

	/// <summary>Get the center-x center-y +z corner of the box.</summary>
	glm::vec3 centerFar()const
	{
		return _center + glm::vec3(0, 0, _halfExtent.z);
	}

	/// <summary>Get the +x center-y +z corner of the box.</summary>
	glm::vec3 centerRightFar()const
	{
		return _center + glm::vec3(_halfExtent.x, 0, _halfExtent.z);
	}

	/// <summary>Get the -x -y -z corner of the box.</summary>
	glm::vec3 bottomLeftNear()const
	{
		return _center + glm::vec3(-_halfExtent.x, -_halfExtent.y, _halfExtent.z);
	}

	/// <summary>Get the center-x -y -z corner of the box.</summary>
	glm::vec3 bottomCenterNear()const
	{
		return _center + glm::vec3(0, -_halfExtent.y, _halfExtent.z);
	}

	/// <summary>Get the +x -y -z corner of the box.</summary>
	glm::vec3 bottomRightNear()const
	{
		return _center + glm::vec3(_halfExtent.x, -_halfExtent.y, _halfExtent.z);
	}

	/// <summary>Get the -x -y +z corner of the box.</summary>
	glm::vec3 bottomLeftFar()const
	{
		return _center + glm::vec3(-_halfExtent.x, -_halfExtent.y, _halfExtent.z);
	}

	/// <summary>Get the center-x -y +z corner of the box.</summary>
	glm::vec3 bottomCenterFar()const
	{
		return _center + glm::vec3(0, -_halfExtent.y, _halfExtent.z);
	}

	/// <summary>Get the +x -y +z corner of the box.</summary>
	glm::vec3 bottomRightFar()const
	{
		return _center + glm::vec3(_halfExtent.x, -_halfExtent.y, _halfExtent.z);
	}

	/// <summary>Set this AABB as the minimum AABB that contains itself and the AABB provided.</summary>
	void mergeBox(const AxisAlignedBox& rhs)
	{
		setMinMax(glm::min(getMin(), rhs.getMin()), glm::max(getMax(), rhs.getMax()));
	}

	bool operator==(const AxisAlignedBox& rhs)const
	{
		return _center == rhs._center && _halfExtent == rhs._halfExtent;
	}

	bool operator!=(const AxisAlignedBox& rhs)const
	{
		return !(*this == rhs);
	}


};


namespace {
inline bool noPointsOnSide(glm::vec3 blf, glm::vec3 tlf, glm::vec3 brf, glm::vec3 trf,
                           glm::vec3 bln, glm::vec3 tln, glm::vec3 brn, glm::vec3 trn,
                           glm::vec4 plane)
{
	if (pointOnSide(blf, plane) ||
	    pointOnSide(tlf, plane) ||
	    pointOnSide(brf, plane) ||
	    pointOnSide(trf, plane) ||
	    pointOnSide(bln, plane) ||
	    pointOnSide(tln, plane) ||
	    pointOnSide(brn, plane) ||
	    pointOnSide(trn, plane))
	{
		return false;
	}
	return true;
}

}

inline bool aabbInFrustum(const AxisAlignedBox& box, const ViewingFrustum& frustum)
{
	//return !(
	//	//Is the whole frustum on internal side of +Z? (far)
	//	pointOnSide(box.bottomLeftFar(), frustum.plusZ) ||
	//	pointOnSide(box.bottomRightFar(), frustum.plusZ) ||
	//	pointOnSide(box.topLeftFar(), frustum.plusZ) ||
	//	pointOnSide(box.topRightFar(), frustum.plusZ) ||

	//	//Is the whole frustum on internal side of -Z? (near)
	//	pointOnSide(box.bottomLeftNear(), frustum.minusZ) ||
	//	pointOnSide(box.bottomRightNear(), frustum.minusZ) ||
	//	pointOnSide(box.topLeftNear(), frustum.minusZ) ||
	//	pointOnSide(box.topRightNear(), frustum.minusZ) ||

	//	// Since frustum is "closing" or "parallel", and the box is "axis aligned"
	//	// it can be proven that we can only test the "near" objects for the sides.
	//	//and then only the two on the right side. So we only test:
	//	//That the "near left" points are inside -X
	//	pointOnSide(box.bottomLeftNear(), frustum.minusX) ||
	//	pointOnSide(box.topLeftNear(), frustum.plusX) ||
	//	//That the "near right" points are inside +X
	//	pointOnSide(box.bottomRightNear(), frustum.minusX) ||
	//	pointOnSide(box.topRightNear(), frustum.plusX) ||

	//	//That the "near bottom" points are inside -Y
	//	pointOnSide(box.bottomLeftNear(), frustum.minusY) ||
	//	pointOnSide(box.bottomRightNear(), frustum.minusY) ||

	//	//And That the "near top" points are inside +Y
	//	pointOnSide(box.topLeftNear(), frustum.plusY) ||
	//	pointOnSide(box.topRightNear(), frustum.plusY));

	//OPTIMIZED VERSION : Move reused calcs together

	glm::vec3 blf = box.bottomLeftFar();
	glm::vec3 tlf = box.topLeftFar();
	glm::vec3 brf = box.bottomRightFar();
	glm::vec3 trf = box.topRightFar();

	glm::vec3 bln = box.bottomLeftNear();
	glm::vec3 tln = box.topLeftNear();
	glm::vec3 brn = box.bottomRightNear();
	glm::vec3 trn = box.topRightNear();

	if (noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.minusX) ||
	    noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.plusX) ||
	    noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.minusY) ||
	    noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.plusY) ||
	    noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.minusZ) ||
	    noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.plusZ))
	{
		return false;
	}
	return true;
}



/// <summary>An AABB with a min-max representation.</summary>
class AxisAlignedBoxMinMax
{
	glm::vec3	_min;
	glm::vec3	_max;
public:
	void setMin(const glm::vec3& min) { _min = min; }
	void setMax(const glm::vec3& max) { _max = max; }
	const glm::vec3& getMin() { return _min; }
	const glm::vec3& getMax() { return _max; }

	void add(const glm::vec3& point)
	{
		_min = glm::min(_min, point);
		_max = glm::max(_max, point);
	}
};
}
}