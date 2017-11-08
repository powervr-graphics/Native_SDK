/*!
\brief Implementations of methods from the Model class.
\file PVRAssets/Model/Model.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/Model.h"
#include "PVRAssets/Model/Camera.h"
#include "PVRAssets/Model/Light.h"
#include "PVRAssets/Model/Mesh.h"
#include "PVRCore/Stream.h"

namespace pvr {
namespace assets {
void Model::allocCameras(uint32_t no)
{
	_data.cameras.resize(no);
}

void Model::allocLights(uint32_t no)
{
	_data.lights.resize(no);
}

void Model::allocMeshes(uint32_t no)
{
	_data.meshes.resize(no);
}

void Model::allocNodes(uint32_t no)
{
	_data.nodes.resize(no);
}

void Model::allocMeshNodes(uint32_t no)
{
	allocNodes(no);
	_data.numMeshNodes = no;
}


void Model::destroyCache()
{
	_cache.worldMatrixFrameN.clear();
	_cache.worldMatrixFrameZero.clear();
	_cache.cachedFrame.clear();
}

glm::mat4x4 Model::getBoneWorldMatrix(uint32_t skinNodeID, uint32_t boneID) const
{
#ifdef DEBUG
	glm::mat4 tst = _cache.worldMatrixFrameZero[boneID] * _cache.worldMatrixFrameZero[skinNodeID];
	assertion(tst[0][3] * tst[0][3] < 1e-15f && tst[1][3] * tst[1][3] < 1e-15f && tst[2][3] * tst[2][3] < 1e-15f,
	          "getBoneWorldMatrix has unsupported skew parameters");
#endif

	// Back transform bone from frame 0 position using the skin's transformation
	// TODO: Inverse is needlessly slow here, but glm's "affineInverse" is actually an ORTHOGONAL inverse, doesn't work at all with scales.
	// Must write an ACTUAL affineInverse.
	//PVR_ALIGNED glm::mat4x4 matrixTest = glm::affineInverse(_cache.worldMatrixFrameZero[boneID]) * _cache.worldMatrixFrameZero[skinNodeID];
	PVR_ALIGNED glm::mat4x4 matrix = glm::inverse(_cache.worldMatrixFrameZero[boneID]) * _cache.worldMatrixFrameZero[skinNodeID];
	return getWorldMatrix(boneID) * matrix;
}

glm::mat4x4 Model::getWorldMatrix(uint32_t id) const
{
#ifdef DEBUG
	++_cache.total;
	_cache.frameHitPerc = static_cast<float>(_cache.frameNCacheHit) / _cache.total;
	_cache.frameZeroHitPerc = static_cast<float>(_cache.frameZeroCacheHit) / _cache.total;
#endif

	// There is a dedicated cache for frame 0 data
	if (_data.currentFrame == 0)
	{
#ifdef DEBUG
		++_cache.frameZeroCacheHit;
#endif
		return _cache.worldMatrixFrameZero[id];
	}
	// Has this matrix been calculated & cached?
	if (_data.currentFrame  == _cache.cachedFrame[id])
	{
#ifdef DEBUG
		++_cache.frameNCacheHit;
#endif
		return _cache.worldMatrixFrameN[id];
	}
	// Calculate the matrix and cache it
	const Node& node = _data.nodes[id];
	int32_t parentID = _data.nodes[id].getParentID();
	if (parentID < 0)
	{
		_cache.worldMatrixFrameN[id] = node.getAnimation().getTransformationMatrix(_cache.frame, _cache.frameFraction);
	}
	else
	{
		internal::optimizedMat4 m1 = internal::optimizedMat4(getWorldMatrix(parentID));
		internal::optimizedMat4 m2 = internal::optimizedMat4(node.getAnimation().getTransformationMatrix(_cache.frame,
		                             _cache.frameFraction));

		_cache.worldMatrixFrameN[id] = internal::toMat4(m1 * m2);
	}
	_cache.cachedFrame[id] = _data.currentFrame;
	return _cache.worldMatrixFrameN[id];
}

glm::vec3 Model::getLightPosition(uint32_t lightNodeId) const
{
	return glm::vec3(getWorldMatrix(getNodeIdFromLightNodeId(0))[3]);
}


glm::mat4x4 Model::getWorldMatrixNoCache(uint32_t id) const
{
	const Node& node = _data.nodes[id];
	const PVR_ALIGNED glm::mat4x4& matrix = node.getAnimation().getTransformationMatrix(_cache.frame, _cache.frameFraction);
	int32_t parentID = node.getParentID();
	if (parentID < 0)
	{
		return matrix;
	}
	return getWorldMatrixNoCache(parentID) * matrix;
}

void Model::initCache()
{
#ifdef DEBUG
	_cache.total = 0;
#endif
	_cache.worldMatrixFrameZero.resize(_data.nodes.size());
	_cache.cachedFrame.resize(_data.nodes.size());
	_cache.worldMatrixFrameN.resize(_data.nodes.size());
	flushCache();
}

void Model::flushCache()
{
	setCurrentFrame(0);
	if (_cache.worldMatrixFrameZero.empty())
	{
		return;
	}
	for (uint32_t i = 0; i < _data.nodes.size(); ++i)
	{
		_cache.worldMatrixFrameZero[i] = getWorldMatrixNoCache(i);
	}
	// Set our caches to frame 0
	if (_cache.worldMatrixFrameN.empty() || _cache.cachedFrame.empty())
	{
		return;
	}
	memcpy(_cache.worldMatrixFrameN.data(), _cache.worldMatrixFrameZero.data(), _data.nodes.size() * sizeof(_cache.worldMatrixFrameN[0]));
	memset(_cache.cachedFrame.data(), 0, _data.nodes.size() * sizeof(_cache.cachedFrame[0]));
}

float Model::getCurrentFrame()
{
	return _data.currentFrame;
}

bool Model::setCurrentFrame(float frame)
{
	if (_data.numFrames)
	{
		//	Limit animation frames.
		//	Example: If there are 100 frames of animation, the highest frame
		//	number allowed is 98, since that will blend between frames 98 and
		//	99. (99 being of course the 100th frame.)
		if (frame >= static_cast<float>(_data.numFrames - 1))
		{
			Log(LogLevel::Error, "Model::setCurrentFrame out of bounds, set to frame %f out of %d", frame, _data.numFrames);
			assertion(0);
			return false;
		}
		_cache.frame = static_cast<uint32_t>(frame);
		_cache.frameFraction = frame - _cache.frame;
	}
	else
	{
		assertion(frame == 0);
		if (static_cast<uint32_t>(frame) != 0)
		{
			Log(LogLevel::Error, "Model::setCurrentFrame out of bounds, set to frame %f out of %d", frame, _data.numFrames);
			assertion(0);
			return false;
		}
		_cache.frame = 0;
		_cache.frameFraction = 0;
	}
	_data.currentFrame = frame;
	return true;
}

void Model::setUserData(uint32_t size, const char* const data)
{
	_data.userData.resize(data ? size : 0);
	if (data && size) { memcpy(_data.userData.data(), data, size); }
}

void Model::getCameraProperties(int32_t index, float& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up, float& nearClip, float& farClip) const
{
	if (static_cast<uint32_t>(index) >= _data.cameras.size())
	{
		Log(LogLevel::Error, "Model::getCameraProperties out of bounds [%d]", index);
		assertion(0);
		return;
	}
	nearClip = _data.cameras[index].getNear();
	farClip = _data.cameras[index].getFar();
	return getCameraProperties(index, fov, from, to, up);
}

void Model::getCameraProperties(int32_t index, float& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up) const
{
	if (static_cast<uint32_t>(index) >= _data.cameras.size())
	{
		assertion(0, "Model::getCameraProperties index out of range");
		Log(LogLevel::Error, "Model::getCameraProperties out of bounds [%d]", index);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(static_cast<uint32_t>(_data.numMeshNodes + _data.lights.size() + index));
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
		float angle = glm::dot(atCurrent, atTarget);
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
	fov = camera.getFOV(_cache.frame, _cache.frameFraction);
}

void Model::getLightDirection(int32_t lightNodeId, glm::vec3& direction) const
{
	if (static_cast<size_t>(lightNodeId) >= getNumLightNodes())
	{
		assertion(0, "Model::getLightDirection out of bounds");
		Log(LogLevel::Error, "Model::getLightDirection out of bounds [%d]", lightNodeId);
		assertion(0);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(_data.numMeshNodes + lightNodeId);
	const Light& light = getLight(lightNodeId);
	int32_t targetIndex = light.getTargetIdx();
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

void Model::getLightPosition(int32_t lightNodeId, glm::vec3& position) const
{
	if (static_cast<uint32_t>(lightNodeId) >= getNumLightNodes())
	{
		assertion(0, "Model::getLightPosition out of bounds");
		Log(LogLevel::Error, "Model::getLightPosition out of bounds [%d]", lightNodeId);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(_data.numMeshNodes + lightNodeId);
	position.x = matrix[3][0];
	position.y = matrix[3][1];
	position.z = matrix[3][2];
}

void Model::getLightPosition(int32_t lightNodeId, glm::vec4& position) const
{
	if (static_cast<uint32_t>(lightNodeId) >= _data.lights.size())
	{
		assertion(0, "Model::getLightPosition out of bounds");
		Log(LogLevel::Error, "Model::getLightPosition out of bounds [%d]", lightNodeId);
		assertion(0);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(_data.numMeshNodes + lightNodeId);
	position.x = matrix[3][0];
	position.y = matrix[3][1];
	position.z = matrix[3][2];
	position.w = 1.0f;
}
}
}
//!\endcond
