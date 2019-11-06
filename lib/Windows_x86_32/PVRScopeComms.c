/*!***********************************************************************

 @file           PVRScopeComms.cpp
 @copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved.
 @brief          PVRScopeComms header file. @copybrief ScopeComms
 
**************************************************************************/

/*
	PVRPerfServer and PVRTune communications
*/

/*!
 @addtogroup ScopeComms PVRScopeComms
 @brief      The PVRScopeComms functionality of PVRScope allows an application to send user defined information to
             PVRTune via PVRPerfServer, both as counters and marks, or as editable data that can be
             passed back to the application.
 @details    PVRScopeComms has the following limitations:
             \li PVRPerfServer must be running on the host device if a @ref ScopeComms enabled
                 application wishes to send custom counters or marks to PVRTune. If the application in
                 question also wishes to communicate with PVRScopeServices without experiencing any
                 undesired behaviour PVRPerfServer should be run with the '--disable-hwperf' flag.
             \li The following types may be sent: Boolean, Enumerator, Float, Integer, String.
 @{
*/

/****************************************************************************
** Includes
****************************************************************************/
#include <PVRScopeComms.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

/****************************************************************************
** Enums
****************************************************************************/
    
/****************************************************************************
** Typedefs
****************************************************************************/
typedef struct SSPSCommsData*(*PPLINITIALISE)(
	const char			* const psName,		///< String to describe the application
	const unsigned int	nNameLen);			///< String length

typedef void (*PPLSHUTDOWN)(
	struct SSPSCommsData	*psData);			///< Context data

typedef void(*PPLWAITFORCONNECTION)(
	struct SSPSCommsData	* const psData,			///< Array of context data pointers
	int						* const pnBoolResults,	///< Array of results - false (0) if timeout
	const unsigned int		nCount,					///< Array length
	const unsigned int		nTimeOutMS);			///< Time-out length in milliseconds

typedef unsigned int (*PPLGETTIMEUS)(
	struct SSPSCommsData	* const psData);	    ///< Context data

typedef int (*PPLSENDMARK)(
	struct SSPSCommsData	* const psData,		///< Context data
	const char				* const psString,	///< String to send
	const unsigned int		nLen);				///< String length

typedef int (*PPLSENDPROCESSINGBEGIN)(
	struct SSPSCommsData	* const psData,		///< Context data
	const char				* const	psString,	///< Name of the processing block
	const unsigned int		nLen,				///< String length
	const unsigned int		nFrame);			///< Iteration (or frame) number, by which processes can be grouped.

typedef int (*PPLSENDPROCESSINGEND)(
	struct SSPSCommsData	* const psData);	///< Context data

typedef int (*PPLLIBRARYCREATE)(
	struct SSPSCommsData				* const psData,	///< Context data
	const struct SSPSCommsLibraryItem	* const pItems,	///< Editable items
	const unsigned int					nItemCount);	///< Number of items

typedef int (*PPLLIBRARYDIRTYGETFIRST)(
	struct SSPSCommsData	* const psData,	        ///< Context data
	unsigned int			* const pnItem,	        ///< Item number
	unsigned int			* const pnNewDataLen,   ///< New data length
	const char				**ppData);        	    ///< New data

typedef int (*PPLCOUNTERSCREATE)(
	struct SSPSCommsData				* const psData,	        ///< Context data
	const struct SSPSCommsCounterDef	* const psCounterDefs,  ///< Counter definitions
	const unsigned int					nCount);    	            ///< Number of counters

typedef int (*PPLCOUNTERSUPDATE)(
	struct SSPSCommsData		* const psData,	            ///< Context data
	const unsigned int			* const psCounterReadings);	///< Counter readings array

typedef int (*PPLSENDFLUSH)(
	struct SSPSCommsData		* const psData);	    ///< Context data

/****************************************************************************
** Structures
****************************************************************************/

// Internal implementation data
struct SSPSCommsData
{
	/*
		Not a circular reference, or a linked list. A pointer to the PVRScope
		structure *of the same name*, NOT a pointer to another instance of this
		structure.
	*/
	struct SSPSCommsData	*psSPSCommsData;

#ifdef _WIN32
	HMODULE					hScopeDLL;
#else
	void					*pScopeSO;
#endif

	PPLINITIALISE			pplInitialise;
	PPLSHUTDOWN				pplShutdown;
	PPLWAITFORCONNECTION	pplWaitForConnection;
	PPLGETTIMEUS			pplGetTimeUS;
	PPLSENDMARK				pplSendMark;
	PPLSENDPROCESSINGBEGIN	pplSendProcessingBegin;
	PPLSENDPROCESSINGEND	pplSendProcessingEnd;
	PPLLIBRARYCREATE		pplLibraryCreate;
	PPLLIBRARYDIRTYGETFIRST	pplLibraryDirtyGetFirst;
	PPLCOUNTERSCREATE		pplCountersCreate;
	PPLCOUNTERSUPDATE		pplCountersUpdate;
	PPLSENDFLUSH			pplSendFlush;
};

/****************************************************************************
** Declarations
****************************************************************************/

    
/*!**************************************************************************
 @brief         Initialise @ref ScopeComms
 @return        @ref ScopeComms data.
****************************************************************************/
struct SSPSCommsData *pplInitialise(
	const char			* const psName,		///< String to describe the application
	const unsigned int	nNameLen)			///< String length
{
	struct SSPSCommsData *psData = (struct SSPSCommsData*)malloc(sizeof(struct SSPSCommsData));
	if (!psData)
	{
		return 0;
	}

	{
		const char *pszLib = getenv("PVRSCOPE_LIBRARY");
		if (!pszLib)
		{
#ifdef _WIN32
			pszLib = "PVRScopeDeveloper.dll";
#else
			pszLib = "libPVRScopeDeveloper.so";
#endif
		}

		printf("\"%s\": trying \"%s\".\n", __FILE__, pszLib);

#ifdef _WIN32
		psData->hScopeDLL = LoadLibrary(pszLib);
		if (!psData->hScopeDLL)
#else
		// We don't specify RTLD_DEEPBIND here, because Android does not
		// support it. Instead, we ensure that PVRScope never calls its own
		// entry points.
		psData->pScopeSO = dlopen(pszLib, RTLD_LAZY | RTLD_LOCAL);
		if (!psData->pScopeSO)
#endif
		{
			free(psData);
			return 0;
		}
	}

	psData->psSPSCommsData = 0;
#ifdef _WIN32
	psData->pplInitialise			= (PPLINITIALISE			)GetProcAddress(psData->hScopeDLL, "pplInitialise");
	psData->pplShutdown				= (PPLSHUTDOWN				)GetProcAddress(psData->hScopeDLL, "pplShutdown");
	psData->pplWaitForConnection	= (PPLWAITFORCONNECTION		)GetProcAddress(psData->hScopeDLL, "pplWaitForConnection");
	psData->pplGetTimeUS			= (PPLGETTIMEUS				)GetProcAddress(psData->hScopeDLL, "pplGetTimeUS");
	psData->pplSendMark				= (PPLSENDMARK				)GetProcAddress(psData->hScopeDLL, "pplSendMark");
	psData->pplSendProcessingBegin	= (PPLSENDPROCESSINGBEGIN	)GetProcAddress(psData->hScopeDLL, "pplSendProcessingBegin");
	psData->pplSendProcessingEnd	= (PPLSENDPROCESSINGEND		)GetProcAddress(psData->hScopeDLL, "pplSendProcessingEnd");
	psData->pplLibraryCreate		= (PPLLIBRARYCREATE			)GetProcAddress(psData->hScopeDLL, "pplLibraryCreate");
	psData->pplLibraryDirtyGetFirst	= (PPLLIBRARYDIRTYGETFIRST	)GetProcAddress(psData->hScopeDLL, "pplLibraryDirtyGetFirst");
	psData->pplCountersCreate		= (PPLCOUNTERSCREATE		)GetProcAddress(psData->hScopeDLL, "pplCountersCreate");
	psData->pplCountersUpdate		= (PPLCOUNTERSUPDATE		)GetProcAddress(psData->hScopeDLL, "pplCountersUpdate");
	psData->pplSendFlush			= (PPLSENDFLUSH				)GetProcAddress(psData->hScopeDLL, "pplSendFlush");
#else
	psData->pplInitialise			= (PPLINITIALISE			)dlsym(psData->pScopeSO, "pplInitialise");
	psData->pplShutdown				= (PPLSHUTDOWN				)dlsym(psData->pScopeSO, "pplShutdown");
	psData->pplWaitForConnection	= (PPLWAITFORCONNECTION		)dlsym(psData->pScopeSO, "pplWaitForConnection");
	psData->pplGetTimeUS			= (PPLGETTIMEUS				)dlsym(psData->pScopeSO, "pplGetTimeUS");
	psData->pplSendMark				= (PPLSENDMARK				)dlsym(psData->pScopeSO, "pplSendMark");
	psData->pplSendProcessingBegin	= (PPLSENDPROCESSINGBEGIN	)dlsym(psData->pScopeSO, "pplSendProcessingBegin");
	psData->pplSendProcessingEnd	= (PPLSENDPROCESSINGEND		)dlsym(psData->pScopeSO, "pplSendProcessingEnd");
	psData->pplLibraryCreate		= (PPLLIBRARYCREATE			)dlsym(psData->pScopeSO, "pplLibraryCreate");
	psData->pplLibraryDirtyGetFirst	= (PPLLIBRARYDIRTYGETFIRST	)dlsym(psData->pScopeSO, "pplLibraryDirtyGetFirst");
	psData->pplCountersCreate		= (PPLCOUNTERSCREATE		)dlsym(psData->pScopeSO, "pplCountersCreate");
	psData->pplCountersUpdate		= (PPLCOUNTERSUPDATE		)dlsym(psData->pScopeSO, "pplCountersUpdate");
	psData->pplSendFlush			= (PPLSENDFLUSH				)dlsym(psData->pScopeSO, "pplSendFlush");
#endif

	psData->psSPSCommsData = psData->pplInitialise(psName, nNameLen);
	if (!psData->psSPSCommsData)
	{
#ifdef _WIN32
		FreeLibrary(psData->hScopeDLL);
#else
		dlclose(psData->pScopeSO);
#endif
		free(psData);
		psData = 0;
	}

	return psData;
}
    
/*!**************************************************************************
 @brief         Shutdown or de-initialise the remote control section of PVRScope.
****************************************************************************/
void pplShutdown(
    struct SSPSCommsData	*psData)			///< Context data
{
	psData->pplShutdown(psData->psSPSCommsData);
#ifdef _WIN32
	FreeLibrary(psData->hScopeDLL);
#else
	dlclose(psData->pScopeSO);
#endif
	free(psData);
}

/*!**************************************************************************
 @brief         Optional function. Sleeps until there is a connection to
				PVRPerfServer, or time-out. Normally, each thread will wait for
				its own connection, and each time-out will naturally happen in
				parallel. But if a thread happens to have multiple connections,
				N, then waiting for them all [in serial] with time-out M would
				take N*M ms if they were all to time-out (e.g. PVRPerfServer is
				not running); therefore this function, is designed to allow an
				entire array of connections to be waited upon simultaneously.
****************************************************************************/
void pplWaitForConnection(
	struct SSPSCommsData	* const psData,			///< Array of context data pointers
	int						* const pnBoolResults,	///< Array of results - false (0) if timeout
	const unsigned int		nCount,					///< Array length
	const unsigned int		nTimeOutMS)				///< Time-out length in milliseconds
{
	psData->pplWaitForConnection(psData->psSPSCommsData, pnBoolResults, nCount, nTimeOutMS);
}

/*!**************************************************************************
 @brief         Query for the time. Units are microseconds, resolution is undefined.
****************************************************************************/
unsigned int pplGetTimeUS(
	struct SSPSCommsData	* const psData)	    ///< Context data
{
	return psData->pplGetTimeUS(psData->psSPSCommsData);
}

/*!**************************************************************************
 @brief         Send a time-stamped string marker to be displayed in PVRTune.
 @details       Examples might be:
                \li switching to outdoor renderer
                \li starting benchmark test N
****************************************************************************/
int pplSendMark(
	struct SSPSCommsData	* const psData,		///< Context data
	const char				* const psString,	///< String to send
	const unsigned int		nLen)				///< String length
{
	return psData->pplSendMark(psData->psSPSCommsData, psString, nLen);
}

/*!**************************************************************************
 @brief         Send a time-stamped begin marker to PVRTune. 
 @details       Every begin must at some point be followed by an end; begin/end
				pairs can be nested. PVRTune will show these as an activity
				timeline, using a "flame graph" style when there is nesting.
				See also the CPPLProcessingScoped helper class.
****************************************************************************/
int pplSendProcessingBegin(
	struct SSPSCommsData	* const psData,		///< Context data
	const char				* const	psString,	///< Name of the processing block
	const unsigned int		nLen,				///< String length
	const unsigned int		nFrame)				///< Iteration (or frame) number, by which processes can be grouped.
{
	return psData->pplSendProcessingBegin(psData->psSPSCommsData, psString, nLen, nFrame);
}

/*!**************************************************************************
 @brief         Send a time-stamped end marker to PVRTune. 
 @details       Every begin must at some point be followed by an end; begin/end
				pairs can be nested. PVRTune will show these as an activity
				timeline, using a "flame graph" style when there is nesting.
				See also the CPPLProcessingScoped helper class.
****************************************************************************/
int pplSendProcessingEnd(
	struct SSPSCommsData	* const psData)      ///< Context data
{
	return psData->pplSendProcessingEnd(psData->psSPSCommsData);
}

/*!**************************************************************************
 @brief         Create a library of remotely editable items
****************************************************************************/
int pplLibraryCreate(
	struct SSPSCommsData				* const psData,	///< Context data
	const struct SSPSCommsLibraryItem	* const pItems,	///< Editable items
	const unsigned int					nItemCount)		///< Number of items
{
	return psData->pplLibraryCreate(psData->psSPSCommsData, pItems, nItemCount);
}

/*!**************************************************************************
 @brief	        Query to see whether a library item has been edited, and also
				retrieve the new data.
****************************************************************************/
int pplLibraryDirtyGetFirst(
	struct SSPSCommsData	* const psData,	        ///< Context data
	unsigned int			* const pnItem,	        ///< Item number
	unsigned int			* const pnNewDataLen,   ///< New data length
	const char				**ppData)        	    ///< New data
{
	return psData->pplLibraryDirtyGetFirst(psData->psSPSCommsData, pnItem, pnNewDataLen, ppData);
}

/*!**************************************************************************
 @brief         Specify the number of custom counters and their definitions
****************************************************************************/
int pplCountersCreate(
	struct SSPSCommsData				* const psData,	        ///< Context data
	const struct SSPSCommsCounterDef	* const psCounterDefs,  ///< Counter definitions
	const unsigned int					nCount)    	            ///< Number of counters
{
	return psData->pplCountersCreate(psData->psSPSCommsData, psCounterDefs, nCount);
}

/*!**************************************************************************
 @brief 	    Send an update for all the custom counters. The
				psCounterReadings array must be nCount long.
****************************************************************************/
int pplCountersUpdate(
	struct SSPSCommsData		* const psData,	            ///< Context data
	const unsigned int			* const psCounterReadings)	///< Counter readings array
{
	return psData->pplCountersUpdate(psData->psSPSCommsData, psCounterReadings);
}

/*!**************************************************************************
 @brief	        Force a cache flush.
 @details       Some implementations store data sends in the cache. If the data
				rate is low, the real send of data can be significantly
				delayed.
                If it is necessary to flush the cache, the best results are
				likely to be achieved by calling this function with a frequency
				between once per second up to once per frame. If data is sent
				extremely infrequently, this function could be called once at
				the end of each bout of data send.
****************************************************************************/
int pplSendFlush(
	struct SSPSCommsData		* const psData)	    ///< Context data
{
	return psData->pplSendFlush(psData->psSPSCommsData);
}

/*****************************************************************************
 End of file (PVRScopeComms.cpp)
*****************************************************************************/
