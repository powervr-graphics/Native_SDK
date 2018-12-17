/*!
\brief Contains an Animation class.
\file PVRAssets/model/Animation.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/math/MathUtils.h"

namespace pvr {
namespace assets {
/// <summary>Represents an Animation that can be applied to different objects.</summary>

struct KeyFrameData
{
	enum struct InterpolationType : uint32_t
	{
		Step = 0,
		Linear = 1,
		CubicSpline = 2,

	};

	std::vector<float> timeInSeconds;
	// At Most One of the following is present
	std::vector<glm::vec3> scale;
	std::vector<glm::quat> rotate;
	std::vector<glm::vec3> translation;
	std::vector<glm::mat4> mat4; // ONLY USED IN POD
	InterpolationType interpolation;
};

class AnimationData
{
public:
	/// <summary>Raw internal structure of the Animation.</summary>
	struct InternalData
	{
		// Since the size is shared by all of those items, we are eschewing the use of vectors in order to save the extra space of size and capacity.
		uint32_t flags; //!< Stores which animation arrays are stored

		// Indices: If you will have loads of repeated values
		std::vector<uint32_t> positionIndices; //!< Index to positions
		std::vector<uint32_t> rotationIndices; //!< Index to rotations
		std::vector<uint32_t> scaleIndices; //!< Index to scales
		std::vector<uint32_t> matrixIndices; //!< Index to matrices

		uint32_t numFrames; //!< The number of frames of animation

		std::string animationName;

		std::vector<float> timeInSeconds;

		// Key Frame animation
		std::vector<KeyFrameData> keyFrames;

		// total durration time of this animation
		float durationTime;

		/// <summary>Constructor. Initializing.</summary>
		InternalData() : flags(0), numFrames(0), durationTime(0.0f) {}
	};

public:
	AnimationData() : _cacheF1(0), _cacheF2(1) {}

	void setAnimationName(const char* animationName)
	{
		_data.animationName = animationName;
	}

	const std::string& getAnimationName() const
	{
		return _data.animationName;
	}

	size_t getNumKeyFrames() const
	{
		return _data.keyFrames.size();
	}

	void allocateKeyFrames(uint32_t keyFrames)
	{
		_data.keyFrames.resize(keyFrames);
	}

	KeyFrameData& getAnimationData(uint32_t index)
	{
		return _data.keyFrames[index];
	}

	float getTotalTimeInSec()
	{
		return _data.durationTime;
	}

	float getTotalTimeInMs()
	{
		return getTotalTimeInSec() * 1000.f;
	}

	/// <summary>Get the transformation matrix of specific frame and amount of interpolation.</summary>
	/// <param name="frame">The first frame for which the transformation matrix will be returned</param>
	/// <param name="interp">Interpolation value used between the frames</param>
	/// <returns>The transformation matrix for the point in time that is at point <paramref name="interp"/>between
	/// frame <paramref name="frame"/>and frame <paramref name="frame+1"/></returns>
	/// <remarks>If the animation consists of Transformation Matrices, they will NOT be interpolated as this would be
	/// a very expensive operation. Rather, the closest matrix will be returned. If the transformation consists of
	/// Scale/ translation vectors and Rotation quaternia, Scale and Translation will be Linear Interpolated, and
	/// Rotation will be SLERPed (Smooth Linear Interpolation) as normal.</remarks>
	glm::mat4x4 getTransformationMatrix(uint32_t frame = 0, float interp = 0) const;

	/// <summary>Get number of frames in this animation.</summary>
	/// <returns>The number of frames</returns>
	uint32_t getNumFrames() const;

	/// <summary>Get the flags data on this animation.</summary>
	/// <returns>The flags</returns>
	uint32_t getFlags() const;

	/// <summary>Get a pointer to the position data of this animation.</summary>
	/// <returns>Position data - null if indexed</returns>
	const float* getPositions() const;

	/// <summary>Get a pointer to the indices of the position data of this animation.</summary>
	/// <returns>Position indexes - null if not indexed</returns>
	const uint32_t* getPositionIndices() const;

	/// <summary>Get a pointer to the rotation data of this animation (normally quaternions).</summary>
	/// <returns>Rotation data - null if indexed</returns>
	const float* getRotations() const;

	/// <summary>Get a pointer to the indices of the rotation data of this animation.</summary>
	/// <returns>Rotation indexes - null if not indexed</returns>
	const uint32_t* getRotationIndices() const;

	/// <summary>Get a pointer to the Scale data of this animation.</summary>
	/// <returns>Scales - null if indexed</returns>
	const float* getScales() const;

	/// <summary>Get a pointer to the indices of the Scale data.</summary>
	/// <returns>Scale indexes - null if not indexed</returns>
	const uint32_t* getScaleIndices() const;

	/// <summary>Get a pointer to the transformation matrices of this animation.</summary>
	/// <returns>Matrices - null if indexed</returns>
	const float* getMatrices() const;

	/// <summary>Get a pointer to the indices of the transformation matrices of this animation.</summary>
	/// <returns>Matrix indexes - null if not indexed</returns>
	const uint32_t* getMatrixIndices() const;

	/// <summary>Set the position transformation data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The position data that will be copied. Must be packed floats, each successive 3 of which
	/// will be interpreted as x,y,z values.</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation
	/// </returns>
	void setPositions(uint32_t numFrames, const float* data,
		const uint32_t* indices = nullptr); // Expects an array of 3 floats

	/// <summary>Set the rotation transformation data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The rotation data that will be copied. Must be packed floats, each successive 4 of which
	/// will be interpreted as x,y,z,w quaternion values.</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation
	/// </returns>
	void setRotations(uint32_t numFrames, const float* const data,
		const uint32_t* const indices = nullptr); // Expects an array of 4 floats

	/// <summary>Set the scale transformation data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The rotation data that will be copied. Must be packed floats, each successive 3 of which
	/// will be interpreted as x,y,z scale factors</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation
	/// </returns>
	void setScales(uint32_t numFrames, const float* const data,
		const uint32_t* const indices = nullptr); // Expects an array of 7 floats

	/// <summary>Set the transformation matrices data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The Transformation Matrices data that will be copied. Must be packed floats, each
	/// successive 16 of which will be interpreted as a matrix packed in the usual way (Column-major, with each column
	/// of the matrix stored successively in</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation
	/// </returns>
	void setMatrices(uint32_t numFrames, const float* const data,
		const uint32_t* const indices = nullptr); // Expects an array of 16 floats

	/// <summary>Gets a direct, modifiable pointer to the data representation of this object. Advanced tasks only.
	/// </summary>
	/// <returns>A pointer to the internal structure of this object</returns>
	InternalData& getInternalData(); // If you know what you're doing

private:
	InternalData _data;
	// cache
	uint32_t _cacheF1, _cacheF2;
};

struct AnimationInstance
{
	struct KeyframeChannel
	{
		std::vector<void*> nodes;
		// keyframe (Scale/ Rotate/ Translate)
		uint32_t keyFrame;
		KeyframeChannel() : keyFrame(0) {}
	};

	// the animation data
	class AnimationData* animationData;
	std::vector<KeyframeChannel> keyframeChannels;

public:
	AnimationInstance() {}
	float getTotalTimeInMs() const
	{
		return animationData->getTotalTimeInMs();
	}
	float getTotalTimeInSec() const
	{
		return animationData->getTotalTimeInSec();
	}

	/// <summary> update animation </summary>
	void updateAnimation(float timeInMs);
};

} // namespace assets
} // namespace pvr
