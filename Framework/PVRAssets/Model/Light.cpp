/*!
\brief Implementations of methods of the Light class.
\file PVRAssets/Model/Light.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/Model/Light.h"
namespace pvr {
namespace assets{
int32 Light::getTargetIdx() const
{
	return _data.spotTargetNodeIdx;
}

const glm::vec3& Light::getColor() const
{
	return _data.color;
}

Light::LightType Light::getType() const
{
	return _data.type;
}

float32 Light::getConstantAttenuation() const
{
	return _data.constantAttenuation;
}

float32 Light::getLinearAttenuation() const
{
	return _data.linearAttenuation;
}

float32 Light::getQuadraticAttenuation() const
{
	return _data.quadraticAttenuation;
}

float32 Light::getFalloffAngle() const
{
	return _data.falloffAngle;
}

float32 Light::getFalloffExponent() const
{
	return _data.falloffExponent;
}

void Light::setTargetNodeIdx(int32 index)
{
	_data.spotTargetNodeIdx = index;
}

void Light::setColor(float32 r, float32 g, float32 b)
{
	_data.color[0] = r;
	_data.color[1] = g;
	_data.color[2] = b;
}

void Light::setType(LightType t)
{
	_data.type = t;
}

void Light::setConstantAttenuation(float32 c)
{
	_data.constantAttenuation = c;
}

void Light::setLinearAttenuation(float32 l)
{
	_data.linearAttenuation = l;
}

void Light::setQuadraticAttenuation(float32 q)
{
	_data.quadraticAttenuation = q;
}

void Light::setFalloffAngle(float32 fa)
{
	_data.falloffAngle = fa;
}

void Light::setFalloffExponent(float32 fe)
{
	_data.falloffExponent = fe;
}

Light::InternalData& Light::getInternalData()
{
	return _data;
}
}
}
//!\endcond