/******************************************************************************

 @File         LinuxDRM/PVRShellOS.cpp

 @Title        LinuxDRM/PVRShellOS

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Direct Rendering Manager (DRM) support for any Linux

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
#include <errno.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

// No Doxygen for CPP files, due to documentation duplication
/// @cond NO_DOXYGEN

#if !defined(CONNAME)
#define CONNAME "/dev/tty"
#endif

#define DRIDEVNAME "/dev/dri/card0"


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
	int retval;

	// In case we're in the background ignore SIGTTIN and SIGTTOU
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	m_pShell->m_pShellData->bFullScreen= true;	// linux overrides default to use fullscreen

	m_ui32NativeDisplay = 0;

	// Keyboard handling
	if ((devfd = open(CONNAME, O_RDWR|O_NDELAY)) <= 0)
	{
		m_pShell->PVRShellOutputDebug("Can't open tty (%s)\n",CONNAME);
	}
	else
	{
		tcgetattr(devfd, &termio_orig);
		tcgetattr(devfd, &termio);
		cfmakeraw(&termio);
		termio.c_oflag |= OPOST | ONLCR; // Turn back on cr-lf expansion on output
		termio.c_cc[VMIN]=1;
		termio.c_cc[VTIME]=0;

		if (tcsetattr(devfd, TCSANOW, &termio) == -1)
		{
			m_pShell->PVRShellOutputDebug("Can't set tty attributes for %s\n",CONNAME);
		}
	}

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

	/*
	 Get rid of the blinking cursor on a screen.

	 It's an equivalent of:
	 <CODE> echo -n -e "\033[?25l" > /dev/tty0 </CODE>
	 if you do the above command then you can undo it with:
	 <CODE> echo -n -e "\033[?25h" > /dev/tty0 </CODE>
	*/
	const char txt[] = { 27 // the ESCAPE ASCII character
			   , '['
			   , '?'
			   , '2'
			   , '5'
			   , 'l'
			   , 0
		   };

	retval = write(devfd, &txt[0], 7);

	gettimeofday(&m_StartTime, NULL);

	m_pShell->m_pShellData->nShellDimX = -1;
	m_pShell->m_pShellData->nShellDimY = -1;
	m_ui32DrmDisplayId = 0;


}

static void pfnCallbackDrmFbDestroy(struct gbm_bo *bo, void *data)
{
	struct SDrmFbWrapper *psFb = (struct SDrmFbWrapper *)data;

	if (psFb->ui32FbId)
	{
		drmModeRmFB(psFb->i32Fd, psFb->ui32FbId);
	}

	delete psFb;
}

static void pfnCallbackDrmPageFlip(int fd, unsigned int frame,
		  unsigned int sec, unsigned int usec, void *data)
{
	int *pi32WaitFlip = (int *)data;
	*pi32WaitFlip = 0;
}

struct SDrmFbWrapper *PVRShellInitOS::DrmFbGetFromBo(struct gbm_bo *bo)
{
	struct SDrmFbWrapper *fb = (struct SDrmFbWrapper *)gbm_bo_get_user_data(bo);
	uint32_t width, height, stride, handle;
	int ret;

	if (fb)
	{
		return fb;
	}

	fb = new struct SDrmFbWrapper;
	fb->psGbmBo = bo;
	fb->i32Fd = m_i32DrmFile;

	width = gbm_bo_get_width(bo);
	height = gbm_bo_get_height(bo);
	stride = gbm_bo_get_stride(bo);
	handle = gbm_bo_get_handle(bo).u32;

	ret = drmModeAddFB(m_i32DrmFile, width, height, 24, 32, stride, handle, &fb->ui32FbId);

	if (ret) 
	{
		delete fb;
		return NULL;
	}

	gbm_bo_set_user_data(bo, fb, pfnCallbackDrmFbDestroy);
	return fb;
}

/*!***********************************************************************
 @Function		OsInitOS
 @description	Saves instance handle and creates main window
				In this function, we save the instance handle in a global variable and
				create and display the main program window.
*************************************************************************/
bool PVRShellInit::OsInitOS()
{
	bool bFound;

	/*
		In the future we could be fancy here by getting the drm device from udev.
		For the time being, we've added a command-line option so the use can pass
		it in. By default just use card0.
	*/
	if((m_i32DrmFile = open(DRIDEVNAME, O_RDWR)) < 0) 
	{
		m_pShell->PVRShellOutputDebug("failed to open drm device %s : %s\n", DRIDEVNAME, strerror(errno));
		return false;
	}

	m_psDrmResources = drmModeGetResources(m_i32DrmFile);
	
	if (!m_psDrmResources) 
	{
		m_pShell->PVRShellOutputDebug("drmModeGetResources failed: %s\n", strerror(errno));
		return false;
	}

	// find a connected connector
	bFound = false;
	
	for (int i = 0; i < m_psDrmResources->count_connectors; ++i) 
	{
		m_psDrmConnector = drmModeGetConnector(m_i32DrmFile, m_psDrmResources->connectors[i]);

		if (m_psDrmConnector->connection != DRM_MODE_CONNECTED) 
		{
			drmModeFreeConnector(m_psDrmConnector);
			continue;
		}

		if (m_ui32DrmDisplayId == 0) 
		{
			bFound = true;
			break;
		}

		if (m_ui32DrmDisplayId == m_psDrmConnector->connector_id) 
		{
			bFound = true;
			break;
		}
	}

	if (bFound == false) 
	{
		m_pShell->PVRShellOutputDebug("No Connector found for requested device\n");
		return false;
	}

	m_ui32DrmConnectorId = m_psDrmConnector->connector_id;
	m_psDrmMode = &m_psDrmConnector->modes[0];

	bFound = false;

	for (int j = 0; j < m_psDrmResources->count_encoders; ++j) 
	{
		m_psDrmEncoder = drmModeGetEncoder(m_i32DrmFile, m_psDrmResources->encoders[j]);
		
		if (m_psDrmEncoder->encoder_id == m_psDrmConnector->encoder_id) 
		{
			bFound = true;
			break;
		}

		drmModeFreeEncoder(m_psDrmEncoder);
	}

	if(!bFound) 
	{
		m_pShell->PVRShellOutputDebug("No Encoder found for requested Connector\n");
		return false;
	}

	m_ui32DrmEncoderId = m_psDrmEncoder->encoder_id;
	m_ui32DrmCrtcId = m_psDrmEncoder->crtc_id;

	for (int j=0; j<m_psDrmResources->count_crtcs; ++j) 
	{
		m_psDrmCrtc = drmModeGetCrtc(m_i32DrmFile, m_psDrmResources->crtcs[j]);
		
		if (m_psDrmCrtc->crtc_id == m_ui32DrmCrtcId) 
		{
			break;
		}

		drmModeFreeCrtc(m_psDrmCrtc);
	}


	m_pShell->m_pShellData->nShellDimX = m_psDrmMode->hdisplay;
	m_pShell->m_pShellData->nShellDimY = m_psDrmMode->vdisplay;

	m_psGbmDev = gbm_create_device(m_i32DrmFile);
	m_psGbmSurface = gbm_surface_create(m_psGbmDev,
			m_pShell->m_pShellData->nShellDimX,
			m_pShell->m_pShellData->nShellDimY,
			GBM_FORMAT_XRGB8888,
			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);

	if (!m_psGbmSurface) 
	{
		m_pShell->PVRShellOutputDebug("failed to create gbm surface\n");
		return false;
	}

	m_ui32NativeDisplay = (unsigned int)m_psGbmDev;
	m_ui32NativeWindow = (unsigned int)m_psGbmSurface;
	m_ui32CurrentFb = 0;
	return true;
}

/*!***********************************************************************
 @Function		OsReleaseOS
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsReleaseOS()
{
	gbm_surface_destroy(m_psGbmSurface);
	gbm_device_destroy(m_psGbmDev);
	drmModeFreeCrtc(m_psDrmCrtc);
	drmModeFreeEncoder(m_psDrmEncoder);
	drmModeFreeConnector(m_psDrmConnector);
	drmModeFreeResources(m_psDrmResources);
	drmClose(m_i32DrmFile);
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
	if (!ApiInitAPI())
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
	int		ckb = 0;
	size_t  bytes_read;
	int ret, i;
	struct gbm_bo *bo;
	struct SDrmFbWrapper *fb;
	int waiting_for_flip = 1;
	fd_set fds;
	drmEventContext evctx;
	evctx.version = DRM_EVENT_CONTEXT_VERSION;
	evctx.page_flip_handler = pfnCallbackDrmPageFlip;

	bo = gbm_surface_lock_front_buffer(m_psGbmSurface);
	fb = DrmFbGetFromBo(bo);
	
	if(!m_ui32CurrentFb) 
	{
		ret = drmModeSetCrtc(m_i32DrmFile, m_ui32DrmCrtcId, fb->ui32FbId, 0, 0,
				&m_ui32DrmConnectorId, 1, m_psDrmMode);

		if (ret) 
		{
			m_pShell->PVRShellOutputDebug("display failed to set mode: %s\n", strerror(errno));
			return;
		}
	} else 
	{
		ret = drmModePageFlip(m_i32DrmFile, m_ui32DrmCrtcId, fb->ui32FbId,
				DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip);

		if (ret) 
		{
			m_pShell->PVRShellOutputDebug("display failed to flip page: %s\n", strerror(errno));
			return;
		}

		FD_ZERO(&fds);
		FD_SET(m_i32DrmFile, &fds);

		while (waiting_for_flip) 
		{
			ret = select(m_i32DrmFile + 1, &fds, NULL, NULL, NULL);

			if (ret < 0) 
			{
				m_pShell->PVRShellOutputDebug("Select Error: %s\n", strerror(errno));
				return;
			} 
			else if (ret == 0) 
			{
				m_pShell->PVRShellOutputDebug("Select Timeout\n");
				return;
			} 
			else if (FD_ISSET(0, &fds)) 
			{
				continue;
			}

			drmHandleEvent(m_i32DrmFile, &evctx);
		}
	}
	// Check keyboard and keypad

	// Keyboard.
	if(devfd > 0)
	{
		while ((bytes_read = read(devfd, &ckb, 1)) == 1);

		switch(ckb)
		{
		case '0':
		case 'q':
		case 'Q':
			nLastKeyPressed = PVRShellKeyNameQUIT;
			break;
		case 13:
			nLastKeyPressed = PVRShellKeyNameSELECT;
			break;
		case ' ':
		case '1':
			nLastKeyPressed = PVRShellKeyNameACTION1;
			break;
		case '2':
			nLastKeyPressed = PVRShellKeyNameACTION2;
			break;
		case 65: // Up Arrow
			nLastKeyPressed = m_eKeyMapUP;
			break;
		case 66: // Down Arrow
			nLastKeyPressed = m_eKeyMapDOWN;
			break;
		case 68: // Left Arrow
			nLastKeyPressed = m_eKeyMapLEFT;
			break;
		case 67: // Right Arrow
			nLastKeyPressed = m_eKeyMapRIGHT;
			break;
		default:
			break;
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
	return (void*) m_ui32NativeDisplay;
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
	return (void *) m_ui32NativeWindow;
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
	switch(prefName)
	{
		case prefNativeDisplay:
			*pn = m_ui32NativeDisplay;
			return true;
		default:
			return false;
	};
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
		case prefDisplayConnector:
			m_ui32DrmDisplayId = i32Value;
			return true;
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
	{
		m_StartTime.tv_sec = 0;
	}

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
	{
		return EXIT_ERR_CODE;
	}

	init.CommandLine((argc-1),&argv[1]);

	//	Initialise/run/shutdown
	while(init.Run()) {};

	return EXIT_NOERR_CODE;
}

/// @endcond

/*****************************************************************************
 End of file (PVRShellOS.cpp)
*****************************************************************************/

