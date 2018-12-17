/*!
\brief A class representing a Third person camera and functionality to manipulate it.
\file PVRCore/TPSCamera.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/glm.h"

// Third persorn camera
namespace pvr {
/// <summary>A simple third person camera implementation</summary>
class TPSCamera
{
public:
	/// <summary>Default Constructor</summary>
	TPSCamera() : _yaw(0.0f), offsetY(0.0f), offsetZ(0.0f), _targetPos(0.0f), _updatePosition(true), _updateView(false) {}

	/// <summary>Set the height from the floor</summary>
	/// <param name="height">height</param>
	void setHeight(float height)
	{
		offsetY = height;
		_updatePosition = true;
		_updateView = true;
	}

	/// <summary>Set the camera distance from the target</summary>
	/// <param name="dist">Distance</param>
	void setDistanceFromTarget(const float dist)
	{
		offsetZ = dist;
		_updatePosition = true;
		_updateView = true;
	}

	/// <summary>Set the camera target position i.e. the 'lookat' value.</summary>
	/// <param name="targetPos">Target position</param>
	void setTargetPosition(const glm::vec3& targetPos)
	{
		_targetPos = glm::vec3(0.f);
		updateTargetPosition(targetPos);
	}
	/// <summary>Update the camera target position i.e. the 'lookat' value.</summary>
	/// <param name="pos">The targets new position</param>
	void updateTargetPosition(const glm::vec3& pos)
	{
		_targetPos += pos;
		_updatePosition = true;
		_updateView = true;
	}
	/// <summary>Update the camera target look angle.</summary>
	/// <param name="angleDeg">The camera's new angle in degrees</param>
	void updateTargetLookAngle(float angleDeg)
	{
		_yaw += angleDeg;
		_updatePosition = true;
		_updateView = true;
	}
	/// <summary>Set the camera target look angle. Tn this implementation, angle 0 means the camera is facing the north.</summary>
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
		const glm::vec3& cameraPos = getCameraPosition();
		// Construct the matrix and return
		if (_updateView)
		{
			_updateView = false;
			// rotate the position of the camera in
			_viewX = glm::lookAt(cameraPos, _targetPos, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		return _viewX;
	}

	const glm::vec3& getCameraPosition() const
	{
		// Construct the matrix and return
		if (_updatePosition)
		{
			_updatePosition = false;
			_updateView = true;
			// this makes the camera aligned behind the target and offset it by 90 degree because our initial axe start from north.
			float rotation = _yaw + 180.f + 90.f;

			glm::vec3 dir = glm::mat3(glm::rotate(glm::radians(rotation), glm::vec3(0.0, 1.f, 0.0))) * glm::vec3(1.0f, 0.0f, 0.0f);
			_cameraPos = dir * offsetZ + _targetPos;
			_cameraPos.y = offsetY;
		}
		return _cameraPos;
	}

private:
	float _yaw;
	float offsetY, offsetZ; // Camera oiffset
	glm::vec3 _targetPos; // Character
	mutable glm::vec3 _cameraPos;
	mutable glm::mat4 _viewX;
	mutable bool _updatePosition, _updateView;
};
} // namespace pvr
