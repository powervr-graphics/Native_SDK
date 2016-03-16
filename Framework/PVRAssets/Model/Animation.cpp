/*!*********************************************************************************************************************
\file         PVRAssets\Model\Animation.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations of methods of the Animation class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/Model/Animation.h"
#include "PVRCore/Maths.h"

namespace pvr {
namespace assets {
glm::mat4x4 Animation::getTranslationMatrix(uint32 frame, float32 interp) const
{
	if (m_data.positions.size())
	{
		if (m_data.flags & Animation::HasPositionAnimation)
		{
			assertion(frame + 1 < m_data.numberOfFrames);
			uint32 index0, index1;
			if (m_data.positionIndices.size())
			{
				index0 = m_data.positionIndices[frame + 0];
				index1 = m_data.positionIndices[frame + 1];
			}
			else
			{
				index0 = 3 * (frame + 0);
				index1 = 3 * (frame + 1);
			}
			glm::vec3 p0(glm::make_vec3(&m_data.positions[index0]));
			glm::vec3 p1(glm::make_vec3(&m_data.positions[index1]));
			return glm::translate(glm::mix(p0, p1, interp));
		}
		else
		{
			return glm::translate(glm::vec3(m_data.positions[0], m_data.positions[1], m_data.positions[2]));
		}
	}
	return glm::mat4x4(1.0f);
}

glm::mat4x4 Animation::getRotationMatrix(uint32 frame, float32 interp) const
{
	if (m_data.rotations.size())
	{
		if (m_data.flags & Animation::HasRotationAnimation)
		{
			assertion(frame + 1 < m_data.numberOfFrames);
			uint32 index0, index1;
			if (m_data.rotationIndices.size())
			{
				index0 = m_data.rotationIndices[frame + 0];
				index1 = m_data.rotationIndices[frame + 1];
			}
			else
			{
				index0 = 4 * (frame + 0);
				index1 = 4 * (frame + 1);
			}
			glm::quat q0(glm::make_quat(&m_data.rotations[index0]));
			glm::quat q1(glm::make_quat(&m_data.rotations[index1]));
			glm::quat q = glm::slerp(q0, q1, interp);
			q.w = -q.w;
			return glm::mat4_cast(q);
		}
		else
		{
			glm::quat q(-m_data.rotations[3], m_data.rotations[0], m_data.rotations[1], m_data.rotations[2]);
			return glm::mat4_cast(q);
		}
	}
	return glm::mat4x4();
}

glm::mat4x4 Animation::getScalingMatrix(uint32 frame, float32 interp) const
{
	if (m_data.scales.size())
	{
		if (m_data.flags & Animation::HasScaleAnimation)
		{
			assertion(frame + 1 < m_data.numberOfFrames);
			uint32 index0, index1;
			if (m_data.scaleIndices.size())
			{
				index0 = m_data.scaleIndices[frame + 0];
				index1 = m_data.scaleIndices[frame + 1];
			}
			else
			{
				index0 = 7 * (frame + 0);
				index1 = 7 * (frame + 1);
			}
			glm::vec3 s0(glm::make_vec3(&m_data.scales[index0]));
			glm::vec3 s1(glm::make_vec3(&m_data.scales[index1]));
			return glm::scale(glm::mix(s0, s1, interp));
		}
		else
		{
			return glm::scale(glm::vec3(m_data.scales[0], m_data.scales[1], m_data.scales[2]));
		}
	}
	return glm::mat4x4();
}

glm::mat4x4 Animation::getTransformationMatrix(uint32 frame, float32 interp) const
{
	if (m_data.matrices.size())
	{
		if (m_data.flags & Animation::HasMatrixAnimation)
		{
			assertion(frame < m_data.numberOfFrames);
			uint32 index;
			if (m_data.matrixIndices.size())
			{
				index = m_data.matrixIndices[frame];
			}
			else
			{
				index = frame * 16;
			}
			return glm::mat4x4(m_data.matrices[index + 0], m_data.matrices[index + 1], m_data.matrices[index + 2], m_data.matrices[index + 3],
			                   m_data.matrices[index + 4], m_data.matrices[index + 5], m_data.matrices[index + 6], m_data.matrices[index + 7],
			                   m_data.matrices[index + 8], m_data.matrices[index + 9], m_data.matrices[index + 10], m_data.matrices[index + 11],
			                   m_data.matrices[index + 12], m_data.matrices[index + 13], m_data.matrices[index + 14], m_data.matrices[index + 15]);
		}
		else if ((m_data.flags & Animation::HasPositionAnimation) && (m_data.flags & Animation::HasScaleAnimation)
		         && (m_data.flags & Animation::HasRotationAnimation))
		{
			internal::optimizedMat4 m1(getTranslationMatrix(frame, interp));
			internal::optimizedMat4 m2(getRotationMatrix(frame, interp));
			internal::optimizedMat4 m3(getScalingMatrix(frame, interp));
			internal::optimizedMat4 m4(m1 * m2 * m3);

			return internal::toMat4(m4);
		}
		else
		{
			return glm::mat4x4(m_data.matrices[0], m_data.matrices[1], m_data.matrices[2], m_data.matrices[3],
			                   m_data.matrices[4], m_data.matrices[5], m_data.matrices[6], m_data.matrices[7],
			                   m_data.matrices[8], m_data.matrices[9], m_data.matrices[10], m_data.matrices[11],
			                   m_data.matrices[12], m_data.matrices[13], m_data.matrices[14], m_data.matrices[15]);

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

bool Animation::setPositions(uint32 numFrames, const float32* const data, const uint32* const indices)
{
	m_data.positions.resize(0);
	m_data.positionIndices.resize(0);
	m_data.flags |= ~HasPositionAnimation;

	if (numFrames > 1 && m_data.flags && numFrames != m_data.numberOfFrames)
	{
		// There is a mismatch in the number of frames between pos/rot/scale/matrix
		pvr::Log(Log.Error, "Wrongly sized data passed to Animation::setPositions. The data must correspond to the number of frames.");
		assertion(0 ,  "Wrongly sized data passed to Animation::setPositions. The data must correspond to the number of frames.");
		return false;
	}
	if (!data || !numFrames)
	{
		pvr::Log(Log.Warning, "Zero sized data passed to Animation::setPositions. No effect.");
		return false;
	}
	uint32 dataSize = 0;
	if (indices)
	{
		for (uint32 i = 0; i < numFrames; ++i)
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
	m_data.positions.resize(dataSize);
	memcpy(m_data.positions.data(), data, dataSize * sizeof(*data));
	if (indices)
	{
		m_data.positionIndices.resize(numFrames);
		memcpy(m_data.positionIndices.data(), indices, numFrames * sizeof(*indices));
	}
	if (numFrames > 1)
	{
		m_data.flags |= HasPositionAnimation;
	}
	return true;
}

bool Animation::setRotations(uint32 numFrames, const float32* const data, const uint32* const indices)
{
	m_data.rotations.resize(0);
	m_data.rotationIndices.resize(0);
	m_data.flags |= ~HasRotationAnimation;
	if (numFrames > 1 && m_data.flags && numFrames != m_data.numberOfFrames)
	{
		// There is a mismatch in the number of frames between pos/rot/scale/matrix
		pvr::Log(Log.Error, "Wrongly sized data passed to Animation::setRotations. The data must correspond to the number of frames.");
		assertion(0 ,  "Wrongly sized data passed to Animation::setRotations. The data must correspond to the number of frames.");
		return false;
	}
	if (!data || !numFrames)
	{
		pvr::Log(Log.Warning, "Zero sized data passed to Animation::setRotations. No effect.");
		return false;
	}
	uint32 dataSize = 0;
	if (indices)
	{
		for (uint32 i = 0; i < numFrames; ++i)
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
	m_data.rotations.resize(dataSize);
	memcpy(m_data.rotations.data(), data, dataSize * sizeof(*data));
	if (indices)
	{
		m_data.rotationIndices.resize(numFrames);
		memcpy(m_data.rotationIndices.data(), indices, numFrames * sizeof(*indices));
	}
	if (numFrames > 1)
	{
		m_data.flags |= HasRotationAnimation;
	}
	return true;
}

bool Animation::setScales(uint32 numFrames, const float32* const data, const uint32* const indices)
{
	m_data.scales.resize(0);
	m_data.scaleIndices.resize(0);
	m_data.flags |= ~HasScaleAnimation;
	if (numFrames > 1 && m_data.flags && numFrames != m_data.numberOfFrames)
	{
		// There is a mismatch in the number of frames between pos/rot/scale/matrix
		pvr::Log(Log.Error, "Wrongly sized data passed to Animation::setScales. The data must correspond to the number of frames.");
		assertion(0 ,  "Wrongly sized data passed to Animation::setScales. The data must correspond to the number of frames.");
		return false;
	}
	if (!data || !numFrames)
	{
		pvr::Log(Log.Warning, "Zero sized data passed to Animation::setScales. No effect.");
		return false;
	}
	uint32 dataSize = 0;
	if (indices)
	{
		for (uint32 i = 0; i < numFrames; ++i)
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
	m_data.scales.resize(dataSize);
	memcpy(m_data.scales.data(), data, dataSize * sizeof(*data));
	if (indices)
	{
		m_data.scaleIndices.resize(numFrames);
		memcpy(m_data.scaleIndices.data(), indices, numFrames * sizeof(*indices));
	}
	if (numFrames > 1)
	{
		m_data.flags |= HasScaleAnimation;
	}
	return true;
}

bool Animation::setMatrices(uint32 numFrames, const float32* const data, const uint32* const indices)
{
	m_data.matrices.resize(0);
	m_data.matrixIndices.resize(0);
	m_data.flags |= ~HasMatrixAnimation;
	if (numFrames > 1 && m_data.flags && numFrames != m_data.numberOfFrames)
	{
		// There is a mismatch in the number of frames between pos/rot/scale/matrix
		pvr::Log(Log.Error, "Wrongly sized datapassed to Animation::setMatrices. The data must correspond to the number of frames.");
		assertion(0 ,  "Wrongly sized datapassed to Animation::setMatrices. The data must correspond to the number of frames.");
		return false;
	}
	if (!data || !numFrames)
	{
		pvr::Log(Log.Warning, "Zero sized data passed to Animation::setMatrices. No effect.");
		return false;
	}
	uint32 dataSize = 0;
	if (indices)
	{
		for (uint32 i = 0; i < numFrames; ++i)
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
	m_data.matrices.resize(dataSize);
	memcpy(m_data.matrices.data(), data, dataSize * sizeof(*data));
	if (indices)
	{
		m_data.matrixIndices.resize(numFrames);
		memcpy(m_data.matrixIndices.data(), indices, numFrames * sizeof(*indices));
	}
	if (numFrames > 1)
	{
		m_data.flags |= HasMatrixAnimation;
	}
	return true;
}

uint32 Animation::getNumFrames() const
{
	return m_data.numberOfFrames;
}

const float32* Animation::getPositions() const
{
	return m_data.positions.data();
}

const uint32* Animation::getPositionIndices() const
{
	return m_data.positionIndices.data();
}

const float32* Animation::getRotations() const
{
	return m_data.rotations.data();
}

const uint32* Animation::getRotationIndices() const
{
	return m_data.rotationIndices.data();
}

const float32* Animation::getScales() const
{
	return m_data.scales.data();
}

const uint32* Animation::getScaleIndices() const
{
	return m_data.scaleIndices.data();
}

const float32* Animation::getMatrices() const
{
	return m_data.matrices.data();
}

const uint32* Animation::getMatrixIndices() const
{
	return m_data.matrixIndices.data();
}

uint32 Animation::getFlags() const
{
	return m_data.flags;
}

Animation::InternalData& Animation::getInternalData()
{
	return m_data;
}
}
}
//!\endcond