/*!*********************************************************************************************************************
\file         PVRAssets\Model\Model.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations of methods from the Model class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRAssets/Model.h"
#include "PVRAssets/Model/Camera.h"
#include "PVRAssets/Model/Light.h"
#include "PVRAssets/Model/Mesh.h"
#include "PVRCore/Stream.h"

namespace pvr {
namespace assets {



void Model::allocCameras(uint32 no)
{
	m_data.cameras.resize(no);
}

void Model::allocLights(uint32 no)
{
	m_data.lights.resize(no);
}

void Model::allocMeshes(uint32 no)
{
	m_data.meshes.resize(no);
}

void Model::allocNodes(uint32 no)
{
	m_data.nodes.resize(no);
}

void Model::destroyCache()
{
	m_cache.worldMatrixFrameN.clear();
	m_cache.worldMatrixFrameZero.clear();
	m_cache.cachedFrame.clear();
}

glm::mat4x4 Model::getBoneWorldMatrix(uint32 skinNodeID, uint32 boneID) const
{
#ifdef DEBUG
	glm::mat4 tst = m_cache.worldMatrixFrameZero[boneID] * m_cache.worldMatrixFrameZero[skinNodeID];
	assertion(tst[0][3] * tst[0][3] < 1e-15f && tst[1][3] * tst[1][3] < 1e-15f && tst[2][3] * tst[2][3] < 1e-15f, "getBoneWorldMatrix has unsupported skew parameters");
#endif

	// Back transform bone from frame 0 position using the skin's transformation
	// TODO: Inverse is needlessly slow here, but glm's "affineInverse" is actually an ORTHOGONAL inverse, doesn't work at all with scales.
	// Must write an ACTUAL affineInverse.
	//PVR_ALIGNED glm::mat4x4 matrixTest = glm::affineInverse(m_cache.worldMatrixFrameZero[boneID]) * m_cache.worldMatrixFrameZero[skinNodeID];
	PVR_ALIGNED glm::mat4x4 matrix = glm::inverse(m_cache.worldMatrixFrameZero[boneID]) * m_cache.worldMatrixFrameZero[skinNodeID];
	return getWorldMatrix(boneID) * matrix;
}

glm::mat4x4 Model::getWorldMatrix(uint32 id) const
{
#ifdef DEBUG
	++m_cache.total;
	m_cache.frameHitPerc = static_cast<float>(m_cache.frameNCacheHit) / m_cache.total;
	m_cache.frameZeroHitPerc = static_cast<float>(m_cache.frameZeroCacheHit) / m_cache.total;
#endif

	// There is a dedicated cache for frame 0 data
	if (m_data.currentFrame == 0)
	{
#ifdef DEBUG
		++m_cache.frameZeroCacheHit;
#endif
		return m_cache.worldMatrixFrameZero[id];
	}
	// Has this matrix been calculated & cached?
	if (m_cache.frame == m_cache.cachedFrame[id])
	{
#ifdef DEBUG
		++m_cache.frameNCacheHit;
#endif
		return m_cache.worldMatrixFrameN[id];
	}
	// Calculate the matrix and cache it
	const Node& node = m_data.nodes[id];
	int32 parentID = m_data.nodes[id].getParentID();
	if (parentID < 0)
	{
		m_cache.worldMatrixFrameN[id] = node.getAnimation().getTransformationMatrix(m_cache.frame, m_cache.frameFraction);
	}
	else
	{
		internal::optimizedMat4 m1 = internal::optimizedMat4(getWorldMatrix(parentID));
		internal::optimizedMat4 m2 = internal::optimizedMat4(node.getAnimation().getTransformationMatrix(m_cache.frame,
		                             m_cache.frameFraction));

		m_cache.worldMatrixFrameN[id] = internal::toMat4(m1 * m2);
	}
	m_cache.cachedFrame[id] = m_data.currentFrame;
	return m_cache.worldMatrixFrameN[id];
}

glm::vec3 Model::getLightPosition(uint32 lightNodeId) const
{
	return glm::vec3(getWorldMatrix(getNodeIdFromLightNodeId(0))[3]);
}


glm::mat4x4 Model::getWorldMatrixNoCache(uint32 id) const
{
	const Node& node = m_data.nodes[id];
	const PVR_ALIGNED glm::mat4x4& matrix = node.getAnimation().getTransformationMatrix(m_cache.frame, m_cache.frameFraction);
	int32 parentID = node.getParentID();
	if (parentID < 0)
	{
		return matrix;
	}
	//return internal::toMat4(internal::optimizedMat4(getWorldMatrixNoCache(parentID)) * internal::optimizedMat4(matrix));
	return getWorldMatrixNoCache(parentID) * matrix;
}

void Model::initCache()
{
#ifdef DEBUG
	m_cache.total = 0;
#endif
	m_cache.worldMatrixFrameZero.resize(m_data.nodes.size());
	m_cache.cachedFrame.resize(m_data.nodes.size());
	m_cache.worldMatrixFrameN.resize(m_data.nodes.size());
	flushCache();
}

void Model::flushCache()
{
	setCurrentFrame(0);
	if (m_cache.worldMatrixFrameZero.empty())
	{
		return;
	}
	for (uint32 i = 0; i < m_data.nodes.size(); ++i)
	{
		m_cache.worldMatrixFrameZero[i] = getWorldMatrixNoCache(i);
	}
	// Set our caches to frame 0
	if (m_cache.worldMatrixFrameN.empty() || m_cache.cachedFrame.empty())
	{
		return;
	}
	memcpy(m_cache.worldMatrixFrameN.data(), m_cache.worldMatrixFrameZero.data(), m_data.nodes.size() * sizeof(m_cache.worldMatrixFrameN[0]));
	memset(m_cache.cachedFrame.data(), 0, m_data.nodes.size() * sizeof(m_cache.cachedFrame[0]));
}

float32 Model::getCurrentFrame()
{
	return m_data.currentFrame;
}

bool Model::setCurrentFrame(float32 frame)
{
	if (m_data.numFrames)
	{
		//	Limit animation frames.
		//	Example: If there are 100 frames of animation, the highest frame
		//	number allowed is 98, since that will blend between frames 98 and
		//	99. (99 being of course the 100th frame.)
		if (frame > static_cast<float32>(m_data.numFrames - 1))
		{
			Log(Log.Error, "Model::setCurrentFrame out of bounds, set to frame %f out of %d", frame, m_data.numFrames);
			assertion(0);
			return false;
		}
		m_cache.frame = static_cast<uint32>(frame);
		m_cache.frameFraction = frame - m_cache.frame;
	}
	else
	{
		assertion(frame == 0);
		if (static_cast<uint32>(frame) != 0)
		{
			Log(Log.Error, "Model::setCurrentFrame out of bounds, set to frame %f out of %d", frame, m_data.numFrames);
			assertion(0);
			return false;
		}
		m_cache.frame = 0;
		m_cache.frameFraction = 0;
	}
	m_data.currentFrame = frame;
	return true;
}

void Model::setUserData(uint32 size, const byte* const data)
{
	m_data.userData.resize(data ? size : 0);
	if (data && size) { memcpy(m_data.userData.data(), data, size); }
}

void Model::getCameraProperties(int32 index, float32& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up, float& nearClip, float& farClip) const
{
	if (static_cast<uint32>(index) >= m_data.cameras.size())
	{
		Log(Log.Error, "Model::getCameraProperties out of bounds [%d]", index);
		assertion(0);
		return;
	}
	nearClip = m_data.cameras[index].getNear();
	farClip = m_data.cameras[index].getFar();
	return getCameraProperties(index, fov, from, to, up);
}

void Model::getCameraProperties(int32 index, float32& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up) const
{
	if (static_cast<uint32>(index) >= m_data.cameras.size())
	{
		assertion(0, "Model::getCameraProperties index out of range");
		Log(Log.Error, "Model::getCameraProperties out of bounds [%d]", index);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(static_cast<uint32>(m_data.numMeshNodes + m_data.lights.size() + index));
	// View position is 0,0,0,1 transformed by world matrix
	from.x = matrix[3][0];
	from.y = matrix[3][1];
	from.z = matrix[3][2];
	// When you rotate the camera from "straight forward" to "straight down", in openGL the UP vector will be [0, 0, -1]
	up.x = -matrix[2][0];
	up.y = -matrix[2][1];
	up.z = -matrix[2][2];
	up = glm::normalize(up);
	const Camera& camera = getCamera(index);

	// TODO: Check the below code as it is experimental but should allow us to calculate the up vector if this camera follows a target
	if (camera.getTargetNodeIndex() != -1)
	{
		glm::vec3 atCurrent, atTarget;
		glm::mat4x4 targetMatrix = getWorldMatrix(camera.getTargetNodeIndex());
		to.x = targetMatrix[3][0];
		to.y = targetMatrix[3][1];
		to.z = targetMatrix[3][2];
		// Rotate our up vector
		atTarget = to - from;
		glm::normalize(atTarget);
		atCurrent = to - from;
		glm::normalize(atCurrent);
		glm::vec3 axis = glm::cross(atCurrent, atTarget);
		float32 angle = glm::dot(atCurrent, atTarget);
		glm::quat q = glm::angleAxis(angle, axis);
		up = glm::mat3_cast(q) * up;
		glm::normalize(up);
	}
	else
	{
		// View direction is 0,-1,0,1 transformed by world matrix
		to.x = -matrix[1][0] + from.x;
		to.y = -matrix[1][1] + from.y;
		to.z = -matrix[1][2] + from.z;
	}
	fov = camera.getFOV(m_cache.frame, m_cache.frameFraction);
}

void Model::getLightDirection(int32 lightNodeId, glm::vec3& direction) const
{
	if (static_cast<size_t>(lightNodeId) >= getNumLightNodes())
	{
		assertion(0, "Model::getLightDirection out of bounds");
		Log(Log.Error, "Model::getLightDirection out of bounds [%d]", lightNodeId);
		assertion(0);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(m_data.numMeshNodes + lightNodeId);
	const Light& light = getLight(lightNodeId);
	int32 targetIndex = light.getTargetIdx();
	if (targetIndex != -1)
	{
		glm::mat4x4 targetMatrix = getWorldMatrix(targetIndex);
		direction.x = targetMatrix[3][0] - matrix[3][0];
		direction.y = targetMatrix[3][1] - matrix[3][1];
		direction.z = targetMatrix[3][2] - matrix[3][2];
		glm::normalize(direction);
	}
	else
	{
		direction.x = -matrix[1][0];
		direction.y = -matrix[1][1];
		direction.z = -matrix[1][2];
	}
}

void Model::getLightPosition(int32 lightNodeId, glm::vec3& position) const
{
	if (static_cast<uint32>(lightNodeId) >= getNumLightNodes())
	{
		assertion(0, "Model::getLightPosition out of bounds");
		Log(Log.Error, "Model::getLightPosition out of bounds [%d]", lightNodeId);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(m_data.numMeshNodes + lightNodeId);
	position.x = matrix[3][0];
	position.y = matrix[3][1];
	position.z = matrix[3][2];
}

void Model::getLightPosition(int32 lightNodeId, glm::vec4& position) const
{
	if (static_cast<uint32>(lightNodeId) >= m_data.lights.size())
	{
		assertion(0, "Model::getLightPosition out of bounds");
		Log(Log.Error, "Model::getLightPosition out of bounds [%d]", lightNodeId);
		assertion(0);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(m_data.numMeshNodes + lightNodeId);
	position.x = matrix[3][0];
	position.y = matrix[3][1];
	position.z = matrix[3][2];
	position.w = 1.0f;
}
}
}
//!\endcond