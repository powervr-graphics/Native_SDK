/*!*********************************************************************************************************************
\file         PVRAssets/Model/Camera.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Represents a Camera in the scene (Model.h).
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace assets {
/*!*******************************************************************************************
\brief Contains all information necessary to recreate a Camera in the scene.
*********************************************************************************************/
class Camera
{
public:
	/*!************************************************************************************************************
	\brief Raw internal structure of the Camera.
	***************************************************************************************************************/
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
	/*!******************************************************************************
	\brief	If the camera points to a specific point, get index to the target node.
	********************************************************************************/
	inline int32 getTargetNodeIndex() const { return m_data.targetNodeIdx; }

	/*!******************************************************************************
	\brief	Sets the specified node as the look-at target of the camera.
	\param	idx Node index of the desired target.
	********************************************************************************/
	inline void setTargetNodeIndex(int32 idx) { m_data.targetNodeIdx = idx; }


	/*!******************************************************************************
	\brief	Get the number of frames that this camera's animation supports.
	********************************************************************************/
	inline uint32	getNumFrames() const { return static_cast<uint32>(m_data.FOVs.size()); }

	/*!******************************************************************************
	\brief	Get the far clipping plane distance.
	********************************************************************************/
	inline float32 getFar()  const { return m_data.farClip; }

	/*!******************************************************************************
	\brief	Get the near clipping plane distance.
	********************************************************************************/
	inline void setFar(float32 farClip) { m_data.farClip = farClip; }

	/*!******************************************************************************
	\brief	Get near clip plan distance.
	********************************************************************************/
	inline float32 getNear() const  { return m_data.nearClip; }

	/*!******************************************************************************
	\brief	Set the near clipping plane distance.
	********************************************************************************/
	inline void setNear(float32 nearClip) { m_data.nearClip = nearClip; }


	/*!******************************************************************************
	\brief	Get field of view for a specific frame. Interpolates between frames. The
	        interpolation point is between \p frame and \p frame +1, with factor 
			\p interp. 		
	\param	frame The initial frame. Interpolation will be between this frame and the next.
	\param	interp Interpolation factor. If zero, frame = \p frame. If one frame = is \p frame + 1.	
	********************************************************************************/
	float32 getFOV(uint32 frame = 0, float32 interp = 0)  const;

	/*!******************************************************************************
	\brief	Set field of view (Radians)
	********************************************************************************/
	void setFOV(float32 fov);

	/*!******************************************************************************
	\brief	Set a field of view animation for a number of frame.
	\param	numFrames The number of frames to set the Fov to.
	\param	fovs An array of packed floats, which will be interpreted as 
	********************************************************************************/
	void setFOV(uint32 numFrames, const float32* fovs);

	/*!******************************************************************************
	\brief	Get a reference to the internal data of this object. Handle with care.
	*******************************************************************************/
	inline InternalData& getInternalData() { return m_data; }
private:
	InternalData m_data;
};
}
}