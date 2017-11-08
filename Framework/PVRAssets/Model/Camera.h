/*!
\brief Represents a Camera in the scene (Model.h).
\file PVRAssets/Model/Camera.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace assets {
/// <summary>Contains all information necessary to recreate a Camera in the scene.</summary>
class Camera
{
public:
	/// <summary>Raw internal structure of the Camera.</summary>
	struct InternalData
	{
		int32_t  targetNodeIdx;   /*!< Index of the target object */ // Should this be a point to the actual node?
		std::vector<float> FOVs;  /*!< Field of view */
		float farClip;      /*!< Far clip plane */
		float nearClip;     /*!< Near clip plane */

		InternalData() : targetNodeIdx(-1), farClip(5000.0f), nearClip(5.0f)
		{
		}
	};

public:
	/// <summary>If the camera points to a specific point, get index to the target node.</summary>
	/// <returns>The target node index</returns>
	inline int32_t getTargetNodeIndex() const { return _data.targetNodeIdx; }

	/// <summary>Sets the specified node as the look-at target of the camera.</summary>
	/// <param name="idx">Node index of the desired target.</param>
	inline void setTargetNodeIndex(int32_t idx) { _data.targetNodeIdx = idx; }


	/// <summary>Get the number of frames that this camera's animation supports.</summary>
	/// <returns>The number of frames</returns>
	inline uint32_t getNumFrames() const { return static_cast<uint32_t>(_data.FOVs.size()); }

	/// <summary>Get the far clipping plane distance.</summary>
	/// <returns>The far clipping plane distance (z)</returns>
	inline float getFar()  const { return _data.farClip; }

	/// <summary>Get the near clipping plane distance.</summary>
	/// <param name="farClip">The Z coord (distance) of the far clipping plane</param>
	inline void setFar(float farClip) { _data.farClip = farClip; }

	/// <summary>Get near clip plan distance.</summary>
	/// <returns>The near clipping plane distance (z)</returns>
	inline float getNear() const  { return _data.nearClip; }

	/// <summary>Set the near clipping plane distance.</summary>
	/// <param name="nearClip">The Z coord (distance) of the near clipping plane</param>
	inline void setNear(float nearClip) { _data.nearClip = nearClip; }

	/// <summary>Get field of view for a specific frame (in radians). Interpolates between frames.
	/// The interpolation point is between <paramref name="frame"/>and <paramref name="frame"/>+1,
	/// with factor <paramref name="interp."/></summary>
	/// <param name="frame">The initial frame. Interpolation will be between this frame and the next.</param>
	/// <param name="interp">Interpolation factor. If zero, frame = <paramref name="frame."/>If one frame = is
	/// <paramref name="frame"/>+ 1.</param>
	/// <returns>The field of vision in radians.</returns>
	float getFOV(uint32_t frame = 0, float interp = 0)  const;

	/// <summary>Set field of view (Radians)</summary>
	/// <param name="fov"/>The FOV to set, in radians</param>
	void setFOV(float fov);

	/// <summary>Set a field of view animation for a number of frame.</summary>
	/// <param name="numFrames">The number of frames to set the Fov to.</param>
	/// <param name="fovs">An array of packed floats, which will be interpreted as</param>
	void setFOV(uint32_t numFrames, const float* fovs);

	/// <summary>Get a reference to the internal data of this object. Handle with care.</summary>
	/// <returns>A (modifiable) reference to the internal data.
	inline InternalData& getInternalData() { return _data; }
private:
	InternalData _data;
};
}
}