/*!*********************************************************************************************************************
\file         PVRAssets/Model/Light.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Represents a Light in the scene (Model).
***********************************************************************************************************************/
#pragma once

#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace assets {
/*!***********************************************************************************************
\brief Represents a Light source in the scene.
*************************************************************************************************/
class Light
{
public:
	enum LightType
	{
		Point = 0,	 	/*!< Point light */
		Directional, 	/*!< Directional light */
		Spot,		 	/*!< Spot light */

		NumLightTypes /*!< number of supported light type */
	};

	/*!************************************************************************************************************
	\brief Raw internal structure of the Light.
	***************************************************************************************************************/
	struct InternalData
	{
		//------------- What is the targetindex?

		int32		spotTargetNodeIdx;    /*!< Index of the target object */ // Should this be a point to the actual node?
		glm::vec3	color;            /*!< Light color (0.0f -> 1.0f for each channel) */
		LightType	type;                 /*!< Light type (point, directional, spot etc.) */
		float32		constantAttenuation;  /*!< Constant attenuation */
		float32		linearAttenuation;    /*!< Linear attenuation */
		float32		quadraticAttenuation; /*!< Quadratic attenuation */
		float32		falloffAngle;         /*!< Falloff angle (in radians) */
		float32		falloffExponent;     /*!< Falloff exponent */

		InternalData() : spotTargetNodeIdx(-1), type(Light::Point), constantAttenuation(1.0f), linearAttenuation(0.0f),
			quadraticAttenuation(0.0f), falloffAngle(glm::pi<float32>()), falloffExponent(0.0f)
		{
			color[0] = color[1] = color[2] = 1.0f;
		}
	};

public:

	/*!******************************************************************************
	\brief	Get the node ID of the target of a light with a direction.
	\return	The Node ID of the target of a light with a direction
	********************************************************************************/
	int32	getTargetIdx() const;

	/*!******************************************************************************
	\brief	Get light color.
	\return	RGB color triplet in a glm::vec3
	********************************************************************************/
	const glm::vec3&	getColor() const;

	/*!******************************************************************************
	\brief	Get light type (spot, point, directional).
	********************************************************************************/
	LightType getType() const;

	/*!******************************************************************************
	\brief	Get the Constant attenuation of a spot or point light.
	********************************************************************************/
	float32	getConstantAttenuation() const;

	/*!******************************************************************************
	\brief	Get the Linear attenuation of a spot or point light.
	********************************************************************************/
	float32	getLinearAttenuation() const;

	/*!******************************************************************************
	\brief	Get the Quadratic attenuation of a spot or point light.
	********************************************************************************/
	float32	getQuadraticAttenuation() const;

	/*!******************************************************************************
	\brief	Get the Falloff angle of a spot light.
	********************************************************************************/
	float32	getFalloffAngle() const;

	/*!******************************************************************************
	\brief	Get the Falloff exponent of a spot light.
	********************************************************************************/
	float32	getFalloffExponent() const;

	/*!******************************************************************************
	\brief	Set a Target for a spot light.
	********************************************************************************/
	void setTargetNodeIdx(int32 idx);

	/*!******************************************************************************
	\brief	Set light color.
	\return	void
	\param	r Red color channel ([0..1])
	\param	g Green color channel ([0..1])
	\param	b Blue color channel ([0..1])
	********************************************************************************/
	void setColor(float32 r, float32 g, float32 b);

	/*!******************************************************************************
	\brief	Set light type.
	\param	t The type of the light
	********************************************************************************/
	void setType(LightType t);
	/*!******************************************************************************
	\brief	Set constant attenuation.
	\param	c Constant attenuation factor
	********************************************************************************/
	void setConstantAttenuation(float32 c);
	/*!******************************************************************************
	\brief	Set linear attenuation.
	\param	l Linear attenuation factor
	********************************************************************************/
	void setLinearAttenuation(float32 l);
	/*!******************************************************************************
	\brief	Set Quadratic attenuation.
	\param	q Quadratic attenuation factor
	********************************************************************************/
	void setQuadraticAttenuation(float32 q);
	/*!******************************************************************************
	\brief	Set spot Falloff angle. This is the angle inside of which the spotlight
	        is full strength.
	\param	fa Falloff angle
	********************************************************************************/
	void setFalloffAngle(float32 fa);

	/*!******************************************************************************
	\brief	Set a spot Falloff exponent. 
	\param	fe Falloff exponent
	********************************************************************************/
	void setFalloffExponent(float32 fe);

	/*!******************************************************************************
	\brief	Get a reference to the internal representation of this object. Handle
	        with care.
	********************************************************************************/
	InternalData& getInternalData(); // If you know what you're doing

private:
	InternalData m_data;
};
}
}