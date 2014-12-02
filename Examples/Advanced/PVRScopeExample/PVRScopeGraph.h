/******************************************************************************

 @File         PVRScopeGraph.h

 @Title        

 @Copyright    Copyright (C)  Imagination Technologies Limited.

 @Platform     

 @Description  

******************************************************************************/
#ifndef _PVRSCOPEGRAPH_H_
#define _PVRSCOPEGRAPH_H_

#include "PVRScopeStats.h"

/****************************************************************************
** Structures
****************************************************************************/
struct SPVRScopeAPIData;

struct SPVRGraphCounter
{
	float *pfValueCB;	// Circular buffer of counter values
	unsigned int nWritePosCB;	// Current write position of circular buffer
	bool bShow; // Show the graph
	float fColour[4];
	float fMaximum;

	SPVRGraphCounter()
	{
		pfValueCB = 0;
		nWritePosCB = 0;
		bShow = true;
		fColour[0] = 1.0f; fColour[1] = 0.0f; fColour[2] = 0.0f; fColour[3] = 1.0f;
		fMaximum = 0.0f;
	}
};
/****************************************************************************
** Class: CPVRScopeGraph
****************************************************************************/
class CPVRScopeGraph
{
protected:
	SPVRScopeImplData		*m_pPVRScopeData;
    SPVRScopeCounterReading	m_sReading;

	unsigned int		m_nCounterNum;
	SPVRScopeCounterDef	*m_pCounters;
	unsigned int		m_nActiveGroup;			// most recent group seen
	unsigned int		m_nActiveGroupSelect;	// users desired group
	bool				m_bActiveGroupChanged;

	unsigned int		m_nSizeCB;

	SPVRScopeAPIData	*m_pPVRScopeAPIData;
	SPVRGraphCounter	*m_pGraphCounters;

	float				m_fX, m_fY, m_fPixelW, m_fGraphH;

	unsigned int		m_nUpdateInterval, m_nUpdateIntervalCounter;
public:
	CPVRScopeGraph();
	~CPVRScopeGraph();
	
	// Disallow copying
	CPVRScopeGraph(const CPVRScopeGraph&);				// no implementation
	CPVRScopeGraph &operator=(const CPVRScopeGraph&);	// no implementation

	void Ping();

	void ShowCounter(unsigned int nCounter, bool bShow);
	bool IsCounterShown(const unsigned int nCounter) const;
	bool IsCounterBeingDrawn(unsigned int nCounter) const;
	bool IsCounterPercentage(const unsigned int nCounter) const;
	bool SetActiveGroup(const unsigned int nActiveGroup);
	unsigned int GetActiveGroup() const { return m_nActiveGroup; }
	float GetMaximumOfData(unsigned int nCounter);
	float GetMaximum(unsigned int nCounter);
	void  SetMaximum(unsigned int nCounter, float fMaximum);

	unsigned int GetCounterNum() const { return m_nCounterNum; }

	const char *GetCounterName(const unsigned int i) const;
	int GetCounterGroup(const unsigned int i) const;

	void position(
		const unsigned int nViewportW, const unsigned int nViewportH,
		const unsigned int nGraphX, const unsigned int nGraphY,
		const unsigned int nGraphW, const unsigned int nGraphH);

	void SetUpdateInterval(const unsigned int nUpdateInverval);

protected:
	SPVRScopeAPIData *APIInit();
	void APISize(SPVRScopeAPIData * const pData, const unsigned int nViewportW, const unsigned int nViewportH);
	void APIShutdown(SPVRScopeAPIData ** const ppData);
	void APIRender(SPVRScopeAPIData * const pData);
};


/****************************************************************************
** Declarations
****************************************************************************/


#endif /* _PVRSCOPEGRAPH_H_ */

/*****************************************************************************
 End of file (PVRScopeGraph.h)
*****************************************************************************/
