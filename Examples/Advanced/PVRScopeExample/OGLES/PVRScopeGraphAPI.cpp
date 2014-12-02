/******************************************************************************

 @File         PVRScopeGraphAPI.cpp

 @Title        

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     

 @Description  

******************************************************************************/
#include "PVRScopeGraph.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#include <GLES/gl.h>

/****************************************************************************
** Defines
****************************************************************************/
// Defines to abstract float/fixed data for Common/CommonLite profiles
#ifdef PVRT_FIXED_POINT_ENABLE
#define VERTTYPE			GLfixed
#define VERTTYPEENUM		GL_FIXED
#define f2vt(x)				((int)((x)*65536))
#define myglColor4			glColor4x
#else
#define VERTTYPE			GLfloat
#define VERTTYPEENUM		GL_FLOAT
#define f2vt(x)				(x)
#define myglColor4			glColor4f
#endif

/****************************************************************************
** Structures
****************************************************************************/
struct SVertex
{
	VERTTYPE x, y;
};

struct SPVRScopeAPIData
{
	SVertex	*pVtx;
	SVertex	*pVtxLines;
};

/****************************************************************************
** Constants
****************************************************************************/

static const GLushort c_pwLineIdx[10] = {
	0, 1,  2, 3,  4, 5,  0, 4,  1, 5
};

/****************************************************************************
** Code
****************************************************************************/
SPVRScopeAPIData *CPVRScopeGraph::APIInit()
{
	SPVRScopeAPIData *pData;

	pData = new SPVRScopeAPIData;

	if(pData)
	{
		pData->pVtx = 0;
		pData->pVtxLines = 0;
	}

	return pData;
}

void CPVRScopeGraph::APISize(SPVRScopeAPIData * const pData, const unsigned int, const unsigned int)
{
	delete[] pData->pVtx;
	delete[] pData->pVtxLines;

	pData->pVtx = new SVertex[m_nSizeCB];
	pData->pVtxLines = new SVertex[6];

	pData->pVtxLines[0].x = f2vt(m_fX);
	pData->pVtxLines[0].y = f2vt(m_fY);

	pData->pVtxLines[1].x = f2vt(m_fX + m_nSizeCB * m_fPixelW);
	pData->pVtxLines[1].y = f2vt(m_fY);

	pData->pVtxLines[2].x = f2vt(m_fX);
	pData->pVtxLines[2].y = f2vt(m_fY + m_fGraphH * 0.5f);

	pData->pVtxLines[3].x = f2vt(m_fX + m_nSizeCB * m_fPixelW);
	pData->pVtxLines[3].y = f2vt(m_fY + m_fGraphH * 0.5f);

	pData->pVtxLines[4].x = f2vt(m_fX);
	pData->pVtxLines[4].y = f2vt(m_fY + m_fGraphH);

	pData->pVtxLines[5].x = f2vt(m_fX + m_nSizeCB * m_fPixelW);
	pData->pVtxLines[5].y = f2vt(m_fY + m_fGraphH);
}

void CPVRScopeGraph::APIShutdown(SPVRScopeAPIData ** const ppData)
{
	SPVRScopeAPIData *pData;

	pData	= *ppData;
	*ppData	= 0;

	delete [] pData->pVtx;
	delete [] pData->pVtxLines;
	delete pData;
}

void CPVRScopeGraph::APIRender(SPVRScopeAPIData * const pData)
{
	float fRatio;
	
	GLboolean bDepthTest = glIsEnabled(GL_DEPTH_TEST);

	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glLoadIdentity();

	// Set the colour for the points
	myglColor4(f2vt(0.5f), f2vt(0.5f), f2vt(0.5f), f2vt(1.0f));

	// Draw the lines
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, VERTTYPEENUM, sizeof(VERTTYPE) * 2, pData->pVtxLines);

	glDrawElements(GL_LINES, 10, GL_UNSIGNED_SHORT, c_pwLineIdx);

	glVertexPointer(2, VERTTYPEENUM, sizeof(VERTTYPE) * 2, pData->pVtx);

	for(unsigned int i = 0; i < m_nCounterNum; ++i)
	{
		if((m_pCounters[i].nGroup == m_nActiveGroup || m_pCounters[i].nGroup == 0xffffffff) && m_pGraphCounters[i].bShow)
		{
			myglColor4(f2vt(m_pGraphCounters[i].fColour[0]), 
						f2vt(m_pGraphCounters[i].fColour[1]), 
						f2vt(m_pGraphCounters[i].fColour[2]), 
						f2vt(m_pGraphCounters[i].fColour[3]));

			float fMaximum = 0.0f;

			if(m_pGraphCounters[i].fMaximum != 0.0f)
				fMaximum = m_pGraphCounters[i].fMaximum;
			else if(!m_pCounters[i].nBoolPercentage)
				fMaximum = GetMaximumOfData(i);
			else
				fMaximum = 100.0f;

			// Generate geometry
			for(int iDst = 0, iSrc = m_pGraphCounters[i].nWritePosCB; iDst < (int)m_nSizeCB; ++iDst, ++iSrc)
			{
				// Wrap the source index when necessary
				if(iSrc >= (int) m_nSizeCB)
					iSrc = 0;

				// X
				pData->pVtx[iDst].x = f2vt(m_fX + iDst * m_fPixelW);

				// Y
				if(m_pGraphCounters[i].pfValueCB[iSrc])
					fRatio = m_pGraphCounters[i].pfValueCB[iSrc] / fMaximum;
				else
					fRatio = 0;
				
				if(fRatio < 0)
					fRatio = 0;
				else if(fRatio > 1)
					fRatio = 1;

				pData->pVtx[iDst].y = f2vt(m_fY + fRatio * m_fGraphH);
			}

			// Render geometry
			glDrawArrays(GL_LINE_STRIP, 0, m_nSizeCB);
		}
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	if(bDepthTest)
		glEnable(GL_DEPTH_TEST);

	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	myglColor4(f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f));
}

/*****************************************************************************
 End of file (PVRScopeGraphAPI.cpp)
*****************************************************************************/

