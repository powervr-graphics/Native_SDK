/*!*********************************************************************************************************************
\file         PVRAssets\Model\Camera.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations of methods of the Camera class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/Model/Camera.h"
namespace pvr {
namespace assets {
float32 Camera::getFOV(uint32 frame, float32 interp)  const
{
	if (getNumFrames())
	{
		if (getNumFrames() > 1)
		{
			assertion(frame < getNumFrames() - 1);
			const float32* fov = &m_data.FOVs[frame];
			return fov[0] + interp * (fov[1] - fov[0]);
		}
		else
		{
			return m_data.FOVs[0];
		}
	}
	else
	{
		return 0.7f;
	}
}

void Camera::setFOV(float32 fov)
{
	return setFOV(1, &fov);
}

void Camera::setFOV(uint32 frames, const float32* const fovs)
{
	m_data.FOVs.resize(frames);

	if (fovs == NULL)
	{
		m_data.FOVs.resize(0);
		return;
	}
	m_data.FOVs.assign(fovs, fovs + frames);
}
}
}
//!\endcond