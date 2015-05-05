/******************************************************************************

 @File         PVRShellOS.cpp

 @Title        LinuxEWS/PVRShellOS

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Example Window System support for Linux

 @Description  Makes programming for 3D APIs easier by wrapping window creation
               and other functions for use by a demo.

******************************************************************************/

#include "PVRShell.h"
#include "PVRShellAPI.h"
#include "PVRShellOS.h"
#include "PVRShellImpl.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <signal.h>

// No Doxygen for CPP files, due to documentation duplication
/// @cond NO_DOXYGEN

/*!***************************************************************************
	Defines
*****************************************************************************/
/*!***************************************************************************
	Declarations
*****************************************************************************/

/*!***************************************************************************
	Class: PVRShellInit
*****************************************************************************/

/*!***********************************************************************
@Function		PVRShellOutputDebug
@Input			format			printf style format followed by arguments it requires
@Description	Writes the resultant string to the debug output (e.g. using
				printf(), OutputDebugString(), ...). Check the SDK release notes for
				details on how the string is output.
*************************************************************************/
void PVRShell::PVRShellOutputDebug(char const * const format, ...) const
{
	if(!format)
		return;

	va_list arg;
	char	buf[1024];

	va_start(arg, format);
	vsnprintf(buf, 1024, format, arg);
	va_end(arg);

	// Passes the data to a platform dependant function
	m_pShellInit->OsDisplayDebugString(buf);
}

/*!***********************************************************************
 @Function		OsInit
 @description	Initialisation for OS-specific code.
*************************************************************************/
void PVRShellInit::OsInit()
{
	// set values to negative to mark that these are unset
	m_pShell->m_pShellData->nShellDimX = -1;
    m_pShell->m_pShellData->nShellDimY = -1;

	// Construct the binary path for GetReadPath() and GetWritePath()

	// Get PID (Process ID)
	pid_t ourPid = getpid();
	char *pszExePath, pszSrcLink[64];
	int len = 64;
	int res;

	sprintf(pszSrcLink, "/proc/%d/exe", ourPid);
	pszExePath = 0;

	do
	{
		len *= 2;
		delete[] pszExePath;
		pszExePath = new char[len];
		res = readlink(pszSrcLink, pszExePath, len);

		if(res < 0)
		{
			m_pShell->PVRShellOutputDebug("Warning Readlink %s failed. The application name, read path and write path have not been set.\n", pszExePath);
			break;
		}
	} while(res >= len);

	if(res >= 0)
	{
		pszExePath[res] = '\0'; // Null-terminate readlink's result
		SetReadPath(pszExePath);
		SetWritePath(pszExePath);
		SetAppName(pszExePath);
	}

	delete[] pszExePath;

	gettimeofday(&m_StartTime,NULL);
}

/*!***********************************************************************
 @Function		OsInitOS
 @description	Saves instance handle and creates main window
				In this function, we save the instance handle in a global variable and
				create and display the main program window.
*************************************************************************/
bool PVRShellInit::OsInitOS()
{
	EWS_COORD windowPosition;
	EWS_SIZE windowSize;
	EWS_PIXELFORMAT ePixelFormat;
	m_EWSDisplay = EWS_NO_DISPLAY;
	m_EWSWindow = EWS_NO_WINDOW;


	this->m_EWSDisplay = EWSOpenDisplay(EWS_DEFAULT_DISPLAY, 0);
	if (this->m_EWSDisplay == EWS_NO_DISPLAY)
	{
		m_pShell->PVRShellOutputDebug("PVRShellOS: EWSOpenDisplay failed\n");
		return false;
	}
	ePixelFormat = EWS_PIXEL_FORMAT_RGB_565;
	m_pShell->m_pShellData->nColorBPP=16;

	if(m_pShell->m_pShellData->nShellDimX <= 0)
	    m_pShell->m_pShellData->nShellDimX = 640;

	if(m_pShell->m_pShellData->nShellDimY <= 0)
	    m_pShell->m_pShellData->nShellDimY = 480;

	windowPosition.iX = m_pShell->PVRShellGet(prefPositionX);
	windowPosition.iY = m_pShell->PVRShellGet(prefPositionY);
	windowSize.uiHeight = m_pShell->m_pShellData->nShellDimY;
	windowSize.uiWidth = m_pShell->m_pShellData->nShellDimX;

	this->m_EWSWindow = EWSCreateWindow(this->m_EWSDisplay, windowPosition, windowSize, ePixelFormat, EWS_ROTATE_0);
	if (this->m_EWSWindow == EWS_NO_WINDOW)
	{
		m_pShell->PVRShellOutputDebug("PVRShellOS: EWSCreateWindow failed\n");
		return false;
	}
	return true;
}

/*!***********************************************************************
 @Function		OsReleaseOS
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsReleaseOS()
{
	EWSDestroyWindow(this->m_EWSWindow);
	EWSCloseDisplay(this->m_EWSDisplay);
}

/*!***********************************************************************
 @Function		OsExit
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsExit()
{
	// Show the exit message to the user
	m_pShell->PVRShellOutputDebug((const char*)m_pShell->PVRShellGet(prefExitMessage));
}

/*!***********************************************************************
 @Function		OsDoInitAPI
 @Return		true on success
 @description	Perform API initialisation and bring up window / fullscreen
*************************************************************************/
bool PVRShellInit::OsDoInitAPI()
{
	if(!ApiInitAPI())
	{
		return false;
	}

	// No problem occured
	return true;
}

/*!***********************************************************************
 @Function		OsDoReleaseAPI
 @description	Clean up after we're done
*************************************************************************/
void PVRShellInit::OsDoReleaseAPI()
{
	ApiReleaseAPI();
}

/*!***********************************************************************
 @Function		OsRenderComplete
 @Returns		false when the app should quit
 @description	Main message loop / render loop
*************************************************************************/
void PVRShellInit::OsRenderComplete()
{
	/* XXX Only sets the last keycode seen per frame */
	EWS_EVENT sEvent;
	while (EWSNextEventIfAvailable(&sEvent))
	{
		if (sEvent.sWindow == this->m_EWSWindow
			&& sEvent.eType == EWS_EVENT_KEYPRESS)
		{
			switch (sEvent.uiKeyCode)
			{

				case EWS_KEY_Q:
				case EWS_KEY_ESC:
					nLastKeyPressed = PVRShellKeyNameQUIT;
					break;
				case EWS_KEY_S:
					nLastKeyPressed = PVRShellKeyNameScreenshot;
					break;
				case EWS_KEY_ENTER:
					nLastKeyPressed = PVRShellKeyNameSELECT;
					break;
				case EWS_KEY_SPACE:
					nLastKeyPressed = PVRShellKeyNameACTION1;
					break;
				case EWS_KEY_UP:
					nLastKeyPressed = m_eKeyMapUP;
					break;
				case EWS_KEY_DOWN:
					nLastKeyPressed = m_eKeyMapDOWN;
					break;
				case EWS_KEY_LEFT:
					nLastKeyPressed = m_eKeyMapLEFT;
					break;
				case EWS_KEY_RIGHT:
					nLastKeyPressed = m_eKeyMapRIGHT;
					break;
				default:;

			}
		}
	}
}

/*!***********************************************************************
 @Function		OsPixmapCopy
 @Return		true if the copy succeeded
 @description	When using pixmaps, copy the render to the display
*************************************************************************/
bool PVRShellInit::OsPixmapCopy()
{
	return false;
}

/*!***********************************************************************
 @Function		OsGetNativeDisplayType
 @Return		The 'NativeDisplayType' for EGL
 @description	Called from InitAPI() to get the NativeDisplayType
*************************************************************************/
void *PVRShellInit::OsGetNativeDisplayType()
{
	return (void*)m_EWSDisplay;
}

/*!***********************************************************************
 @Function		OsGetNativePixmapType
 @Return		The 'NativePixmapType' for EGL
 @description	Called from InitAPI() to get the NativePixmapType
*************************************************************************/
void *PVRShellInit::OsGetNativePixmapType()
{
	// Pixmap support: return the pixmap
	return 0;
}

/*!***********************************************************************
 @Function		OsGetNativeWindowType
 @Return		The 'NativeWindowType' for EGL
 @description	Called from InitAPI() to get the NativeWindowType
*************************************************************************/
void *PVRShellInit::OsGetNativeWindowType()
{
	return (void*) this->m_EWSWindow;
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName	Name of value to get
 @Modified		pn A pointer set to the value asked for
 @Returns		true on success
 @Description	Retrieves OS-specific data
*************************************************************************/
bool PVRShellInit::OsGet(const prefNameIntEnum prefName, int *pn)
{
	return false;
}

/*!***********************************************************************
 @Function		OsGet
 @Input			prefName	Name of value to get
 @Modified		pp A pointer set to the value asked for
 @Returns		true on success
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
	switch(prefName)
	{
	default:
		return false;
	};
}

/*!***********************************************************************
 @Function		OsDisplayDebugString
 @Input			str		string to output
 @Description	Prints a debug string
*************************************************************************/
void PVRShellInit::OsDisplayDebugString(char const * const str)
{
	printf("%s",str);
}

/*!***********************************************************************
 @Function		OsGetTime
 @Return		An incrementing time value measured in milliseconds
 @Description	Returns an incrementing time value measured in milliseconds
*************************************************************************/
unsigned long PVRShellInit::OsGetTime()
{
	timeval tv;
	gettimeofday(&tv,NULL);

	if(tv.tv_sec < m_StartTime.tv_sec)
		m_StartTime.tv_sec = 0;

	unsigned long sec = tv.tv_sec - m_StartTime.tv_sec;
	return (unsigned long)((sec*(unsigned long)1000) + (tv.tv_usec/1000.0));
}

/*****************************************************************************
 Class: PVRShellInitOS
*****************************************************************************/

/*****************************************************************************
 Global code
*****************************************************************************/

/*!***************************************************************************
@function		main
@input			argc	count of args from OS
@input			argv	array of args from OS
@returns		result code to OS
@description	Main function of the program
*****************************************************************************/
int main(int argc, char **argv)
{
	PVRShellInit init;

	// Initialise the demo, process the command line, create the OS initialiser.
	if(!init.Init())
		return EXIT_ERR_CODE;

	init.CommandLine((argc-1),&argv[1]);

	//	Initialise/run/shutdown
	while(init.Run());

	return EXIT_NOERR_CODE;
}

/// @endcond

/*****************************************************************************
 End of file (PVRShellOS.cpp)
*****************************************************************************/

