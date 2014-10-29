/******************************************************************************

 @File         LinuxNullWS/PVRShellOS.cpp

 @Title        LinuxNullWS/PVRShellOS

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Non-windowed support for any Linux

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

#if !defined(CONNAME)
#define CONNAME "/dev/tty"
#endif

#if defined(PVRSHELL_OMAP3_TS_SUPPORT)
#ifndef TOUCHSCREEN_INPUT
#define TOUCHSCREEN_INPUT "/dev/input/event1"
#endif
#endif

#if !defined(KEYPAD_INPUT)
#define KEYPAD_INPUT "/dev/input/event1"
#endif

#if !defined(FBNAME)
#define FBNAME  "/dev/fb0"
#endif

#if defined(PVRSHELL_INTEL_CE_PIC24_REMOTE)

#include "LR_PICInterface.h"

// Callback function for the remote control
INT32 pic24_client_callback(UINT8 type, UINT8 length, void *data, void *clientData);

char clientData[128];
LR_PICInterface pic_if(pic24_client_callback, clientData);
unsigned short g_usRemoteLastKey;
#endif

#if !defined(REMOTE)
#define REMOTE "/dev/ttyS1"
#endif

/*! X dimension of the gdl plane that is created */
#define SHELL_DISPLAY_DIM_X	1280
/*! Y dimension of the gdl plane that is created */
#define SHELL_DISPLAY_DIM_Y	720

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
	// In case we're in the background ignore SIGTTIN and SIGTTOU
	signal( SIGTTIN, SIG_IGN );
	signal( SIGTTOU, SIG_IGN );

	remote_fd = 0;

	m_pShell->m_pShellData->bFullScreen= true;	// linux overrides default to use fullscreen

	m_ui32NativeDisplay = 0;

	// Keyboard handling
	if((devfd=open(CONNAME, O_RDWR|O_NDELAY)) <= 0)
	{
		m_pShell->PVRShellOutputDebug("Can't open tty (%s)\n",CONNAME);
	}
	else
	{
		tcgetattr(devfd,&termio_orig);
		tcgetattr(devfd,&termio);
		cfmakeraw(&termio);
		termio.c_oflag |= OPOST | ONLCR; // Turn back on cr-lf expansion on output
		termio.c_cc[VMIN]=1;
		termio.c_cc[VTIME]=0;

		if(tcsetattr(devfd,TCSANOW,&termio) == -1)
		{
			m_pShell->PVRShellOutputDebug("Can't set tty attributes for %s\n",CONNAME);
		}
	}

	// Keypad handling.
	if ((keypad_fd = open(KEYPAD_INPUT, O_RDONLY | O_NDELAY)) <= 0)
	{
		m_pShell->PVRShellOutputDebug("Can't open keypad input device (%s)\n",KEYPAD_INPUT);
	}

#if defined(PVRSHELL_OMAP3_TS_SUPPORT)
	/*************************************************
	 * Touchscreen code
	 * NOTE: For the init code to work, these variables have to be set prior to the app launch.
	 *
	 * export TSLIB_TSDEVICE=/dev/input/event1
	 * export TSLIB_CONFFILE=/etc/ts.conf
	 * export TSLIB_CALIBFILE=/etc/pointercal
	 * export TSLIB_CONSOLEDEVICE=/dev/tty
	 * export TSLIB_FBDEVICE=/dev/fb0
	 *************************************************/

	ts = ts_open(TOUCHSCREEN_INPUT, 1);

	if (!ts)
	{
		m_pShell->PVRShellOutputDebug("Can't open the touchscreen input device\n");
	}
	else if (ts_config(ts))
	{
		m_pShell->PVRShellOutputDebug("Can't open the touchscreen input device\n");
	}
#endif

   // Remote Control handling
#if defined(PVRSHELL_INTEL_CE_PIC24_REMOTE)
	g_usRemoteLastKey = 0x0;
	pic_if.Init(REMOTE);
#else
    if((remote_fd = open(REMOTE, O_RDONLY|O_NDELAY)) <= 0)
	{
		m_pShell->PVRShellOutputDebug("Can't open remote control input device (%s)\n",REMOTE);
	}
    else
    {
		tcgetattr(remote_fd, &remios_orig);
		remios.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
		remios.c_iflag = IGNPAR | ICRNL;
		remios.c_oflag = 0;
		remios.c_lflag = 0;
		remios.c_cc[VMIN] = 1;
		remios.c_cc[VTIME]= 0;

		tcflush(remote_fd, TCIFLUSH);
		tcsetattr(remote_fd, TCSANOW, &remios);
    }
#endif

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
	FILE *tty = 0;
	tty = fopen("/dev/tty0", "w");
	if (tty != 0)
	{
		const char txt[] = { 27 /* the ESCAPE ASCII character */
						   , '['
						   , '?'
						   , '2'
						   , '5'
						   , 'l'
						   , 0
						   };

		fprintf(tty, "%s", txt);
		fclose(tty);
	}

	gettimeofday(&m_StartTime,NULL);

#if defined(USE_GDL_PLANE)
	gdl_init(0);

	// Set the width and height to fill the screen
	gdl_display_info_t di;
	gdl_get_display_info(GDL_DISPLAY_ID_0, &di);
	m_pShell->m_pShellData->nShellDimX = di.tvmode.width;
	m_pShell->m_pShellData->nShellDimY = di.tvmode.height;
#endif
}

/*!***********************************************************************
 @Function		OsInitOS
 @description	Saves instance handle and creates main window
				In this function, we save the instance handle in a global variable and
				create and display the main program window.
*************************************************************************/
bool PVRShellInit::OsInitOS()
{
#if defined(USE_GDL_PLANE)
	gdl_display_info_t di;
	gdl_get_display_info(GDL_DISPLAY_ID_0, &di);

	m_Plane = GDL_PLANE_ID_UPP_C;

 	gdl_pixel_format_t pixelFormat 	= GDL_PF_ARGB_32;

 	// Change the colour bpp default to 32 bits per pixel to match the GDL pixel format
 	m_pShell->m_pShellData->nColorBPP = 32;

    gdl_color_space_t colorSpace 	= GDL_COLOR_SPACE_RGB;
    gdl_rectangle_t srcRect, dstRect;

    gdl_ret_t rc = GDL_SUCCESS;

    rc = gdl_plane_config_begin(m_Plane);

    if(rc != GDL_SUCCESS)
    {
        m_pShell->PVRShellOutputDebug("Failed to begin config of GDL plane. (Error code 0x%x)\n", rc);
        return false;
    }

	if(m_pShell->m_pShellData->bFullScreen)
		dstRect.origin.x = dstRect.origin.y = 0;
	else
	{
		dstRect.origin.x = m_pShell->m_pShellData->nShellPosX;
		dstRect.origin.y = m_pShell->m_pShellData->nShellPosY;
	}

	srcRect.origin.x = srcRect.origin.y = 0;
    srcRect.width  = m_pShell->m_pShellData->nShellDimX;
    srcRect.height = m_pShell->m_pShellData->nShellDimY;

	bool bUpscaling = false;

	if(m_pShell->m_pShellData->bFullScreen)
	{
		dstRect.width = di.tvmode.width;
		bUpscaling = true;
	}
	else
		dstRect.width = srcRect.width;

	if(m_pShell->m_pShellData->bFullScreen)
	{
		dstRect.height = di.tvmode.height;
		bUpscaling = true;
	}
	else
		dstRect.height = srcRect.height;

	rc = gdl_plane_set_uint(GDL_PLANE_SCALE, bUpscaling ? GDL_TRUE : GDL_FALSE);

	if(rc != GDL_SUCCESS)
	{
		m_pShell->PVRShellOutputDebug("Failed to set upscale of GDL plane. (Error code 0x%x)\n", rc);
		return false;
	}

    rc = gdl_plane_set_attr(GDL_PLANE_SRC_COLOR_SPACE, &colorSpace);

    if(rc != GDL_SUCCESS)
    {
        m_pShell->PVRShellOutputDebug("Failed to set color space of GDL plane. (Error code 0x%x)\n", rc);
        return false;
	}

    rc = gdl_plane_set_attr(GDL_PLANE_PIXEL_FORMAT, &pixelFormat);

    if(rc != GDL_SUCCESS)
    {
        m_pShell->PVRShellOutputDebug("Failed to set pixel format of GDL plane. (Error code 0x%x)\n", rc);
        return false;
    }

    rc = gdl_plane_set_attr(GDL_PLANE_DST_RECT, &dstRect);

    if(rc != GDL_SUCCESS)
    {
        m_pShell->PVRShellOutputDebug("Failed to set dst rect of GDL plane. (Error code 0x%x)\n", rc);
        return false;
    }

    rc = gdl_plane_set_attr(GDL_PLANE_SRC_RECT, &srcRect);

    if(rc != GDL_SUCCESS)
    {
        m_pShell->PVRShellOutputDebug("Failed to set src rect of GDL plane. (Error code 0x%x)\n", rc);
        return false;
    }


    rc = gdl_plane_config_end(GDL_FALSE);

    if(rc != GDL_SUCCESS)
    {
        gdl_plane_config_end(GDL_TRUE);
        m_pShell->PVRShellOutputDebug("Failed to end config of GDL plane. (Error code 0x%x)\n", rc);
        return false;
	}
#endif
	return true;
}

/*!***********************************************************************
 @Function		OsReleaseOS
 @description	Destroys main window
*************************************************************************/
void PVRShellInit::OsReleaseOS()
{
#if defined(USE_GDL_PLANE)
	gdl_close();
#endif
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
	int		ckb = 0;
	size_t  bytes_read;

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
		case 0x71000000:
			nLastKeyPressed = PVRShellKeyNameQUIT;
			break;
		case 95: // F11
		case 's':
		case 0x73000000:
			nLastKeyPressed = PVRShellKeyNameScreenshot;
			break;
		case 13:
		case 0x0D000000:
			nLastKeyPressed = PVRShellKeyNameSELECT;
			break;
		case ' ':
		case 49:
		case 0x20000000:
			nLastKeyPressed = PVRShellKeyNameACTION1;
			break;
		case 50:
		case 0x20000001:
			nLastKeyPressed = PVRShellKeyNameACTION2;
			break;
		case 65: // Up Arrow
		case 0x41000000:
			nLastKeyPressed = m_eKeyMapUP;
			break;
		case 66: // Down Arrow
		case 0x42000000:
			nLastKeyPressed = m_eKeyMapDOWN;
			break;
		case 68: // Left Arrow
		case 0x44000000:
			nLastKeyPressed = m_eKeyMapLEFT;
			break;
		case 67: // Right Arrow
		case 0x43000000:
			nLastKeyPressed = m_eKeyMapRIGHT;
			break;
		default:;

		}
	}

	// Keypad.
	if(keypad_fd > 0)
	{
	    struct input_event {
        	struct timeval time;
        	unsigned short type;
        	unsigned short code;
        	unsigned int value;
    	}keyinfo;

        int bytes=read(keypad_fd,&keyinfo,sizeof(struct input_event));

        if(bytes == sizeof(struct input_event) && keyinfo.type==0x01)
        {
			// keyinfo.value posibilities:
			// 0: key released.
			// 1: key pressed.
			// 2: key is being held.
			if (keyinfo.value == 1 || keyinfo.value == 2)
			{
            	//printf("bytes_read=%d, time=%ld; sec %ld microsec,code=%d,value=%d\n",
				//		bytes,
            	//		keyinfo.time.tv_sec,
            	//		keyinfo.time.tv_usec,
            	//		keyinfo.code,
            	//		keyinfo.value
            	//		);

				switch(keyinfo.code)
				{
				case 22:
				case 64:
				case 107: // End call button on Zoom2
					  nLastKeyPressed = PVRShellKeyNameQUIT;	break;
				case 37:
				case 65:
					  nLastKeyPressed = PVRShellKeyNameScreenshot;	break;
				case 28:  nLastKeyPressed = PVRShellKeyNameSELECT;	break;
				case 46:
				case 59:
					  nLastKeyPressed = PVRShellKeyNameACTION1;	break;
				case 60:
					  nLastKeyPressed = PVRShellKeyNameACTION2;	break;
				case 103: nLastKeyPressed = m_eKeyMapUP;	break;
				case 108: nLastKeyPressed = m_eKeyMapDOWN;	break;
				case 105: nLastKeyPressed = m_eKeyMapLEFT;	break;
				case 106: nLastKeyPressed = m_eKeyMapRIGHT;	break;
				default:;
				}
			}
        }
	}

#if defined(PVRSHELL_INTEL_CE_PIC24_REMOTE)
	switch(g_usRemoteLastKey)
	{
		case 0x4522: // Exit
			nLastKeyPressed = PVRShellKeyNameQUIT;
			break;
		case 0x4533: // Rec
			nLastKeyPressed = PVRShellKeyNameScreenshot;
			break;
		case 0x4521: // Ok
			nLastKeyPressed = PVRShellKeyNameSELECT;
			break;
		case 0x4523: // Chapter back
			nLastKeyPressed = PVRShellKeyNameACTION1;
			break;
		case 0x4524: // Chapter forward
			nLastKeyPressed = PVRShellKeyNameACTION2;
			break;
		case 0x4580: // Up
			nLastKeyPressed = m_eKeyMapUP;
			break;
		case 0x4581: // Down
			nLastKeyPressed = m_eKeyMapDOWN;
			break;
		case 0x4551: // Left
			nLastKeyPressed = m_eKeyMapLEFT;
			break;
		case 0x454d: // Right
			nLastKeyPressed = m_eKeyMapRIGHT;
			break;
	}

	g_usRemoteLastKey = 0;
#else
	// Remote control
	if(remote_fd > 0)
	{
		int ret = 0;
		unsigned char input[32];
		ret = read(remote_fd, input, sizeof(input));

		if(input[0] == 0x87)
		{
			switch(input[1])
			{
				case 0x56: // Stop
					nLastKeyPressed = PVRShellKeyNameQUIT;
				break;
				case 0x75: // Rec
					nLastKeyPressed = PVRShellKeyNameScreenshot;
				break;
				case 0x63: // Ok
					nLastKeyPressed = PVRShellKeyNameSELECT;
				break;
				case 0x65: // Chapter back
					nLastKeyPressed = PVRShellKeyNameACTION1;
				break;
				case 0x66: // Chapter forward
					nLastKeyPressed = PVRShellKeyNameACTION2;
				break;
				case 0xc2: // Up
					nLastKeyPressed = m_eKeyMapUP;
				break;
				case 0xc3: // Down
					nLastKeyPressed = m_eKeyMapDOWN;
				break;
				case 0x93: // Left
					nLastKeyPressed = m_eKeyMapLEFT;
				break;
				case 0x8f: // Right
					nLastKeyPressed = m_eKeyMapRIGHT;
				break;
			}
		}
	}
#endif
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
	return 0;
}

/*!***********************************************************************
 @Function		OsGetNativeWindowType
 @Return		The 'NativeWindowType' for EGL
 @description	Called from InitAPI() to get the NativeWindowType
*************************************************************************/
void *PVRShellInit::OsGetNativeWindowType()
{
	return 0;
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
#if defined(PVRSHELL_OMAP3_TS_SUPPORT)
	switch(prefName)
	{
	case prefPointerLocation:
	{
		int ret = ts_read(ts, samples, PVRSHELL_TS_SAMPLES);

		if (ret <= 0)
		{
			//m_bPointer = false;
			*pp = NULL;
			return false;
		}
		struct ts_sample &samp = samples[ret - 1];
		m_vec2PointerLocation[0] = (float)samp.x / m_pShell->m_pShellData->nShellDimX;
		m_vec2PointerLocation[1] = (float)samp.y / m_pShell->m_pShellData->nShellDimY;
		//m_bPointer = true;
		*pp = m_vec2PointerLocation;

		return true;
	}
	default:
		return false;
	}
#endif
	return false;
}

#if defined(PVRSHELL_INTEL_CE_PIC24_REMOTE)
INT32 pic24_client_callback(UINT8 type, UINT8 length, void *data, void *clientData)
{
	unsigned char *buffer = (unsigned char *)data;

	if (length == 3 && buffer[0] == 0x01)
	{
		g_usRemoteLastKey = (unsigned short)buffer[1] << 8 | buffer[2];

		if(g_usRemoteLastKey == 0x4512)
		{
			printf("poweroff\n");
			system("poweroff &");
		}
	}

	return 0;
}
#endif

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
	case prefNativeDisplay:
		m_ui32NativeDisplay = i32Value;
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

