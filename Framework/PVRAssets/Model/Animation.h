/*!
\brief Contains an Animation class.
\file PVRAssets/Model/Animation.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace assets {
/// <summary>Represents an Animation that can be applied to different objects.</summary>
class Animation
{
public:
	enum Flags
	{
		HasPositionAnimation = 0x01, //!< position animation data
		HasRotationAnimation = 0x02,//!< rotation animation data
		HasScaleAnimation	 = 0x04,//!< scale animation data
		HasMatrixAnimation   = 0x08//!< matrix animation data
	};

	/// <summary>Raw internal structure of the Animation.</summary>
	struct InternalData
	{
		//Since the size is shared by all of those items, we are eschewing the use of vectors in order to save the extra space of size and capacity.
		uint32   flags;	/*!< Stores which animation arrays are stored */

		std::vector<float32> positions;	/*!< 3 floats per frame of animation. */
		std::vector<float32> rotations;	/*!< 4 floats per frame of animation. */
		std::vector<float32> scales;	/*!< 7 floats per frame of animation. */
		std::vector<float32> matrices;	/*!< 16 floats per frame of animation. */

		// Indices: If you will have loads of repeated values
		std::vector<uint32> positionIndices;
		std::vector<uint32> rotationIndices;
		std::vector<uint32> scaleIndices;
		std::vector<uint32> matrixIndices;

		uint32   numberOfFrames;

		InternalData() : flags(0), numberOfFrames(0)
		{
		}
	};

public:

	/// <summary>Get the transformation matrix of specific frame and amount of interpolation.</summary>
	/// <param name="frame">The first frame for which the transformation matrix will be returned</param>
	/// <param name="interp">Interpolation value used between the frames</param>
	/// <returns>The transformation matrix for the point in time that is at point <paramref name="interp"/>between
	/// frame <paramref name="frame"/>and frame <paramref name="frame+1"/></returns>
	/// <remarks>If the animation consists of Transformation Matrices, they will NOT be interpolated as this would be
	/// a very expensive operation. Rather, the closest matrix will be returned. If the transformation consists of
	/// Scale/ translation vectors and Rotation quaternia, Scale and Translation will be Linear Interpolated, and
	/// Rotation will be SLERPed (Smooth Linear Interpolation) as normal.</remarks>
	glm::mat4x4 getTransformationMatrix(uint32 frame = 0, float32 interp = 0) const;
//
//TO IMPLEMENT
//	\brief	Get translation for specific frame and interpolation. The aniumation MUST
//	        be in Quaternion/vector mode. Matrix mode does not support these operations.
//	\return	glm::vec3
//	\param	uint32 frame
//	\param	float32 interp interpolation value used between #frame and the next frame(0..1)
//	glm::vec3 getTranslation(uint32 frame = 0, float32 interp = 0) const;
//
//	\brief	Get translation for specific frame.
//	\return	glm::vec3
//	glm::vec3 getTranslation(float32 frame = 0) const;
//
//	\brief	Get rotation for specific frame and interpolation.
//	\return	glm::quat
//	\param	uint32 frame
//	\param	float32 interp interpolation value used between frames
//	glm::quat getRotation(uint32 frame = 0, float32 interp = 0) const;
//
//	\brief	Get scaling for specific frame and interpolation.
//	\return	glm::vec3
//	\param	uint32 frame
//	\param	float32 interp interpolation value used between frames
//	glm::vec3 getScaling(uint32 frame = 0, float32 interp = 0) const;
//
	/// <summary>Get number of frames in this animation.</summary>
	uint32 getNumFrames() const;

	/// <summary>Get the flags data on this animation.</summary>
	uint32 getFlags() const;

	/// <summary>Get a pointer to the position data of this animation.</summary>
	const float32* getPositions() const;

	/// <summary>Get a pointer to the indexes of the position data of this animation.</summary>
	const uint32* getPositionIndices() const;

	/// <summary>Get a pointer to the rotation data of this animation (normally quaternions).</summary>
	const float32* getRotations() const;

	/// <summary>Get a pointer to the indexes of the rotation data of this animation.</summary>
	const uint32* getRotationIndices() const;

	/// <summary>Get a pointer to the Scale data of this animation.</summary>
	const float32* getScales() const;

	/// <summary>Get a pointer to the indexes of the Scale data.</summary>
	const uint32* getScaleIndices() const;

	/// <summary>Get a pointer to the transformation matrices of this animation.</summary>
	const float32* getMatrices() const;

	/// <summary>Get a pointer to the indexes of the transformation matrices of this animation.</summary>
	const uint32* getMatrixIndices() const;


	/// <summary>Set the position transformation data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The position data that will be copied. Must be packed floats, each successive 3 of which
	/// will be interpreted as x,y,z values.</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation
	/// </returns>
	bool setPositions(uint32 numFrames, const float32* data,
	                          const uint32* indices = NULL);   // Expects an array of 3 floats

	/// <summary>Set the rotation transformation data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The rotation data that will be copied. Must be packed floats, each successive 4 of which
	/// will be interpreted as x,y,z,w quaternion values.</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation
	/// </returns>
	bool setRotations(uint32 numFrames, const float32* const data,
	                          const uint32* const indices = 0);   // Expects an array of 4 floats

	/// <summary>Set the scale transformation data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The rotation data that will be copied. Must be packed floats, each successive 3 of which
	/// will be interpreted as x,y,z scale factors</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation
	/// </returns>
	bool setScales(uint32 numFrames, const float32* const data,
	                       const uint32* const indices = 0);   // Expects an array of 7 floats

	/// <summary>Set the transformation matrices data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The Transformation Matrices data that will be copied. Must be packed floats, each
	/// successive 16 of which will be interpreted as a matrix packed in the usual way (Column-major, with each column
	/// of the matrix stored successively in</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation
	/// </returns>
	bool setMatrices(uint32 numFrames, const float32* const data,
	                         const uint32* const indices = 0);   // Expects an array of 16 floats

	/// <summary>Gets a direct, modifiable pointer to the data representation of this object. Advanced tasks only.
	/// </summary>
	InternalData& getInternalData();  // If you know what you're doing

private:
	glm::mat4x4 getTranslationMatrix(uint32 frame = 0, float32 interp = 0) const;

	glm::mat4x4 getRotationMatrix(uint32 frame = 0, float32 interp = 0) const;

	glm::mat4x4 getScalingMatrix(uint32 frame = 0, float32 interp = 0) const;

	InternalData _data;
};
}
}