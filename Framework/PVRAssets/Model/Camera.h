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
		int32	 targetNodeIdx;		/*!< Index of the target object */ // Should this be a point to the actual node?
		std::vector<float32> FOVs;	/*!< Field of view */
		float32 farClip;			/*!< Far clip plane */
		float32 nearClip;			/*!< Near clip plane */

		InternalData() : targetNodeIdx(-1), farClip(5000.0f), nearClip(5.0f)
		{
		}
	};

public:
	/// <summary>If the camera points to a specific point, get index to the target node.</summary>
	inline int32 getTargetNodeIndex() const { return _data.targetNodeIdx; }

	/// <summary>Sets the specified node as the look-at target of the camera.</summary>
	/// <param name="idx">Node index of the desired target.</param>
	inline void setTargetNodeIndex(int32 idx) { _data.targetNodeIdx = idx; }


	/// <summary>Get the number of frames that this camera's animation supports.</summary>
	inline uint32	getNumFrames() const { return static_cast<uint32>(_data.FOVs.size()); }

	/// <summary>Get the far clipping plane distance.</summary>
	inline float32 getFar()  const { return _data.farClip; }

	/// <summary>Get the near clipping plane distance.</summary>
	inline void setFar(float32 farClip) { _data.farClip = farClip; }

	/// <summary>Get near clip plan distance.</summary>
	inline float32 getNear() const  { return _data.nearClip; }

	/// <summary>Set the near clipping plane distance.</summary>
	inline void setNear(float32 nearClip) { _data.nearClip = nearClip; }


	/// <summary>Get field of view for a specific frame. Interpolates between frames. The interpolation point is
	/// between <paramref name="frame"/>and <paramref name="frame"/>+1, with factor <paramref name="interp."/>
	/// </summary>
	/// <param name="frame">The initial frame. Interpolation will be between this frame and the next.</param>
	/// <param name="interp">Interpolation factor. If zero, frame = <paramref name="frame."/>If one frame = is
	/// <paramref name="frame"/>+ 1.</param>
	float32 getFOV(uint32 frame = 0, float32 interp = 0)  const;

	/// <summary>Set field of view (Radians)</summary>
	void setFOV(float32 fov);

	/// <summary>Set a field of view animation for a number of frame.</summary>
	/// <param name="numFrames">The number of frames to set the Fov to.</param>
	/// <param name="fovs">An array of packed floats, which will be interpreted as</param>
	void setFOV(uint32 numFrames, const float32* fovs);

	/// <summary>Get a reference to the internal data of this object. Handle with care.</summary>
	inline InternalData& getInternalData() { return _data; }
private:
	InternalData _data;
};
}
}