/*!*********************************************************************************************************************
\file         PVRAssets/Model/Animation.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains an Animation class.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace assets {
/*!************************************************************************************************************
\brief Represents an Animation that can be applied to different objects.
***************************************************************************************************************/
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

	/*!************************************************************************************************************
	\brief Raw internal structure of the Animation.
	***************************************************************************************************************/
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

	/*!******************************************************************************
	\brief	Get the transformation matrix of specific frame and amount of interpolation.
	\return	The transformation matrix for the point in time that is at point \p interp
	        between frame \p frame and frame \p frame+1
	\param	frame The first frame for which the transformation matrix will be returned
	\param	interp Interpolation value used between the frames
	\remarks If the animation consists of Transformation Matrices, they will NOT be
	         interpolated as this would be a very expensive operation. Rather, the 
			 closest matrix will be returned. If the transformation consists of Scale/
			 translation vectors and Rotation quaternia, Scale and Translation will be
			 Linear Interpolated, and Rotation will be SLERPed (Smooth Linear Interpolation)
			 as normal.
	********************************************************************************/
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
	/*!******************************************************************************
	\brief	Get number of frames in this animation.
	********************************************************************************/
	uint32 getNumFrames() const;

	/*!******************************************************************************
	\brief	Get the flags data on this animation.
	********************************************************************************/
	uint32 getFlags() const;

	/*!******************************************************************************
	\brief	Get a pointer to the position data of this animation.
	********************************************************************************/
	const float32* getPositions() const;

	/*!******************************************************************************
	\brief	Get a pointer to the indexes of the position data of this animation.
	********************************************************************************/
	const uint32* getPositionIndices() const;

	/*!******************************************************************************
	\brief	Get a pointer to the rotation data of this animation (normally quaternions).
	********************************************************************************/
	const float32* getRotations() const;

	/*!******************************************************************************
	\brief	Get a pointer to the indexes of the rotation data of this animation.
	********************************************************************************/
	const uint32* getRotationIndices() const;

	/*!******************************************************************************
	\brief	Get a pointer to the Scale data of this animation.
	********************************************************************************/
	const float32* getScales() const;

	/*!******************************************************************************
	\brief	Get a pointer to the indexes of the Scale data.
	********************************************************************************/
	const uint32* getScaleIndices() const;

	/*!******************************************************************************
	\brief	Get a pointer to the transformation matrices of this animation.
	********************************************************************************/
	const float32* getMatrices() const;

	/*!******************************************************************************
	\brief	Get a pointer to the indexes of the transformation matrices of this animation.
	********************************************************************************/
	const uint32* getMatrixIndices() const;


	/*!******************************************************************************
	\brief	Set the position transformation data for this animation.
	\return	True on success, false if passing the wrong amount of data for the number
	        of frames in the animation
	\param	numFrames The number of frames of animation to set
	\param	data The position data that will be copied. Must be packed floats, each
	        successive 3 of which will be interpreted as x,y,z values.
	\param	indices If this array is not NULL, the position data will be indexed.
			Default NULL.
	********************************************************************************/
	bool setPositions(uint32 numFrames, const float32* data,
	                          const uint32* indices = NULL);   // Expects an array of 3 floats

	/*!******************************************************************************
	\brief	Set the rotation transformation data for this animation.
	\return	True on success, false if passing the wrong amount of data for the number
	        of frames in the animation
	\param	numFrames The number of frames of animation to set
	\param	data The rotation data that will be copied. Must be packed floats, each
	        successive 4 of which will be interpreted as x,y,z,w quaternion values.
	\param	indices If this array is not NULL, the position data will be indexed.
			Default NULL.
	********************************************************************************/
	bool setRotations(uint32 numFrames, const float32* const data,
	                          const uint32* const indices = 0);   // Expects an array of 4 floats

	/*!******************************************************************************
	\brief	Set the scale transformation data for this animation.
	\return	True on success, false if passing the wrong amount of data for the number
	        of frames in the animation
	\param	numFrames The number of frames of animation to set
	\param	data The rotation data that will be copied. Must be packed floats, each
	        successive 3 of which will be interpreted as x,y,z scale factors
	\param	indices If this array is not NULL, the position data will be indexed.
			Default NULL.
	********************************************************************************/
	bool setScales(uint32 numFrames, const float32* const data,
	                       const uint32* const indices = 0);   // Expects an array of 7 floats

	/*!******************************************************************************
	\brief	Set the transformation matrices data for this animation.
	\return	True on success, false if passing the wrong amount of data for the number
	        of frames in the animation
	\param	numFrames The number of frames of animation to set
	\param	data The Transformation Matrices data that will be copied. Must be packed 
			floats, each successive 16 of which will be interpreted as a matrix packed
			in the usual way (Column-major, with each column of the matrix stored 
			successively in 
	\param	indices If this array is not NULL, the position data will be indexed.
			Default NULL.
	********************************************************************************/
	bool setMatrices(uint32 numFrames, const float32* const data,
	                         const uint32* const indices = 0);   // Expects an array of 16 floats

	/*!******************************************************************************
	\brief	Gets a direct, modifiable pointer to the data representation of this object. 
			Advanced tasks only.
	********************************************************************************/
	InternalData& getInternalData();  // If you know what you're doing

private:
	glm::mat4x4 getTranslationMatrix(uint32 frame = 0, float32 interp = 0) const;

	glm::mat4x4 getRotationMatrix(uint32 frame = 0, float32 interp = 0) const;

	glm::mat4x4 getScalingMatrix(uint32 frame = 0, float32 interp = 0) const;

	InternalData m_data;
};
}
}