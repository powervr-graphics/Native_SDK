/*!****************************************************************************
 @File          Particle.h

 @Title         Particle class for OGLESParticles.cpp

 @Author        PowerVR

 @Copyright     Copyright (C) by Imagination Technologies Limited.

 @Platform      Independant

 @Description   Requires the OGLESShell.

******************************************************************************/
#include "OGLESTools.h"

class CParticle
{
public:
	// Dynamic properties
	PVRTVec3	m_fPosition;
	PVRTVec3	m_fVelocity;
	PVRTVec3	m_fColour;
	float		m_fAge;

	// Inherent properties
	float		m_fLifeTime;
	float		m_fMass;

	float		m_fSize;

	PVRTVec3	m_fInitialColour;
	PVRTVec3	m_fHalfwayColour;
	PVRTVec3	m_fEndColor;

public:
	CParticle() { }	// Allow default construct
	CParticle(const PVRTVec3 &fPos, const PVRTVec3 &fVel, float fM, float fLife) :  m_fPosition(fPos), 
																					m_fVelocity(fVel), 
																					m_fAge(0), 
																					m_fLifeTime(fLife), 
																					m_fMass(fM), 
																					m_fSize(0)  { }

	bool Step(float fDelta_t, PVRTVec3 &aForce)
	{
		PVRTVec3 fAccel;
		PVRTVec3 fForce = aForce;

		if (m_fPosition.y < 0)
		{
			if(fDelta_t != 0.0)
			{
				fForce.y += ((0.5f *m_fVelocity.y) * m_fVelocity.y) * (m_fMass + 9.8f * m_fMass);
			}
		}

		float fInvMass = 1.0f/m_fMass;
		fAccel.x = 0.0f + fForce.x * fInvMass;
		fAccel.y = -9.8f + fForce.y * fInvMass;
		fAccel.z = 0.0f + fForce.z * fInvMass;

		m_fVelocity.x += fDelta_t * fAccel.x;
		m_fVelocity.y += fDelta_t * fAccel.y;
		m_fVelocity.z += fDelta_t * fAccel.z;

		m_fPosition.x += fDelta_t * m_fVelocity.x;
		m_fPosition.y += fDelta_t * m_fVelocity.y;
		m_fPosition.z += fDelta_t * m_fVelocity.z;
		m_fAge += fDelta_t;

		if(m_fAge <= m_fLifeTime / 2)
		{
			float mu = m_fAge / (m_fLifeTime/2.0f);
			m_fColour.x = ((1-mu) * m_fInitialColour.x) + (mu * m_fHalfwayColour.x);
			m_fColour.y = ((1-mu) * m_fInitialColour.y) + (mu * m_fHalfwayColour.y);
			m_fColour.z = ((1-mu) * m_fInitialColour.z) + (mu * m_fHalfwayColour.z);
		}
		else
		{
			float mu = ((m_fAge-m_fLifeTime)/2.0f) / (m_fLifeTime/2.0f);
			m_fColour.x = (1-mu) * m_fHalfwayColour.x + mu * m_fEndColor.x;
			m_fColour.y = (1-mu) * m_fHalfwayColour.y + mu * m_fEndColor.y;
			m_fColour.z = (1-mu) * m_fHalfwayColour.z + mu *m_fEndColor.z;
		}

		return (m_fAge >= m_fLifeTime);
	}
};


