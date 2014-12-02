/******************************************************************************

 @File         OSX/PVRShellOS.cpp

 @Title        OSX/PVRShellOS

 @Version       @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     OSX

 @Description  Makes programming for 3D APIs easier by wrapping surface
               initialization and other functions for use by a demo.

******************************************************************************/

#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "PVRShell.h"
#include "PVRShellAPI.h"
#include "PVRShellOS.h"
#include "PVRShellImpl.h"

// No Doxygen for CPP files, due to documentation duplication
/// @cond NO_DOXYGEN

/*!***************************************************************************
	Constants & #defines
*****************************************************************************/

/*! X dimension of the window that is created */
#define SHELL_DISPLAY_DIM_X	800
/*! Y dimension of the window that is created */
#define SHELL_DISPLAY_DIM_Y	600

/*****************************************************************************
	Declarations
*****************************************************************************/
//static Bool WaitForMapNotify( Display *d, XEvent *e, char *arg );

/*!***************************************************************************
	Class: PVRShellInit
*****************************************************************************/

/*
	OS functionality
*/

void PVRShell::PVRShellOutputDebug(char const * const format, ...) const
{
    if(!format)
        return;
    
	va_list arg;
	char	buf[1024];

	va_start(arg, format);
	vsnprintf(buf, 1024, format, arg);
	va_end(arg);

	/* Passes the data to a platform dependant function */
	m_pShellInit->OsDisplayDebugString(buf);
}



/*!***********************************************************************
 @Function		OsInit
 @description	Initialisation for OS-specific code.
*************************************************************************/
void PVRShellInit::OsInit()
{
	// Setup the default window size
	m_pShell->m_pShellData->nShellDimX = SHELL_DISPLAY_DIM_X;
	m_pShell->m_pShellData->nShellDimY = SHELL_DISPLAY_DIM_Y;

    mach_timebase_info(&m_sTimeBaseInfo);
	
    ObjC_OSInit(this);
}

/*!***********************************************************************
 @Function		OsInitOS
 @description	Saves instance handle and creates main window
				In this function, we save the instance handle in a global variable and
				create and display the main program window.
*************************************************************************/
bool PVRShellInit::OsInitOS()
{
	return ObjC_OSInitOS(this);
}

/*!***********************************************************************
 @Function		OsReleaseOS
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsReleaseOS()
{
}

/*!***********************************************************************
 @Function		OsExit
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsExit()
{
	/*
		Show the exit message to the user
	*/
    ObjC_ExitMessage(this, (const char*)m_pShell->PVRShellGet(prefExitMessage));
}

/*!***********************************************************************
 @Function		OsDoInitAPI
 @Return		true on success
 @description	Perform GL initialization and bring up window / fullscreen
*************************************************************************/
bool PVRShellInit::OsDoInitAPI()
{

	if(!ApiInitAPI())
	{
		return false;
	}

	/* No problem occured */
	return true;
}

/*!***********************************************************************
 @Function		OsDoReleaseAPI
 @description	Clean up after we're done
*************************************************************************/
void PVRShellInit::OsDoReleaseAPI()
{
	ApiReleaseAPI();

	if(m_pShell->m_pShellData->bNeedPixmap)
	{
	}

}

/*!***********************************************************************
 @Function		OsRenderComplete
 @Returns		false when the app should quit
 @description	Main message loop / render loop
*************************************************************************/
void PVRShellInit::OsRenderComplete()
{
}

/*!***********************************************************************
 @Function		OsPixmapCopy
 @Return		true if the copy succeeded
 @description	When using pixmaps, copy the render to the display
*************************************************************************/
bool PVRShellInit::OsPixmapCopy()
{
	return true;
}

/*!***********************************************************************
 @Function		OsGetNativeDisplayType
 @Return		The 'NativeDisplayType' for EGL
 @description	Called from InitAPI() to get the NativeDisplayType
*************************************************************************/
void *PVRShellInit::OsGetNativeDisplayType()
{
	return NULL;
}

/*!***********************************************************************
 @Function		OsGetNativePixmapType
 @Return		The 'NativePixmapType' for EGL
 @description	Called from InitAPI() to get the NativePixmapType
*************************************************************************/
void *PVRShellInit::OsGetNativePixmapType()
{
	// Pixmap support: return the pixmap
	return NULL;
}

/*!***********************************************************************
 @Function		OsGetNativeWindowType
 @Return		The 'NativeWindowType' for EGL
 @description	Called from InitAPI() to get the NativeWindowType
*************************************************************************/
void *PVRShellInit::OsGetNativeWindowType()
{
	return m_pView;
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName		value to retrieve
 @Output		pn				pointer to which to write the value
 @Description	Retrieves OS-specific data
*************************************************************************/
bool PVRShellInit::OsGet(const prefNameIntEnum prefName, int *pn)
{
	return ObjC_OSGet(this, prefName, pn);
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName		value to retrieve
 @Output		pp				pointer to which to write the value
 @Description	Retrieves OS-specific data
*************************************************************************/
bool PVRShellInit::OsGet(const prefNamePtrEnum prefName, void **pp)
{
	return false;
}

/*!***********************************************************************
 @Function		OsSet
 @Input			prefName				Name of preference to set to value
 @Input			value					Value
 @Return		true for success
 @Description	Sets OS-specific data
*************************************************************************/
bool PVRShellInit::OsSet(const prefNameBoolEnum prefName, const bool value)
{
	return false;
}

/*!***********************************************************************
 @Function		OsSet
 @Input			prefName	Name of value to set
 @Input			i32Value 	The value to set our named value to
 @Returns		true on success
 @Description	Sets OS-specific data
*************************************************************************/
bool PVRShellInit::OsSet(const prefNameIntEnum prefName, const int i32Value)
{
	return false;
}

/*!***********************************************************************
 @Function		OsDisplayDebugString
 @Input			str		string to output
 @Description	Prints a debug string
*************************************************************************/
void PVRShellInit::OsDisplayDebugString(char const * const str)
{
#ifndef NO_SHELL_DEBUG
	fprintf(stderr, "%s", str);
#endif
}

/*!***********************************************************************
 @Function		OsGetTime
 @Return		An incrementing time value measured in milliseconds
 @Description	Returns an incrementing time value measured in milliseconds
*************************************************************************/
unsigned long PVRShellInit::OsGetTime()
{
	uint64_t time = mach_absolute_time();
	uint64_t millis = (time * (m_sTimeBaseInfo.numer/m_sTimeBaseInfo.denom))/1000000.0;
	return static_cast<unsigned long>(millis);
}

/// @endcond

/*****************************************************************************
 End of file (PVRShellOS.cpp)
*****************************************************************************/

