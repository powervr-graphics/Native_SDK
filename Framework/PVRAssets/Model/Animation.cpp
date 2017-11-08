/*!
\brief Implementations of methods of the Animation class.
\file PVRAssets/Model/Animation.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/Model/Animation.h"
#include "PVRCore/Maths.h"

namespace pvr {
namespace assets {
glm::mat4x4 Animation::getTranslationMatrix(uint32_t frame, float interp) const
{
	if (_data.positions.size())
	{
		if (_data.flags & Animation::HasPositionAnimation)
		{
			assertion(frame + 1 < _data.numFrames);
			uint32_t index0, index1;
			if (_data.positionIndices.size())
			{
				index0 = _data.positionIndices[frame + 0];
				index1 = _data.positionIndices[frame + 1];
			}
			else
			{
				index0 = 3 * (frame + 0);
				index1 = 3 * (frame + 1);
			}
			glm::vec3 p0(glm::make_vec3(&_data.positions[index0]));
			glm::vec3 p1(glm::make_vec3(&_data.positions[index1]));
			return glm::translate(glm::mix(p0, p1, interp));
		}
		else
		{
			return glm::translate(glm::vec3(_data.positions[0], _data.positions[1], _data.positions[2]));
		}
	}
	return glm::mat4x4(1.0f);
}

glm::mat4x4 Animation::getRotationMatrix(uint32_t frame, float interp) const
{
	if (_data.rotations.size())
	{
		if (_data.flags & Animation::HasRotationAnimation)
		{
			assertion(frame + 1 < _data.numFrames);
			uint32_t index0, index1;
			if (_data.rotationIndices.size())
			{
				index0 = _data.rotationIndices[frame + 0];
				index1 = _data.rotationIndices[frame + 1];
			}
			else
			{
				index0 = 4 * (frame + 0);
				index1 = 4 * (frame + 1);
			}
			glm::quat q0(glm::make_quat(&_data.rotations[index0]));
			glm::quat q1(glm::make_quat(&_data.rotations[index1]));
			glm::quat q = glm::slerp(q0, q1, interp);
			q.w = -q.w;
			return glm::mat4_cast(q);
		}
		else
		{
			glm::quat q(-_data.rotations[3], _data.rotations[0], _data.rotations[1], _data.rotations[2]);
			return glm::mat4_cast(q);
		}
	}
	return glm::mat4x4();
}

glm::mat4x4 Animation::getScalingMatrix(uint32_t frame, float interp) const
{
	if (_data.scales.size())
	{
		if (_data.flags & Animation::HasScaleAnimation)
		{
			assertion(frame + 1 < _data.numFrames);
			uint32_t index0, index1;
			if (_data.scaleIndices.size())
			{
				index0 = _data.scaleIndices[frame + 0];
				index1 = _data.scaleIndices[frame + 1];
			}
			else
			{
				index0 = 7 * (frame + 0);
				index1 = 7 * (frame + 1);
			}
			glm::vec3 s0(glm::make_vec3(&_data.scales[index0]));
			glm::vec3 s1(glm::make_vec3(&_data.scales[index1]));
			return glm::scale(glm::mix(s0, s1, interp));
		}
		else
		{
			return glm::scale(glm::vec3(_data.scales[0], _data.scales[1], _data.scales[2]));
		}
	}
	return glm::mat4x4();
}

glm::mat4x4 Animation::getTransformationMatrix(uint32_t frame, float interp) const
{
	if (_data.matrices.size())
	{
		if (_data.flags & Animation::HasMatrixAnimation)
		{
			assertion(frame + 1 < _data.numFrames);
			uint32_t index;
			if (_data.matrixIndices.size())
			{
				index = _data.matrixIndices[frame];
			}
			else
			{
				index = frame * 16;
			}
			return glm::mat4x4(_data.matrices[index + 0], _data.matrices[index + 1], _data.matrices[index + 2], _data.matrices[index + 3],
			                   _data.matrices[index + 4], _data.matrices[index + 5], _data.matrices[index + 6], _data.matrices[index + 7],
			                   _data.matrices[index + 8], _data.matrices[index + 9], _data.matrices[index + 10], _data.matrices[index + 11],
			                   _data.matrices[index + 12], _data.matrices[index + 13], _data.matrices[index + 14], _data.matrices[index + 15]);
		}
		else if ((_data.flags & Animation::HasPositionAnimation) && (_data.flags & Animation::HasScaleAnimation)
		         && (_data.flags & Animation::HasRotationAnimation))
		{
			internal::optimizedMat4 m1(getTranslationMatrix(frame, interp));
			internal::optimizedMat4 m2(getRotationMatrix(frame, interp));
			internal::optimizedMat4 m3(getScalingMatrix(frame, interp));
			internal::optimizedMat4 m4(m1 * m2 * m3);

			return internal::toMat4(m4);
		}
		else
		{
			return glm::mat4x4(_data.matrices[0], _data.matrices[1], _data.matrices[2], _data.matrices[3],
			                   _data.matrices[4], _data.matrices[5], _data.matrices[6], _data.matrices[7],
			                   _data.matrices[8], _data.matrices[9], _data.matrices[10], _data.matrices[11],
			                   _data.matrices[12], _data.matrices[13], _data.matrices[14], _data.matrices[15]);

		}
	}
	else
	{
		internal::optimizedMat4 m1(getTranslationMatrix(frame, interp));
		internal::optimizedMat4 m2(getRotationMatrix(frame, interp));
		internal::optimizedMat4 m3(getScalingMatrix(frame, interp));
		internal::optimizedMat4 m4(m1 * m2 * m3);

		PVR_ALIGNED glm::mat4 m = internal::toMat4(m4);

		return m;
	}
}

bool Animation::setPositions(uint32_t numFrames, const float* const data, const uint32_t* const indices)
{
	_data.positions.resize(0);
	_data.positionIndices.resize(0);
	_data.flags |= ~HasPositionAnimation;

	if (numFrames > 1 && _data.flags && numFrames != _data.numFrames)
	{
		// There is a mismatch in the number of frames between pos/rot/scale/matrix
		Log(LogLevel::Error, "Wrongly sized data passed to Animation::setPositions. The data must correspond to the number of frames.");
		assertion(0, "Wrongly sized data passed to Animation::setPositions. The data must correspond to the number of frames.");
		return false;
	}
	if (!data || !numFrames)
	{
		Log(LogLevel::Warning, "Zero sized data passed to Animation::setPositions. No effect.");
		return false;
	}
	uint32_t dataSize = 0;
	if (indices)
	{
		for (uint32_t i = 0; i < numFrames; ++i)
		{
			if (indices[i] > dataSize)
			{
				dataSize = indices[i];
			}
		}
		dataSize *= 3;
	}
	else
	{
		dataSize = numFrames * 3;
	}
	_data.positions.resize(dataSize);
	memcpy(_data.positions.data(), data, dataSize * sizeof(*data));
	if (indices)
	{
		_data.positionIndices.resize(numFrames);
		memcpy(_data.positionIndices.data(), indices, numFrames * sizeof(*indices));
	}
	if (numFrames > 1)
	{
		_data.flags |= HasPositionAnimation;
	}
	return true;
}

bool Animation::setRotations(uint32_t numFrames, const float* const data, const uint32_t* const indices)
{
	_data.rotations.resize(0);
	_data.rotationIndices.resize(0);
	_data.flags |= ~HasRotationAnimation;
	if (numFrames > 1 && _data.flags && numFrames != _data.numFrames)
	{
		// There is a mismatch in the number of frames between pos/rot/scale/matrix
		Log(LogLevel::Error, "Wrongly sized data passed to Animation::setRotations. The data must correspond to the number of frames.");
		assertion(0, "Wrongly sized data passed to Animation::setRotations. The data must correspond to the number of frames.");
		return false;
	}
	if (!data || !numFrames)
	{
		Log(LogLevel::Warning, "Zero sized data passed to Animation::setRotations. No effect.");
		return false;
	}
	uint32_t dataSize = 0;
	if (indices)
	{
		for (uint32_t i = 0; i < numFrames; ++i)
		{
			if (indices[i] > dataSize)
			{
				dataSize = indices[i];
			}
		}
		dataSize *= 4;
	}
	else
	{
		dataSize = numFrames * 4;
	}
	_data.rotations.resize(dataSize);
	memcpy(_data.rotations.data(), data, dataSize * sizeof(*data));
	if (indices)
	{
		_data.rotationIndices.resize(numFrames);
		memcpy(_data.rotationIndices.data(), indices, numFrames * sizeof(*indices));
	}
	if (numFrames > 1)
	{
		_data.flags |= HasRotationAnimation;
	}
	return true;
}

bool Animation::setScales(uint32_t numFrames, const float* const data, const uint32_t* const indices)
{
	_data.scales.resize(0);
	_data.scaleIndices.resize(0);
	_data.flags |= ~HasScaleAnimation;
	if (numFrames > 1 && _data.flags && numFrames != _data.numFrames)
	{
		// There is a mismatch in the number of frames between pos/rot/scale/matrix
		Log(LogLevel::Error, "Wrongly sized data passed to Animation::setScales. The data must correspond to the number of frames.");
		assertion(0, "Wrongly sized data passed to Animation::setScales. The data must correspond to the number of frames.");
		return false;
	}
	if (!data || !numFrames)
	{
		Log(LogLevel::Warning, "Zero sized data passed to Animation::setScales. No effect.");
		return false;
	}
	uint32_t dataSize = 0;
	if (indices)
	{
		for (uint32_t i = 0; i < numFrames; ++i)
		{
			if (indices[i] > dataSize)
			{
				dataSize = indices[i];
			}
		}
		dataSize *= 7;
	}
	else
	{
		dataSize = numFrames * 7;
	}
	_data.scales.resize(dataSize);
	memcpy(_data.scales.data(), data, dataSize * sizeof(*data));
	if (indices)
	{
		_data.scaleIndices.resize(numFrames);
		memcpy(_data.scaleIndices.data(), indices, numFrames * sizeof(*indices));
	}
	if (numFrames > 1)
	{
		_data.flags |= HasScaleAnimation;
	}
	return true;
}

bool Animation::setMatrices(uint32_t numFrames, const float* const data, const uint32_t* const indices)
{
	_data.matrices.resize(0);
	_data.matrixIndices.resize(0);
	_data.flags |= ~HasMatrixAnimation;
	if (numFrames > 1 && _data.flags && numFrames != _data.numFrames)
	{
		// There is a mismatch in the number of frames between pos/rot/scale/matrix
		Log(LogLevel::Error, "Wrongly sized datapassed to Animation::setMatrices. The data must correspond to the number of frames.");
		assertion(0, "Wrongly sized datapassed to Animation::setMatrices. The data must correspond to the number of frames.");
		return false;
	}
	if (!data || !numFrames)
	{
		Log(LogLevel::Warning, "Zero sized data passed to Animation::setMatrices. No effect.");
		return false;
	}
	uint32_t dataSize = 0;
	if (indices)
	{
		for (uint32_t i = 0; i < numFrames; ++i)
		{
			if (indices[i] > dataSize)
			{
				dataSize = indices[i];
			}
		}
		dataSize *= 16;
	}
	else
	{
		dataSize = numFrames * 16;
	}
	_data.matrices.resize(dataSize);
	memcpy(_data.matrices.data(), data, dataSize * sizeof(*data));
	if (indices)
	{
		_data.matrixIndices.resize(numFrames);
		memcpy(_data.matrixIndices.data(), indices, numFrames * sizeof(*indices));
	}
	if (numFrames > 1)
	{
		_data.flags |= HasMatrixAnimation;
	}
	return true;
}

uint32_t Animation::getNumFrames() const
{
	return _data.numFrames;
}

const float* Animation::getPositions() const
{
	return _data.positions.data();
}

const uint32_t* Animation::getPositionIndices() const
{
	return _data.positionIndices.data();
}

const float* Animation::getRotations() const
{
	return _data.rotations.data();
}

const uint32_t* Animation::getRotationIndices() const
{
	return _data.rotationIndices.data();
}

const float* Animation::getScales() const
{
	return _data.scales.data();
}

const uint32_t* Animation::getScaleIndices() const
{
	return _data.scaleIndices.data();
}

const float* Animation::getMatrices() const
{
	return _data.matrices.data();
}

const uint32_t* Animation::getMatrixIndices() const
{
	return _data.matrixIndices.data();
}

uint32_t Animation::getFlags() const
{
	return _data.flags;
}

Animation::InternalData& Animation::getInternalData()
{
	return _data;
}
}
}
//!\endcond