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
float32 Camera::getFOV(uint32 frame, float32 interp)  const
{
	if (getNumFrames())
	{
		if (getNumFrames() > 1)
		{
			assertion(frame < getNumFrames() - 1);
			const float32* fov = &_data.FOVs[frame];
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

void Camera::setFOV(float32 fov)
{
	return setFOV(1, &fov);
}

void Camera::setFOV(uint32 frames, const float32* const fovs)
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