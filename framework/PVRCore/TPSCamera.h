/*!
\brief A class representing a Third person camera and functionality to manipulate it.
\file PVRCore/TPSCamera.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "../../external/glm/glm.hpp"
#include "../../external/glm/gtc/matrix_transform.hpp"
// Third persorn camera
namespace pvr {
/// <summary>A simple third person camera implementation</summary>
class TPSCamera
{
public:
	/// <summary>Default Constructor</summary>
	TPSCamera() : _yaw(0.0f), offsetY(0.0f), offsetZ(0.0f), _pos(0.0f), _isDirty(true) {}

	/// <summary>Set the height from the floor</summary>
	/// <param name="height">height</param>
	void setHeight(float height)
	{
		offsetY = height;
		_isDirty = true;
	}

	/// <summary>Set the camera distance from the target</summary>
	/// <param name="dist">Distance</param>
	void setDistanceFromTarget(const float dist)
	{
		offsetZ = dist;
		_isDirty = true;
	}

	/// <summary>Set the camera target position i.e. the 'lookat' value.</summary>
	/// <param name="targetPos">Target position</param>
	void setTargetPosition(const glm::vec3& targetPos)
	{
		_pos = glm::vec3(0.f);
		updateTargetPosition(targetPos);
	}
	/// <summary>Update the camera target position i.e. the 'lookat' value.</summary>
	/// <param name="pos">The targets new position</param>
	void updateTargetPosition(const glm::vec3& pos)
	{
		_pos += pos;
		_isDirty = true;
	}
	/// <summary>Update the camera target look angle.</summary>
	/// <param name="angleDeg">The camera's new angle in degrees</param>
	void updateTargetLookAngle(float angleDeg)
	{
		_yaw += angleDeg;
		_isDirty = true;
	}
	/// <summary>Set the camera target look angle.</summary>
	/// <param name="angleDeg">The camera's new angle in degrees</param>
	void setTargetLookAngle(float angleDeg)
	{
		_yaw = 0.0f;
		updateTargetLookAngle(angleDeg);
	}

	/// <summary>Calculates and returns the camera view matrix based on the most up to date camera properties if they have been updated (are dirty) or returns the last view matrix
	/// caluclated.</summary>
	/// <returns>The TPS camera view matrix</returns>
	const glm::mat4& getViewMatrix() const
	{
		// Construct the matrix and return
		if (_isDirty)
		{
			_isDirty = false;
			// this makes the camera aligned behind the target.
			float rotation = _yaw + 180.f + 90.f;

			glm::vec3 cameraPos = glm::vec3(_pos.x, offsetY, _pos.z + offsetZ);

			// Rotate the camera around the target origin
			const glm::vec3& rotateOrigin = _pos;

			// Read from bottom to top.
			cameraPos = glm::vec3(glm::translate(rotateOrigin) * // translate back
				glm::rotate(glm::radians(rotation), glm::vec3(0.0, 1.f, 0.0)) * // apply rotation
				glm::translate(-rotateOrigin) * // bring the rotation to the center of the origin
				glm::vec4(cameraPos, 1.f));

			// rotate the position of the camera in
			_viewX = glm::lookAt(cameraPos, _pos, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		return _viewX;
	}

private:
	float _yaw;
	float offsetY, offsetZ; // Camera oiffset
	glm::vec3 _pos; // Character position
	mutable glm::mat4 _viewX;
	mutable bool _isDirty;
};
} // namespace pvr
