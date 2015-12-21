/*!*********************************************************************************************************************
\file         PVRAssets\Model\Light.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations of methods of the Light class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/Model/Light.h"
namespace pvr {
namespace assets{
int32 Light::getTargetIdx() const
{
	return m_data.spotTargetNodeIdx;
}

const glm::vec3& Light::getColor() const
{
	return m_data.color;
}

Light::LightType Light::getType() const
{
	return m_data.type;
}

float32 Light::getConstantAttenuation() const
{
	return m_data.constantAttenuation;
}

float32 Light::getLinearAttenuation() const
{
	return m_data.linearAttenuation;
}

float32 Light::getQuadraticAttenuation() const
{
	return m_data.quadraticAttenuation;
}

float32 Light::getFalloffAngle() const
{
	return m_data.falloffAngle;
}

float32 Light::getFalloffExponent() const
{
	return m_data.falloffExponent;
}

void Light::setTargetNodeIdx(int32 index)
{
	m_data.spotTargetNodeIdx = index;
}

void Light::setColor(float32 r, float32 g, float32 b)
{
	m_data.color[0] = r;
	m_data.color[1] = g;
	m_data.color[2] = b;
}

void Light::setType(LightType t)
{
	m_data.type = t;
}

void Light::setConstantAttenuation(float32 c)
{
	m_data.constantAttenuation = c;
}

void Light::setLinearAttenuation(float32 l)
{
	m_data.linearAttenuation = l;
}

void Light::setQuadraticAttenuation(float32 q)
{
	m_data.quadraticAttenuation = q;
}

void Light::setFalloffAngle(float32 fa)
{
	m_data.falloffAngle = fa;
}

void Light::setFalloffExponent(float32 fe)
{
	m_data.falloffExponent = fe;
}

Light::InternalData& Light::getInternalData()
{
	return m_data;
}
}
}
//!\endcond