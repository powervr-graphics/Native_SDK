/*!
\brief Implementations of methods of the Camera class.
\file PVRAssets/Model/Camera.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/Model/Camera.h"
namespace pvr {
namespace assets {
float Camera::getFOV(uint32_t frame, float interp)  const
{
	if (getNumFrames())
	{
		if (getNumFrames() > 1)
		{
			assertion(frame < getNumFrames() - 1);
			const float* fov = &_data.FOVs[frame];
			return fov[0] + interp * (fov[1] - fov[0]);
		}
		else
		{
			return _data.FOVs[0];
		}
	}
	else
	{
		return 0.7f;
	}
}

void Camera::setFOV(float fov)
{
	return setFOV(1, &fov);
}

void Camera::setFOV(uint32_t frames, const float* const fovs)
{
	_data.FOVs.resize(frames);

	if (fovs == NULL)
	{
		_data.FOVs.resize(0);
		return;
	}
	_data.FOVs.assign(fovs, fovs + frames);
}
}
}
//!\endcond