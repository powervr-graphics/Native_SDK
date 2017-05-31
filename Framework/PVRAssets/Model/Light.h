/*!
\brief Represents a Light in the scene (Model).
\file PVRAssets/Model/Light.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace assets {
/// <summary>Represents a Light source in the scene.</summary>
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

	/// <summary>Raw internal structure of the Light.</summary>
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

	/// <summary>Get the node ID of the target of a light with a direction.</summary>
	/// <returns>The Node ID of the target of a light with a direction</returns>
	int32	getTargetIdx() const;

	/// <summary>Get light color.</summary>
	/// <returns>RGB color triplet in a glm::vec3</returns>
	const glm::vec3&	getColor() const;

	/// <summary>Get light type (spot, point, directional).</summary>
	LightType getType() const;

	/// <summary>Get the Constant attenuation of a spot or point light.</summary>
	float32	getConstantAttenuation() const;

	/// <summary>Get the Linear attenuation of a spot or point light.</summary>
	float32	getLinearAttenuation() const;

	/// <summary>Get the Quadratic attenuation of a spot or point light.</summary>
	float32	getQuadraticAttenuation() const;

	/// <summary>Get the Falloff angle of a spot light.</summary>
	float32	getFalloffAngle() const;

	/// <summary>Get the Falloff exponent of a spot light.</summary>
	float32	getFalloffExponent() const;

	/// <summary>Set a Target for a spot light.</summary>
	void setTargetNodeIdx(int32 idx);

	/// <summary>Set light color.</summary>
	/// <param name="r">Red color channel ([0..1])</param>
	/// <param name="g">Green color channel ([0..1])</param>
	/// <param name="b">Blue color channel ([0..1])</param>
	/// <returns>void</returns>
	void setColor(float32 r, float32 g, float32 b);

	/// <summary>Set light type.</summary>
	/// <param name="t">The type of the light</param>
	void setType(LightType t);
	/// <summary>Set constant attenuation.</summary>
	/// <param name="c">Constant attenuation factor</param>
	void setConstantAttenuation(float32 c);
	/// <summary>Set linear attenuation.</summary>
	/// <param name="l">Linear attenuation factor</param>
	void setLinearAttenuation(float32 l);
	/// <summary>Set Quadratic attenuation.</summary>
	/// <param name="q">Quadratic attenuation factor</param>
	void setQuadraticAttenuation(float32 q);
	/// <summary>Set spot Falloff angle. This is the angle inside of which the spotlight is full strength.</summary>
	/// <param name="fa">Falloff angle</param>
	void setFalloffAngle(float32 fa);

	/// <summary>Set a spot Falloff exponent.</summary>
	/// <param name="fe">Falloff exponent</param>
	void setFalloffExponent(float32 fe);

	/// <summary>Get a reference to the internal representation of this object. Handle with care.</summary>
	InternalData& getInternalData(); // If you know what you're doing

private:
	InternalData _data;
};
}
}