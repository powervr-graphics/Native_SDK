/******************************************************************************

 @File         iOS/PVRShellOS.cpp

 @Title        iOS/PVRShellOS

 @Version 

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     iOS

 @Description  Makes programming for 3D APIs easier by wrapping surface
               initialization, Texture allocation and other functions for use by a demo.

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
	m_pShell->m_pShellData->bFullScreen = true;
	mach_timebase_info(&m_sTimeBaseInfo);
}

/*!***********************************************************************
 @Function		OsInitOS
 @description	Saves instance handle and creates main window
				In this function, we save the instance handle in a global variable and
				create and display the main program window.
*************************************************************************/
bool PVRShellInit::OsInitOS()
{
	// Initialize global strings
	szTitle = (char*)"PVRShell";
	return true;
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
	m_pShell->PVRShellOutputDebug((const char*)m_pShell->PVRShellGet(prefExitMessage));
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
	//return (void*)x11pixmap;
	return NULL;
}

/*!***********************************************************************
 @Function		OsGetNativeWindowType
 @Return		The 'NativeWindowType' for EGL
 @description	Called from InitAPI() to get the NativeWindowType
*************************************************************************/
void *PVRShellInit::OsGetNativeWindowType()
{
	//return (void*)x11window;
	return NULL;
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName		value to retrieve
 @Output		pn				pointer to which to write the value
 @Description	Retrieves OS-specific data
*************************************************************************/
bool PVRShellInit::OsGet(const prefNameIntEnum prefName, int *pn)
{
	return false;
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName		value to retrieve
 @Output		pp				pointer to which to write the value
 @Description	Retrieves OS-specific data
*************************************************************************/
bool PVRShellInit::OsGet(const prefNamePtrEnum prefName, void **pp)
{
	switch(prefName)
	{
		case prefAccelerometer:
			*pp = m_vec3Accel;
			return true;
		default:{}
	}

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
	return millis;
}

/*****************************************************************************
 Class: PVRShellInitOS
*****************************************************************************/

/*!***********************************************************************
 @Function		BeganTouch
 @Description	Called from API when a touch is registered
*************************************************************************/

void PVRShellInitOS::BeganTouch(float vec2Location[2], PVRShellInit* init)
{
	float Position[2] = { vec2Location[0] / (float)init->m_pShell->PVRShellGet(prefWidth) , vec2Location[1] / (float)init->m_pShell->PVRShellGet(prefHeight) };
	init->TouchBegan(Position);
}

/*!***********************************************************************
 @Function		MovedTouch
 @Description	Called from API when a touch is registered
*************************************************************************/

void PVRShellInitOS::MovedTouch(float vec2Location[2], PVRShellInit* init)
{
	float Position[2] = { vec2Location[0] / (float)init->m_pShell->PVRShellGet(prefWidth) , vec2Location[1] / (float)init->m_pShell->PVRShellGet(prefHeight) };
	init->TouchMoved(Position);
}

/*!***********************************************************************
 @Function		EndedTouch
 @Description	Called from API when a touch is registered
*************************************************************************/

void PVRShellInitOS::EndedTouch(float vec2Location[2], PVRShellInit* init)
{
	float Position[2] = { vec2Location[0] / (float)init->m_pShell->PVRShellGet(prefWidth) , vec2Location[1] / (float)init->m_pShell->PVRShellGet(prefHeight) };
	init->TouchEnded(Position);
}

/// @endcond

/*****************************************************************************
 End of file (PVRShellOS.cpp)
*****************************************************************************/

